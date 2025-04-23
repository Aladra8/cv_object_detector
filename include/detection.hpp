// // detection.hpp
// #ifndef DETECTION_HPP
// #define DETECTION_HPP

// #include <opencv2/opencv.hpp>
// #include <string>
// #include <vector>

// struct DetectionResult
// {
//     std::string object_name;
//     cv::Rect bounding_box;
//     float score;
// };

// std::vector<cv::Mat> loadTemplateImages(const std::string &path);
// std::vector<cv::Mat> loadTemplateMasks(const std::string &path);
// std::vector<DetectionResult> detectObjectsORB(
//     const cv::Mat &scene,
//     const std::vector<cv::Mat> &templates,
//     const std::vector<cv::Mat> &masks,
//     const std::string &object_name);

// void drawDetections(cv::Mat &image, const std::vector<DetectionResult> &results);
// void saveBoundingBoxes(const std::string &filename, const std::vector<DetectionResult> &results);

// #endif // DETECTION_HPP

#ifndef DETECTION_HPP
#define DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct DetectionResult {
    std::string object_name;
    cv::Rect bounding_box;
    float score; // Lower is better (average match distance)
};

std::vector<cv::Mat> loadTemplateImages(const std::string& path);
std::vector<cv::Mat> loadTemplateMasks(const std::string& path);
std::vector<DetectionResult> detectObjectsORB(
    const cv::Mat& scene,
    const std::vector<cv::Mat>& templates,
    const std::vector<cv::Mat>& masks,
    const std::string& object_name
);

void drawDetections(cv::Mat& image, const std::vector<DetectionResult>& results);
void saveBoundingBoxes(const std::string& filename, const std::vector<DetectionResult>& results);

#endif // DETECTION_HPP
