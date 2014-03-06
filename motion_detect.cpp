/*
 * motion_detect.cpp
 * To accompany instructions at:
 *      http://sidekick.windforwings.com/2012/12/opencv-motion-detection-based-action.html
 *
 *  Created on: Dec 02, 2012
 *	Author: tan
 *
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <libconfig.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

const char 		*PROG_NAME;
const char 		*WINDOW_NAME = "Motion Detect";
IplImage		*img_read, *img_smooth, *img_color, *img_diff, *img_temp, *img_edge_color;
IplImage		*img_work, *img_gray, *img_edge_gray, *img_contour, *img_moving_avg;
CvCapture 		*inp_device;
CvMemStorage	*mem_store;
CvSize 			sz_of_img;
int 			depth_of_img, channels_of_img;

int 			param_display_stage = 7;
int 			param_moving_avg_wt = 2;
int 			param_detect_threshold = 20;
int 			param_min_obj_size = 2;
int 			param_dilation_amt = 30;
int 			param_erosion_amt = 10;
int 			param_brightness_factor = 50;
int 			param_contrast_factor = 0;
const char 		*param_command_on_motion;
bool 			param_is_file_mode = false;
int 			param_proc_delay = 1000;
const int 		MAX_PROC_DELAY = 1000;
const int 		MIN_PROC_DELAY = 100;


/*****************************************************
 * save and load settings
 *****************************************************/
void save_settings() {
	config_t config;
	config_init(&config);

	config_setting_t *cfg_root = config_root_setting(&config);
	config_setting_set_int(config_setting_add(cfg_root, "display_stage", 		CONFIG_TYPE_INT), param_display_stage);
	config_setting_set_int(config_setting_add(cfg_root, "moving_avg_wt", 		CONFIG_TYPE_INT), param_moving_avg_wt);
	config_setting_set_int(config_setting_add(cfg_root, "detect_threshold", 	CONFIG_TYPE_INT), param_detect_threshold);
	config_setting_set_int(config_setting_add(cfg_root, "min_obj_size", 		CONFIG_TYPE_INT), param_min_obj_size);
	config_setting_set_int(config_setting_add(cfg_root, "dilation_amt", 		CONFIG_TYPE_INT), param_dilation_amt);
	config_setting_set_int(config_setting_add(cfg_root, "erosion_amt", 			CONFIG_TYPE_INT), param_erosion_amt);
	config_setting_set_int(config_setting_add(cfg_root, "brightness_factor", 	CONFIG_TYPE_INT), param_brightness_factor);
	config_setting_set_int(config_setting_add(cfg_root, "contrast_factor", 		CONFIG_TYPE_INT), param_contrast_factor);
	config_setting_set_string(config_setting_add(cfg_root, "command_on_motion", CONFIG_TYPE_STRING), param_command_on_motion);

	config_write_file(&config, "motion_detect.cfg");
	config_destroy(&config);
}

void load_settings() {
	config_t config;
	config_init(&config);
	if(CONFIG_TRUE != config_read_file(&config, "motion_detect.cfg")) return;

	config_setting_t *cfg_root = config_root_setting(&config);
	param_display_stage 		= config_setting_get_int(config_setting_get_member(cfg_root, "display_stage"));
	param_moving_avg_wt 		= config_setting_get_int(config_setting_get_member(cfg_root, "moving_avg_wt"));
	param_detect_threshold 		= config_setting_get_int(config_setting_get_member(cfg_root, "detect_threshold"));
	param_min_obj_size 			= config_setting_get_int(config_setting_get_member(cfg_root, "min_obj_size"));
	param_dilation_amt 			= config_setting_get_int(config_setting_get_member(cfg_root, "dilation_amt"));
	param_erosion_amt 			= config_setting_get_int(config_setting_get_member(cfg_root, "erosion_amt"));
	param_brightness_factor 	= config_setting_get_int(config_setting_get_member(cfg_root, "brightness_factor"));
	param_contrast_factor 		= config_setting_get_int(config_setting_get_member(cfg_root, "contrast_factor"));

	config_setting_t *on_motion_setting = config_setting_get_member(cfg_root, "command_on_motion");
	if(NULL != on_motion_setting) param_command_on_motion = config_setting_get_string(on_motion_setting);
	else param_command_on_motion = NULL;

	if(NULL == param_command_on_motion) param_command_on_motion = "echo \"Motion detected!\"; sleep 10;";
	else param_command_on_motion = strdup(param_command_on_motion);			// make a copy so that we can destroy the config structure

	config_destroy(&config);
}

/*****************************************************
 * print program usage and exit if instructed to
 *****************************************************/
void print_usage(int do_exit) {
	printf("Usage %s [cam <camera #> | file <video file>] [act]\n", PROG_NAME);
	if(do_exit) exit(1);
}

/*****************************************************
 * print what optimization libraries are available
 *****************************************************/
