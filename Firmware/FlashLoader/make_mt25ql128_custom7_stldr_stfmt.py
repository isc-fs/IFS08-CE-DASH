from __future__ import annotations

from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "MT25QL128A_STM32H7XX-CUSTOM7-TENTATIVE.stldr"
OUTPUT = ROOT / "MT25QL128A_STM32H7XX-CUSTOM7-STFMT.stldr"


def main() -> None:
    data = bytearray(SOURCE.read_bytes())

    shoff = struct.unpack_from("<I", data, 0x20)[0]
    shentsize = struct.unpack_from("<H", data, 0x2E)[0]
    shnum = struct.unpack_from("<H", data, 0x30)[0]
    shstrndx = struct.unpack_from("<H", data, 0x32)[0]

    new_shstr = (
        b"\x00.symtab\x00.strtab\x00.shstrtab\x00"
        b"DevInfo\x00PrgCode\x00PrgData\x00PrgData\x00PrgData\x00"
    )
    new_shstr_off = len(data)
    data.extend(new_shstr)

    # Update .shstrtab section header location/size.
    shstr_hdr = shoff + shstrndx * shentsize
    struct.pack_into("<I", data, shstr_hdr + 0x10, new_shstr_off)
    struct.pack_into("<I", data, shstr_hdr + 0x14, len(new_shstr))

    # Update section name indices:
    # 1 .info -> DevInfo
    # 2 .prog -> PrgCode
    # 3 .rodata -> PrgData
    # 4 .data -> PrgData
    # 5 .bss -> PrgData
    name_offsets = {
        1: 27,
        2: 35,
        3: 43,
        4: 51,
        5: 59,
    }

    for sec_index, name_off in name_offsets.items():
        hdr = shoff + sec_index * shentsize
        struct.pack_into("<I", data, hdr + 0x00, name_off)

    OUTPUT.write_bytes(data)
    print(f"Created ST-style tentative loader: {OUTPUT}")


if __name__ == "__main__":
    main()
