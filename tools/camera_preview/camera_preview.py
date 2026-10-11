import argparse
import base64
import socket

import cv2
import numpy as np
import serial

# ---------------- Config ----------------
CONNECTION_MODE = "serial"  # "serial" or "tcp" -- default if no CLI args given

SERIAL_PORT = "COM3"
BAUD = 115200

TCP_HOST = "192.168.1.100"
TCP_PORT = 8080
# -----------------------------------------


def get_config():
    parser = argparse.ArgumentParser(description="Camera stream receiver (Serial or TCP)")
    parser.add_argument("--mode", choices=["serial", "tcp"], default=CONNECTION_MODE,
                         help="Connection mode: serial or tcp")
    parser.add_argument("--port", default=SERIAL_PORT, help="Serial port (e.g. COM3)")
    parser.add_argument("--baud", type=int, default=BAUD, help="Serial baud rate")
    parser.add_argument("--host", default=TCP_HOST, help="TCP host/IP")
    parser.add_argument("--tcp-port", type=int, default=TCP_PORT, help="TCP port")
    return parser.parse_args()


class SerialBinMixin:
    """Adds read_exact support for pyserial-backed sources."""
    def read_exact(self, n):
        data = self.ser.read(n)
        return data if len(data) == n else None


class SerialSource(SerialBinMixin):
    def __init__(self, port, baud):
        self.ser = serial.Serial(port, baud, timeout=2)
        print(f"Listening on serial {port}...")

    def readline(self):
        return self.ser.readline().decode("ascii", errors="ignore").strip()

    def close(self):
        self.ser.close()


class TCPSource:
    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        self.buf = b""
        print(f"Connected to TCP {host}:{port}")

    def readline(self):
        while b"\n" not in self.buf:
            chunk = self.sock.recv(4096)
            if not chunk:
                return ""  # connection closed
            self.buf += chunk
        line, self.buf = self.buf.split(b"\n", 1)
        return line.decode("ascii", errors="ignore").strip()

    def read_exact(self, n):
        while len(self.buf) < n:
            chunk = self.sock.recv(4096)
            if not chunk:
                return None  # connection closed
            self.buf += chunk
        data, self.buf = self.buf[:n], self.buf[n:]
        return data

    def close(self):
        self.sock.close()


def track_color(track_id):
    np.random.seed(track_id)
    return tuple(int(c) for c in np.random.randint(64, 255, size=3))


def main():
    args = get_config()

    if args.mode == "serial":
        source = SerialSource(args.port, args.baud)
    else:
        source = TCPSource(args.host, args.tcp_port)

    buffer = ""
    boxes = []
    tracks = []
    receiving = False

    cv2.namedWindow("Camera", cv2.WINDOW_NORMAL)

    while True:
        try:
            line = source.readline()

            if line == "" and args.mode == "tcp":
                print("Connection closed by server.")
                break

            if line == "IMG_START":
                buffer = ""
                boxes = []
                tracks = []
                receiving = True

            elif line.startswith("BOX:") and receiving:
                try:
                    x, y, w, h, score, class_id = map(int, line[4:].split(","))
                    boxes.append((x, y, w, h, score, class_id))
                    print(f"Inference: class={class_id}, score={score}, x={x}, y={y}, w={w}, h={h}")
                except ValueError:
                    pass

            elif line.startswith("TRACK:") and receiving:
                try:
                    track_id, x, y, w, h, score = line[6:].split(",")
                    tracks.append((int(track_id), float(x), float(y), float(w), float(h), float(score)))
                except ValueError:
                    pass

            elif line.startswith("PERF:"):
                print(line)

            elif line.startswith("IMG_BIN:"):
                try:
                    n = int(line.split(":", 1)[1])
                except ValueError:
                    n = 0

                jpeg_bytes = source.read_exact(n) if n > 0 else None

                if jpeg_bytes is None:
                    print("Binary frame read failed/connection closed.")
                    if args.mode == "tcp":
                        break
                else:
                    try:
                        image = cv2.imdecode(np.frombuffer(jpeg_bytes, dtype=np.uint8), cv2.IMREAD_COLOR)

                        if image is not None:
                            mode = "TRACKING" if tracks else ("DETECTING" if boxes else "NO OBJECTS")
                            cv2.putText(image, mode, (10, 25), cv2.FONT_HERSHEY_SIMPLEX,
                                        0.7, (0, 255, 255), 2, cv2.LINE_AA)

                            if not tracks:
                                for x, y, w, h, score, class_id in boxes:
                                    x1, y1 = int(x - w / 2), int(y - h / 2)
                                    x2, y2 = int(x + w / 2), int(y + h / 2)
                                    cv2.rectangle(image, (x1, y1), (x2, y2), (0, 128, 0), 2)
                                    cv2.putText(image, f"Class {class_id}: {score}%", (x1, max(y1 - 8, 15)),
                                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 128, 0), 2, cv2.LINE_AA)

                            for track_id, x, y, w, h, score in tracks:
                                x1, y1 = int(x), int(y)
                                x2, y2 = int(x + w), int(y + h)
                                color = track_color(track_id)

                                cv2.rectangle(image, (x1, y1), (x2, y2), color, 3)
                                label = f"ID {track_id}: {score * 100:.0f}%"
                                cv2.putText(image, label, (x1, max(y1 - 8, 15)),
                                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2, cv2.LINE_AA)

                            cv2.imshow("Camera", image)

                    except Exception:
                        pass

                buffer = ""
                boxes = []
                tracks = []
                receiving = False

            elif line == "IMG_END":
                if receiving and buffer:
                    try:
                        jpeg_bytes = base64.b64decode(buffer, validate=True)
                        image = cv2.imdecode(np.frombuffer(jpeg_bytes, dtype=np.uint8), cv2.IMREAD_COLOR)

                        if image is not None:
                            mode = "TRACKING" if tracks else ("DETECTING" if boxes else "NO OBJECTS")
                            cv2.putText(image, mode, (10, 25), cv2.FONT_HERSHEY_SIMPLEX,
                                        0.7, (0, 255, 255), 2, cv2.LINE_AA)

                            if not tracks:
                                for x, y, w, h, score, class_id in boxes:
                                    x1, y1 = int(x - w / 2), int(y - h / 2)
                                    x2, y2 = int(x + w / 2), int(y + h / 2)
                                    cv2.rectangle(image, (x1, y1), (x2, y2), (0, 128, 0), 2)
                                    cv2.putText(image, f"Class {class_id}: {score}%", (x1, max(y1 - 8, 15)),
                                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 128, 0), 2, cv2.LINE_AA)

                            for track_id, x, y, w, h, score in tracks:
                                x1, y1 = int(x), int(y)
                                x2, y2 = int(x + w), int(y + h)
                                color = track_color(track_id)

                                cv2.rectangle(image, (x1, y1), (x2, y2), color, 3)
                                label = f"ID {track_id}: {score * 100:.0f}%"
                                cv2.putText(image, label, (x1, max(y1 - 8, 15)),
                                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2, cv2.LINE_AA)

                            cv2.imshow("Camera", image)

                    except Exception:
                        pass

                buffer = ""
                boxes = []
                tracks = []
                receiving = False

            elif receiving:
                buffer += line

            elif line:
                print(line)

            if cv2.waitKey(1) & 0xFF == 27:
                break

        except KeyboardInterrupt:
            break
        except Exception:
            pass

    source.close()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()