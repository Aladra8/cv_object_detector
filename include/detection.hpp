
#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct DetectionResult
{
    std::string object_name;
    cv::Rect bounding_box;
    float score;
};

std::vector<cv::Mat> loadTemplateImages(const std::string& model_dir);
std::vector<cv::Mat> loadTemplateMasks(const std::string& model_dir);

std::vector<DetectionResult> detectObjectsSIFT(
    const cv::Mat& scene,
    const std::vector<cv::Mat>& templates,
    const std::vector<cv::Mat>& masks,
    const std::string& object_name
);

void drawDetections(cv::Mat& image, const std::vector<DetectionResult>& detections);
void saveBoundingBoxes(const std::string& filename, const std::vector<DetectionResult>& detections);
