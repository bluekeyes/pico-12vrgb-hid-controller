#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "hardware/flash.h"
#include "hardware/sync.h"

#include "controller/persist.h"
#include "debug.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/vendor/report.h"

/*
 * Default settings are stored in flash as report structs used in the HID
 * interface. This makes it easy to use the same code to initialize animations
 * on startup.
 *
 * Settings are stored in the last PERSIST_FLASH_SIZE bytes of flash. Reports
 * are written from the start of the sector until it is full, with
 * PERSIST_REPORTS_PER_PAGE reports in each flash page.
 *
 * To load settings, walk through reports to find the last report for the lamp
 * ID. To save settings, find the first empty slot and write the report there.
 *
 * Each slot starts with a two-byte marker (0x01ed, "led"). If this marker word
 * is 0xFFFF, assume the slot is empty.
 *
 * On startup, check the marker word in all slots. If any slot has a marker
 * that is not empty and is not the expected value, erase memory range.
 *
 * If all slots are full, reclaim space by finding the most recent report for
 * each lamp, erasing flash, and then writing the most recent reports at the
 * start of the sector. This reduces wear from write-erase cycles.
 */

#define EACH_REPORT_SLOT \
    (void *slot = PERSIST_ADDR(0); slot < PERSIST_ADDR(PERSIST_FLASH_SIZE); slot += PERSIST_REPORT_SLOT_SIZE) 

static bool is_empty_slot(void *slot);
static bool is_flash_in_known_state();
static struct Vendor12VRGBAnimationReport *get_report(void *slot);
static void *reclaim_report_slots(uint8_t *pagebuf, uint8_t target_lamp_id);
static void write_flash_page(uint8_t *pagebuf, uint32_t page_num);
static void write_reports(uint8_t *pagebuf, uint32_t offset, struct Vendor12VRGBAnimationReport *reports, uint8_t count);

void ctrl_persist_init()
{
    if (!is_flash_in_known_state()) {
        ctrl_persist_clear();
    }
}

void ctrl_persist_clear()
{
    uint32_t interupts = save_and_disable_interrupts(); 
    flash_range_erase(PERSIST_FLASH_OFFSET, PERSIST_FLASH_SIZE);
    restore_interrupts(interupts);
}

void ctrl_persist_save_report(struct Vendor12VRGBAnimationReport *report)
{
    uint8_t pagebuf[FLASH_PAGE_SIZE];

    void *target_slot = NULL;
    for EACH_REPORT_SLOT {
        if (is_empty_slot(slot)) {
            target_slot = slot;
            break;
        }
    }

    // All of our flash sectors are full, reclaim space for this report
    if (target_slot == NULL) {
        target_slot = reclaim_report_slots(pagebuf, report->lamp_id);
    }

    uint32_t pos = PERSIST_OFFSET(target_slot) % FLASH_PAGE_SIZE;
    write_reports(pagebuf, pos, report, 1);
    write_flash_page(pagebuf, PERSIST_OFFSET(target_slot) / FLASH_PAGE_SIZE);
}

struct Vendor12VRGBAnimationReport *ctrl_persist_find_report(uint8_t lamp_id)
{
    struct Vendor12VRGBAnimationReport *last_report = NULL;
    for EACH_REPORT_SLOT {
        if (is_empty_slot(slot)) {
            break;
        }
        struct Vendor12VRGBAnimationReport *r = get_report(slot);
        if (r->lamp_id == lamp_id) {
            last_report = r;
        }
    }
    return last_report;
}

