#!/usr/bin/env python3
"""CANable SLCAN test tool for the TouchGFX display telemetry path.

Requires:
    pip install pyserial

Examples:
    python Utilities/canable_display_test.py --port COM5 --bitrate 500000 once
    python Utilities/canable_display_test.py --port COM5 once
    python Utilities/canable_display_test.py --port COM5 demo
    python Utilities/canable_display_test.py --port COM5 monitor --decode-display

Notes:
    - The STM32 project currently uses classic CAN at 500000 bit/s.
    - Standard SLCAN speed `S6` matches 500000 bit/s, so no custom BTR is
      needed for a typical CANable setup.
"""

from __future__ import annotations

import argparse
import sys
import time
from dataclasses import dataclass
from typing import List, Optional, Sequence, Tuple

try:
    import serial
except ImportError as exc:  # pragma: no cover - import guard for user environment
    raise SystemExit(
        "pyserial no esta instalado. Ejecuta: pip install pyserial"
    ) from exc


DISPLAY_TELEMETRY_CAN_ID_CONTROL = 0x100
DISPLAY_TELEMETRY_CAN_ID_ELECTRICAL = 0x101
DISPLAY_TELEMETRY_CAN_ID_THERMAL = 0x102

STANDARD_SLCAN_BITRATES = {
    10_000: "S0",
    20_000: "S1",
    50_000: "S2",
    100_000: "S3",
    125_000: "S4",
    250_000: "S5",
    500_000: "S6",
    800_000: "S7",
    1_000_000: "S8",
}


@dataclass
class EditorLaunchConfig:
    enabled: bool = True
    command: str = "demo"
    port: str = "COM11"
    serial_baud: int = 115200
    bitrate: int = 500000
    slcan_btr: str = ""
    timeout: float = 0.2
    accel: int = 35
    brake: int = 0
    soc: int = 76
    dc_bus: int = 540
    v_min: int = 3580
    inv_state: int = 3
    temp_inv: int = 46
    temp_accu: int = 33
    temp_motor: int = 54
    inter_frame_ms: float = 2.0
    period_ms: float = 100.0
    steps: int = 21
    count: int = 0
    decode_display: bool = True
    duration_s: float = 0.0


# Configuracion rapida para lanzar desde PyCharm sin argumentos.
# Cambia estos valores y ejecuta el script tal cual.
EDITOR_LAUNCH = EditorLaunchConfig()


@dataclass
class DisplayTelemetry:
    accel: int = 0
    brake: int = 0
    soc: int = 0
    dc_bus: int = 0
    v_min: int = 0
    inv_state: int = 0
    temp_inv: int = 0
    temp_accu: int = 0
    temp_motor: int = 0


def clamp_u16(name: str, value: int) -> int:
    if value < 0 or value > 0xFFFF:
        raise ValueError(f"{name} fuera de rango uint16: {value}")
    return int(value)


def pack_u16_le(*values: int) -> bytes:
    payload = bytearray()
    for value in values:
        payload.extend(clamp_u16("value", value).to_bytes(2, byteorder="little"))
    return bytes(payload)


def telemetry_to_frames(telemetry: DisplayTelemetry) -> List[Tuple[int, bytes]]:
    return [
        (
            DISPLAY_TELEMETRY_CAN_ID_CONTROL,
            pack_u16_le(
                telemetry.accel,
                telemetry.brake,
                telemetry.soc,
                telemetry.inv_state,
            ),
        ),
        (
            DISPLAY_TELEMETRY_CAN_ID_ELECTRICAL,
            pack_u16_le(
                telemetry.dc_bus,
                telemetry.v_min,
                telemetry.temp_inv,
                telemetry.temp_accu,
            ),
        ),
        (
            DISPLAY_TELEMETRY_CAN_ID_THERMAL,
            pack_u16_le(telemetry.temp_motor, 0, 0, 0),
        ),
    ]


def decode_display_frame(identifier: int, data: bytes) -> Optional[str]:
    if len(data) != 8:
        return None

    words = [
        int.from_bytes(data[offset : offset + 2], byteorder="little")
        for offset in range(0, 8, 2)
    ]

    if identifier == DISPLAY_TELEMETRY_CAN_ID_CONTROL:
        return (
            f"CONTROL accel={words[0]} brake={words[1]} "
            f"soc={words[2]} invState={words[3]}"
        )
    if identifier == DISPLAY_TELEMETRY_CAN_ID_ELECTRICAL:
        return (
            f"ELECTRICAL dcBus={words[0]} vMin={words[1]} "
            f"tempInv={words[2]} tempAccu={words[3]}"
        )
    if identifier == DISPLAY_TELEMETRY_CAN_ID_THERMAL:
        return f"THERMAL tempMotor={words[0]}"
    return None


