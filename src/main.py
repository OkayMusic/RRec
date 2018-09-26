import detector
import cv2
import struct
import numpy as np

if __name__ == "__main__":
    target_bin = "../bin/b.out"

    recognizer = detector.Detector(mode="local", binary=target_bin)

    test_array = np.array(([0, 0, 0], [0, 50, 255]), dtype=np.uint8)

    recognizer.main_image = test_array

    img = recognizer.image_request(2, 3)

    cv2.imshow("image", img)
    cv2.waitKey()
