#include "Camera.h"
#include "auto_aim.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
int main() {
    Camera camera;
    camera.init();
    cv::Mat frame;
    ArmorDetector armorDetector;
    if(armorDetector.debug_) {
        cv::namedWindow("binary_brightness_img", 1);
        cv::namedWindow("binary_color_img", 1);
        cv::createTrackbar("Color", "binary_color_img", &armorDetector.color_th_, 255);
        cv::createTrackbar("Gray", "binary_brightness_img",&armorDetector.gray_th_,255);
    }
    while (true) {
        camera.getImage(frame);
        armorDetector.execute(frame);


    }


}