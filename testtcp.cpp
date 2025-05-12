#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>
#include <fcntl.h>
int setupServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    std::cout << "[INFO] Waiting for Node.js client to connect on port " << port << "...\n";
    return server_fd;
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Camera not opened.\n";
        return -1;
    }

    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::VideoWriter writer("output.avi", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(frame_width, frame_height));

    int server_fd = setupServer(8000);
    if (server_fd == -1) return -1;

    std::atomic<int> client_sock(-1);
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Function to handle client acceptance
    auto accept_client = [&]() {
        while (true) {
            int new_sock = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
            if (new_sock < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("accept");
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // Close previous connection if exists
            int old_sock = client_sock.exchange(new_sock);
            if (old_sock >= 0) {
                close(old_sock);
            }

            std::cout << "[INFO] Node.js client connected.\n";
            
            // Set non-blocking for the new socket
            int flags = fcntl(new_sock, F_GETFL, 0);
            fcntl(new_sock, F_SETFL, flags | O_NONBLOCK);
        }
    };

    // Start accept thread
    std::thread accept_thread(accept_client);

    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        writer.write(frame);

        int current_sock = client_sock.load();
        if (current_sock >= 0) {
            cv::imencode(".jpg", frame, buffer, params);
            int32_t size = buffer.size();
            
            // Send size
            ssize_t sent = send(current_sock, &size, sizeof(size), MSG_NOSIGNAL);
            if (sent <= 0) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    std::cerr << "[WARN] Node.js disconnected.\n";
                    close(current_sock);
                    client_sock.store(-1);
                    continue;
                }
            }
            
            // Send data
            sent = send(current_sock, buffer.data(), size, MSG_NOSIGNAL);
            if (sent <= 0) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    std::cerr << "[WARN] Node.js disconnected.\n";
                    close(current_sock);
                    client_sock.store(-1);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // Cleanup
    if (accept_thread.joinable()) {
        pthread_cancel(accept_thread.native_handle());
        accept_thread.join();
    }
    
    int current_sock = client_sock.load();
    if (current_sock >= 0) close(current_sock);
    close(server_fd);

    cap.release();
    writer.release();
    return 0;
}