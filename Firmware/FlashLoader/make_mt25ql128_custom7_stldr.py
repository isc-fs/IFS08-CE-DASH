from __future__ import annotations

from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "stm32extldr" / "h7_w25q256" / "CLIVEONE-W25Q256_STM32H7XX-PB2-PB10-PF8-PF9-PF7-PF6.stldr"
OUTPUT = ROOT / "MT25QL128A_STM32H7XX-CUSTOM7-TENTATIVE.stldr"

# .prog section metadata for this exact source loader
PROG_FILE_OFFSET = 0xFC
PROG_VMA = 0x24000004

# Offsets inside .info for the ST external loader StorageInfo structure
INFO_FILE_OFFSET = 0x34
INFO_SIZE = 0xC8
INFO_NAME_MAX = 96
INFO_DEVICE_SIZE_OFFSET = 108

NEW_NAME = "CLIVEONE-MT25QL128A_STM32H7XX-PB2-PB10-PF8-PF9-PF7-PF6"
NEW_DEVICE_SIZE = 0x01000000  # 16 MiB


def vma_to_file_offset(vma: int) -> int:
    return PROG_FILE_OFFSET + (vma - PROG_VMA)


PATCHES = {
    # Accept MT25QL128A/N25Q128A style ID instead of Winbond IDs.
    # W25Q256 source compares against 0x40EF and 0x60EF.
    vma_to_file_offset(0x24000934): bytes.fromhex("4BF62022"),  # movw r2, #0xBA20
    vma_to_file_offset(0x2400093E): bytes.fromhex("4BF62022"),  # keep same fallback compare
    # Use Micron/N25-style masked compare later in Init.
    vma_to_file_offset(0x24000964): bytes.fromhex("4FF6FF63"),  # movw r3, #0xFEFF
    vma_to_file_offset(0x2400096A): bytes.fromhex("4BF62022"),  # movw r2, #0xBA20
}


def patch_name(info: bytearray) -> None:
    name_bytes = NEW_NAME.encode("ascii")
    if len(name_bytes) >= INFO_NAME_MAX:
        raise ValueError("New loader name is too long for .info string area")
    info[:INFO_NAME_MAX] = b"\x00" * INFO_NAME_MAX
    info[: len(name_bytes)] = name_bytes


def main() -> None:
    data = bytearray(SOURCE.read_bytes())

    info = data[INFO_FILE_OFFSET : INFO_FILE_OFFSET + INFO_SIZE]
    patch_name(info)
    struct.pack_into("<I", info, INFO_DEVICE_SIZE_OFFSET, NEW_DEVICE_SIZE)
    data[INFO_FILE_OFFSET : INFO_FILE_OFFSET + INFO_SIZE] = info

    for file_offset, patch_bytes in PATCHES.items():
        data[file_offset : file_offset + len(patch_bytes)] = patch_bytes

    OUTPUT.write_bytes(data)
    print(f"Created tentative loader: {OUTPUT}")


if __name__ == "__main__":
    main()
