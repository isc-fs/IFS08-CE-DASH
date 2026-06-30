import queue
import re
import threading
import time
import tkinter as tk
from collections import deque
from tkinter import ttk, messagebox

import serial
import serial.tools.list_ports


TLM_RE = re.compile(
    r"TLM seq=(?P<seq>\d+) dt=(?P<dt>\d+)ms can=0x(?P<can>[0-9A-Fa-f]+) "
    r"acc=(?P<acc>\d+) brk=(?P<brk>\d+) soc=(?P<soc>\d+) inv=(?P<inv>\d+) "
    r"dc=(?P<dc>\d+) vm=(?P<vm>\d+) tinv=(?P<tinv>\d+) tacc=(?P<tacc>\d+) tmot=(?P<tmot>\d+)"
)

STATS_RE = re.compile(
    r"\[RADIO\] stats pps=(?P<pps>[0-9.]+) total=(?P<total>\d+) tlm=(?P<tlm>\d+) ack=(?P<ack>\d+) "
    r"unk=(?P<unk>\d+) lastSeq=(?P<lastseq>\d+)"
)


class SerialReader(threading.Thread):
    def __init__(self, port_name, baudrate, output_queue):
        super().__init__(daemon=True)
        self.port_name = port_name
        self.baudrate = baudrate
        self.output_queue = output_queue
        self._stop_event = threading.Event()
        self.serial_port = None

    def run(self):
        try:
            self.serial_port = serial.Serial(self.port_name, self.baudrate, timeout=0.25)
            self.output_queue.put(("status", f"Conectado a {self.port_name} @ {self.baudrate}"))

            while not self._stop_event.is_set():
                raw = self.serial_port.readline()
                if not raw:
                    continue
                try:
                    line = raw.decode("utf-8", errors="replace").strip()
                except Exception:
                    line = str(raw)
                if line:
                    self.output_queue.put(("line", line))
        except Exception as exc:
            self.output_queue.put(("error", f"Error serie: {exc}"))
        finally:
            if self.serial_port is not None:
                try:
                    self.serial_port.close()
                except Exception:
                    pass
            self.output_queue.put(("closed", "Puerto serie cerrado"))

    def stop(self):
        self._stop_event.set()


