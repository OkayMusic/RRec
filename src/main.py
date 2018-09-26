import detector
import cv2
import struct
import numpy as np

if __name__ == "__main__":
    target_bin = "../bin/b.out"

    recognizer = detector.Detector(mode="local", binary=target_bin)

    print "(PYTHON): ", 0
    recognizer.main_image = "fr_0.jpg"
    print "(PYTHON): ", 1
    recognizer.equalize()
    print "(PYTHON): ", 2

    recognizer.calculate_background(51)
    print "(PYTHON): ", 3

    recognizer.calculate_signal(3)
    print "(PYTHON): ", 4

    recognizer.calculate_significance(1)
    print "(PYTHON): ", 5

    recognizer.cluster()
    print "(PYTHON): ", 6

    test_image = recognizer.image_request()
    cv2.imshow("image", test_image)
    cv2.waitKey()
