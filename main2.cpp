#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <ctime>

// Camera settings
const int WIDTH = 640;
const int HEIGHT = 480;
const int FPS = 30;

std::string get_output_filename() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "video_%Y%m%d_%H%M%S.avi", std::localtime(&now_time));
    return std::string(buffer);
}

int main() {
    // GStreamer pipeline for CSI camera
    std::string pipeline = "nvarguscamerasrc ! "
                           "video/x-raw(memory:NVMM), width=(int)" + std::to_string(WIDTH) + 
                           ", height=(int)" + std::to_string(HEIGHT) + 
                           ", format=(string)NV12, framerate=(fraction)" + std::to_string(FPS) + "/1 ! "
                           "nvvidconv ! "
                           "video/x-raw, format=(string)BGRx ! "
                           "videoconvert ! "
                           "video/x-raw, format=(string)BGR ! "
                           "appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }

    std::string output_file = get_output_filename();
    cv::VideoWriter out;
    int fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    out.open(output_file, fourcc, FPS, cv::Size(WIDTH, HEIGHT));
    
    if (!out.isOpened()) {
        std::cerr << "Cannot create video writer" << std::endl;
        return -1;
    }

    std::cout << "Recording started. Saving to " << output_file << std::endl;
    std::cout << "Press 'q' to stop recording..." << std::endl;

    cv::Mat frame;
    try {
        while (true) {
            if (!cap.read(frame)) {
                std::cerr << "Can't receive frame. Exiting..." << std::endl;
                break;
            }

            // Add timestamp to frame
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
            cv::putText(frame, time_str, cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

            // Write frame
            out.write(frame);

            // Display
            cv::imshow("CSI Camera Recording", frame);

            if (cv::waitKey(1) == 'q') {
                break;
            }
        }
    } catch (...) {
        std::cerr << "Exception occurred" << std::endl;
    }

    cap.release();
    out.release();
    cv::destroyAllWindows();
    std::cout << "Recording saved to " << output_file << std::endl;

    return 0;
}
