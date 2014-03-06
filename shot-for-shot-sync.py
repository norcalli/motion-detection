from contextlib import closing
# from urllib.request import urlopen
from urllib2 import urlopen
from averaged_motion_processor import AveragedMotionProcessor

from io import BytesIO

import numpy as np
import cv2
import cv2.cv as cv
from datetime import datetime
import time

class ImageGrabber:
  def __init__(self, url):
    self.url = url

  def getFrame(self):
    with closing(urlopen(self.url)) as page:
      # data = BytesIO(page.read())
      data = np.fromstring(page.read(), dtype=np.uint8)
    image = cv2.imdecode(data, cv2.CV_LOAD_IMAGE_COLOR)
    return image

class MonitorUI:
  def initialize(self, initThreshold, upperThreshold=20):
    self.name = 'Overlay'
    cv2.namedWindow(self.name)
    self.threshold = initThreshold

    def updateThreshold(value):
      self.threshold = value
    
    cv.CreateTrackbar('threshold (%d)' % upperThreshold, self.name, self.threshold, upperThreshold, updateThreshold)
  
  def show(self, image):
    cv2.imshow(self.name, image)

class Recorder:
  fileDateFormat = "%b %d %H:%M"
  timeStampFormat = "%b %d, %H:%M:%S"

  def initialize(self, filename, interval, resolution):
    self.codec = cv.CV_FOURCC('M', 'P', '4', '2')
    self.interval = interval
    self.fps = 1.0/interval
    self.resolution = resolution
    self.filename = filename
    self.writer = cv2.VideoWriter(filename, self.codec, self.fps, self.resolution, True)

    # Font initialization details.
    self.fontFace = cv2.FONT_HERSHEY_SIMPLEX
    self.fontScale = 0.60
    self.fontColor = (255, 255, 255)
    self.org = (25, 30)

  def write(self, image):
    timeStamp = nowInFormat(Recorder.timeStampFormat)
    cv2.putText(image, timeStamp, self.org, self.fontFace, self.fontScale, self.fontColor)
    self.writer.write(image)

class SimpleTimer:
  def __init__(self):
    self.start = time.time()

  def Reset(self):
    self.start = time.time()

  def Elapsed(self):
    return time.time() - self.start

  def __str__(self):
    return repr(self)

  def __repr__(self):
    return '%.2f ms' % (self.Elapsed()*1000,)

import logging

import sys

def initializeLogging(filename, format):
  logFormatter = logging.Formatter(format)
  rootLogger = logging.getLogger()
  rootLogger.setLevel(logging.INFO)

  fileHandler = logging.FileHandler(filename)
  fileHandler.setFormatter(logFormatter)
  rootLogger.addHandler(fileHandler)

  consoleHandler = logging.StreamHandler()
  consoleHandler.setFormatter(logFormatter)
  rootLogger.addHandler(consoleHandler)
  
def nowInFormat(format):
  return datetime.now().strftime(format)

import threading
import Queue
  
class AsyncRecorder(threading.Thread):
  def __init__(self, queue):
    super(AsyncRecorder, self).__init__()
    self.queue = queue
    self.recorder = Recorder()

  def initialize(self, filename, interval, resolution):
    self.recorder.initialize(filename, interval, resolution)

  def run(self):
    while True:
      data = self.queue.get()
      if data is None:  # We can send it None to indicate a stop.
        self.queue.task_done()
        break
      if isinstance(data, list):
        for image in data:
          self.recorder.write(image)
      else:
        self.recorder.write(data)
      self.queue.task_done()

# class AsyncFrameGrabber(threading.Thread):
  

from itertools import islice
from collections import deque
import math

class VideoBuffer:
  def __init__(self, fps, seconds):
    self.fps = fps
    self.seconds = seconds
    self.length = self.secondsToFrames(seconds)
    self.buffer = deque([], self.length)

  def secondsToFrames(self, seconds):
    return math.ceil(self.fps * seconds)

  def __len__(self):
    return len(self.buffer)

  def __iter__(self):
    for x in self.buffer:
      yield x

  def clear(self):
    self.buffer.clear()

  def push(self, image):
    self.buffer.append(image)

  def popOldest(self):
    return self.buffer.popleft()

  def popRecent(self):
    return self.buffer.popleft()

  def snapShot(self):
    return list(self.buffer)

  def getLastNSeconds(self, n):
    length = self.secondsToFrames(n)
    if length >= self.length:
      return self.buffer
    # return islice(self.buffer, length)
    return islice(self.buffer, self.length - length)

from Queue import Queue
import atexit

