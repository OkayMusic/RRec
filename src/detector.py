import server
import struct
import numpy as np
from time import sleep


class Detector(server.Server):
    def __init__(self, mode, binary=None):
        # call server's __init__ method
        super(Detector, self).__init__(mode, binary)
        self._main_image = None

    # definition of the main_image property:
    @property
    def main_image(self):
        return self.image_request()

    @main_image.setter
    def main_image(self, value):
        if type(value) == str:
            self.load_file(value)
        elif type(value) == np.ndarray:
            self.load_array(value)
        else:
            raise TypeError(
                "main_image must be set to a string or a numpy array")

    def equalize(self):
        self._send_instruction(server.Server.equalize)
        if self.read(4) != server.Server.success:
            print "(PYTHON): Error in equalize"
            print self.readline()

    def calculate_background(self, brightness_variance):
        if type(brightness_variance) != int:
            raise TypeError("Arg to calculate_background must be an integer")
        else:
            self._send_instruction(server.Server.calculateBackground)
            self.request(struct.pack('i', brightness_variance))

            if self.read(4) != server.Server.success:
                print "(PYTHON): Error in calculate_background"
                print self.readline()

    def calculate_signal(self, signal_size):
        if type(signal_size) != int:
            raise TypeError("Arg to calculate_background must be an integer")
        else:
            self._send_instruction(server.Server.calculateSignal)
            self.request(struct.pack('i', signal_size))

            response = self.read(4)
            if response != server.Server.success:
                print "(PYTHON): Error in calculate_signal"
                print "Response:", struct.unpack('i', response)[0]

                print "Instruction received, ", self.readline()

    def calculate_significance(self, sigma):
        if type(sigma) != float and type(sigma) != int:
            raise TypeError(
                "Arg to calculate_background must be an int or a float")
        else:
            self._send_instruction(server.Server.calculateSignificance)
            self.request(struct.pack('d', sigma))
            response = self.read(4)
            if response != server.Server.success:
                print "(PYTHON): Error in calculate_significance"
                print "Response:", struct.unpack('i', response)[0]

                print "Instruction received, ", self.readline()

    def cluster(self):
        self._send_instruction(server.Server.cluster)

        response = self.read(4)
        if response != server.Server.success:
            print "(PYTHON): Error in cluster"
            print "Response:", struct.unpack('i', response)[0]
            print "Instruction received, ", self.readline()
