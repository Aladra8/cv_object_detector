// // detection.cpp
// #include "detection.hpp"
// #include <experimental/filesystem>
// #include <fstream>
// #include <algorithm>

// namespace fs = std::experimental::filesystem;

// std::vector<cv::Mat> loadTemplateImages(const std::string &path)
// {
//     std::vector<cv::Mat> templates;
//     for (const auto &file : fs::directory_iterator(path))
//     {
//         if (file.path().extension() == ".png" && file.path().string().find("color") != std::string::npos)
//         {
//             templates.push_back(cv::imread(file.path().string(), cv::IMREAD_COLOR));
//         }
//     }
//     return templates;
// }

// std::vector<cv::Mat> loadTemplateMasks(const std::string &path)
// {
//     std::vector<cv::Mat> masks;
//     for (const auto &file : fs::directory_iterator(path))
//     {
//         if (file.path().extension() == ".png" && file.path().string().find("mask") != std::string::npos)
//         {
//             masks.push_back(cv::imread(file.path().string(), cv::IMREAD_GRAYSCALE));
//         }
//     }
//     return masks;
// }

// std::vector<DetectionResult> detectObjectsORB(
//     const cv::Mat &scene,
//     const std::vector<cv::Mat> &templates,
//     const std::vector<cv::Mat> &masks,
//     const std::string &object_name)
// {
//     std::vector<DetectionResult> raw_results;
//     cv::Ptr<cv::ORB> orb = cv::ORB::create();
//     std::vector<cv::KeyPoint> scene_kp;
//     cv::Mat scene_desc;
//     orb->detectAndCompute(scene, cv::noArray(), scene_kp, scene_desc);

//     for (size_t i = 0; i < templates.size(); ++i)
//     {
//         const auto &templ = templates[i];
//         const auto &mask = masks[i];
//         std::vector<cv::KeyPoint> kp;
//         cv::Mat desc;
//         orb->detectAndCompute(templ, cv::noArray(), kp, desc);

//         if (desc.empty() || scene_desc.empty())
//             continue;

//         std::vector<cv::DMatch> matches;
//         cv::BFMatcher matcher(cv::NORM_HAMMING);
//         matcher.match(desc, scene_desc, matches);

//         if (matches.size() < 10)
//             continue;

//         std::sort(matches.begin(), matches.end());
//         matches.resize(30);

//         // Compute average distance of matches
//         double avg_distance = 0.0;
//         for (const auto &m : matches)
//             avg_distance += m.distance;
//         avg_distance /= matches.size();

//         std::cout << "Template " << i << " match avg distance: " << avg_distance << std::endl;

//         if (avg_distance > 50.0)
//             continue;

//         std::vector<cv::Point2f> pts1, pts2;
//         for (const auto &m : matches)
//         {
//             pts1.push_back(kp[m.queryIdx].pt);
//             pts2.push_back(scene_kp[m.trainIdx].pt);
//         }

//         cv::Mat H = cv::findHomography(pts1, pts2, cv::RANSAC);
//         if (!H.empty())
//         {
//             std::vector<cv::Point2f> corners = {
//                 {0, 0},
//                 {(float)templ.cols, 0},
//                 {(float)templ.cols, (float)templ.rows},
//                 {0, (float)templ.rows}};
//             std::vector<cv::Point2f> transformed;
//             cv::perspectiveTransform(corners, transformed, H);
//             cv::Rect bbox = cv::boundingRect(transformed);

//             if (bbox.x >= 0 && bbox.y >= 0 && bbox.width > 30 && bbox.height > 30 &&
//                 bbox.x + bbox.width < scene.cols && bbox.y + bbox.height < scene.rows)
//             {
//                 if (!mask.empty())
//                 {
//                     cv::Mat warped_mask;
//                     cv::warpPerspective(mask, warped_mask, H, scene.size());
//                     cv::Mat cropped = warped_mask(bbox);

//                     double nonZeroRatio = (double)cv::countNonZero(cropped) / (bbox.width * bbox.height);
//                     // if (nonZeroRatio < 0.2)
//                     if (nonZeroRatio < 0.02)
//                         continue; // Not enough overlap
//                 }

//                 raw_results.push_back({object_name, bbox, static_cast<float>(avg_distance)});
//             }
//         }
//     }

//     std::sort(raw_results.begin(), raw_results.end(), [](const DetectionResult &a, const DetectionResult &b)
//               { return a.score < b.score; });

//     if (!raw_results.empty())
//     {
//         return {raw_results.front()}; // Best only
//     }
//     return {};
// }

// void drawDetections(cv::Mat &image, const std::vector<DetectionResult> &results)
// {
//     for (const auto &res : results)
//     {
//         cv::rectangle(image, res.bounding_box, cv::Scalar(0, 255, 0), 2);
//         cv::putText(image, res.object_name, res.bounding_box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
//     }
// }

// void saveBoundingBoxes(const std::string &filename, const std::vector<DetectionResult> &results)
// {
//     std::ofstream ofs(filename);
//     for (const auto &res : results)
//     {
//         ofs << res.object_name << " "
//             << res.bounding_box.x << " "
//             << res.bounding_box.y << " "
//             << res.bounding_box.x + res.bounding_box.width << " "
//             << res.bounding_box.y + res.bounding_box.height << "\n";
//     }
//     ofs.close();
// }

