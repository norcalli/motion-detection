#ifdef DEBUG
// #define DEXEC(x) std::cout << __FILE__ ":" << __LINE__ <<  " " #x << std::endl; x;
#define DEXEC(x) std::cout << __FILE__ ":" << __LINE__ <<  ": in function " << __FUNCTION__ << ": executing " #x << std::endl; x;

#define DOUT(x) std::cout << #x << " = (" << (x) << ')' << std::endl;
#else
#define DEXEC(x) x;
#define DOUT(x)
#endif

#include <string>
#include <sstream>

std::string getImageType(int number) {
  // find type
  int imgTypeInt = number % 8;
  std::string imgTypeString;

  switch (imgTypeInt) {
    case 0:   imgTypeString = "8U";  break;
    case 1:   imgTypeString = "8S";  break;
    case 2:   imgTypeString = "16U"; break;
    case 3:   imgTypeString = "16S"; break;
    case 4:   imgTypeString = "32S"; break;
    case 5:   imgTypeString = "32F"; break;
    case 6:   imgTypeString = "64F"; break;
    default:  break;
  }

  // find channel
  int channel = (number >> 3) + 1;

  std::stringstream type;
  type << "CV_" << imgTypeString << "C" << channel;

  return type.str();
}


#include <cv.h>
#include <highgui.h>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
#include <iostream>
#include "averaged_motion_processor.h"
#include "motion_processor.h"
#include "simple_timer.h"

bool win() {
  std::cout << "Win\n";
  return true;
}

bool lose() {
  std::cout << "Lose\n";
  return false;
}

/*****************************************************
 * limit max size of image to be processed to 800x600
 *****************************************************/
void get_approp_size(CvCapture *input, CvSize &img_size, int &img_depth, int &img_channels) {
	IplImage *img = cvQueryFrame(input);
	CvSize ori_size = cvGetSize(img);
	img_depth = img->depth;
	img_channels = img->nChannels;

	printf("Frame size: %d x %d\nDepth: %d\nChannels: %d\n", ori_size.width, ori_size.height, img_depth, img_channels);

	float div_frac_h = 600.0/((float)ori_size.height);
	float div_frac_w = 800.0/((float)ori_size.width);
	float div_frac = div_frac_w < div_frac_h ? div_frac_w : div_frac_h;
	if(div_frac > 1) div_frac = 1;

	img_size.height = ori_size.height * div_frac;
	img_size.width = ori_size.width * div_frac;
}


// int main(int argc, char** argv) {
//   std::string videoStreamAddress = "http://192.168.0.103:8080/videofeed?.mjpg";
//   if (argc > 1) {
//     videoStreamAddress = argv[1];
//   } else {
//     std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
//     return 1;
//   }

//   // true && win() || lose();
//   // false && win() || lose();
//   // return 0;

//   cv::VideoCapture vcap;
//   cv::Mat image;

//   // std::string videoStreamAddress = "http://192.168.0.103:8080/videofeed?.mjpg";
//   // const std::string videoStreamAddress = "rtsp://cam_address:554/live.sdp";
//   // const std::string videoStreamAddress = "rtsp://cam_address:554/live.sdp";

//   /* it may be an address of an mjpeg stream,
//      e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

//   //open the video stream and make sure it's opened
//   if(!vcap.open(videoStreamAddress)) {
//     std::cout << "Error opening video stream or file" << std::endl;
//     return -1;
//   }

//   MotionDetector motion;
  
//   // Initialize with first frame.
//   vcap.read(image);
//   motion.Initialize(image);

//   // Wait for the camera to stop detecting motion.
//   do {
//     vcap.read(image);
//     motion.ProcessImage(image);
//   } while (motion.DetectMotion());

//   // while (DetectMotion())
//   //   sleep(100);

//   /*
//     Transitions:
//     - Still to motion: if motion is detected for:
//       - x seconds
//       - a large enough object is detected?
//     - Motion to Still: if still for:
//       - y seconds
//    */


//   double motion_threshold_time = 5;
//   double stillness_threshold_time = 5;

//   double fps = vcap.get(CV_CAP_PROP_FPS);
//   uint64_t frame_count = 0;

//   auto to_seconds = [fps](uint64_t count) { return count / fps; };
//   auto motion_duration = make_clock(frame_count);
//   auto stillness_duration = make_clock(frame_count);

