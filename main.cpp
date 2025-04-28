
#include "../include/detection.hpp"
#include "../include/evaluate.hpp"
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    // Check input arguments for dataset and output folder
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <dataset_root> <output_dir>" << std::endl;
        return -1;
    }

    std::string dataset_root = argv[1];
    std::string output_dir = argv[2];

    // Create output directories for results
    fs::create_directories(output_dir + "/images");
    fs::create_directories(output_dir + "/detections");

    // Iterate over object categories inside dataset_root
    for (const auto &category : fs::directory_iterator(dataset_root))
    {
        if (!category.is_directory())
            continue;

        std::string category_full_name = category.path().filename().string();
        std::string category_name = category_full_name.substr(category_full_name.find('_') + 1);
        fs::path models_path = category.path() / "models";
        fs::path images_path = category.path() / "test_images";

        if (!fs::exists(models_path) || !fs::exists(images_path))
            continue;

        // Load template images and masks
        auto templates = loadTemplateImages(models_path.string());
        auto masks = loadTemplateMasks(models_path.string());

        if (templates.empty())
        {
            std::cout << "[DEBUG] No templates for: " << category_name << std::endl;
            continue;
        }

        // Process each test image in the category
        for (const auto &img_file : fs::directory_iterator(images_path))
        {
            if (img_file.path().extension() != ".jpg" && img_file.path().extension() != ".png")
                continue;

            cv::Mat scene = cv::imread(img_file.path().string());
            if (scene.empty())
                continue;

            std::string img_name = img_file.path().filename().string();

            // Run object detection using SIFT-based matching
            auto detections = detectObjectsSIFT(scene, templates, masks, category_name);

            // Draw bounding boxes on scene image
            drawDetections(scene, detections);

            // Save bounding box text file
            saveBoundingBoxes(output_dir + "/detections/" + img_name.substr(0, img_name.find_last_of('.')) + ".txt", detections);

            // Save image with drawn detections
            cv::imwrite(output_dir + "/images/" + img_name, scene);

            std::cout << "[DEBUG] Processed: " << img_name << " - Detections: " << detections.size() << std::endl;
        }
    }

    // After all detections, run evaluation on predicted results
    std::cout << "\n=== Running Evaluation ===" << std::endl;
    evaluateDetections(output_dir + "/detections", dataset_root);

    return 0;
}
