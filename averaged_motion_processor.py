import cv2
import cv2.cv as cv
import numpy as np

class AveragedMotionProcessor:
  """
  """

  def initialize(self, image):
    self.previousFrame = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY);

    # Should convert because after runningavg take 32F pictures
    self.averageFrame = np.float32(self.previousFrame);

    width, height, depth = image.shape
    self.pixelCount = width * height
    self.nonZero = 0
    
  def motionDetected(self, threshold):
    percentage = self.nonZero * 100.0 / self.pixelCount
    return percentage > threshold

  def processImage(self, image, alpha=0.05):
    self.currentFrame = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    self.previousFrame = np.uint8(self.averageFrame)
    self.result = cv2.absdiff(self.previousFrame, self.currentFrame)
    self.result = cv2.blur(self.result, (5, 5))
    self.result = cv2.morphologyEx(self.result, cv2.MORPH_OPEN, None)
    self.result = cv2.morphologyEx(self.result, cv2.MORPH_CLOSE, None)
    retval, self.result = cv2.threshold(self.result, 10, 255, cv2.THRESH_BINARY)
    # self.averageFrame = cv2.accumulateWeighted(self.currentFrame, self.averageFrame, alpha)
    cv2.accumulateWeighted(self.currentFrame, self.averageFrame, alpha)
    self.nonZero = cv2.countNonZero(self.result)

    # cv.Smooth(curframe, curframe) # Remove false positives
    
    # # For the first time put values in difference, temp and moving_average
    # if not self.absdiff_frame:
    #   self.absdiff_frame = cv.CloneImage(curframe)
    #   self.previous_frame = cv.CloneImage(curframe)

    #   # Should convert because after runningavg take 32F pictures
    #   cv.Convert(curframe, self.average_frame)
    # else:
    #   cv.RunningAvg(curframe, self.average_frame, 0.05) #Compute the average
            
    # cv.Convert(self.average_frame, self.previous_frame) #Convert back to 8U frame
          
    # cv.AbsDiff(curframe, self.previous_frame, self.absdiff_frame) # moving_average - curframe
          
    # cv.CvtColor(self.absdiff_frame, self.gray_frame, cv.CV_RGB2GRAY) #Convert to gray otherwise can't do threshold
    # cv.Threshold(self.gray_frame, self.gray_frame, 50, 255, cv.CV_THRESH_BINARY)
    # cv.Dilate(self.gray_frame, self.gray_frame, None, 15) #to get object blobs
    # cv.Erode(self.gray_frame, self.gray_frame, None, 10)
