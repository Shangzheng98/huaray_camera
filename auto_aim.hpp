#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include "armor.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>


using std::placeholders::_1;
class ArmorDetector {
public:
    ArmorDetector() {
        this->debug_ = true;
    }

    ~ArmorDetector() = default;

    void execute(cv::Mat &cameraFrame);

    bool DetectArmor(cv::Mat &img, const cv::Rect& roi);

    void subscription_execute();

    int color_th_ = 41;
    int gray_th_ = 26;
    int OFFSET_INT_YAW = 1800;
    int OFFSET_INT_PITCH = 1800;
    int OFFSET_YAW{};
    int OFFSET_PITCH{};

    bool debug_{};
private:
    bool makeRectSafe(cv::Rect &rect, const cv::Size &size) {
        if (rect.x < 0)
            rect.x = 0;
        if (rect.x + rect.width > size.width)
            rect.width = size.width - rect.x;
        if (rect.y < 0)
            rect.y = 0;
        if (rect.y + rect.height > size.height)
            rect.height = size.height - rect.y;
        return !(rect.width <= 0 || rect.height <= 0);
    }
    cv::Rect GetRoi(const cv::Mat &img);
    bool ROI_enable_{};



    cv::Rect last_target_;
    int lost_count = 0;
    int detect_count = 0;

    uint8_t color_ = 1;
    uint8_t level_{};
    uint8_t type_{};
    std::vector<cv::Point2f> final_armor_2Dpoints;


    // the armor type
    bool is_small = false;

    // solvePnP
    std::vector<cv::Point3f> small_real_armor_points;
    std::vector<cv::Point3f> big_real_armor_points;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    //debug

    
};