class App:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 nRF24 Serial Monitor")
        self.root.geometry("1120x700")

        self.queue = queue.Queue()
        self.reader = None

        self.last_telemetry_wall = 0.0
        self.quality_percent = 100.0
        self.recent_dt_ms = deque(maxlen=40)
        self.last_dt_ms = None

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value="115200")
        self.status_var = tk.StringVar(value="Desconectado")

        self.telemetry_vars = {
            "seq": tk.StringVar(value="-"),
            "dt": tk.StringVar(value="-"),
            "can": tk.StringVar(value="-"),
            "acc": tk.StringVar(value="-"),
            "brk": tk.StringVar(value="-"),
            "soc": tk.StringVar(value="-"),
            "inv": tk.StringVar(value="-"),
            "dc": tk.StringVar(value="-"),
            "vm": tk.StringVar(value="-"),
            "tinv": tk.StringVar(value="-"),
            "tacc": tk.StringVar(value="-"),
            "tmot": tk.StringVar(value="-"),
        }
        self.stats_vars = {
            "pps": tk.StringVar(value="0.0"),
            "total": tk.StringVar(value="0"),
            "tlm": tk.StringVar(value="0"),
            "ack": tk.StringVar(value="0"),
            "unk": tk.StringVar(value="0"),
            "lastseq": tk.StringVar(value="0"),
            "quality": tk.StringVar(value="100.0%"),
            "state": tk.StringVar(value="Sin datos"),
            "dtavg": tk.StringVar(value="-"),
            "dtmax": tk.StringVar(value="-"),
            "age": tk.StringVar(value="-"),
        }

        self._build_ui()
        self.refresh_ports()
        self._poll_queue()
        self._update_age()

    def _build_ui(self):
        top = ttk.Frame(self.root, padding=10)
        top.pack(fill="x")

        ttk.Label(top, text="Puerto").pack(side="left")
        self.port_combo = ttk.Combobox(top, textvariable=self.port_var, width=28, state="readonly")
        self.port_combo.pack(side="left", padx=(6, 12))

        ttk.Button(top, text="Refrescar", command=self.refresh_ports).pack(side="left")
        ttk.Label(top, text="Baud").pack(side="left", padx=(14, 4))
        ttk.Entry(top, textvariable=self.baud_var, width=10).pack(side="left")
        ttk.Button(top, text="Conectar", command=self.connect).pack(side="left", padx=(14, 6))
        ttk.Button(top, text="Desconectar", command=self.disconnect).pack(side="left")
        ttk.Label(top, textvariable=self.status_var).pack(side="right")

        content = ttk.Panedwindow(self.root, orient="horizontal")
        content.pack(fill="both", expand=True, padx=10, pady=(0, 10))

        left = ttk.Frame(content, padding=10)
        right = ttk.Frame(content, padding=10)
        content.add(left, weight=3)
        content.add(right, weight=2)

        telemetry_box = ttk.LabelFrame(left, text="Telemetria", padding=10)
        telemetry_box.pack(fill="x")
        self._build_key_values(
            telemetry_box,
            [
                ("Seq", "seq"),
                ("dt ms", "dt"),
                ("CAN ID", "can"),
                ("Accel", "acc"),
                ("Brake", "brk"),
                ("SOC", "soc"),
                ("Inv", "inv"),
                ("DC", "dc"),
                ("VMin", "vm"),
                ("T Inv", "tinv"),
                ("T Accu", "tacc"),
                ("T Motor", "tmot"),
            ],
            self.telemetry_vars,
            columns=3,
        )

        stats_box = ttk.LabelFrame(left, text="Calidad del enlace", padding=10)
        stats_box.pack(fill="x", pady=(10, 0))
        self._build_key_values(
            stats_box,
            [
                ("pps", "pps"),
                ("Total", "total"),
                ("TLM", "tlm"),
                ("ACK", "ack"),
                ("UNK", "unk"),
                ("Last Seq", "lastseq"),
                ("Quality", "quality"),
                ("Estado", "state"),
                ("dt medio", "dtavg"),
                ("dt max", "dtmax"),
                ("Edad ultima TLM", "age"),
            ],
            self.stats_vars,
            columns=3,
        )

        help_box = ttk.LabelFrame(right, text="Comandos", padding=10)
        help_box.pack(fill="x")
        ttk.Label(
            help_box,
            text="Puedes escribir por el monitor serie del ESP comandos como:\nradio 1\nradio 0\nsd 1\nsd 0",
            justify="left",
        ).pack(anchor="w")

        log_box = ttk.LabelFrame(right, text="Log serie", padding=10)
        log_box.pack(fill="both", expand=True, pady=(10, 0))
        self.log = tk.Text(log_box, wrap="word", height=30)
        self.log.pack(fill="both", expand=True)

    def _build_key_values(self, parent, items, var_dict, columns):
        for index, (label, key) in enumerate(items):
            row = index // columns
            col = (index % columns) * 2
            ttk.Label(parent, text=label + ":").grid(row=row, column=col, sticky="w", padx=(0, 6), pady=4)
            ttk.Label(parent, textvariable=var_dict[key], width=14).grid(row=row, column=col + 1, sticky="w", padx=(0, 18), pady=4)

    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo["values"] = ports
        if ports and self.port_var.get() not in ports:
            self.port_var.set(ports[0])
        elif not ports:
            self.port_var.set("")

    def connect(self):
        if self.reader is not None:
            return

        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("Puerto", "Selecciona un puerto serie")
            return

        try:
            baudrate = int(self.baud_var.get().strip())
        except ValueError:
            messagebox.showwarning("Baudrate", "Baudrate no valido")
            return

        self.reader = SerialReader(port, baudrate, self.queue)
        self.reader.start()
        self.status_var.set("Conectando...")

    def disconnect(self):
        if self.reader is not None:
            self.reader.stop()
            self.reader = None
        self.status_var.set("Desconectado")

    def _append_log(self, line):
        self.log.insert("end", line + "\n")
        self.log.see("end")

    def _handle_line(self, line):
        self._append_log(line)

        telemetry_match = TLM_RE.match(line)
        if telemetry_match:
            data = telemetry_match.groupdict()
            for key in self.telemetry_vars:
                if key == "can":
                    self.telemetry_vars[key].set("0x" + data[key].upper())
                else:
                    self.telemetry_vars[key].set(data[key])

            dt_ms = int(data["dt"])
            if dt_ms > 0:
                self.recent_dt_ms.append(dt_ms)
                self.last_dt_ms = dt_ms

            self.last_telemetry_wall = time.time()
            self._update_link_quality()
            return

        stats_match = STATS_RE.match(line)
        if stats_match:
            data = stats_match.groupdict()
            for key, value in data.items():
                self.stats_vars[key].set(value)
            return

    def _poll_queue(self):
        try:
            while True:
                kind, payload = self.queue.get_nowait()
                if kind == "line":
                    self._handle_line(payload)
                elif kind == "status":
                    self.status_var.set(payload)
                    self._append_log(payload)
                elif kind == "error":
                    self.status_var.set("Error")
                    self._append_log(payload)
                    self.reader = None
                elif kind == "closed":
                    self.status_var.set("Desconectado")
                    self._append_log(payload)
                    self.reader = None
        except queue.Empty:
            pass

        self.root.after(100, self._poll_queue)

    def _update_link_quality(self):
        if not self.recent_dt_ms:
            self.stats_vars["quality"].set("100.0%")
            self.stats_vars["state"].set("Sin datos")
            self.stats_vars["dtavg"].set("-")
            self.stats_vars["dtmax"].set("-")
            return

        avg_dt = sum(self.recent_dt_ms) / len(self.recent_dt_ms)
        max_dt = max(self.recent_dt_ms)

        # Calidad relativa al objetivo de 500 ms. 500 ms ~= 100%.
        # 1000 ms ~= 50%. Se penaliza adicionalmente por pausas largas.
        quality = 100.0 * min(1.0, 500.0 / max(avg_dt, 1.0))
        if max_dt > 1500:
            quality *= 0.7
        if max_dt > 2500:
            quality *= 0.5
        quality = max(0.0, min(100.0, quality))
        self.quality_percent = quality

        if avg_dt <= 700 and max_dt <= 900:
            state = "Buena"
        elif avg_dt <= 1200 and max_dt <= 1800:
            state = "Media"
        else:
            state = "Mala"

        self.stats_vars["quality"].set(f"{quality:.1f}%")
        self.stats_vars["state"].set(state)
        self.stats_vars["dtavg"].set(f"{avg_dt:.0f} ms")
        self.stats_vars["dtmax"].set(f"{max_dt} ms")

    def _update_age(self):
        if self.last_telemetry_wall == 0.0:
            self.stats_vars["age"].set("-")
        else:
            age_ms = int((time.time() - self.last_telemetry_wall) * 1000.0)
            self.stats_vars["age"].set(f"{age_ms} ms")
            self._update_link_quality()
            if age_ms > 2500:
                self.stats_vars["state"].set("Sin enlace")
                self.stats_vars["quality"].set("0.0%")

        self.root.after(250, self._update_age)


def main():
    root = tk.Tk()
    app = App(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.disconnect(), root.destroy()))
    root.mainloop()


if __name__ == "__main__":
    main()
