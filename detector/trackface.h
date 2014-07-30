#ifndef INC_OPENCV
#define INC_OPENCV
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

cv::Mat* capture_video_frame (cv::VideoCapture*);
void detect_face (const cv::Mat&, cv::CascadeClassifier*, std::vector<cv::Rect> &);
void cleanup (std::string name, cv::CascadeClassifier* cascade);
void print_help (char*);

//used by capture_video_frame, so we don't have to keep creating.
cv::Mat frame_curr;
cv::Mat frame_copy;
