import numpy as np

N = 1024

# gen signal
pulses = 10
for i in range(0,pulses):
    


lookup = np.exp( np.complex(0,-2*np.pi/N) * np.arange(0,N))

