#ifndef HID_DATA_H_
#define HID_DATA_H_

#define HID_SET_FLAG_N(FIELD, BIT)  ((FIELD) |= (1UL << BIT))
#define HID_SET_FLAG(FIELD)         HID_SET_FLAG_N(FIELD, 0)

#define HID_CLEAR_FLAG_N(FIELD, BIT)    ((FIELD) &= ~(1UL << BIT))
#define HID_CLEAR_FLAG(FIELD)           HID_CLEAR_FLAG_N(FIELD, 0)

#define HID_GET_FLAG_N(FIELD, BIT)  ((FIELD) & (1UL << BIT))
#define HID_GET_FLAG(FIELD)         HID_GET_FLAG_N(FIELD, 0)

#endif // HID_DATA_H_
