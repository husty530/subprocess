import time
import json
import numpy as np
from fast_std_io import *

if __name__ == '__main__':

  w = int(sys.argv[1])
  h = int(sys.argv[2])
  size_to_read = w * h * 3

  while True:
    buf = os_read(size_to_read)
    frame = np.frombuffer(buf, dtype=np.uint8).reshape(h, w, 3)
    # run DNN inference
    time.sleep(0.05) # heavy task
    # get box or polygon with label
    # write list of dict like this:
    dummy_box1 = { 'label': "person", 'coordinates': [ [100, 100], [200, 100], [200, 100], [200, 200] ] }
    dummy_box2 = { 'label': "dog",    'coordinates': [ [450, 300], [500, 300], [450, 450], [500, 450] ] }
    result = json.dumps([dummy_box1, dummy_box2])
    os_writeline(result)