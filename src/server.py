import struct
import subprocess
import atexit

import numpy as np


class Server(object):
    # commonly used strings go here
    local = "local"
    online = "online"

    # define integers used as responses from the C++ end
    success = struct.pack('i', 0)
    notImplemented = struct.pack('i', 1)
    error = struct.pack('i', 2)

    # define integers used as flags for the C++ end
    imageRequest = struct.pack('i', 0)
    loadFromFile = struct.pack('i', 1)
    loadFromPython = struct.pack('i', 2)
    runAlgorithm = struct.pack('i', 3)
    equalize = struct.pack('i', 4)
    calculateBackground = struct.pack('i', 5)
    calculateSignal = struct.pack('i', 6)
    calculateSignificance = struct.pack('i', 7)
    cluster = struct.pack('i', 8)

    def __init__(self, mode=local, binary=None):
        # if mode is local, run the subprocess binary on local machine
        self.mode = mode
        if mode == Server.local:
            self.process = subprocess.Popen("./" + binary,
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE)
        else:
            # for now, throw an error if the mode is set to be anything else
            print "Set mode='local', mode='online' has not yet been implemented"
            print "exiting..."
            exit()

        # kill subprocess on program termination
        atexit.register(lambda: self.process.kill())

    def request(self, message):
        """
        Sends an ascii message to the stdin of the C++ backend.
        """
        if self.mode == Server.local:
            return self.process.stdin.write(message)

    def read(self, num_chars):
        """
        Reads num_chars characters from the C++ backend's stdout file.
        """
        if self.mode == Server.local:
            return self.process.stdout.read(num_chars)

    def readline(self):
        """
        Reads a line from the C++ backend's stdout file.
        """
        if self.mode == Server.local:
            return self.process.stdout.readline()

    def load_array(self, array):
        """
        Loads a numpy array into the C++ code's main image.
        """
        self._send_instruction(Server.loadFromPython)
        self.request(array)

    def load_file(self, path, dimensions=None):
        """
        Loads an image stored in a file into the C++ code's main image. This 
        method can handle .pic files, as well as all standard image formats.
        """
        self._send_instruction(Server.loadFromFile)
        self.request(str(path) + '\n')

        response = self.read(4)
        if response != Server.success:
            print self.readline()
            print "(PYTHON): An error occurred"
            print struct.unpack('i', response)[0]

    def image_request(self):
        """
        Grabs the main image from the C++ end, and returns the corresponding 
        numpy array.
        """
        # send the image request instruction
        self._send_instruction(Server.imageRequest)
        return np.reshape(np.array(np.fromstring(self.read(1296*1728),
                                                 dtype=np.uint8)), (1296, 1728),
                          order='C')

    def _send_instruction(self, instruction):
        """
        Sends a single integer instruction to the C++ backend.
        """
        # make sure the instruction is 4 bytes long
        if len(instruction) != 4:
            raise ValueError(
                "Incorrect instruction length, do not call the _send_instruction method manually")
        self.request(instruction)
