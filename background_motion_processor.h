#ifndef MOTION_PROCESSOR_H_
#define MOTION_PROCESSOR_H_

class BackgroundMotionProcessor {
 private:
  cv::Mat backgroundFrame;
  cv::Mat currentFrame;
  size_t pixelCount;
  // bool motionDetected_;
 public:
  cv::Mat result;
  int non_zero;
  // int threshold = 8;

  // MotionProcessor();

  // void setThreshold(int t) {
  //   threshold = t;
  // }

  void initialize(const cv::Mat& image) {
    cv::cvtColor(image, backgroundFrame, CV_RGB2GRAY);

    // cv::blur(result, result, cv::Size(5, 5));

    pixelCount = image.rows * image.cols;
  }

  bool motionDetected(int threshold) const {
    double percentage = non_zero * 100.0 / pixelCount;
    return percentage > threshold;
    // return motionDetected_;
  }

  void setBackground(const cv::Mat& image) {
    image.copyTo(backgroundFrame);
  }

  void processImage(const cv::Mat& image) {
    cv::cvtColor(image, currentFrame, CV_RGB2GRAY);

    cv::absdiff(backgroundFrame, currentFrame, result);

    cv::blur(result, result, cv::Size(5, 5));
    cv::morphologyEx(result, result, cv::MORPH_OPEN, cv::Mat());
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, cv::Mat());
    // cv::threshold(result, result, 10, 255, THRESH_BINARY_INV);
    cv::threshold(result, result, 10, 255, cv::THRESH_BINARY);
    // currentFrame.copyTo(previousFrame);

    // int non_zero = cv::countNonZero(result);
    non_zero = cv::countNonZero(result);
    // double average = non_zero * 100.0 / pixelCount;
    // motionDetected_ = average > threshold;
  }
  // virtual ~MotionProcessor();
 private:
  
};


#endif  // MOTION_PROCESSOR_H_
