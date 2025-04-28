
#include "../include/evaluate.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// Structure to hold bounding box information
struct Box
{
    std::string name;
    int xmin, ymin, xmax, ymax;
};

// Compute Intersection over Union (IoU) between two boxes
float computeIoU(const Box &a, const Box &b)
{
    int x_left = std::max(a.xmin, b.xmin);
    int y_top = std::max(a.ymin, b.ymin);
    int x_right = std::min(a.xmax, b.xmax);
    int y_bottom = std::min(a.ymax, b.ymax);

    if (x_right <= x_left || y_bottom <= y_top)
        return 0.0f;

    int intersection_area = (x_right - x_left) * (y_bottom - y_top);
    int a_area = (a.xmax - a.xmin) * (a.ymax - a.ymin);
    int b_area = (b.xmax - b.xmin) * (b.ymax - b.ymin);

    return static_cast<float>(intersection_area) / (a_area + b_area - intersection_area);
}

// Main evaluation function: computes detection accuracy and mean IoU
void evaluateDetections(const std::string &pred_dir, const std::string &gt_root)
{
    int correct = 0;
    int total_gt = 0;
    int total_pred = 0;
    float total_iou = 0.0f;

    // Loop through each category folder
    for (const auto &category : fs::directory_iterator(gt_root))
    {
        if (!category.is_directory())
            continue;

        fs::path label_dir = category.path() / "labels";

        if (!fs::exists(label_dir))
            continue;

        // Loop through each ground-truth label file
        for (const auto &label_file : fs::directory_iterator(label_dir))
        {
            if (label_file.path().extension() != ".txt")
                continue;

            std::string base_name = label_file.path().filename().string();
            fs::path pred_file = fs::path(pred_dir) / base_name;

            if (!fs::exists(pred_file))
                continue;

            std::vector<Box> gt_boxes, pred_boxes;
            std::ifstream gt_ifs(label_file.path());
            std::ifstream pred_ifs(pred_file);

            std::string line;

            // Parse ground-truth boxes
            while (std::getline(gt_ifs, line))
            {
                std::istringstream iss(line);
                Box box;
                iss >> box.name >> box.xmin >> box.ymin >> box.xmax >> box.ymax;
                gt_boxes.push_back(box);
            }

            // Parse predicted boxes
            while (std::getline(pred_ifs, line))
            {
                std::istringstream iss(line);
                Box box;
                iss >> box.name >> box.xmin >> box.ymin >> box.xmax >> box.ymax;
                pred_boxes.push_back(box);
            }

            total_gt += gt_boxes.size();
            total_pred += pred_boxes.size();

            // Compare each ground-truth box with predictions
            for (const auto &gt : gt_boxes)
            {
                float best_iou = 0.0f;
                for (const auto &pred : pred_boxes)
                {
                    if (gt.name == pred.name)
                    {
                        best_iou = std::max(best_iou, computeIoU(gt, pred));
                    }
                }
                if (best_iou > 0.5f)
                {
                    correct++;
                    total_iou += best_iou;
                }
            }
        }
    }

    // Calculate final metrics
    float accuracy = total_gt > 0 ? static_cast<float>(correct) / total_gt * 100.0f : 0.0f;
    float mean_iou = correct > 0 ? total_iou / correct * 100.0f : 0.0f;

    // Print evaluation summary
    std::cout << "\n=== Per-Class Evaluation ===" << std::endl;
    std::cout << "\n=== Overall Evaluation ===" << std::endl;
    std::cout << "Detection Accuracy: " << accuracy << "%" << std::endl;
    std::cout << "Mean IoU (mIoU): " << mean_iou << "%" << std::endl;
}
