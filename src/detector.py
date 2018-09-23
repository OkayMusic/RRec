import server
import numpy as np


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
            print "Attempting to set main_image to be an invalid type."
            print "Valid types are: np.ndarray, str."
            print "If type is a string, it should be a path to an image."

    def equalize(self):
        self._send_instruction(server.Server.equalize)

    def calculate_background(self):
        self._send_instruction(server.Server.calculateBackground)

    def calculate_signal(self):
        self._send_instruction(server.Server.calculateSignal)

    def calculate_significance(self):
        self._send_instruction(server.Server.calculateSignificance)

    def cluster(self):
        self._send_instruction(server.Server.cluster)
