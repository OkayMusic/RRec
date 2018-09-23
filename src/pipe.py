import subprocess


class Pipe(object):
    def __init__(self, binary):
        self.process = subprocess.Popen("./" + binary,
                                        stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE)

    def request(self, message):
        return self.process.stdin.write(message)

    def read(self, numChars):
        return self.process.stdout.read(numChars)

    def readline(self):
        return self.process.stdout.readline()
