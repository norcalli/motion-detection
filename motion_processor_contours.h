#ifndef MOTION_PROCESSOR_CONTOURS_H_
#define MOTION_PROCESSOR_CONTOURS_H_


class MotionProcessorContours {
 private:
  cv::Mat work;
 public:
  // MotionProcessor();

  void initialize(const cv::Mat& image) {
    image.copyTo(work);
    
  }

  bool motionDetected() const {
    return false;
  }

  void processImage(const cv::Mat& image) {
    DOUT(getImageType(image.type()));
    // // resize image to a size we can work with
    // cv::resize(img_read, img_work);
    image.copyTo(work);

    // smoothen the image
    // // cvSmooth(img_work, img_smooth, CV_GAUSSIAN, 7);
    // cvSmooth(img_work, img_smooth, CV_BILATERAL, 5, 5, 30, 30);
    cv::Mat work_bilateral;
    // cv::bilateralFilter(work, work, 5, 30, 30);
    cv::bilateralFilter(work, work_bilateral, 5, 30, 30);
    DOUT(getImageType(work.type()));

    // increase contrast and adjust brightness
    // cvAddWeighted(img_smooth, 1, img_smooth, 1, param_brightness_factor-50, img_color);
    // cv::addWeighted(work, 1, work, 1, param_brightness_factor-50, work);
    cv::addWeighted(work_bilateral, 1, work_bilateral, 1, param_brightness_factor-50, work_bilateral);
    DOUT(getImageType(work_bilateral.type()));

    // increase contrast further if specified
    for(int contrast_idx = 0; contrast_idx < param_contrast_factor; contrast_idx++) {
      // cvAddWeighted(img_color, 1, img_color, 1, 0, img_color);
      // cv::addWeighted(work, 1, work, 1, 0, work);
      cv::addWeighted(work_bilateral, 1, work_bilateral, 1, 0, work_bilateral);
    }

    // cv::Mat edge(image.size(), CV_16U);
    // cv::Mat edge_grey(image.size(), CV_U);
    cv::Mat edge;
    cv::Mat edge_grey;
    // cvLaplace(img_color, img_edge_color, 3);
    // cv::Laplacian(work, edge, 3);
    // DEXEC(cv::Laplacian(work_bilateral, edge, 1, 1));
    // DEXEC(cv::Laplacian(work_bilateral, edge, 3));
    DEXEC(cv::Laplacian(work_bilateral, edge, CV_8U));
    DOUT(getImageType(edge.type()));
    // cvCvtColor(img_edge_color, img_edge_gray, CV_RGB2GRAY);
    // cv::cvtColor(edge, edge, CV_RGB2GRAY);

    // cv::imshow("Work", work_bilateral);
    // cv::imshow("Edge", edge);
    // cv::waitKey(0);
    // edge.convertTo(edge, CV_16UC3);
    DOUT(getImageType(edge.type()));
    DEXEC(cv::cvtColor(edge, edge_grey, CV_RGB2GRAY));
    // cvThreshold(img_edge_gray, img_edge_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);
    // cv::threshold(edge, edge, 25+param_detect_threshold, 255, CV_THRESH_BINARY);
    cv::threshold(edge_grey, edge_grey, 25+param_detect_threshold, 255, CV_THRESH_BINARY);
    // cvCvtColor(img_edge_gray, img_edge_color, CV_GRAY2RGB);
    // cv::cvtColor(edge, edge, CV_GRAY2RGB);
    DEXEC(cv::cvtColor(edge_grey, edge, CV_GRAY2RGB));

    cv::imshow("Edges", edge);

    // cvAdd(img_edge_color, img_color, img_color, NULL);
    // cv::add(edge, work, work);
    cv::add(edge, work_bilateral, work);

    cv::waitKey(0);
// if (first) {
// 	cvConvertScale(img_color, img_moving_avg, 1.0, 0.0);
// 	first = false;
// }
// else {
// 	cvRunningAvg(img_color, img_moving_avg, ((float)param_moving_avg_wt)/100.0, NULL);
// }

// cvConvertScale(img_moving_avg, img_temp, 1.0, 0);	// convert the moving avg to a format usable for diff
// cvAbsDiff(img_color, img_temp, img_diff);	// subtract current from moving average.
// cvCvtColor(img_diff, img_gray, CV_RGB2GRAY);// convert image to gray
// cvThreshold(img_gray, img_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);	// convert image to black and white
// //cvThreshold(greyImage, greyImage, 70, 255, CV_THRESH_BINARY);	//Convert the image to black and white.
// //cvAdaptiveThreshold(img_gray, img_gray, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 0);

// // dilate and erode to reduce noise and join irregular blobs
// cvErode(img_gray, img_gray, 0, 2); 	// erode to remove noise
// cvDilate(img_gray, img_gray, 0, param_dilation_amt+2);// dilate to join and fill blobs
// cvErode(img_gray, img_gray, 0, param_erosion_amt);	// erode again to get some of the original proportion back
// cvConvertScale(img_gray, img_contour, 1.0, 0.0);	// copy image to the contour image for contour detection

// // find the contours of the moving images in the frame.
// cvClearMemStorage(mem_store);
// CvSeq* contour = 0;
// cvFindContours(img_contour, mem_store, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

  }
  // virtual ~MotionProcessor();
 private:
  
};


#endif  // MOTION_PROCESSOR_CONTOURS_H_