void print_lib_version() {
	const char* libraries;
	const char* modules;
	cvGetModuleInfo(NULL, &libraries, &modules);
	printf("Libraries: %s\nModules: %s\n", libraries, modules);
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

/*****************************************************
 * create capture input either from file or camera
 *****************************************************/
CvCapture *capture_input(int argc, char **argv) {
	PROG_NAME = argv[0];
	if (argc < 2) print_usage(1);

	CvCapture *input;
	if(0 == strcasecmp("file", argv[1])) {
		input = cvCreateFileCapture(argv[2]);
		param_is_file_mode = true;
	} else if(0 == strcasecmp("cam", argv[1])) {
		input = cvCreateCameraCapture(atoi(argv[2]));
	}
	else {
		print_usage(1);
		exit(1);
	}

	if (!input) {
		printf("Can't open %s %s\n", argv[1], argv[2]);
		exit(1);
	}
	return input;
}

/*****************************************************
 * init and destroy globals
 *****************************************************/
void init(bool is_silent_mode) {
	if(!is_silent_mode) {
		cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);			//Create a new window.
		cvCreateTrackbar("Process Stage", WINDOW_NAME, &param_display_stage, 7, NULL);
		cvCreateTrackbar("Moving Avg Wt", WINDOW_NAME, &param_moving_avg_wt, 50, NULL);
		cvCreateTrackbar("Threshold", WINDOW_NAME, &param_detect_threshold, 100, NULL);
		cvCreateTrackbar("Min Object Size", WINDOW_NAME, &param_min_obj_size, 25, NULL);
		cvCreateTrackbar("Dilation", WINDOW_NAME, &param_dilation_amt, 50, NULL);
		cvCreateTrackbar("Erosion", WINDOW_NAME, &param_erosion_amt, 50, NULL);
		cvCreateTrackbar("Brightness", WINDOW_NAME, &param_brightness_factor, 100, NULL);
		cvCreateTrackbar("Contrast", WINDOW_NAME, &param_contrast_factor, 10, NULL);
	}

	img_work			= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_color			= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_diff			= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_temp			= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_edge_color		= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_smooth			= cvCreateImage(sz_of_img, depth_of_img, channels_of_img);
	img_gray 			= cvCreateImage(sz_of_img, IPL_DEPTH_8U, 1);
	img_edge_gray		= cvCreateImage(sz_of_img, IPL_DEPTH_8U, 1);
	img_contour 		= cvCreateImage(sz_of_img, IPL_DEPTH_8U, 1);
	img_moving_avg 		= cvCreateImage(sz_of_img, IPL_DEPTH_32F, 3);

	mem_store 			= cvCreateMemStorage(0);
}

void destroy(bool is_silent_mode) {
	cvReleaseMemStorage(&mem_store);
	cvReleaseImage(&img_work);
	cvReleaseImage(&img_smooth);
	cvReleaseImage(&img_temp);
	cvReleaseImage(&img_color);
	cvReleaseImage(&img_diff);
	cvReleaseImage(&img_gray);
	cvReleaseImage(&img_contour);
	cvReleaseImage(&img_moving_avg);
	cvReleaseImage(&img_edge_color);
	cvReleaseImage(&img_edge_gray);
	if(!is_silent_mode) cvDestroyWindow(WINDOW_NAME);
	cvReleaseCapture(&inp_device);
}


/*****************************************************
 * display the frames in a window
 *****************************************************/
char display_frame(bool motion_detected) {
	if(motion_detected) 							param_proc_delay = MIN_PROC_DELAY;
	else if(param_proc_delay < MAX_PROC_DELAY)		param_proc_delay += 10;

	if(1 == param_display_stage) cvShowImage(WINDOW_NAME, img_color);
	else if((0 == param_display_stage) || (6 == param_display_stage) || (7 == param_display_stage)) {
		cvShowImage(WINDOW_NAME, img_work);
	}
	else if(2 == param_display_stage) 	cvShowImage(WINDOW_NAME, img_edge_gray);
	else if(3 == param_display_stage)	cvShowImage(WINDOW_NAME, img_diff);
	else if(4 == param_display_stage) 	cvShowImage(WINDOW_NAME, img_gray);
	else if(5 == param_display_stage) 	cvShowImage(WINDOW_NAME, img_contour);

	// break if escape was pressed
	return cvWaitKey(param_is_file_mode ? 10 : param_proc_delay);
}


/*****************************************************
 * main loop
 *****************************************************/
int main(int argc, char* argv[]) {
	inp_device = capture_input(argc, argv);
	bool		is_action_mode = ((argc > 3) && (0 == strcmp(argv[3], "act")));
	print_lib_version();
	load_settings();

	get_approp_size(inp_device, sz_of_img, depth_of_img, channels_of_img);
	init(is_action_mode);

	bool first = true;

	img_read = NULL;
	for (img_read = cvQueryFrame(inp_device); img_read; img_read = cvQueryFrame(inp_device)) {
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

		cvConvertScale(img_moving_avg, img_temp, 1.0, 0);									// convert the moving avg to a format usable for diff
		cvAbsDiff(img_color, img_temp, img_diff);											// subtract current from moving average.
		cvCvtColor(img_diff, img_gray, CV_RGB2GRAY);										// convert image to gray
		cvThreshold(img_gray, img_gray, 25+param_detect_threshold, 255, CV_THRESH_BINARY);	// convert image to black and white
		//cvThreshold(greyImage, greyImage, 70, 255, CV_THRESH_BINARY);	//Convert the image to black and white.
		//cvAdaptiveThreshold(img_gray, img_gray, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 0);

		// dilate and erode to reduce noise and join irregular blobs
		cvErode(img_gray, img_gray, 0, 2); 							// erode to remove noise
		cvDilate(img_gray, img_gray, 0, param_dilation_amt+2);		// dilate to join and fill blobs
		cvErode(img_gray, img_gray, 0, param_erosion_amt);			// erode again to get some of the original proportion back
		cvConvertScale(img_gray, img_contour, 1.0, 0.0);			// copy image to the contour image for contour detection

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
	}

	destroy(is_action_mode);

	return 0;
}
