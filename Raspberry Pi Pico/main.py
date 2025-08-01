from time import sleep
import utime as time
from threeinch5.screen import MyScreen
from lib.uart_comm import UARTComm
import gc
import os
import machine

# --- Init display ---
lcd = MyScreen()
lcd.init_display()
lcd.draw_menu()
machine.freq(200_000_000)

MAX_SAMPLES = 1000

# --- Init UART communication to ESP32 ---
uart = UARTComm(uart_id=0, tx_pin=0, rx_pin=1, baudrate=9600)
uart.send_cmd("STOP")


while True:
    # 1) Get user setpoint
    while True:
        touch_data = lcd.touch_get()
        sp = lcd.get_set_point(touch_data)
        if sp is not None:
            set_point = sp
            break
        sleep(0.1)

    # 1a) Send the new setpoint to ESP32
    
    uart.set_setpoint(set_point)

    # 1b) Tell ESP32 to start PID/control loop
    uart.send_cmd("START")

    # 2) Run control loop, updating display
    flow_data = []
    motor_iterations = 0
    UPDATES_PER_SCREEN = 50
    flow = 0
    
    while True:
        # Read one flow sample
        
        start = time.ticks_ms()
        flow = int(uart.read_flow())
        sleep(0.001)

        print("Flow:", flow)
        flow_data.append(flow)
        
        if len(flow_data) > 1000:
            flow_data.pop(0)

        motor_iterations += 1
        if motor_iterations >= UPDATES_PER_SCREEN:
            lcd.current_flow_rate = flow
            lcd.max_flow_rate = max(flow_data)
            lcd.last_flow_rate = flow

            lcd.draw_flow_display()
            motor_iterations = 0

            # If user holds "save/quit" button, stop ESP and save data
            if lcd.user_pressed_quit(lcd.touch_get()):
                uart.send_cmd("STOP")
                
                # Save data to file
                try:
                    os.remove("/flow_data.txt")
                except OSError:
                    pass
                lcd.save_txt_file(flow_data, set_point)
                break

    # 3) Syringe reposition screen
    lcd.draw_syringe_screen()
    sleep(1)

    while True:
        t = lcd.touch_get()

        if lcd.right_arrow_pressed(t):
            uart.send_reverse()
        elif lcd.left_arrow_pressed(t):
            uart.send_cmd("FORWARD")
        elif lcd.stop_pressed(t):
            uart.send_cmd("STOP")
        elif lcd.continue_pressed(t):
            uart.send_cmd("STOP")
            sleep(1)
            break

    # Loop back to setpoint entry