def format_can_frame(identifier: int, data: bytes) -> str:
    encoded = " ".join(f"{byte:02X}" for byte in data)
    decoded = decode_display_frame(identifier, data)
    frame = f"0x{identifier:03X} [{encoded}]"
    if decoded:
        return f"{frame}  {decoded}"
    return frame


class SlcanAdapter:
    def __init__(
        self,
        port: str,
        serial_baud: int,
        bitrate: int,
        slcan_btr: Optional[str],
        timeout: float,
    ) -> None:
        self.port = port
        self.serial_baud = serial_baud
        self.bitrate = bitrate
        self.slcan_btr = slcan_btr.upper() if slcan_btr else None
        self.timeout = timeout
        self.handle: Optional[serial.Serial] = None
        self.version_info: str = ""
        self.no_ack_feedback: bool = False

    def __enter__(self) -> "SlcanAdapter":
        self.open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def open(self) -> None:
        self.handle = serial.Serial(
            port=self.port,
            baudrate=self.serial_baud,
            timeout=self.timeout,
            write_timeout=1.0,
        )
        self.handle.reset_input_buffer()
        self.handle.reset_output_buffer()

        self.version_info = self.command("V", allow_no_response=True)
        if not self.version_info:
            raise RuntimeError(
                f"El puerto {self.port} abre, pero no responde al comando SLCAN 'V'. "
                "Revisa que sea el CANable correcto y no otro puerto serie."
            )

        if "canable-fw" in self.version_info.lower():
            self.no_ack_feedback = True

        self.command("C", allow_no_response=self.no_ack_feedback)
        if self.slcan_btr:
            self.command(f"s{self.slcan_btr}", allow_no_response=self.no_ack_feedback)
        else:
            bitrate_command = STANDARD_SLCAN_BITRATES.get(self.bitrate)
            if bitrate_command is None:
                raise ValueError(
                    "Ese bitrate no tiene alias SLCAN estandar. "
                    "Usa --slcan-btr si tu CANable soporta sXXXX."
                )
            self.command(bitrate_command, allow_no_response=self.no_ack_feedback)
        self.command("O", allow_no_response=self.no_ack_feedback)

    def close(self) -> None:
        if self.handle is None:
            return
        try:
            self.command("C", allow_no_response=True)
        finally:
            self.handle.close()
            self.handle = None

    def command(self, command: str, allow_no_response: bool = False) -> str:
        if self.handle is None:
            raise RuntimeError("Adaptador SLCAN no abierto")

        self.handle.write((command + "\r").encode("ascii"))
        self.handle.flush()

        response = self.handle.read_until(b"\r")
        if response == b"":
            if allow_no_response:
                return ""
            raise TimeoutError(f"Sin respuesta del CANable al comando {command!r}")
        if b"\a" in response:
            raise RuntimeError(f"El CANable rechazo el comando {command!r}")
        return response.decode("ascii", errors="replace").strip()

    def send_standard(self, identifier: int, data: bytes) -> None:
        if identifier < 0 or identifier > 0x7FF:
            raise ValueError(f"ID CAN estandar invalido: 0x{identifier:X}")
        if len(data) > 8:
            raise ValueError("SLCAN clasico solo soporta hasta 8 bytes")

        command = f"t{identifier:03X}{len(data):X}{data.hex().upper()}"
        self.command(command, allow_no_response=self.no_ack_feedback)

    def send_snapshot(
        self, telemetry: DisplayTelemetry, inter_frame_delay_s: float = 0.0
    ) -> None:
        for index, (identifier, payload) in enumerate(telemetry_to_frames(telemetry)):
            print(f"TX {format_can_frame(identifier, payload)}", flush=True)
            self.send_standard(identifier, payload)
            if inter_frame_delay_s > 0.0 and index < 2:
                time.sleep(inter_frame_delay_s)

    def read_frame(self, timeout_s: float) -> Optional[Tuple[int, bytes, str]]:
        if self.handle is None:
            raise RuntimeError("Adaptador SLCAN no abierto")

        previous_timeout = self.handle.timeout
        self.handle.timeout = timeout_s
        try:
            raw = self.handle.read_until(b"\r")
        finally:
            self.handle.timeout = previous_timeout

        if not raw:
            return None

        line = raw.decode("ascii", errors="replace").strip()
        if not line or line[0] != "t":
            return None
        if len(line) < 5:
            return None

        identifier = int(line[1:4], 16)
        dlc = int(line[4], 16)
        expected_length = 5 + (dlc * 2)
        if len(line) < expected_length:
            return None
        data = bytes.fromhex(line[5:expected_length])
        return identifier, data, line