//   State state;
//   // state.still = true;
//   state.setStill();
//   stillness_duration.Reset();
//   do {
//     if(!vcap.read(image)) {
//       std::cout << "No frame" << std::endl;
//       cv::waitKey();
//     }


//     // ProcessImage could return whether or not there is a change in motion.
//     motion.ProcessImage(image);

//     // Process image and check if a change (motion) has been detected.
//     // if (motion.ProcessImage(image)) {
//     if (motion.MotionDetected()) {
//       // We were still and are now detecting motion.
//       if (state.isStill()) {
        

//         // Start measuring how long the motion will be going on for.
//         motion_duration.Reset();

//       // We were detecting motion and are still detecting motion.
//       } else {
        
//       }

//     // Motion hasn't been detected in this frame.
//     } else {
//       // We were still and are still still.
//       if (state.isStill()) {

//       // We were detecting motion and are now still.
//       } else {
        
//       }
//     }

//     cv::imshow("Output Window", image);


//   }  while (cv::waitKey(1) >= 0);

//   // motion.DetectMotion()

//   for(;;) {
//     if(!vcap.read(image)) {
//       std::cout << "No frame" << std::endl;
//       cv::waitKey();
//     }
//     cv::imshow("Output Window", image);
//     if(cv::waitKey(1) >= 0) break;
//   }
// }


int 			param_display_stage = 7;
int 			param_moving_avg_wt = 2;
int 			param_detect_threshold = 20;
int 			param_min_obj_size = 2;
int 			param_dilation_amt = 30;
int 			param_erosion_amt = 10;
int 			param_brightness_factor = 50;
int 			param_contrast_factor = 0;

/*
// resize image to a size we can work with
cvResize(img_read, img_work);

// smoothen the image
// cvSmooth(img_work, img_smooth, CV_GAUSSIAN, 7);
cvSmooth(img_work, img_smooth, CV_BILATERAL, 5, 5, 30, 30);

// increase contrast and adjust brightness
cvAddWeighted(img_smooth, 1, img_smooth, 1, param_brightness_factor-50, img_color);

// increase contrast further if specified
for(int contrast_idx = 0; contrast_idx < param_contrast_factor; contrast_idx++) {
	cvAddWeighted(img_color, 1, img_color, 1, 0, img_color);
}

cvLaplace(img_color, img_edge_color, 3);
cvCvtColor(img_edge_color, img_edge_gray, CV_RGB2GRAY);
cvThreshold(img_edge_gray, img_edge_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);
cvCvtColor(img_edge_gray, img_edge_color, CV_GRAY2RGB);
cvAdd(img_edge_color, img_color, img_color, NULL);

if (first) {
	cvConvertScale(img_color, img_moving_avg, 1.0, 0.0);
	first = false;
}
else {
	cvRunningAvg(img_color, img_moving_avg, ((float)param_moving_avg_wt)/100.0, NULL);
}

cvConvertScale(img_moving_avg, img_temp, 1.0, 0);	// convert the moving avg to a format usable for diff
cvAbsDiff(img_color, img_temp, img_diff);	// subtract current from moving average.
cvCvtColor(img_diff, img_gray, CV_RGB2GRAY);// convert image to gray
cvThreshold(img_gray, img_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);	// convert image to black and white
//cvThreshold(greyImage, greyImage, 70, 255, CV_THRESH_BINARY);	//Convert the image to black and white.
//cvAdaptiveThreshold(img_gray, img_gray, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 0);

// dilate and erode to reduce noise and join irregular blobs
cvErode(img_gray, img_gray, 0, 2); 	// erode to remove noise
cvDilate(img_gray, img_gray, 0, param_dilation_amt+2);// dilate to join and fill blobs
cvErode(img_gray, img_gray, 0, param_erosion_amt);	// erode again to get some of the original proportion back
cvConvertScale(img_gray, img_contour, 1.0, 0.0);	// copy image to the contour image for contour detection

// find the contours of the moving images in the frame.
cvClearMemStorage(mem_store);
CvSeq* contour = 0;
cvFindContours(img_contour, mem_store, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

// process each moving contour in the current frame...
bool motion_detected = false;
for (; contour != 0; contour = contour->h_next) {
	CvRect bnd_rect = cvBoundingRect(contour, 0);	// get a bounding rect around the moving object.

	// discard objects smaller than our expected object size
	int obj_size_pct = bnd_rect.width * bnd_rect.height * 100 / (sz_of_img.height * sz_of_img.width);
	if (obj_size_pct < param_min_obj_size) continue;

	// either draw the contours or motion detection marker
	if(6 == param_display_stage) {
cvDrawContours(img_work, contour, CV_RGB(0,255,0), CV_RGB(0,255,0), 3, CV_FILLED);
	}
	else if(7 == param_display_stage) {
CvPoint center;
center.x = bnd_rect.x + bnd_rect.width/2;
center.y = bnd_rect.y + bnd_rect.height/2;
int rad = (bnd_rect.width < bnd_rect.height ? bnd_rect.width : bnd_rect.height)/2;

while (rad > 0) {
	cvCircle(img_work, center, rad, CV_RGB(153,204,50), 1, CV_AA);
	rad -= 8;
}
	}
	motion_detected = true;
}

if(is_action_mode) {
	if(motion_detected) {
cvReleaseCapture(&inp_device);
system(param_command_on_motion);
inp_device = capture_input(argc, argv);
	}
}
else {
	char c = display_frame(motion_detected);
	if(27 == c) break;
	else if('s' == c) save_settings();
	else if('r' == c) load_settings();
}
 */

