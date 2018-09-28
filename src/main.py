import detector
import cv2
import struct
import numpy as np

if __name__ == "__main__":
    target_bin = "../bin/b.out"

    recognizer = detector.Detector(mode="local", binary=target_bin)

    recognizer.main_image = "fr_0.jpg"

    recognizer.cluster()

    img = recognizer.image_request(1296, 1728)

    cv2.imwrite("test.jpg", img)

    cv2.imshow("image", img)
    cv2.waitKey()
