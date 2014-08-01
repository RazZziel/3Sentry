/** Basic OpenCV example for face detecion and tracking with CamShift.
  * First run Haar classifier face detection on every frame until finding face,
  * then switch over to CamShift using face region to track.
 **/
#include "trackface.h"
#include "camshifting.h"
#include <iostream>

#define OPENCV_DATA "/usr/share/opencv/haarcascades/"
const char *classifer = OPENCV_DATA "haarcascade_frontalface_default.xml";
//char *classifer = OPENCV_DATA "haarcascade_frontalface_alt.xml";
//char *classifer = OPENCV_DATA "haarcascade_frontalface_alt2.xml";
//char *classifer = OPENCV_DATA "haarcascade_frontalface_alt_tree.xml";
//char *classifer = OPENCV_DATA "haarcascade_eye.xml";
//char *classifer = OPENCV_DATA "haarcascade_upperbody.xml";
//char *classifer = OPENCV_DATA "haarcascade_lowerbody.xml";
//char *classifer = OPENCV_DATA "haarcascade_fullbody.xml";

//used by capture_video_frame, so we don't have to keep creating.
cv::Mat frame_curr;
cv::Mat frame_copy;

int trackface (int argc, char** argv) {
    if (argc != 2) print_help(argv[0]); //check usage
    //declare
    cv::CascadeClassifier* cascade = NULL;
    std::string window_name = "haar window";

    //initialize
    cascade = new cv::CascadeClassifier(classifer);
    //validate
    assert(cascade);

    //create window
    cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);

    cv::Rect* face_rect = 0;

    //video file?
    cv::VideoCapture* capture = NULL;
    int cam=0;
    if (sscanf(argv[1], "%d", &cam) == 1)
        capture = new cv::VideoCapture(cam);
    else
        capture = new cv::VideoCapture(argv[1]);

    assert(capture);

    cv::Mat *image = NULL;

    std::cout << "Detecting..." << std::endl;

    //run loop, exit on ESC
    while (1) {
        image = capture_video_frame(capture);
        if (!image) break;

        std::vector<cv::Rect> faces;
        detect_face(*image, cascade, faces);

        for (uint i=0; i<faces.size(); ++i)
        {
            std::cout << "Detected" << std::endl;

            face_rect = &faces.at(i);

#if 0
            //face detected, exit loop and track
            break;
#else
            cv::rectangle(*image,
                          *face_rect,
                          cv::Scalar(0,0,255),
                          3, CV_AA, 0);
        }
#endif

        cv::imshow(window_name, *image);  //display

        //exit program on ESC
        if ((char)27 == cv::waitKey(10)) {
            capture->release();
            cleanup(window_name, cascade);
            exit(0);
        }
    }
    
    std::cout << "Tracking..." << std::endl;

    TrackedObj* tracked_obj = create_tracked_object(image, face_rect);
    cv::RotatedRect face_box; //area to draw
    
    //track detected face with camshift
    while (1) {
        //get next video frame
        image = capture_video_frame(capture);
        if (!image) break;

        //track the face in the new frame
        face_box = camshift_track_face(image, tracked_obj);

        //outline face ellipse
        cv::ellipse(*image,
                    face_box,
                    cv::Scalar(0,0,255),
                    3, CV_AA);

        cv::imshow(window_name, *image); //display

        //exit on ESC
        if ((char)27 == cv::waitKey(10)) break;
    }

    std::cout << "Finished" << std::endl;

    //free memory
    destroy_tracked_object(tracked_obj);
    capture->release();

    //release resources and exit
    cleanup(window_name, cascade);

    return 0;
}


/* Given an image and a classider, detect and return region. */
void detect_face (const cv::Mat& image,
                  cv::CascadeClassifier* cascade,
                  std::vector<cv::Rect> &faces) {

    //get a sequence of faces in image
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;
    cascade->detectMultiScale(image,
                              faces,
                              rejectLevels,
                              levelWeights,
                              1.1,                       //increase search scale by 10% each pass
                              6,                         //require 6 neighbors
                              CV_HAAR_DO_CANNY_PRUNING,  //skip regions unlikely to contain a face
                              cv::Size(100, 100),            //use default face size from xml
                              cv::Size(500, 500));

}

/* Capture frame and return a copy so not to write to source. */
cv::Mat* capture_video_frame (cv::VideoCapture* capture) {
    //capture the next frame
    capture->read(frame_curr);
    frame_curr.copyTo(frame_copy);

    //make copy of frame so we don't write to src
    frame_curr.copyTo(frame_copy);
#if 0
    frame_copy->origin = frame_curr->origin;

    //invert if needed, 1 means the image is inverted
    if (frame_copy->origin == 1) {
        cvFlip(frame_copy, 0, 0);
        frame_copy->origin = 0;
    }
#endif

    return &frame_copy;
}

void cleanup (std::string name,
              cv::CascadeClassifier* cascade) {
    //cleanup and release resources
    cv::destroyWindow(name);
    delete cascade;
}

void print_help (char* name) {
    std::cout << "Usage: " << name << " [file/cam]" << std::endl;
    exit(-1);
}
