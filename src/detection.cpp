
#include "../include/detection.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

// Load color template images from the given model directory
std::vector<cv::Mat> loadTemplateImages(const std::string &model_dir)
{
    std::vector<cv::Mat> images;
    for (const auto &file : fs::directory_iterator(model_dir))
    {
        if (file.path().extension() == ".png" && file.path().string().find("color") != std::string::npos)
        {
            images.push_back(cv::imread(file.path().string(), cv::IMREAD_COLOR));
        }
    }
    return images;
}

// Load grayscale masks corresponding to templates
std::vector<cv::Mat> loadTemplateMasks(const std::string &model_dir)
{
    std::vector<cv::Mat> masks;
    for (const auto &file : fs::directory_iterator(model_dir))
    {
        if (file.path().extension() == ".png" && file.path().string().find("mask") != std::string::npos)
        {
            masks.push_back(cv::imread(file.path().string(), cv::IMREAD_GRAYSCALE));
        }
    }
    return masks;
}

// Perform object detection using SIFT feature matching
std::vector<DetectionResult> detectObjectsSIFT(const cv::Mat &scene, const std::vector<cv::Mat> &templates, const std::vector<cv::Mat> &masks, const std::string &object_name)
{
    std::vector<DetectionResult> detections;
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    std::vector<cv::KeyPoint> scene_kp;
    cv::Mat scene_desc;
    sift->detectAndCompute(scene, cv::noArray(), scene_kp, scene_desc);

    if (scene_desc.empty())
    {
        std::cout << "[DEBUG] Scene descriptors empty. Skipping.\n";
        return {};
    }

    // Process each template
    for (size_t i = 0; i < templates.size(); ++i)
    {
        const auto &templ = templates[i];
        const auto &mask = masks[i];

        std::vector<cv::KeyPoint> temp_kp;
        cv::Mat temp_desc;
        sift->detectAndCompute(templ, mask, temp_kp, temp_desc);

        if (temp_desc.empty())
            continue;

        cv::BFMatcher matcher(cv::NORM_L2);
        std::vector<std::vector<cv::DMatch>> knn_matches;
        matcher.knnMatch(temp_desc, scene_desc, knn_matches, 2);

        // Apply Lowe's ratio test
        std::vector<cv::DMatch> good_matches;
        for (const auto &m : knn_matches)
        {
            if (m.size() == 2 && m[0].distance < 0.75f * m[1].distance)
            {
                good_matches.push_back(m[0]);
            }
        }

        if (good_matches.size() < 8)
            continue;

        // Find Homography between template and scene
        std::vector<cv::Point2f> temp_pts, scene_pts;
        for (const auto &match : good_matches)
        {
            temp_pts.push_back(temp_kp[match.queryIdx].pt);
            scene_pts.push_back(scene_kp[match.trainIdx].pt);
        }

        cv::Mat H = cv::findHomography(temp_pts, scene_pts, cv::RANSAC);

        if (H.empty())
            continue;

        // Estimate object bounding box
        std::vector<cv::Point2f> corners = {
            {0, 0}, {(float)templ.cols, 0}, {(float)templ.cols, (float)templ.rows}, {0, (float)templ.rows}};
        std::vector<cv::Point2f> projected;
        cv::perspectiveTransform(corners, projected, H);

        cv::Rect bbox = cv::boundingRect(projected);

        if (bbox.area() < 500 || bbox.x < 0 || bbox.y < 0 || bbox.x + bbox.width > scene.cols || bbox.y + bbox.height > scene.rows)
            continue;

        // Average descriptor distance for match quality
        double avg_dist = 0.0;
        for (const auto &m : good_matches)
            avg_dist += m.distance;
        avg_dist /= good_matches.size();

        detections.push_back(DetectionResult{object_name, bbox, static_cast<float>(avg_dist)});

        std::cout << "[DEBUG] Object: " << object_name << " - Matches: " << good_matches.size() << " - AvgDist: " << avg_dist << std::endl;
    }

    // Sort detections by best match score
    std::sort(detections.begin(), detections.end(), [](const DetectionResult &a, const DetectionResult &b)
              { return a.score < b.score; });

    return detections;
}

// Draw detected bounding boxes on the image
void drawDetections(cv::Mat &image, const std::vector<DetectionResult> &detections)
{
    for (const auto &det : detections)
    {
        cv::rectangle(image, det.bounding_box, cv::Scalar(0, 255, 0), 2);
        cv::putText(image, det.object_name, det.bounding_box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    }
}

// Save detected bounding boxes to a text file
void saveBoundingBoxes(const std::string &filename, const std::vector<DetectionResult> &detections)
{
    std::ofstream ofs(filename);
    for (const auto &det : detections)
    {
        ofs << det.object_name << " "
            << det.bounding_box.x << " "
            << det.bounding_box.y << " "
            << det.bounding_box.x + det.bounding_box.width << " "
            << det.bounding_box.y + det.bounding_box.height << "\n";
    }
    ofs.close();
}
