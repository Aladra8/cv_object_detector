// #include "detection.hpp"
// #include <experimental/filesystem>
// #include <iostream>

// namespace fs = std::experimental::filesystem;

// int main()
// {
//     std::string object_name = "sugar_box";
// std::string base_path = "/home/aladra/cv_object_detector/data/004_sugar_box";    std::string scene_dir = base_path + "/test_images";
//     std::string model_dir = base_path + "/models";

//     std::vector<cv::Mat> templates = loadTemplateImages(model_dir);
//     std::vector<cv::Mat> masks = loadTemplateMasks(model_dir);  // Just call it, not define

//     for (const auto &entry : fs::directory_iterator(scene_dir))
//     {
//         std::string scene_path = entry.path().string();
//         if (scene_path.find("color") == std::string::npos || entry.path().extension() != ".jpg")
//             continue;

//         std::string filename = entry.path().filename().string();
//         std::string base_name = filename.substr(0, filename.find("-"));

//         cv::Mat scene = cv::imread(scene_path);
//         if (scene.empty())
//         {
//             std::cerr << "Could not load " << scene_path << std::endl;
//             continue;
//         }

//         auto detections = detectObjectsORB(scene, templates, masks, object_name);

//         drawDetections(scene, detections);

//         std::string output_img = "/home/aladra/cv_object_detector/output/images/" + base_name + "-detected.jpg";
//         std::string output_txt = "/home/aladra/cv_object_detector/output/detections/" + base_name + "-box.txt";

//         saveBoundingBoxes(output_txt, detections);
//         cv::imwrite(output_img, scene);

//         std::cout << "Processed: " << filename << std::endl;
//     }

//     std::cout << "All images processed!" << std::endl;
//     return 0;
// }

#include "detection.hpp"
#include <experimental/filesystem>
#include <iostream>
#include <map>

namespace fs = std::experimental::filesystem;

int main() {
    std::string base_data_path = "/home/aladra/cv_object_detector/data";
    std::string output_image_dir = "/home/aladra/cv_object_detector/output/images/";
    std::string output_text_dir = "/home/aladra/cv_object_detector/output/detections/";

    std::map<std::string, std::string> object_dirs = {
        {"power_drill", "035_power_drill"},
        {"sugar_box", "004_sugar_box"},
        {"mustard_bottle", "006_mustard_bottle"}
    };

    for (const auto& entry : object_dirs) {
        std::string object_name = entry.first;
        std::string folder = entry.second;

        std::string model_dir = base_data_path + "/" + folder + "/models";
        std::string image_dir = base_data_path + "/" + folder + "/test_images";

        std::vector<cv::Mat> templates = loadTemplateImages(model_dir);
        std::vector<cv::Mat> masks = loadTemplateMasks(model_dir);

        for (const auto& image_file : fs::directory_iterator(image_dir)) {
            if (image_file.path().extension() != ".jpg" || image_file.path().string().find("color") == std::string::npos)
                continue;

            cv::Mat scene = cv::imread(image_file.path().string(), cv::IMREAD_COLOR);
            if (scene.empty()) {
                std::cerr << "Could not load image: " << image_file.path().string() << std::endl;
                continue;
            }

            auto detections = detectObjectsORB(scene, templates, masks, object_name);

            drawDetections(scene, detections);

            std::string filename = image_file.path().filename().string();
            std::string base_name = filename.substr(0, filename.find("-"));

            std::string output_image = output_image_dir + base_name + "-" + object_name + "-detected.jpg";
            std::string output_text = output_text_dir + base_name + "-" + object_name + "-box.txt";

            saveBoundingBoxes(output_text, detections);
            cv::imwrite(output_image, scene);

            std::cout << "Processed: " << filename << " for object: " << object_name << std::endl;
        }
    }

    std::cout << "All images processed for all known objects!" << std::endl;
    return 0;
}