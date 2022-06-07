# read data from the imu and plot

# sudo apt-get install python3-pip
# python -m pip install pyserial
# sudo apt-get install python-matplotlib

import serial
ser = serial.Serial('COM5',230400)
print('Opening port: ')
print(ser.name)

read_samples = 10 # anything bigger than 1 to start out
ax = []
ay = []
az = []
gx = []
gy = []
gz = []
temp = []
print('Requesting data collection...')
ser.write(b'\n')
while read_samples > 1:
    data_read = ser.read_until(b'\n',200) # get the data as bytes
    data_text = str(data_read,'utf-8') # turn the bytes to a string
    data = [float(i) for i in data_text.split()] # turn the string into a list of floats

    if(len(data)==8):
        read_samples = int(data[0]) # keep reading until this becomes 1
        ax.append(data[1])
        ay.append(data[2])
        az.append(data[3])
        gx.append(data[4])
        gy.append(data[5])
        gz.append(data[6])
        temp.append(data[7])
print('Data collection complete')

#complementary_filter
def CF (raw_data, A=0.02, dt=0.01):
    ax, ay, az, gx, gy, gz, temp = raw_data
    import math
    accP = []
    accR = []
    gP = []
    gR = []
    cfP = []
    cfR = []
    for i in range(len(gx)):
        accP.append(math.degrees(math.atan2(ay[i],az[i])))
        accR.append(math.degrees(math.atan2(ax[i],az[i])))
        if i==0:
            gP.append(accP[0])
            gR.append(accR[0])
        else:
            gP.append(gp[-1] + dt * gx[i-1])
            gR.append(gR[-1] + dt * gy[i-1])
        cfP.append((1-A) * (cfP[-1] + dt * gx[i-1]) + A*accP[-1])
        cfR.append((1-A) * (cfR[-1] + dt * gy[i-1]) + A*accR[-1])
    return (accP, accR, gP, gR, cfP, cfR)
    


# plot it
import matplotlib.pyplot as plt 
t = range(len(ax)) # time array
plt.plot(t,ax,'r*-',t,ay,'b*-',t,az,'k*-')
plt.ylabel('G value')
plt.xlabel('sample')
plt.show()

t = range(len(gx)) # time array
plt.plot(t,gx,'r*-',t,gy,'b*-',t,gz,'k*-')
plt.ylabel('Omega value')
plt.xlabel('sample')
plt.show()

t = range(len(temp)) # time array
plt.plot(t,temp,'r*-')
plt.ylabel('Temperature value')
plt.xlabel('sample')
plt.show()

accP, accR, gP, gR, cfP, cfR = CF((ax, ay, az, gx, gy, gz, temp))
t = range(len(temp)) # time array
plt.plot(t,accP,'r*-',t,gP,'b*-',t,cfP,'k*-')
plt.ylabel('Pitch with gyro acc and cf value')
plt.xlabel('sample')
plt.show()

t = range(len(temp)) # time array
plt.plot(t,accR,'r*-',t,gR,'b*-',t,cfR,'k*-')
plt.ylabel('Roll with gyro acc and cf value')
plt.xlabel('sample')
plt.show()

# be sure to close the port
ser.close()
