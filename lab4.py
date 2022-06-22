import pandas as pd
import numpy as np
data = pd.read_csv ("rx_trace.txt", delimiter="\t", header=None)
thr = np.sum(data[1]) #--> 23747360
time = data[0].iloc[-1] - data [0].iloc[0] #--> iloc[-1]: last line
    #time --> 18.9929299999
print((thr*8/time)/1e6) #--> 10.0026 [Mbps]
data2 = pd.read_csv("rtt_trace.txt", delimiter="\t", header=None)
print(np.mean(data2[1]/1e3)) #--> 5 [ms] (tx=5, rx=5, process=1)