def build_demo_telemetry(step: int, steps: int) -> DisplayTelemetry:
    cycle = max(steps - 1, 1)
    phase = step % (cycle * 2)
    ramp = phase if phase < cycle else (cycle * 2) - phase
    accel = int((100 * ramp) / cycle)
    brake = 0 if accel > 15 else 25 - accel
    soc = 76
    dc_bus = 520 + accel
    v_min = 3580 - (brake * 2)
    inv_state = 3
    temp_inv = 40 + (accel // 5)
    temp_accu = 31 + ((step // 4) % 8)
    temp_motor = 47 + (accel // 4)
    return DisplayTelemetry(
        accel=accel,
        brake=brake,
        soc=soc,
        dc_bus=dc_bus,
        v_min=v_min,
        inv_state=inv_state,
        temp_inv=temp_inv,
        temp_accu=temp_accu,
        temp_motor=temp_motor,
    )


def build_editor_argv() -> List[str]:
    config = EDITOR_LAUNCH
    argv = [
        "--port",
        config.port,
        "--serial-baud",
        str(config.serial_baud),
        "--bitrate",
        str(config.bitrate),
        "--timeout",
        str(config.timeout),
    ]

    if config.slcan_btr:
        argv.extend(["--slcan-btr", config.slcan_btr])

    argv.append(config.command)

    if config.command == "once":
        argv.extend(
            [
                "--accel",
                str(config.accel),
                "--brake",
                str(config.brake),
                "--soc",
                str(config.soc),
                "--dc-bus",
                str(config.dc_bus),
                "--v-min",
                str(config.v_min),
                "--inv-state",
                str(config.inv_state),
                "--temp-inv",
                str(config.temp_inv),
                "--temp-accu",
                str(config.temp_accu),
                "--temp-motor",
                str(config.temp_motor),
                "--inter-frame-ms",
                str(config.inter_frame_ms),
            ]
        )
    elif config.command == "demo":
        argv.extend(
            [
                "--period-ms",
                str(config.period_ms),
                "--steps",
                str(config.steps),
                "--count",
                str(config.count),
                "--inter-frame-ms",
                str(config.inter_frame_ms),
            ]
        )
    elif config.command == "monitor":
        if config.decode_display:
            argv.append("--decode-display")
        argv.extend(["--duration-s", str(config.duration_s)])
    else:
        raise ValueError(
            "EDITOR_LAUNCH.command debe ser 'once', 'demo' o 'monitor'"
        )

    return argv


def add_adapter_arguments(
    parser: argparse.ArgumentParser, *, required: bool = False
) -> None:
    config = EDITOR_LAUNCH
    parser.add_argument(
        "--port",
        required=required,
        default=config.port,
        help="Puerto serie del CANable, por ejemplo COM5",
    )
    parser.add_argument(
        "--serial-baud",
        type=int,
        default=config.serial_baud,
        help="Baudrate del puerto serie SLCAN",
    )
    parser.add_argument(
        "--bitrate",
        type=int,
        default=config.bitrate,
        help="Bitrate nominal CAN. Para velocidades no estandar usa --slcan-btr",
    )
    parser.add_argument(
        "--slcan-btr",
        default=config.slcan_btr,
        help="Valor SLCAN sXXXX para bitrate custom si no usas una velocidad estandar",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=config.timeout,
        help="Timeout serie en segundos",
    )


def build_parser() -> argparse.ArgumentParser:
    config = EDITOR_LAUNCH
    adapter_parent = argparse.ArgumentParser(add_help=False)
    add_adapter_arguments(adapter_parent)

    parser = argparse.ArgumentParser(
        description="Envio y monitorizacion CAN para probar la ruta CAN -> TouchGFX",
    )
    add_adapter_arguments(parser)
    subparsers = parser.add_subparsers(dest="command")

    once = subparsers.add_parser(
        "once",
        help="Envia una captura completa una sola vez",
        parents=[adapter_parent],
    )
    once.add_argument("--accel", type=int, default=config.accel)
    once.add_argument("--brake", type=int, default=config.brake)
    once.add_argument("--soc", type=int, default=config.soc)
    once.add_argument("--dc-bus", type=int, default=config.dc_bus, dest="dc_bus")
    once.add_argument("--v-min", type=int, default=config.v_min, dest="v_min")
    once.add_argument("--inv-state", type=int, default=config.inv_state, dest="inv_state")
    once.add_argument("--temp-inv", type=int, default=config.temp_inv, dest="temp_inv")
    once.add_argument("--temp-accu", type=int, default=config.temp_accu, dest="temp_accu")
    once.add_argument("--temp-motor", type=int, default=config.temp_motor, dest="temp_motor")
    once.add_argument(
        "--inter-frame-ms",
        type=float,
        default=config.inter_frame_ms,
        dest="inter_frame_ms",
        help="Pausa entre las 3 tramas del snapshot",
    )

    demo = subparsers.add_parser(
        "demo",
        help="Envia valores animados hasta Ctrl+C",
        parents=[adapter_parent],
    )
    demo.add_argument(
        "--period-ms",
        type=float,
        default=config.period_ms,
        dest="period_ms",
        help="Periodo de envio de snapshots",
    )
    demo.add_argument(
        "--steps",
        type=int,
        default=config.steps,
        help="Pasos del barrido de acelerador",
    )
    demo.add_argument(
        "--count",
        type=int,
        default=config.count,
        help="Numero de snapshots a enviar. 0 = infinito",
    )
    demo.add_argument(
        "--inter-frame-ms",
        type=float,
        default=config.inter_frame_ms,
        dest="inter_frame_ms",
        help="Pausa entre las 3 tramas del snapshot",
    )

    monitor = subparsers.add_parser(
        "monitor",
        help="Escucha tramas recibidas por el CANable",
        parents=[adapter_parent],
    )
    monitor.add_argument(
        "--decode-display",
        action="store_true",
        default=config.decode_display,
        help="Decodifica los IDs 0x100..0x102 del display",
    )
    monitor.add_argument(
        "--duration-s",
        type=float,
        default=config.duration_s,
        dest="duration_s",
        help="Duracion en segundos. 0 = infinito",
    )

    return parser


def run_once(adapter: SlcanAdapter, args: argparse.Namespace) -> int:
    telemetry = DisplayTelemetry(
        accel=args.accel,
        brake=args.brake,
        soc=args.soc,
        dc_bus=args.dc_bus,
        v_min=args.v_min,
        inv_state=args.inv_state,
        temp_inv=args.temp_inv,
        temp_accu=args.temp_accu,
        temp_motor=args.temp_motor,
    )
    adapter.send_snapshot(telemetry, inter_frame_delay_s=args.inter_frame_ms / 1000.0)
    print(
        "Snapshot enviado:",
        telemetry,
        flush=True,
    )
    return 0


def run_demo(adapter: SlcanAdapter, args: argparse.Namespace) -> int:
    sent = 0
    step = 0
    period_s = args.period_ms / 1000.0
    inter_frame_delay_s = args.inter_frame_ms / 1000.0

    while args.count == 0 or sent < args.count:
        telemetry = build_demo_telemetry(step, args.steps)
        adapter.send_snapshot(telemetry, inter_frame_delay_s=inter_frame_delay_s)
        print(
            f"[{sent:05d}] accel={telemetry.accel:3d} brake={telemetry.brake:3d} "
            f"soc={telemetry.soc:3d} dcBus={telemetry.dc_bus:4d} "
            f"vMin={telemetry.v_min:4d} tInv={telemetry.temp_inv:3d} "
            f"tAccu={telemetry.temp_accu:3d} tMotor={telemetry.temp_motor:3d}",
            flush=True,
        )
        sent += 1
        step += 1
        time.sleep(period_s)
    return 0


def run_monitor(adapter: SlcanAdapter, args: argparse.Namespace) -> int:
    deadline = time.time() + args.duration_s if args.duration_s > 0.0 else None

    while deadline is None or time.time() < deadline:
        frame = adapter.read_frame(timeout_s=0.25)
        if frame is None:
            continue

        identifier, data, line = frame
        decoded = decode_display_frame(identifier, data) if args.decode_display else None
        if decoded:
            print(f"{line}  {decoded}", flush=True)
        else:
            print(line, flush=True)
    return 0


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = build_parser()

    effective_argv = argv
    if effective_argv is None:
        effective_argv = sys.argv[1:]

    preview_args = parser.parse_args(effective_argv)

    if EDITOR_LAUNCH.enabled and not preview_args.command:
        effective_argv = build_editor_argv()
        print(
            "Usando configuracion EDITOR_LAUNCH del archivo",
            flush=True,
        )
        args = parser.parse_args(effective_argv)
    else:
        args = preview_args

    if not EDITOR_LAUNCH.enabled and not args.command:
        args.command = "demo"

    if not args.port:
        parser.error("--port es obligatorio")

    print(
        f"Abriendo CANable en {args.port} a {args.serial_baud} baud serie, "
        f"CAN bitrate {args.bitrate}"
        + (f", BTR custom {args.slcan_btr}" if args.slcan_btr else ""),
        flush=True,
    )

    with SlcanAdapter(
        port=args.port,
        serial_baud=args.serial_baud,
        bitrate=args.bitrate,
        slcan_btr=args.slcan_btr,
        timeout=args.timeout,
    ) as adapter:
        if args.command == "once":
            return run_once(adapter, args)
        if args.command == "demo":
            return run_demo(adapter, args)
        if args.command == "monitor":
            return run_monitor(adapter, args)
        parser.error(f"Comando no soportado: {args.command}")
    return 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nInterrumpido por el usuario", file=sys.stderr)
        raise SystemExit(130)
