from machine import UART, Pin
import utime as time
import sys

class UARTComm:
    def __init__(self, uart_id=0, tx_pin=0, rx_pin=1, baudrate=9600):

        self.uart = UART(
            uart_id,
            baudrate,   # positional
            8,          # bits
            None,       # parity
            1,          # stop bits
            tx=Pin(tx_pin),
            rx=Pin(rx_pin),
        )
        self.buffer_size = 64

    def send_cmd(self, cmd: str):
        try:
            self.uart.write(cmd.encode() + b'\n')
        except Exception as e:
            print("UART write error:", e)

    def read_flow(self) -> float:
        try:
            buf = b""
            start = time.ticks_ms()
            timeout = 100
            while True:
                if self.uart.any():
                    c = self.uart.read(1)
                    if c == b'\n':
                        break
                    buf += c
                elif time.ticks_diff(time.ticks_ms(), start) > timeout:
                    break
                time.sleep_ms(1)

            if buf:
                line = buf.decode().strip()
                # expecting something like "FLOW:123.4"
                if line.startswith("FLOW:"):
                    try:
                        return float(line.split("FLOW:")[1])
                    except ValueError:
                        pass
        except Exception as e:
            sys.print_exception(e)
        return 0.0

    def reverse(self):
        self.send_cmd("REVERSE")

    def set_setpoint(self, microl_per_min: int):
        self.send_cmd(f"SETPOINT:{microl_per_min}")

if __name__ == "__main__":
    uart_comm = UARTComm(uart_id=0, tx_pin=0, rx_pin=1, baudrate=9600)
    uart_comm.send_cmd("START")
    while True:
        flow = uart_comm.read_flow()
        print(flow)
        time.sleep(.01)
