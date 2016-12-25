import numpy as np

a = np.random.randint(1,100,[1000,1000])
b = np.random.randint(1,100,[1000,1000])

for i in range(5000):
        c = np.multiply(a,b)