static void *reclaim_report_slots(uint8_t *pagebuf, uint8_t target_lamp_id)
{
    struct Vendor12VRGBAnimationReport *report_addrs[LAMP_COUNT] = { NULL };
    struct Vendor12VRGBAnimationReport reports[LAMP_COUNT-1];
    
    // Find the most recent report for each lamp
    for EACH_REPORT_SLOT {
        struct Vendor12VRGBAnimationReport *r = get_report(slot);
        report_addrs[r->lamp_id] = r;
    }

    // Copy the relevant reports in to RAM so we can erase flash
    uint32_t report_count = 0;
    for (uint8_t id = 0; id <= MAX_LAMP_ID; id++) {
        if (id != target_lamp_id && report_addrs[id] != NULL) {
            reports[report_count] = *report_addrs[id];
            report_count++;
        }
    }

    // Erase flash
    ctrl_persist_clear();

    // If the entire storage was filled with reports for the lamp we're
    // updating, there's nothing saved to write back to flash; return the
    // address of the first slot
    if (report_count == 0) {
        return PERSIST_ADDR(0);
    }

    // Write saved reports back to flash by page
    uint32_t page_count = ((report_count - 1) / PERSIST_REPORTS_PER_PAGE) + 1; 
    for (uint32_t page = 0; page < page_count; page++) {
        uint32_t ridx = page * PERSIST_REPORTS_PER_PAGE;
        write_reports(pagebuf, 0, reports + ridx, (report_count - ridx) % PERSIST_REPORTS_PER_PAGE);
        write_flash_page(pagebuf, page);
    }

    return PERSIST_ADDR(report_count * PERSIST_REPORT_SLOT_SIZE);
}

/**
 * @brief Checks each report location to determine if the sectors of flash used
 * for persistence are in a known state.
 *
 * Flash is in a know state if first word of all report slots is either empty
 * (0xFFFF) or the marker value.
 */
static bool is_flash_in_known_state()
{
    for EACH_REPORT_SLOT {
        uint16_t marker = *((uint16_t *) slot);
        if (marker != 0xFFFF && marker != PERSIST_REPORT_MARKER) {
            return false;
        }
    }
    return true;
}

static bool is_empty_slot(void *slot)
{
    return *((uint16_t *) slot) == 0xFFFF;
}

static struct Vendor12VRGBAnimationReport *get_report(void *slot)
{
    return (struct Vendor12VRGBAnimationReport *)(slot + PERSIST_REPORT_MARKER_BYTES);
}

/**
 * @brief Writes a page to flash.
 *
 * @param pagebuf  the buffer to write, must contain FLASH_PAGE_SIZE elements
 * @param page_num the page number in the report sector to write
 */
static void write_flash_page(uint8_t *pagebuf, uint32_t page_num)
{
    uint32_t interupts = save_and_disable_interrupts(); 
    flash_range_program(PERSIST_FLASH_OFFSET + page_num * FLASH_PAGE_SIZE, pagebuf, FLASH_PAGE_SIZE);
    restore_interrupts(interupts);
}

/**
 * @brief Stores reports in the page buffer for writing to flash.
 *
 * This function initializes pagebuf to all 0xFF before writing. As a result,
 * calls cannot stack and a single call should write all the reports for the
 * page.
 *
 * @param pagebuf the buffer to write to, must contain FLASH_PAGE_SIZE elements
 * @param offset  the offset within @p pagebuf to start at, must be aligned to
 *                a slot boundary and allow enough space for @p count reports
 * @param reports pointer to the reports to write
 * @param count   the number of reports pointed to by @p reports
 */
static void write_reports(uint8_t *pagebuf, uint32_t offset, struct Vendor12VRGBAnimationReport *reports, uint8_t count)
{
    // Set the buffer to all 0xFF, i.e. the erased state of flash. This means
    // we can "overwrite" existing data in the page without changing it,
    // reducing erase cycles
    memset(pagebuf, 0xFF, FLASH_PAGE_SIZE);

    for (uint8_t i = 0; i < count; i++) {
        uint8_t *p = pagebuf + offset + (i * PERSIST_REPORT_SLOT_SIZE);
        *((uint16_t *) p) = PERSIST_REPORT_MARKER;
        *((struct Vendor12VRGBAnimationReport *) (p + PERSIST_REPORT_MARKER_BYTES)) = reports[i];
    }
}

void ctrl_persist_dump()
{
    dump_buffer(PERSIST_ADDR(0), PERSIST_FLASH_SIZE, true);
}

// ----------
// Assertions
// ----------

static_assert(
    PERSIST_REPORT_SLOT_SIZE >= (PERSIST_REPORT_MARKER_BYTES + sizeof(struct Vendor12VRGBAnimationReport)),
    "struct Vendor12VRGBAnimationReport is too large for the defined slot size"
);

static_assert(
    LAMP_COUNT <= PERSIST_REPORTS_PER_PAGE * (PERSIST_FLASH_SIZE / FLASH_PAGE_SIZE),
    "insufficient space to save default Vendor12VRGBAnimationReport for every lamp"
);

