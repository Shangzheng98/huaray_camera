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
        float x, y, z=0;
        float small_width = 140, small_height = 60, big_width = 230, big_height= 60;


        this->debug_ = true;
        x = -small_width / 2;
        y = small_height / 2;
        small_real_armor_points.emplace_back(x, y, z);
        x = small_width / 2;
        y = small_height / 2;
        small_real_armor_points.emplace_back(x, y, z);
        x = small_width / 2;
        y = -small_height / 2;
        small_real_armor_points.emplace_back(x, y, z);
        x = -small_width / 2;
        y = -small_height / 2;
        small_real_armor_points.emplace_back(x, y, z);

        //**********************************************************************//
        x = -big_width / 2;
        y = big_height / 2;
        big_real_armor_points.emplace_back(x, y, z);
        x = big_width / 2;
        y = big_height / 2;

        big_real_armor_points.emplace_back(x, y, z);
        x = big_width / 2;
        y = -big_height / 2;

        big_real_armor_points.emplace_back(x, y, z);
        x = -big_width / 2;
        y = -big_height / 2;

        big_real_armor_points.emplace_back(x, y, z);
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
    bool ROI_enable_ = true;



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
    cv::Mat cameraMatrix= (cv::Mat_<double>(3, 3) << 2082.375442362139, 0, 593.5048651101249,
            0.0000000000000000, 2104.836563830438, 560.7595695330034,
            0.0000000000000000, 0.0000000000000000, 1.0000000000000000);
    cv::Mat distCoeffs = (cv::Mat_<double>(1, 5)
            << -0.01106433971492507, -2.138295206151652, 0.005371540669007034, 0.001804155139176244, 28.83014861596617);

    //debug

    cv::Point3f getPose() {
        cv::Mat rvec;
        cv::Mat tvec;
        if (is_small) {
            solvePnP(small_real_armor_points, final_armor_2Dpoints, cameraMatrix, distCoeffs, rvec, tvec, false,
                     cv::SOLVEPNP_ITERATIVE);
        } else {
            solvePnP(big_real_armor_points, final_armor_2Dpoints, cameraMatrix, distCoeffs, rvec, tvec, false,
                     cv::SOLVEPNP_ITERATIVE);
        }

        return  cv::Point3f(tvec);
    }
};


