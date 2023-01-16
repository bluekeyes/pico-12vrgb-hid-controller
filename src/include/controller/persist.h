#ifndef CONTROLLER_PERSIST_H_
#define CONTROLLER_PERSIST_H_

#include "hardware/flash.h"

#include "hid/vendor/report.h"

#define PERSIST_REPORT_MARKER       0x01ed
#define PERSIST_REPORT_MARKER_BYTES 2

#define PERSIST_REPORTS_PER_PAGE    4
#define PERSIST_REPORT_SLOT_SIZE    (FLASH_PAGE_SIZE / PERSIST_REPORTS_PER_PAGE)

#define PERSIST_FLASH_SIZE          (FLASH_SECTOR_SIZE)
#define PERSIST_FLASH_OFFSET        (PICO_FLASH_SIZE_BYTES - PERSIST_FLASH_SIZE)

#define PERSIST_ADDR(offset)        ((void *) (XIP_BASE + PERSIST_FLASH_OFFSET + (offset)))
#define PERSIST_OFFSET(addr)        (((uint32_t) (addr)) - XIP_BASE - PERSIST_FLASH_OFFSET)

/**
 * @brief Initializes persistent flash storage.
 *
 * This checks the content of flash and will perform an erase if the relevant
 * sector is not empty and does not contain valid data.
 */
void ctrl_persist_init();

/**
 * @brief Clears any existing saved settings in flash storage.
 */
void ctrl_persist_clear();

/**
 * @brief Writes the given report to persistent flash storage.
 */
void ctrl_persist_save_report(struct Vendor12VRGBAnimationReport *report);

/**
 * @brief Finds the most recent saved report for the lamp ID.
 *
 * @returns The most recent report or NULL if no reports exist.
 */
struct Vendor12VRGBAnimationReport *ctrl_persist_find_report(uint8_t lamp_id);

#endif /* CONTROLLER_PERSIST_H_ */
