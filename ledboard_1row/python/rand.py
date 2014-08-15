#!/usr/bin/python
"""A python controller for the LED board."""

import random
import serial
import sys
import time

class Board(object):
  def __init__(self):
    self.port = serial.Serial('/dev/ttyUSB0', 57600, timeout=1);
    self.port.write('\r')
    self.readywait()

  def readywait(self, text=None):
    """Waits for the specified text to appear in the serial buffer."""
    buf = ''
    text = text or 'arduino>'
    while text not in buf:
      buf += self.port.read()

  def bitmap(self, data):
    """Writes the given 512-bit data to the board."""
    self.port.write('bitmap\r');
    self.readywait('bitmap data.')
    self.port.write(data)
    self.port.flush()
    self.readywait()

  def text(self, text):
    """Write the given text to the board."""
    self.port.write('text\r')
    self.readywait('a newline:')
    self.port.write(text)
    self.port.write('\r')
    self.port.flush()
    self.readywait()

  def scroll(self, amount):
    """Scroll by the given number of pixels."""
    self.port.write('scroll %d\r' % amount)
    self.readywait()

  def pos(self, position):
    """Set the absolute horizontal position of the text row."""
    self.port.write('pos %d\r' % position)
    self.readywait()

  def vpos(self, position):
    """Set the absolute vertical position of the text row."""
    self.port.write('vpos %d\r' % position)
    self.readywait()

  def timeloop(self):
    """Displays the current time in a loop."""
    start = time.time()
    while True:
      if int(time.time()) == int(start):
        time.sleep(0.1)
        continue
      start = time.time()
      t = time.localtime()
      self.text('%02d:%02d.%02d' % (t.tm_hour, t.tm_min, t.tm_sec))

  def randomdots(self):
    """Show random dots on the board."""
    start = time.time()
    count = 0
    while True:
      data = []
      for i in xrange(128):
        data.append(chr(random.randint(0, 255)))
      self.bitmap(''.join(data))
      count += 1
      if time.time() - start > 1:
        sys.stdout.write('\r%2.1f fps' % (count))
        sys.stdout.flush()
        count = 0
        start = time.time()


if __name__ == '__main__':
  b = Board()
  b.timeloop()
