#include "auto_aim.hpp"

using namespace cv;
using namespace std;

//double calc_distance(Point2f p1, Point2f p2) {
//    return pow(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2), 0.5);
//}

cv::Rect ArmorDetector::GetRoi(const cv::Mat &img) {
    Size img_size = img.size();
    Rect rect_tmp = last_target_;
    Rect rect_roi;
    if (rect_tmp.x == 0 || rect_tmp.y == 0
        || rect_tmp.width == 0 || rect_tmp.height == 0
        || lost_count >= 15) {
        last_target_ = Rect(0, 0, img_size.width, img_size.height);
        rect_roi = Rect(0, 0, img_size.width, img_size.height);
        return rect_roi;
    } else {
        float scale = 2;
        if (lost_count <= 35)
            scale = 2.3;
        else if (lost_count <= 60)
            scale = 3;
        else if (lost_count <= 100)
            scale = 3.5;

        int w = int(rect_tmp.width * scale);
        int h = int(rect_tmp.height * scale);
        int x = int(rect_tmp.x - (w - rect_tmp.width) * 0.5f);
        int y = int(rect_tmp.y - (h - rect_tmp.height) * 0.5f);

        rect_roi = Rect(x, y, w, h);

        if (!makeRectSafe(rect_roi, img_size)) {
            rect_roi = Rect(0, 0, img_size.width, img_size.height);
        }
    }
    return rect_roi;
}

