#include "Camera.h"

int main() {
    Camera camera;
    camera.init();
    cv::Mat frame;
    while (true) {
        camera.getImage(frame);

        cv::imshow("window0",frame);
        cv::waitKey(1);
    }


}