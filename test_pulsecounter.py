#!/usr/bin/env python

import pulse_counter
import time
import numpy

counter = pulse_counter.PulseCounter()

for i in xrange(3):
    counter.count()
    last_time = time.time()

data = []
times = []

for i in xrange(2000):
    counts = counter.count()
    if i > 3:
        times.append(last_time)
        data.append(counts)
    dt = counts[-1]/48e6
    dt_err = ((counts[-1]/(time.time() - last_time)) - 48e6)/48e6
    print '%d %g %g (48 MHz %+.1f%%)' % (i, counts[0][0]/dt, counts[0][1]/dt, 100*dt_err)
    last_time = time.time()
    time.sleep(0.1)

numpy.savez('pmt_counts', data=data, times=times)
    