bool ArmorDetector::DetectArmor(cv::Mat &img, const cv::Rect &roi) {
    Mat roi_image = img(roi);
    Point2f offset_roi_point(roi.x, roi.y);
    vector<Mat> BGR_channels;
    vector<LED_bar> LED_bars;

    bool found_flag = false;
    Mat binary_brightness_img, binary_color_img, gray, debug_img, color_result_img;
    debug_img = img.clone();

    cvtColor(roi_image, gray, COLOR_BGR2GRAY);
    split(roi_image, BGR_channels);
    if (1 == 1) // opposite red
    {
        subtract(BGR_channels[2], BGR_channels[0], color_result_img);
    } else {
        subtract(BGR_channels[0], BGR_channels[2], color_result_img);
    }
    threshold(gray, binary_brightness_img, gray_th_, 255, THRESH_BINARY);
    threshold(color_result_img, binary_color_img, color_th_, 255, THRESH_BINARY);

    if (debug_) {
        imshow("binary_brightness_img", binary_brightness_img);
        imshow("binary_color_img", binary_color_img);
        //for testing

    }
    vector<vector<Point> > contours_light;
    vector<vector<Point> > contours_brightness;

    findContours(binary_color_img, contours_light, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    findContours(binary_brightness_img, contours_brightness, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (unsigned int i = 0; i < contours_brightness.size(); i++) {
        drawContours( debug_img, contours_brightness, (int)i, Scalar(255, 0, 255), 2, LINE_8 );
    }
    for (unsigned int j = 0; j < contours_light.size(); j++) {
        drawContours( debug_img, contours_light, (int)j, Scalar(0, 0, 255), 2, LINE_8);
    }

    if (contours_brightness.size() < 2 || contours_light.size() < 2 || contours_brightness.size() > 10 ||
        contours_light.size() > 10) {
        return found_flag;
    }

    for (unsigned int i = 0; i < contours_brightness.size(); i++) {
        double area = contourArea(contours_brightness[i]);
        if (area > 1e5) {
            continue;
        }
        for (unsigned int j = 0; j < contours_light.size(); j++) {

            if (pointPolygonTest(contours_light[j], contours_brightness[i][0], false) >= 0.0) {
                double length = arcLength(contours_brightness[i], true); // 灯条周长
                if (length > 20 && length < 4000) {
                    RotatedRect RRect = fitEllipse(contours_brightness[i]);
                    //RotatedRect RRect = minAreaRect(contours_brightness[i]);
                    auto light_aspect_ratio =
                            std::max(RRect.size.width, RRect.size.height) /
                            std::min(RRect.size.width, RRect.size.height);

                    if (RRect.angle > 90.0f)
                        RRect.angle = RRect.angle - 180.0f;

                    if (fabs(RRect.angle) < 30) {
                        if (debug_) {
                            char temp[20];
                            sprintf(temp, "%0.2f", light_aspect_ratio);
                            putText(debug_img, temp, RRect.center + Point2f(0, -40) + offset_roi_point,
                                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
                            Point2f rect_point[4];
                            RRect.points(rect_point);
                            for (int i = 0; i < 4; i++) {
                                line(debug_img, rect_point[i] + offset_roi_point, rect_point[(i + 1) % 4] + offset_roi_point, Scalar(255, 0, 255), 1);
                            }
                            char temp1[20];
                            sprintf(temp1, "%0.2f", RRect.angle);
                            putText(debug_img, temp1, RRect.center + Point2f(0, -10) + offset_roi_point,
                                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
                        }
                        LED_bar r(RRect);
                        LED_bars.emplace_back(r);

                    }
                }
                break;
            }
        }
    }

    //==========================================possible armor=========================================
    for (size_t i = 0; i < LED_bars.size(); i++) {
        for (size_t j = i + 1; j < LED_bars.size(); j++) {
            armor temp_armor(LED_bars.at(i), LED_bars.at(j));
            if (temp_armor.error_angle < 7.0f) {
                if (temp_armor.is_suitable_size()) {
                    //temp_armor.draw_rect(debug_img,offset_roi_point);
                    if (temp_armor.get_average_intensity(gray) < 70) {
                        temp_armor.max_match(LED_bars, i, j);

                    }
                }
            }
        }
    }
    //====================================find final armors============================================
    vector<armor> final_armor_list;

    for (size_t i = 0; i < LED_bars.size(); i++) {
        if (LED_bars.at(i).matched) {
            LED_bars.at(LED_bars.at(i).match_index).matched = false; //clear another matching flag
            armor arm_tmp(LED_bars.at(i), LED_bars.at(LED_bars.at(i).match_index));
            //arm_tmp.draw_rect(debug_img, offset_roi_point);
            final_armor_list.push_back(arm_tmp);
        }
    }
    //printf("final armor size %zu\n", final_armor_list.size());


    float dist = 1e8;

    armor target;
    Point2f roi_center(roi.width / 2, roi.height / 2);
    float dx, dy;
    for (auto &i : final_armor_list) {

        dx = pow((i.center.x - roi_center.x), 2.0f);
        dy = pow((i.center.y - roi_center.y), 2.0f);

        if (dx + dy < dist) {
            target = i;
            dist = dx + dy;
        }
        if (debug_) {
            i.draw_rect(debug_img, offset_roi_point);
        }

        found_flag = true;
    }
    if (ROI_enable_) {
        rectangle(debug_img, roi, Scalar(255, 0, 255), 1);
    }
    if (found_flag) {

        detect_count++;
        Point2f point_tmp[4];
        Point2f point_2d[4];

        // 左右灯条分类，本别提取装甲板四个外角点
        RotatedRect R, L;
        if (target.led_bars[0].rect.center.x > target.led_bars[1].rect.center.x) {
            R = target.led_bars[0].rect;
            L = target.led_bars[1].rect;
        } else {
            R = target.led_bars[1].rect;
            L = target.led_bars[0].rect;
        }
        L.points(point_tmp);
        point_2d[0] = point_tmp[1];
        point_2d[3] = point_tmp[0];
        R.points(point_tmp);
        point_2d[1] = point_tmp[2];
        point_2d[2] = point_tmp[3];
        vector<Point2f> points_roi_tmp;
        final_armor_2Dpoints.clear();
        for (int i = 0; i < 4; i++) {
            points_roi_tmp.push_back(point_2d[i] + offset_roi_point);
            final_armor_2Dpoints.push_back(point_2d[i] + offset_roi_point);
            circle(debug_img, final_armor_2Dpoints.at(i), 5, Scalar(255, 255, 255), -1);
            circle(debug_img, final_armor_2Dpoints.at(i), 3, Scalar(i * 50, i * 50, 255), -1);
        }

        float armor_h = target.rect.height;
        float armor_w = target.rect.width;
        is_small = armor_w / armor_h < 3.3f;

        //get the new target
        last_target_ = boundingRect(points_roi_tmp);
        if(debug_) {
            rectangle(debug_img, last_target_, Scalar(255, 255, 255), 1);
        }
        
        lost_count = 0;
    } else { // not find
        lost_count++;
    }


    if (debug_) {
        imshow("debug_img", debug_img);
        waitKey(1);
    }
    
    return found_flag;
}

void ArmorDetector::execute(cv::Mat &cameraFrame) {
    
    Rect roi;
    if (this->ROI_enable_) {
        roi = GetRoi(cameraFrame);
    }
    else {
        Size img_size = cameraFrame.size();
        roi = Rect(0, 0, img_size.width, img_size.height);
    }

    Point3f target_3d = {0, 0, 0};

    //for testing
    // OFFSET_YAW = (OFFSET_INT_YAW - 1800);
    // OFFSET_PITCH = (OFFSET_INT_PITCH - 1800);
    if (DetectArmor(cameraFrame, roi)) {


        //target_3d = getPose();

        // int pitch = int((atan2(target_3d.y - 80, target_3d.z) + (float) (OFFSET_PITCH * CV_PI / 1800)) * 0.6 * 10000);
        //int pitch = 15000;
        // int yaw = int((-atan2(target_3d.x, target_3d.z) + (float) (OFFSET_YAW * CV_PI / 1800)) * 0.4 * 10000);
        // if (debug_) {
        //     RCLCPP_INFO(nh_->get_logger(),"x:%f y:%f z:%f\n", target_3d.x, target_3d.y, target_3d.z);
            
        //     RCLCPP_INFO(nh_->get_logger(),"yaw: %d, pitch: %d\n", yaw, pitch);
        // }
    }
}