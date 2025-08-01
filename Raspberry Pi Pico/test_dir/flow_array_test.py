import array
import random

CHUNK_SIZE = 100
last_flow_array = array.array('i', [0] * CHUNK_SIZE)
current_flow_array = array.array('i', [0] * CHUNK_SIZE)
complete_flow_array = array.array('i')  # To store the complete picture
timer = 0

# Simulates a random flow rate
def simulate_get_flow():
    return random.randint(990, 1010)

# Simulates a timer increment
def simulate_get_time():
    global timer
    timer += 12600  # Increment in microseconds
    return timer

# Fill current_flow_array with simulated values
def fill_flow_array(flow_array):
    for i in range(CHUNK_SIZE):
        if i % 2 == 0:
            flow_array[i] = simulate_get_time()
        else:
            flow_array[i] = simulate_get_flow()

# Main loop to simulate the process
for _ in range(5):  # Simulate 5 rounds of data collection
    # Fill the current_flow_array
    fill_flow_array(current_flow_array)
    
    # Extend the complete_flow_array with last_flow_array
    complete_flow_array.extend(last_flow_array)
    
    # Update last_flow_array to hold the data from current_flow_array
    last_flow_array = current_flow_array[:]
    
    # Reinitialize current_flow_array for new data
    current_flow_array = array.array('i', [0] * CHUNK_SIZE)
    
    print("Last Flow Array:", last_flow_array)
    print("Complete Flow Array:", complete_flow_array)

print("Final Complete Flow Array Length:", len(complete_flow_array))
