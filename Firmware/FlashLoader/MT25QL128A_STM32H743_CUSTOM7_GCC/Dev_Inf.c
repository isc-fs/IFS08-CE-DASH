#include "Dev_Inf.h"

__attribute__((section(".DevInfo")))
const struct StorageInfo StorageInfo = {
    "MT25QL128A_STM32H743_CUSTOM7",
    NOR_FLASH,
    0x90000000,
    0x01000000,
    0x00000100,
    0xFF,
    {
        {0x00000100, 0x00010000},
        {0x00000000, 0x00000000},
    }
};
