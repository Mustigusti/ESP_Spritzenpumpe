from machine import UART, Pin
import utime as time
import sys

class UARTComm:
    def __init__(self, uart_id=0, tx_pin=0, rx_pin=1, baudrate=9600):
        # Initialize UART on specified bus and pins
        self.uart = UART(
            uart_id,
            baudrate,
            bits=8,
            parity=None,
            stop=1,
            tx=Pin(tx_pin),
            rx=Pin(rx_pin),
            timeout=20
        )

    def send_cmd(self, cmd: str):
        """
        Send a command string terminated by newline over UART.
        """
        try:
            self.uart.write(cmd.encode('utf-8') + b'\n')
        except Exception as e:
            print("UART write error:", e)

    def read_flow(self) -> float:
        """
        Read one line from UART (up to '\n') and parse a float after 'FLOW:'.
        Returns 0.0 on timeout or parse failure.
        """
        # Wait for a full line
        line = self.uart.readline()
        if not line:
            return 0.0
        try:
            decoded = line.decode('utf-8').strip()
        except Exception:
            return 0.0

        # Expect lines like "FLOW:123.45"
        if not decoded.startswith("FLOW:"):
            return 0.0

        try:
            return float(decoded[5:])
        except ValueError:
            return 0.0
        
    def send_forward(self):
        self.send_cmd("FORWARD")

    def send_reverse(self):
        self.send_cmd("REVERSE")

    def set_setpoint(self, microl_per_min: int):
        self.send_cmd(f"SETPOINT:{microl_per_min}")


if __name__ == "__main__":
    # Simple test loop: send START, then continually print flow readings
    uart_comm = UARTComm(uart_id=0, tx_pin=0, rx_pin=1, baudrate=9600)
    uart_comm.send_cmd("STOP")
    while True:
        flow = uart_comm.read_flow()
        print(flow)
        time.sleep_ms(12)