cv::Size newSizeWithSameRatio(cv::Size size, cv::Size bound) {
  double ratioW = bound.width / (double)size.width;
  double ratioH = bound.height / (double)size.height;
  double ratio = std::min(ratioW, ratioH);

  size.width *= ratio;
  size.height *= ratio;
  return size;
}


void onTrackbar(int, void*) {}

int main(int argc, char** argv) {
  std::string videoStreamAddress = "http://192.168.0.103:8080/videofeed?.mjpg";
  if (argc > 1) {
    videoStreamAddress = argv[1];
  } else {
    std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
    return 1;
  }

  // true && win() || lose();
  // false && win() || lose();
  // return 0;

  cv::VideoCapture vcap;
  cv::Mat image;

  // std::string videoStreamAddress = "http://192.168.0.103:8080/videofeed?.mjpg";
  // const std::string videoStreamAddress = "rtsp://cam_address:554/live.sdp";
  // const std::string videoStreamAddress = "rtsp://cam_address:554/live.sdp";

  /* it may be an address of an mjpeg stream,
     e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

  //open the video stream and make sure it's opened
  if(!vcap.open(videoStreamAddress)) {
  // if(!vcap.open(0)) {
    std::cout << "Error opening video stream or file" << std::endl;
    return -1;
  }

  // MotionProcessor processor;
  AveragedMotionProcessor processor;
  
  int init_wait_time = 1000; // ms

  // Initialize with first frame.
  vcap.read(image);
  processor.initialize(image);

  // Wait a few seconds for the camera to start up.
  std::this_thread::sleep_for(std::chrono::milliseconds(init_wait_time));

  int threshold = 4;

  const int thresholdMax = 20;

  // cv::namedWindow("Overlay", 1);
  cv::namedWindow("Overlay", CV_WINDOW_AUTOSIZE);
  char trackbarName[50];
  sprintf(trackbarName, "Threshold x %d", thresholdMax);

  // cv::createTrackbar(trackbarName, "Overlay", &threshold, thresholdMax, onTrackbar);
  cv::createTrackbar("threshold", "Overlay", &threshold, thresholdMax, nullptr);

  // // Wait for the camera to stop detecting motion.
  // do {
  //   vcap.read(image);
  //   processor.processImage(image);
  // } while (processor.motionDetected(threshold));

  // int record_time = 10*1000;
  std::chrono::milliseconds record_time{10*1000};
  auto dateFormat = "%b %d, %H:%M:%S";

  // auto now_c = std::time(nullptr);
  auto dateAsString = [](const char* format) { auto now_c = std::time(nullptr); return (std::ostringstream() << std::put_time(std::localtime(&now_c), format)).str(); };

  double fps = vcap.get(CV_CAP_PROP_FPS);
  long width = vcap.get(CV_CAP_PROP_FRAME_WIDTH);
  long height = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
  auto codec = CV_FOURCC('M','P','4','2');

  DOUT(width);
  DOUT(height);

  std::string filename = dateAsString("%b %d %H:%M") + ".mkv";

  // TODO: How to detect if is color?
  cv::VideoWriter recorder(filename, codec, fps, cv::Size(width, height), true);
  // cv::VideoWriter recorder;
  // recorder.open(filename, codec, fps, cv::Size(width, height), true);

  // cv::VideoWriter recorder(filename, codec, 15, cv::Size(width, height), true);

  auto fontFace = cv::FONT_HERSHEY_SIMPLEX;
  auto fontScale = 0.60;
  auto fontColor = cv::Scalar(0xFF, 0xFF, 0xFF);
  // auto font = cv::InitFont(cv.CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 2, 8)

  // using thirty_hertz = duration<long, std::ratio<1, 120>>;
  // thirty_hertz time_delta{1};

  // using millihertz = std::chrono::duration<long, std::ratio<1, 1000>>;
  // auto time_delta = long(fps * 1000) * millihertz{1};
  auto time_delta = std::chrono::microseconds(long(1e6 / fps));
  
  uint64_t frame_count = 0; // Good for 2^64 / 30Hz ~ 1.95e10 years

  // auto to_seconds = [time_delta](uint64_t count) { return count / time_delta; };
  auto to_seconds = [time_delta](uint64_t count) { return count * time_delta; };

  struct State {
    bool recording = false;
    uint64_t trigger_time;
  } state;

  timer::SimpleTimer timer;
  // processor.threshold = 15;

  do {
    if(!vcap.read(image)) {
      std::cout << "No frame" << std::endl;
      cv::waitKey(1);
// vcap.open(videoStreamAddress);
//       continue;
break;
    }
    ++frame_count;

    processor.processImage(image);
    if (!state.recording) {
      // processor.processImage(image);
      if (processor.motionDetected(threshold)) {
        std::cout << "Starting recording at " << dateAsString(dateFormat) << std::endl;
        // state.trigger_time = frame_count;
        timer.Reset();
        state.recording = true;
      }
    } else {  // You are currently recording.
      // Only record for 10 seconds at a time.
      // if (to_seconds(frame_count - state.trigger_time) >= record_time) {
      if (timer.Elapsed() >= record_time) {
        // Check to see whether you should continue recording.
        // processor.processImage(image);
        if (processor.motionDetected(threshold))
          // state.trigger_time = frame_count; // Reset trigger time to record.
          timer.Reset(); // Reset trigger time to record.
        else {  // If no motion detected, stop recording.
          std::cout << "Stopping recording at " << dateAsString(dateFormat) << std::endl;
          state.recording = false;
        }
      } else {  // You haven't passed the minimum record time yet, so just record.
        if (processor.motionDetected(threshold)) {
          timer.Reset();
        }
        // std::stringstream stream;
        // stream << std::put_time(std::localtime(&now_c), "%b %d, %H:%M:%S");
        auto timeStamp = dateAsString(dateFormat);
        // cv::putText(image, timeStamp, cv::Point(25, 30), timeStampFont, 0);
        cv::putText(image, timeStamp, cv::Point(25, 30), fontFace, fontScale, fontColor);

        // TODO: could add timestamp to frame.
        // recorder.recordFrame(image);

        recorder << image;
      }
    }
    // cv::add(processor.result, image, image);
    cv::Mat color_overlay;
    cv::cvtColor(processor.result, color_overlay, CV_GRAY2RGB);
    image += color_overlay;

    cv::imshow("Overlay", image);
    if (cv::waitKey(1) >= 0)
      break;

  // }  while (cv::waitKey(1)  0);
  }  while (true);
}


// /*****************************************************
//  * main loop
//  *****************************************************/
// int main(int argc, char* argv[]) {
// 	inp_device = capture_input(argc, argv);
// 	bool		is_action_mode = ((argc > 3) && (0 == strcmp(argv[3], "act")));
// 	print_lib_version();
// 	load_settings();

// 	get_approp_size(inp_device, sz_of_img, depth_of_img, channels_of_img);
// 	init(is_action_mode);

// 	bool first = true;

// 	img_read = NULL;
// 	for (img_read = cvQueryFrame(inp_device); img_read; img_read = cvQueryFrame(inp_device)) {
// 		// resize image to a size we can work with
// 		cvResize(img_read, img_work);

// 		// smoothen the image
// 		// cvSmooth(img_work, img_smooth, CV_GAUSSIAN, 7);
// 		cvSmooth(img_work, img_smooth, CV_BILATERAL, 5, 5, 30, 30);

// 		// increase contrast and adjust brightness
// 		cvAddWeighted(img_smooth, 1, img_smooth, 1, param_brightness_factor-50, img_color);

// 		// increase contrast further if specified
// 		for(int contrast_idx = 0; contrast_idx < param_contrast_factor; contrast_idx++) {
// 			cvAddWeighted(img_color, 1, img_color, 1, 0, img_color);
// 		}

// 		cvLaplace(img_color, img_edge_color, 3);
// 		cvCvtColor(img_edge_color, img_edge_gray, CV_RGB2GRAY);
// 		cvThreshold(img_edge_gray, img_edge_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);
// 		cvCvtColor(img_edge_gray, img_edge_color, CV_GRAY2RGB);
// 		cvAdd(img_edge_color, img_color, img_color, NULL);

// 		if (first) {
// 			cvConvertScale(img_color, img_moving_avg, 1.0, 0.0);
// 			first = false;
// 		}
// 		else {
// 			cvRunningAvg(img_color, img_moving_avg, ((float)param_moving_avg_wt)/100.0, NULL);
// 		}

// 		cvConvertScale(img_moving_avg, img_temp, 1.0, 0);									// convert the moving avg to a format usable for diff
// 		cvAbsDiff(img_color, img_temp, img_diff);											// subtract current from moving average.
// 		cvCvtColor(img_diff, img_gray, CV_RGB2GRAY);										// convert image to gray
// 		cvThreshold(img_gray, img_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);	// convert image to black and white
// 		//cvThreshold(greyImage, greyImage, 70, 255, CV_THRESH_BINARY);	//Convert the image to black and white.
// 		//cvAdaptiveThreshold(img_gray, img_gray, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 0);

// 		// dilate and erode to reduce noise and join irregular blobs
// 		cvErode(img_gray, img_gray, 0, 2); 							// erode to remove noise
// 		cvDilate(img_gray, img_gray, 0, param_dilation_amt+2);		// dilate to join and fill blobs
// 		cvErode(img_gray, img_gray, 0, param_erosion_amt);			// erode again to get some of the original proportion back
// 		cvConvertScale(img_gray, img_contour, 1.0, 0.0);			// copy image to the contour image for contour detection

// 		// find the contours of the moving images in the frame.
// 		cvClearMemStorage(mem_store);
// 		CvSeq* contour = 0;
// 		cvFindContours(img_contour, mem_store, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

// 		// process each moving contour in the current frame...
// 		bool motion_detected = false;
// 		for (; contour != 0; contour = contour->h_next) {
// 			CvRect bnd_rect = cvBoundingRect(contour, 0);	// get a bounding rect around the moving object.

// 			// discard objects smaller than our expected object size
// 			int obj_size_pct = bnd_rect.width * bnd_rect.height * 100 / (sz_of_img.height * sz_of_img.width);
// 			if (obj_size_pct < param_min_obj_size) continue;

// 			// either draw the contours or motion detection marker
// 			if(6 == param_display_stage) {
// 				cvDrawContours(img_work, contour, CV_RGB(0,255,0), CV_RGB(0,255,0), 3, CV_FILLED);
// 			}
// 			else if(7 == param_display_stage) {
// 				CvPoint center;
// 				center.x = bnd_rect.x + bnd_rect.width/2;
// 				center.y = bnd_rect.y + bnd_rect.height/2;
// 				int rad = (bnd_rect.width < bnd_rect.height ? bnd_rect.width : bnd_rect.height)/2;

// 				while (rad > 0) {
// 					cvCircle(img_work, center, rad, CV_RGB(153,204,50), 1, CV_AA);
// 					rad -= 8;
// 				}
// 			}
// 			motion_detected = true;
// 		}

// 		if(is_action_mode) {
// 			if(motion_detected) {
// 				cvReleaseCapture(&inp_device);
// 				system(param_command_on_motion);
// 				inp_device = capture_input(argc, argv);
// 			}
// 		}
// 		else {
// 			char c = display_frame(motion_detected);
// 			if(27 == c) break;
// 			else if('s' == c) save_settings();
// 			else if('r' == c) load_settings();
// 		}
// 	}

// 	destroy(is_action_mode);

// 	return 0;
// }

