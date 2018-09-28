import detector
import cv2
import struct
import numpy as np


def my_func():
    pass


if __name__ == "__main__":
    target_bin = "../bin/b.out"

    my_detector = detector.Detector(mode="local", binary=target_bin)

    my_detector.main_image = "fr_0.jpg"

    clusters = my_detector.cluster()

    print len(clusters)

    # img = my_detector.image_request(1296, 1728)

    # cv2.imwrite("test.jpg", img)

    # cv2.imshow("image", img)

    # cv2.waitKey()
