import detector
import pipe
import numpy as np

if __name__ == "__main__":
    target_bin = "../bin/a.out"

    lambda: detector.Detector(mode="local", binary=target_bin)
    recognizer = detector.Detector(mode="local", binary=target_bin)
    for i in range(500):
        recognizer.read(1297)
    testarray = np.array(np.fromstring(recognizer.read(1297), dtype=np.uint8))

    for i in range(1297):
        if testarray[i] != 0:
            print testarray[i]
