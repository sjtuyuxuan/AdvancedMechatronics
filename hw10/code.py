from ulab import numpy as np
print("Start!")
t = np.linspace(0, 10, 1024)
value = np.sin(200 * t) + np.cos(30 * t) + np.sin(t + 1)
out = np.fft.fft(value)
for i in out[0]:
    print(i)
for i in out[1]:
    print(i)
