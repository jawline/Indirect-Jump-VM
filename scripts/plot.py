#!/usr/bin/env python3

import matplotlib.pyplot as plt
import sys
import json
import numpy as np
from scipy.interpolate import make_interp_spline, BSpline

data = {'x': [], 'fast': [], 'slow': []}

with open(sys.argv[-1]) as f:
  lines = [line.rstrip() for line in f]
  for line in lines:
    nl = json.loads(line)
    data['x'].append(int(nl[0]))
    data['fast'].append(float(nl[1]))
    data['slow'].append(float(nl[2])) 

def smoother(points, num_points):
  # 300 represents number of points to make between T.min and T.max
  xnew = np.linspace(points[0].min(), points[0].max(), num_points)
  spl = make_interp_spline(points[0], points[1], k=3) 
  return (xnew, spl(xnew))

fastline = np.array([data['x'], data['fast']])
slowline = np.array([data['x'], data['slow']])

xnew, fast = smoother(fastline, 5000)
plt.plot(xnew, fast, label="Computed Goto")

xnew, slow = smoother(slowline, 5000)
plt.plot(xnew, slow, label="Jump Table")

#plt.plot(data['x'], data['fast'], label="Computed #Goto")
#plt.plot(data['x'], data['slow'], label="Jump Table")

plt.ylabel('time')
plt.xlabel('inseq')

plt.title("Computed goto vs Jump table handling random sequences")
plt.legend()
plt.show()