# On the off chance that the image grabbing takes more time than
# the processing of the image, I should handle overflow by
# either buffering (which would mean an infinite build up of
# frames not being processed) or having a buffer of size 1 and
# overwriting the buffer, thereby skipping some frames.
class AsyncImageGrabber(threading.Thread):
  def __init__(self, url, interval, size=3):
    super(AsyncImageGrabber, self).__init__()
    self.queue = Queue(3)
    self.imageGrabber = ImageGrabber(url)
    self.stop = False
    self.interval = interval

  # def initialize(self, filename, interval, resolution):
  #   self.recorder.initialize(filename, interval, resolution)

  def clean(self):
    self.queue.clear()

  def get(self):
    return self.queue.get()

  def run(self):
    while not self.stop:
      self.queue.put(self.imageGrabber.getFrame())
      time.sleep(self.interval)
      
      # data = self.queue.get()
      # if data is None:  # We can send it None to indicate a stop.
      #   self.queue.task_done()
      #   break
      # if isinstance(data, list):
      #   for image in data:
      #     self.recorder.write(image)
      # else:
      #   self.recorder.write(data)
      # self.queue.task_done()

def main(script_name, url=None):
  if url is None:
    print('Usage: {0} <url>'.format(script_name))
    return 1

  # interval = 1.0/20
  interval = 1.0/10
  fps = 1.0/(interval+50e-3)
  alpha = 0.15
  recordTime = 10.0
  initialThreshold = 4
  initialWaitTime = 1.0

  logFormat = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
  # initializeLogging('shot-for-shot.log', logFormat)
  logFilename = 'shot-for-shot-{}.log'.format(nowInFormat(Recorder.fileDateFormat))
  initializeLogging(logFilename, logFormat)

  buffer = VideoBuffer(fps, 5.0)
  recordingQueue = Queue()
  networkQueue = Queue()
  ui = MonitorUI()
  # recorder = Recorder()
  asyncRecorder = AsyncRecorder(recordingQueue)
  recordingTimer, waitTimer = (SimpleTimer(), SimpleTimer())
  # imageGrabber = ImageGrabber(url)
  asyncImageGrabber = AsyncImageGrabber(url, interval)
  processor = AveragedMotionProcessor()

  logging.debug('Waiting for %g seconds.' % (initialWaitTime,))
  time.sleep(initialWaitTime)

  try:
    # image = imageGrabber.getFrame()
    image = asyncImageGrabber.imageGrabber.getFrame()
  except Exception as e:
    logging.error('Failed to grab image due to exception(%s)' % (str(e),))
    raise
  
  height, width, depth = image.shape
  resolution = (width, height)
  outputFileName = nowInFormat(Recorder.fileDateFormat) + '.mkv'

  ui.initialize(initialThreshold)
  # recorder.initialize(outputFileName, interval, resolution)
  asyncRecorder.initialize(outputFileName, interval, resolution)
  # asyncRecorder.setDaemon(True)
  asyncRecorder.start()
  asyncImageGrabber.start()

  logging.info('initializing recording for file "%s"' % (outputFileName,))
  processor.initialize(image)

  recording = False
  # stop = False

  # @atexit.register
  def cleanUp():
    logging.info('Sending "stop" signal to async recorder.')
    recordingQueue.put(None)
    # logging.info('Waiting for recording to finish.')
    logging.info('Waiting for queue to finish.')
    recordingQueue.join()
    logging.info('Waiting for recording to finish.')
    asyncRecorder.join()
    logging.info('Recorder finished up queue.')

  while True:
    try:
      image = imageGrabber.getFrame()
    except Exception as e:
      logging.error('Failed to grab image due to exception(%s)' % (str(e),))
      cleanUp()
      return 1
    
    buffer.push(image)
    processor.processImage(image, alpha)
    if not recording:
      if processor.motionDetected(ui.threshold):
        logging.info('Detected motion, starting recording at %s' % (nowInFormat(Recorder.timeStampFormat),))
        recordingTimer.Reset()
        recording = True
        recordingQueue.put(buffer.snapShot())
    else:
      if processor.motionDetected(ui.threshold):
        recordingTimer.Reset()
      else:
        if recordingTimer.Elapsed() >= recordTime:
          logging.info('Stopping recording at %s' % (nowInFormat(Recorder.timeStampFormat),))
          recording = False
      # recorder.write(image)
      recordingQueue.put(image)

      # if recordingTimer.Elapsed() >= recordTime:
      #   if processor.motionDetected(ui.threshold):
      #     recordingTimer.Reset()
      #   else:
      #     logging.info('Stopping recording at %s' % (nowInFormat(Recorder.timeStampFormat),))
      #     recording = False
      # else:
      #   if processor.motionDetected(ui.threshold):
      #     recordingTimer.Reset()
      #   # recorder.write(image)
      #   queue.put(image)
    
    # Numpy addition is faster, but also modulo 256 (in uint8),
    # so for threshold addition, you have to use cv.add
    # image += cv2.cvtColor(processor.result, cv.CV_GRAY2RGB)
    image = cv2.add(cv2.cvtColor(processor.result, cv.CV_GRAY2RGB), image)
    ui.show(image)
    
    waitTimer.Reset()
    while waitTimer.Elapsed() < interval:
      time.sleep(1e-2)
      if cv2.waitKey(1) == 27:   # Escape was pressed.
        logging.info('Received escape key while waiting, quitting.')
        cleanUp()
        return 0
    #     stop = True
    #     break
    # if stop:
    #   logging.info('Received stop, breaking main loop')
    #   break
  cleanUp()
  return 0


if __name__ == '__main__':
  import sys
  sys.exit(main(*sys.argv))
