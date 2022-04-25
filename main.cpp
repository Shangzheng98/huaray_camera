#include "Camera.h"
#include <iostream>

int main() {
    Camera camera;
    camera.init();
    cv::Mat frame;
    while (true) {
        camera.getImage(frame);
        cv::imshow("window0.png",frame);
        cv::waitKey(10);
    }


}
