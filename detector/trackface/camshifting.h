#ifndef INC_OPENCV
#define INC_OPENCV
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

#ifndef INC_CAMSHIFTING
#define INC_CAMSHIFTING

typedef struct {
  cv::Mat* hsv;     //input image converted to HSV
  cv::Mat* hue;     //hue channel of HSV image
  cv::Mat* mask;    //image for masking pixels
  cv::Mat* prob;    //face probability estimates for each pixel

  cv::Mat* hist; //histogram of hue in original face image

  cv::Rect prev_rect;  //location of face in previous frame
  cv::RotatedRect curr_box;  //current face location estimate
} TrackedObj;

TrackedObj* create_tracked_object (cv::Mat *image, cv::Rect* face_rect);
void destroy_tracked_object (TrackedObj* tracked_obj);
cv::RotatedRect camshift_track_face (cv::Mat* image, TrackedObj* imgs);
void update_hue_image (const cv::Mat* image, TrackedObj* imgs);

#endif
