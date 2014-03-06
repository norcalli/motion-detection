#ifndef MOTION_PROCESSOR_H_
#define MOTION_PROCESSOR_H_

class AveragedMotionProcessor {
 private:
  cv::Mat previousFrame;
  cv::Mat currentFrame;
  cv::Mat averageFrame;
  size_t pixelCount;
  // bool motionDetected_;
 public:
  cv::Mat result;
  int non_zero;

  void initialize(const cv::Mat& image) {
    cv::cvtColor(image, previousFrame, CV_RGB2GRAY);
    
    previousFrame.convertTo(averageFrame, CV_32F);

    pixelCount = image.rows * image.cols;
  }

  bool motionDetected(int threshold) const {
    double percentage = non_zero * 100.0 / pixelCount;
    return percentage > threshold;
    // return motionDetected_;
  }

  void processImage(const cv::Mat& image, double alpha = 0.05) {
    cv::cvtColor(image, currentFrame, CV_RGB2GRAY);

    // cv::blur(currentFrame, currentFrame, cv::Size(5, 5));
    averageFrame.convertTo(previousFrame, CV_8U);
    // averageFrame.convertTo(previousFrame, image.type());

    DOUT(getImageType(previousFrame.type()));
    DOUT(getImageType(currentFrame.type()));
    DEXEC(cv::absdiff(previousFrame, currentFrame, result));

    cv::blur(result, result, cv::Size(5, 5));
    cv::morphologyEx(result, result, cv::MORPH_OPEN, cv::Mat());
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, cv::Mat());
    // cv::threshold(result, result, 10, 255, THRESH_BINARY_INV);
    cv::threshold(result, result, 10, 255, cv::THRESH_BINARY);

    // currentFrame.copyTo(previousFrame);
    DEXEC(cv::accumulateWeighted(currentFrame, averageFrame, alpha));
    // currentFrame.copyTo(previousFrame);

    // int non_zero = cv::countNonZero(result);
    DEXEC(non_zero = cv::countNonZero(result));
    // double average = non_zero * 100.0 / pixelCount;
    // motionDetected_ = average > threshold;
  }
  // virtual ~MotionProcessor();
 private:
  
};


#endif  // MOTION_PROCESSOR_H_
