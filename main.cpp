#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <csignal>
#include <memory>

const int WIDTH = 1280;
const int HEIGHT = 720;
const int FPS = 30;
const int SEGMENT_DURATION = 300; // 5 minutes in seconds
bool running = true;

void signalHandler(int signum) {
    running = false;
}

std::string get_output_filename() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "video_%Y%m%d_%H%M%S.mp4", std::localtime(&now_time));
    return std::string(buffer);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Capture pipeline
    std::string capture_pipeline = 
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), width=" + std::to_string(WIDTH) + 
        ", height=" + std::to_string(HEIGHT) + 
        ", format=NV12, framerate=" + std::to_string(FPS) + "/1 ! "
        "nvvidconv flip-method=0 ! "
        "video/x-raw, format=BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=BGR ! "
        "appsink drop=true";

    cv::VideoCapture cap(capture_pipeline, cv::CAP_GSTREAMER);
    
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }

    std::unique_ptr<cv::VideoWriter> out;
    std::string current_output_file;
    auto segment_start_time = std::chrono::steady_clock::now();
    auto last_status_time = segment_start_time;
    int frame_count = 0;
    int segment_number = 0;

    auto create_new_writer = [&]() {
        current_output_file = get_output_filename();
        std::string encode_pipeline = 
            "appsrc ! "
            "videoconvert ! "
            "video/x-raw, format=NV12 ! "
            "nvvidconv ! "
            "video/x-raw(memory:NVMM), format=NV12 ! "
            "nvv4l2h264enc insert-sps-pps=true insert-vui=true bitrate=2000000 ! "
            "h264parse ! "
            "qtmux ! "
            "filesink location=" + current_output_file;

        out = std::make_unique<cv::VideoWriter>();
        out->open(encode_pipeline, cv::CAP_GSTREAMER, 0, FPS, cv::Size(WIDTH, HEIGHT), true);
        
        if (!out->isOpened()) {
            std::cerr << "Cannot create video writer" << std::endl;
            return false;
        }
        
        std::cout << "New segment started: " << current_output_file << std::endl;
        segment_number++;
        segment_start_time = std::chrono::steady_clock::now();
        return true;
    };

    if (!create_new_writer()) {
        return -1;
    }

    std::cout << "Recording started (720p H.264). New file every 5 minutes." << std::endl;
    std::cout << "Press Ctrl+C to stop recording..." << std::endl;

    cv::Mat frame;
    try {
        while (running) {
            if (!cap.read(frame)) {
                std::cerr << "Can't receive frame. Exiting..." << std::endl;
                break;
            }

            // Add timestamp
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
            cv::putText(frame, time_str, cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

            out->write(frame);
            frame_count++;

            // Check if we need to start a new segment
            auto current_time = std::chrono::steady_clock::now();
            auto segment_duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - segment_start_time).count();
            
            if (segment_duration >= SEGMENT_DURATION) {
                out->release();
                if (!create_new_writer()) {
                    break;
                }
            }

            // Print status every 30 seconds
            if (std::chrono::duration_cast<std::chrono::seconds>(current_time - last_status_time).count() >= 30) {
                auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - segment_start_time);
                double fps = frame_count / total_duration.count();
                std::cout << "Segment " << segment_number << ": " << frame_count << " frames (" 
                          << fps << " fps), Duration: " << total_duration.count() << "s" << std::endl;
                last_status_time = current_time;
            }
        }
    } catch (...) {
        std::cerr << "Exception occurred" << std::endl;
    }

    cap.release();
    if (out && out->isOpened()) {
        out->release();
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - segment_start_time);
    std::cout << "Recording stopped" << std::endl;
    std::cout << "Total segments: " << segment_number << std::endl;
    std::cout << "Current segment frames: " << frame_count << std::endl;
    std::cout << "Current segment duration: " << duration.count() << " seconds" << std::endl;

    return 0;
}
