from machine import Pin, PWM, ADC
from time import sleep_us, sleep_ms, ticks_us
from motor import drive

def go():
    
    MAX_DUTY = 2**16-1
    MINIMAL_FALL_HEIGHT = 5_000



    levelshifter_VCC = Pin("GP3", Pin.OUT, value = 1)
    OE = Pin("GP7", Pin.OUT, value=1)


    forwardPWM = PWM(Pin("GP4")); forwardPWM.freq(100_000)
    backwardPWM = PWM(Pin("GP6")); backwardPWM.freq(100_000)

    drive.backward(forwardPin=forwardPWM, backwardPin=backwardPWM, dutycycle = int(0*MAX_DUTY))

    print("Start")

if __name__ == '__main__':
    go()