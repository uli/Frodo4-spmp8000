const uint8 c64loader [] = {
 0x01,0x08,0x0b,0x08,0x0a,0x00,0x9e,0x32,
 0x30,0x36,0x31,0x00,0x00,0x00,0x20,0x1c,
 0x0a,0xa0,0x04,0xb9,0x3f,0x0b,0x20,0xa8,
 0xff,0x88,0x10,0xf7,0x20,0xae,0xff,0xa9,
 0x01,0xa2,0x44,0xa0,0x0b,0x20,0xbd,0xff,
 0xa9,0x80,0x85,0x9d,0x0a,0x85,0xb9,0xaa,
 0xa0,0x40,0x20,0xd5,0xff,0x90,0x03,0x4c,
 0xd1,0xe1,0xa9,0xe3,0xa0,0x0a,0x20,0x1e,
 0xab,0xa2,0x00,0xa0,0x20,0x86,0xfb,0x84,
 0xfc,0xa2,0x03,0xa0,0x40,0x86,0xfd,0x84,
 0xfe,0xa0,0x01,0x84,0xf9,0x88,0x84,0xff,
 0x8c,0x51,0x0b,0x4c,0xaf,0x08,0x20,0x45,
 0x0b,0xd0,0xfb,0x20,0x45,0x0b,0x20,0x45,
 0x0b,0x20,0x45,0x0b,0x20,0x50,0x0b,0x20,
 0x45,0x0b,0x20,0x45,0x0b,0xf0,0x68,0xc9,
 0x22,0xd0,0xf7,0x20,0x45,0x0b,0xc9,0x22,
 0xf0,0x15,0x91,0xfb,0xc9,0x20,0x90,0x07,
 0xaa,0x10,0x06,0xc9,0xa0,0xb0,0x02,0xa9,
 0x2a,0x20,0xd2,0xff,0xc8,0xd0,0xe4,0xa5,
 0xfb,0x20,0x50,0x0b,0xa5,0xfc,0x20,0x50,
 0x0b,0x98,0x20,0x50,0x0b,0x18,0x65,0xfb,
 0x85,0xfb,0x90,0x02,0xe6,0xfb,0xe6,0xff,
 0xa5,0xff,0xc9,0x1a,0xf0,0x29,0xc9,0x0d,
 0xd0,0x08,0xa2,0x15,0x86,0xf9,0xa2,0x05,
 0x86,0xd6,0x18,0x69,0x41,0x85,0x22,0x48,
 0xa6,0xd6,0xe8,0xa4,0xf9,0x18,0x20,0xf0,
 0xff,0x68,0x20,0xd2,0xff,0xa9,0x20,0x20,
 0xd2,0xff,0xa0,0x00,0x4c,0x5d,0x08,0xa9,
 0x14,0x20,0xd2,0xff,0x20,0xd2,0xff,0xa9,
 0xff,0x85,0xf8,0xa9,0x13,0x85,0xd6,0x20,
 0xe4,0xff,0xc9,0x85,0xd0,0x03,0x4c,0x1e,
 0x08,0xc9,0x88,0xd0,0x14,0xa2,0x05,0xbd,
 0x97,0x04,0x49,0x80,0x9d,0x97,0x04,0xca,
 0x10,0xf5,0x8a,0x45,0xf8,0x85,0xf8,0xb0,
 0xde,0xc9,0x41,0x90,0xda,0xc5,0x22,0xb0,
 0xd6,0x38,0xe9,0x41,0x0a,0x0a,0xaa,0xbd,
 0x00,0x38,0x85,0x22,0xbd,0x01,0x38,0x85,
 0xbb,0xbd,0x02,0x38,0x85,0xbc,0xbd,0x03,
 0x38,0x85,0xb7,0xa2,0x05,0x78,0x86,0xc6,
 0xbd,0x39,0x0b,0x9d,0x76,0x02,0xca,0xd0,
 0xf7,0xa9,0x80,0x85,0x9d,0xa9,0x02,0x85,
 0x7b,0xa0,0x31,0xb9,0x56,0x0b,0x99,0x1f,
 0x02,0x88,0xd0,0xf7,0x84,0x0a,0xa6,0x22,
 0xe0,0x03,0x90,0x08,0xa5,0xf8,0xf0,0x33,
 0xe0,0xcb,0xb0,0x04,0x98,0x4c,0x75,0xe1,
 0x20,0x4f,0x0a,0xa2,0x0d,0xbd,0x8c,0x0a,
 0xdd,0xd2,0xee,0xd0,0x09,0xca,0x10,0xf5,
 0xa9,0x84,0xa0,0x0a,0xd0,0x04,0xa9,0x26,
 0xa0,0x0a,0x85,0xfb,0x84,0xfc,0xa0,0x7f,
 0xb1,0xfb,0x99,0x34,0x03,0x88,0x10,0xf8,
 0x4c,0x34,0x03,0x20,0x4f,0x0a,0xa2,0x00,
 0xa9,0x05,0x86,0x22,0x85,0x23,0x20,0x0e,
 0x0a,0xa9,0x57,0xa4,0x22,0x20,0x15,0x0a,
 0xa5,0x23,0xa0,0x20,0x20,0x15,0x0a,0xa6,
 0x22,0xbd,0x00,0x0d,0x20,0xa8,0xff,0xe8,
 0x88,0xd0,0xf6,0x86,0x22,0x20,0xae,0xff,
 0xa6,0x22,0xd0,0xda,0x20,0x0e,0x0a,0xa9,
 0x45,0x20,0xa8,0xff,0xa9,0x00,0xa0,0x05,
 0x20,0x15,0x0a,0x20,0xae,0xff,0x2c,0x00,
 0xdd,0x70,0xfb,0x78,0xa9,0x27,0x8d,0x00,
 0xdd,0xa0,0x53,0xb9,0x82,0x0b,0x99,0xa7,
 0x02,0x88,0x10,0xf7,0x78,0xa2,0x1e,0x9a,
 0xa0,0x00,0x84,0x90,0xb9,0x00,0x0c,0x99,
 0x20,0x01,0x88,0xd0,0xf7,0x20,0xa7,0x02,
 0x20,0xc1,0x02,0xaa,0x20,0xc1,0x02,0x20,
 0xc1,0x02,0xca,0xca,0x4c,0x20,0x02,0x20,
 0x1c,0x0a,0xa9,0x4d,0xa0,0x2d,0x20,0xa8,
 0xff,0x98,0x4c,0xa8,0xff,0xa5,0xba,0x20,
 0xb1,0xff,0xa9,0x6f,0x4c,0x93,0xff,0xa0,
 0x00,0x84,0x90,0x20,0xa5,0xff,0xaa,0xa5,
 0x90,0x4a,0x4a,0xb0,0xf2,0x8a,0x78,0xe6,
 0x01,0x91,0xae,0xc6,0x01,0xe6,0xae,0xd0,
 0x02,0xe6,0xaf,0x24,0x90,0x50,0xe0,0x20,
 0xae,0xff,0x20,0x42,0xf6,0x4c,0x3d,0x02,
 0x20,0xaf,0xf5,0xa9,0x00,0x85,0x90,0xa9,
 0x60,0x85,0xb9,0x20,0xd5,0xf3,0xa5,0xba,
 0x20,0xb4,0xff,0xa5,0xb9,0x20,0x96,0xff,
 0x20,0xa5,0xff,0x85,0xae,0xa5,0x90,0x4a,
 0x4a,0xb0,0x0d,0x20,0xa5,0xff,0x85,0xaf,
 0xa9,0x00,0x8d,0x02,0x08,0x4c,0xd2,0xf5,
 0xa2,0x04,0x4c,0x37,0xa4,0xae,0x0c,0xdc,
 0x30,0x03,0x4c,0x7d,0x09,0xa0,0x51,0x20,
 0xeb,0xf0,0x2c,0x0d,0xdd,0x20,0x6f,0xf7,
 0x20,0xf4,0xef,0xf0,0xed,0x20,0xa1,0xf5,
 0xaa,0x2c,0x00,0xdd,0x70,0x29,0x2c,0x0d,
 0xdd,0xf0,0xf6,0x2c,0x01,0xdd,0xa0,0x00,
 0x8a,0x2c,0x00,0xdd,0x70,0x19,0x2c,0x0d,
 0xdd,0xf0,0xf6,0xad,0x01,0xdd,0x78,0xe6,
 0x01,0x91,0xae,0xc6,0x01,0x58,0xe6,0xae,
 0xd0,0xe6,0xe6,0xaf,0x18,0x90,0xe1,0x20,
 0x97,0xee,0xa0,0x40,0x8a,0x2c,0x0d,0xdd,
 0xd0,0x05,0xe8,0xd0,0xf8,0xa0,0x42,0x84,
 0x90,0x4c,0x3d,0x02,0x93,0x11,0x20,0x2a,
 0x2a,0x2a,0x2a,0x2a,0x20,0x41,0x43,0x54,
 0x49,0x4f,0x4e,0x20,0x52,0x45,0x50,0x4c,
 0x41,0x59,0x20,0x56,0x33,0x2e,0x30,0x20,
 0x20,0x4c,0x4f,0x41,0x44,0x45,0x52,0x20,
 0x2a,0x2a,0x2a,0x2a,0x2a,0x0d,0x0d,0x20,
 0x20,0x20,0x46,0x31,0x2f,0x4e,0x45,0x58,
 0x54,0x20,0x44,0x49,0x53,0x4b,0x20,0x20,
 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
 0x20,0x20,0x20,0x20,0x20,0x20,0x0d,0x0d,
 0x00,0x00,0x0d,0x52,0x55,0x4e,0x3a,0x0d,
 0x30,0x4d,0x3e,0x30,0x55,0x24,0xe6,0xfd,
 0xd0,0x02,0xe6,0xfe,0xa2,0x00,0xa1,0xfd,
 0x60,0x8d,0x00,0x38,0xee,0x51,0x0b,0x60,
 0x20,0xc1,0x02,0xa0,0x00,0xe6,0x01,0x91,
 0xae,0xc6,0x01,0xe6,0xae,0xd0,0x02,0xe6,
 0xaf,0xca,0xd0,0xec,0x20,0xa7,0x02,0x20,
 0xc1,0x02,0xaa,0xd0,0xe3,0xa6,0xae,0xa4,
 0xaf,0x86,0x2d,0x84,0x2e,0x20,0x60,0xa6,
 0x4c,0x7b,0xe3,0xad,0x00,0xdd,0x29,0x0f,
 0x09,0x20,0x8d,0x00,0xdd,0x20,0xc0,0x02,
 0xa9,0x40,0x2c,0x00,0xdd,0xf0,0xfb,0xa9,
 0x7f,0x4a,0xb0,0xfd,0x60,0x38,0xad,0x12,
 0xd0,0xe9,0x31,0x90,0x04,0x29,0x06,0xf0,
 0xf5,0xa9,0x0f,0x2d,0x00,0xdd,0x8d,0x00,
 0xdd,0x09,0x20,0xa8,0xea,0xea,0xea,0xea,
 0xad,0x00,0xdd,0x4a,0x4a,0xea,0x4d,0x00,
 0xdd,0x4a,0x4a,0xea,0x4d,0x00,0xdd,0x4a,
 0x4a,0xea,0x4d,0x00,0xdd,0x8c,0x00,0xdd,
 0x49,0xf9,0xa8,0xb9,0x20,0x01,0x60,0x04,
 0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,
 0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,
 0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,
 0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,
 0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,
 0xea,0x00,0x80,0x20,0xa0,0x40,0xc0,0x60,
 0xe0,0x10,0x90,0x30,0xb0,0x50,0xd0,0x70,
 0xf0,0x08,0x88,0x28,0xa8,0x48,0xc8,0x68,
 0xe8,0x18,0x98,0x38,0xb8,0x58,0xd8,0x78,
 0xf8,0x02,0x82,0x22,0xa2,0x42,0xc2,0x62,
 0xe2,0x12,0x92,0x32,0xb2,0x52,0xd2,0x72,
 0xf2,0x0a,0x8a,0x2a,0xaa,0x4a,0xca,0x6a,
 0xea,0x1a,0x9a,0x3a,0xba,0x5a,0xda,0x7a,
 0xfa,0x04,0x84,0x24,0xa4,0x44,0xc4,0x64,
 0xe4,0x14,0x94,0x34,0xb4,0x54,0xd4,0x74,
 0xf4,0x0c,0x8c,0x2c,0xac,0x4c,0xcc,0x6c,
 0xec,0x1c,0x9c,0x3c,0xbc,0x5c,0xdc,0x7c,
 0xfc,0x06,0x86,0x26,0xa6,0x46,0xc6,0x66,
 0xe6,0x16,0x96,0x36,0xb6,0x56,0xd6,0x76,
 0xf6,0x0e,0x8e,0x2e,0xae,0x4e,0xce,0x6e,
 0xee,0x1e,0x9e,0x3e,0xbe,0x5e,0xde,0x7e,
 0xfe,0x01,0x81,0x21,0xa1,0x41,0xc1,0x61,
 0xe1,0x11,0x91,0x31,0xb1,0x51,0xd1,0x71,
 0xf1,0x09,0x89,0x29,0xa9,0x49,0xc9,0x69,
 0xe9,0x19,0x99,0x39,0xb9,0x59,0xd9,0x79,
 0xf9,0x03,0x83,0x23,0xa3,0x43,0xc3,0x63,
 0xe3,0x13,0x93,0x33,0xb3,0x53,0xd3,0x73,
 0xf3,0x0b,0x8b,0x2b,0xab,0x4b,0xcb,0x6b,
 0xeb,0x1b,0x9b,0x3b,0xbb,0x5b,0xdb,0x7b,
 0xfb,0x05,0x85,0x25,0xa5,0x45,0xc5,0x65,
 0xe5,0x15,0x95,0x35,0xb5,0x55,0xd5,0x75,
 0xf5,0x0d,0x8d,0x2d,0xad,0x4d,0xcd,0x6d,
 0xed,0x1d,0x9d,0x3d,0xbd,0x5d,0xdd,0x7d,
 0xfd,0x07,0x87,0x27,0xa7,0x47,0xc7,0x67,
 0xe7,0x17,0x97,0x37,0xb7,0x57,0xd7,0x77,
 0xf7,0x0f,0x8f,0x2f,0xaf,0x4f,0xcf,0x6f,
 0xef,0x1f,0x9f,0x3f,0xbf,0x5f,0xdf,0x7f,
 0xff,0xa5,0x18,0x85,0x08,0xa5,0x19,0x85,
 0x09,0xa9,0x08,0x8d,0x00,0x18,0xad,0x00,
 0x1c,0x09,0x08,0x8d,0x00,0x1c,0xa9,0x00,
 0x85,0x83,0xa9,0xf0,0x85,0x84,0xa9,0x01,
 0x85,0x1c,0x78,0xa9,0x08,0x8d,0x00,0x18,
 0xad,0x07,0x1c,0xa0,0x01,0x8c,0x05,0x1c,
 0x8d,0x07,0x1c,0xc8,0x84,0x8b,0x58,0xa9,
 0x80,0x85,0x01,0xa5,0x01,0x30,0xfc,0xc9,
 0x01,0xf0,0x1f,0xc6,0x8b,0x30,0x14,0xd0,
 0x04,0xa9,0xc0,0x85,0x01,0xa5,0x16,0x85,
 0x12,0xa5,0x17,0x85,0x13,0xa5,0x01,0x30,
 0xfc,0x10,0xdb,0xa2,0x02,0x4c,0x0a,0xe6,
 0xd0,0xa7,0x78,0xae,0x01,0x04,0x86,0x09,
 0xad,0x00,0x04,0x85,0x08,0xf0,0x02,0xa2,
 0xff,0x86,0x15,0xca,0x8e,0x01,0x04,0xa0,
 0x00,0xa9,0x00,0x8d,0x00,0x18,0xc8,0xb9,
 0x00,0x04,0x4a,0x4a,0x4a,0x4a,0xaa,0xa9,
 0x01,0x8d,0x00,0x18,0x2c,0x00,0x18,0xd0,
 0xfb,0x8e,0x00,0x18,0x8a,0x0a,0x29,0x0f,
 0x8d,0x00,0x18,0xb9,0x00,0x04,0x29,0x0f,
 0x8d,0x00,0x18,0x0a,0x29,0x0f,0xea,0x8d,
 0x00,0x18,0xc4,0x15,0xd0,0xd0,0xad,0x00,
 0x04,0xf0,0x02,0xd0,0xab,0x8d,0x00,0x18,
 0xa2,0x76,0xbd,0x4b,0xeb,0x9d,0x00,0x04,
 0xca,0x10,0xf7,0xa9,0x60,0x8d,0x77,0x04,
 0x20,0x00,0x04,0x4c,0xe7,0xeb,0xea,0xea,
 0xea
};
