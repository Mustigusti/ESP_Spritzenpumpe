import array
import random
import time

CHUNK_SIZE = 100
array = array.array('i', [0] * CHUNK_SIZE)
timer = 0

def simulate_get_flow():
    return random.randint(990, 1010)

def simulate_get_time():
    global timer
    timer = timer + 12600
    return timer


for i in range(CHUNK_SIZE):
    if i % 2 == 0:
        array[i] = simulate_get_time()
    else:
        array[i] = simulate_get_flow()
    
    
print(array)