// detection.cpp

// detection.cpp
#include "detection.hpp"
#include <experimental/filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

namespace fs = std::experimental::filesystem;

std::vector<cv::Mat> loadTemplateImages(const std::string &path)
{
    std::vector<cv::Mat> templates;
    for (const auto &file : fs::directory_iterator(path))
    {
        if (file.path().extension() == ".png" && file.path().string().find("color") != std::string::npos)
        {
            templates.push_back(cv::imread(file.path().string(), cv::IMREAD_COLOR));
        }
    }
    return templates;
}

std::vector<cv::Mat> loadTemplateMasks(const std::string &path)
{
    std::vector<cv::Mat> masks;
    for (const auto &file : fs::directory_iterator(path))
    {
        if (file.path().extension() == ".png" && file.path().string().find("mask") != std::string::npos)
        {
            masks.push_back(cv::imread(file.path().string(), cv::IMREAD_GRAYSCALE));
        }
    }
    return masks;
}

std::vector<DetectionResult> detectObjectsORB(
    const cv::Mat &scene,
    const std::vector<cv::Mat> &templates,
    const std::vector<cv::Mat> &masks,
    const std::string &object_name)
{

    std::vector<DetectionResult> valid_detections;
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    std::vector<cv::KeyPoint> scene_kp;
    cv::Mat scene_desc;
    orb->detectAndCompute(scene, cv::noArray(), scene_kp, scene_desc);

    for (size_t i = 0; i < templates.size(); ++i)
    {
        const auto &templ = templates[i];
        const auto &mask = masks[i];

        std::vector<cv::KeyPoint> kp;
        cv::Mat desc;
        orb->detectAndCompute(templ, mask, kp, desc);
        if (desc.empty() || scene_desc.empty())
            continue;

        std::vector<std::vector<cv::DMatch>> knn_matches;
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        matcher.knnMatch(desc, scene_desc, knn_matches, 2);

        std::vector<cv::DMatch> good_matches;
        for (const auto &pair : knn_matches)
        {
            if (pair.size() == 2 && pair[0].distance < 0.75f * pair[1].distance)
            {
                good_matches.push_back(pair[0]);
            }
        }
        if (good_matches.size() < 12)
            continue;

        std::sort(good_matches.begin(), good_matches.end(), [](const cv::DMatch &a, const cv::DMatch &b)
                  { return a.distance < b.distance; });
        good_matches.resize(std::min<size_t>(30, good_matches.size()));

        double avg_distance = 0.0;
        for (const auto &m : good_matches)
            avg_distance += m.distance;
        avg_distance /= good_matches.size();
        if (avg_distance > 50.0)
            continue;

        std::vector<cv::Point2f> pts1, pts2;
        for (const auto &m : good_matches)
        {
            pts1.push_back(kp[m.queryIdx].pt);
            pts2.push_back(scene_kp[m.trainIdx].pt);
        }

        cv::Mat H = cv::findHomography(pts1, pts2, cv::RANSAC);
        if (H.empty())
            continue;

        std::vector<cv::Point2f> corners = {{0, 0}, {(float)templ.cols, 0}, {(float)templ.cols, (float)templ.rows}, {0, (float)templ.rows}};
        std::vector<cv::Point2f> transformed;
        cv::perspectiveTransform(corners, transformed, H);
        cv::Rect bbox = cv::boundingRect(transformed);

        if (bbox.x < 0 || bbox.y < 0 || bbox.width < 30 || bbox.height < 30 ||
            bbox.x + bbox.width > scene.cols || bbox.y + bbox.height > scene.rows)
            continue;

        bool mask_pass = true;
        if (!mask.empty())
        {
            cv::Mat warped_mask;
            cv::warpPerspective(mask, warped_mask, H, scene.size());
            cv::Mat cropped = warped_mask(bbox);
            double nonZeroRatio = (double)cv::countNonZero(cropped) / (bbox.width * bbox.height);
            std::cout << "  → Mask overlap: " << nonZeroRatio << std::endl;
            if (nonZeroRatio < 0.20)
                mask_pass = false;
        }

        if (mask_pass)
        {
            valid_detections.push_back({object_name, bbox, static_cast<float>(avg_distance)});
        }
    }

    return valid_detections;
}

void drawDetections(cv::Mat &image, const std::vector<DetectionResult> &results)
{
    for (const auto &res : results)
    {
        cv::rectangle(image, res.bounding_box, cv::Scalar(0, 255, 0), 2);
        cv::putText(image, res.object_name, res.bounding_box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
    }
}

void saveBoundingBoxes(const std::string &filename, const std::vector<DetectionResult> &results)
{
    std::ofstream ofs(filename);
    for (const auto &res : results)
    {
        ofs << res.object_name << " "
            << res.bounding_box.x << " "
            << res.bounding_box.y << " "
            << res.bounding_box.x + res.bounding_box.width << " "
            << res.bounding_box.y + res.bounding_box.height << "\n";
    }
    ofs.close();
}