// Example requires activation method "ABP" and disabled frame counter checks

// 1. Rename to Credentials.h

// 2. Insert your "Network Session Key" in hex msb format, e.g., {0x12, 0x34 ...}
static const PROGMEM u1_t NWKSKEY[16] = ;
// 3. "App Session Key" in hex msb format, e.g., {0x12, 0x34 ...}
static const u1_t PROGMEM APPSKEY[16] = ;
// 4. "Device Address" not in msb, just in hex format, e.g., 0x123456AB
static const u4_t DEVADDR = ;
