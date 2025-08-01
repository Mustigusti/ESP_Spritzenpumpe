from machine import Pin, I2C, PWM
import utime
import time
import array
from utime import ticks_us
import gc

class MyMotor:
    def __init__(self):
        # CONSTANTS
        self.MAX_DUTY = 2**16-1
        self.BEGINNING_CHUNK_SIZE = 1500
        
        # Initialize variables
        
        # flow data variables
        self.set_point = 0
        self.flow_rate = 0
        self.last_flow_rate = 0
        self.max_flow_rate = 0
        self.flow_average_counter = 0
        
        #flow array variables
        self.populating_flow_array_counter = 0
        self.current_flow_array = array.array('i', [0] * self.BEGINNING_CHUNK_SIZE)
        self.time_per_motor_cycle = 0
        self.last_time_per_motor_cycle = 0
        self.rp_memory_full = False
        
        #control variables
        self.integral = 0
        self.derivative = 0
        self.previous_error = 0
        self.Soll_wert = 0

        # Initialize pins
        self.scl = Pin(1, Pin.OPEN_DRAIN, Pin.PULL_UP)
        self.sda = Pin(0, Pin.OPEN_DRAIN, Pin.PULL_UP)
        self.levelshifter_VCC = Pin("GP3", Pin.OUT, value=1)
        self.OE = Pin("GP7", Pin.OUT, value=1)
        self.forwardPWM = PWM(Pin("GP4"))
        self.forwardPWM.freq(100_000)
        self.backwardPWM = PWM(Pin("GP6"))
        self.backwardPWM.freq(100_000)
        
        # Initialize I2C
        self.i2c = I2C(0, scl=self.scl, sda=self.sda, freq=400_000)
        self.SENSOR_ADDRESS = 0x08  # I2C address
        
        # Initialize PID constants
        self.K_u = 0.004227
        self.T_u = 0.176571
        self.K_p = 0.6 * self.K_u
        self.K_i = 2 * self.K_p / self.T_u
        self.K_d = self.K_p * self.T_u / 8
        
        self.K_p = 0.4 * self.K_p
        self.K_i = 0.001 * self.K_i
        self.K_d = 1.1 * self.K_d
        
        # Power-up the sensor
        self.sensorVCC = Pin("GP2", Pin.OUT, value=1)
        utime.sleep_ms(25)  # Power-up delay
    
    def check_crc(self,data):
        # data has this structure:
        # data = [most significant bit, least significant bit, checksum]
        polynomial = 0x31  # x^8 + x^5 + x^4 + 1 (0x31)
        calculated_crc = 0xFF         # Initialization with 0xFF
        
        for byte in data[:2]:
            calculated_crc ^= byte    # XOR byte into CRC
            for _ in range(8):  # Process 8 bits
                if calculated_crc & 0x80:  # If the leftmost bit is 1
                    calculated_crc = (calculated_crc << 1) ^ polynomial
                else:
                    calculated_crc <<= 1
                calculated_crc &= 0xFF  # Keep CRC within 8 bits
        
        received_crc = data[2]
        if calculated_crc == received_crc:
            return True
        else:
            return False
    
    def go_forward(self, dutycycle) -> None:
        self.forwardPWM.duty_u16(dutycycle)
        self.backwardPWM.duty_u16(0)
    
    def go_backward(self) -> None:
        self.backwardPWM.duty_u16(self.MAX_DUTY)
        self.forwardPWM.duty_u16(0)
        
    def stop(self):
        self.backwardPWM.duty_u16(0)
        self.forwardPWM.duty_u16(0)
    
    def update_max_flow_rate(self):
        if self.flow_rate > self.max_flow_rate:
            self.max_flow_rate = self.flow_rate
            return self.max_flow_rate
        
    def update_flow_array(self):
        
        if self.rp_memory_full == False:
            try:
                gc.collect()
                self.current_flow_array[0+self.populating_flow_array_counter] = int(self.time_per_motor_cycle)
                self.current_flow_array[1+self.populating_flow_array_counter] = int(self.flow_rate)
                self.populating_flow_array_counter += 2
            except IndexError:
                self.rp_memory_full = True # 2100 Bytes is the larrgest amount of memory I can allocate to the flow_array without having a MemoryError
                print("Memory Full!")
            
    def read_sensor(self):
        self.i2c.writeto(self.SENSOR_ADDRESS, b'\x36\x08', False)
        utime.sleep_ms(12)  # Sensor warm-up time

        msb_flow, lsb_flow, received_crc_flow = self.i2c.readfrom(self.SENSOR_ADDRESS, 3, True)
        flow_rate = (msb_flow << 8) | lsb_flow
        self.flow_rate = flow_rate / 10  # Update flow rate

        # Check CRC and process flow rate
        if self.check_crc([msb_flow, lsb_flow, received_crc_flow]):
            if self.flow_rate > 3250:
                self.flow_rate = 0
            #print(f'{self.flow_rate}')
            self.last_flow_rate = self.flow_rate
            return self.flow_rate

    def run_pid(self, set_point: microL/second):
        
        self.time_start = ticks_us()
        
        self.set_point = set_point
        self.Soll_wert = self.set_point  # Desired flow rate in muL/min
        #print("Setpoint", self.set_point)
        
        # PID Regulation logic
        error = self.Soll_wert - self.flow_rate
        self.integral += error
        self.derivative = error - self.previous_error
        controlled_signal = self.K_p * error + self.K_i * self.integral + self.K_d * self.derivative
        controlled_dutycycle = int(controlled_signal * self.MAX_DUTY)
        controlled_dutycycle = max(0, min(controlled_dutycycle, self.MAX_DUTY))
        self.go_forward(dutycycle=controlled_dutycycle)
        self.update_max_flow_rate()
        # Start measurement
        self.flow_rate = self.read_sensor()
        self.previous_error = error
        
        self.time_end = ticks_us()
        self.time_per_motor_cycle += self.time_end - self.time_start
        #print("time difference", self.time_per_motor_cycle)
        
        self.update_flow_array()
        gc.collect()

            
if __name__ == '__main__':
    set_point = 0  # set point in mu L/min (example)
    motor_pid = MyMotor()
    motor_pid.go_backward()
    motor_pid.stop()