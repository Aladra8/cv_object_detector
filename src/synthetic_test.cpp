#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    std::string template_path = "../data/004_sugar_box/models/000000-color.png"; // <-- fixed

    cv::Mat template_img = cv::imread(template_path);
    if (template_img.empty())
    {
        std::cerr << "Failed to load template image: " << template_path << std::endl;
        return -1;
    }

    cv::Mat scene = cv::Mat::zeros(480, 640, CV_8UC3); // black background
    cv::Rect roi(200, 100, template_img.cols, template_img.rows);

    if (roi.x + roi.width <= scene.cols && roi.y + roi.height <= scene.rows)
    {
        template_img.copyTo(scene(roi));
    }
    else
    {
        std::cerr << "Template too big for synthetic scene!" << std::endl;
        return -1;
    }

    cv::rectangle(scene, roi, cv::Scalar(0, 255, 0), 2);
    cv::putText(scene, "sugar_box", roi.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    cv::imshow("Synthetic Scene", scene);
    cv::imwrite("../output/synthetic_test_output.png", scene);
    cv::waitKey(0);

    return 0;
}
