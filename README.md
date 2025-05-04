
# 📹 TF Models Capture for Jetson

Capture training data using Jetson's camera (e.g. `nvarguscamerasrc`) with OpenCV and GStreamer — optimized for TensorFlow models like EfficientDet or SSD.

---

## 🚀 Quick Start (Jetson Orin Nano)

### 1. Open terminal and clone the repo
```bash
cd ~/Desktop
git clone https://github.com/Trung78z/tf-models-capture.git
```

### 2. Move to the correct directory
```bash
mv tf-models-capture datasets
cd datasets
```

### 3. Install dependencies
```bash
make install
```

### 4. Run the capture program
```bash
make run
```

Captured data will be saved in `datasets/collects/`.

---

## 🛠 Features

- ✅ Uses `nvarguscamerasrc` for Jetson camera input
- ✅ OpenCV + GStreamer pipeline
- ✅ Auto-detects `/dev/video0` or `/dev/video1`
- ✅ Saves video/images with timestamped filenames
- ✅ Ready for labeling & training with TF Object Detection API

---

## 📂 Project Structure

```
datasets/
├── collects/           # Captured video/images
├── script.sh           # Startup wrapper
├── capture             # Compiled C++ capture binary
├── Makefile
└── README.md
```

---

## 💡 Tip

To auto-run on boot with systemd, use the following service file:

```ini
[Unit]
Description=Capture Autostart
After=multi-user.target

[Service]
ExecStart=/home/jetson/Desktop/datasets/script.sh
WorkingDirectory=/home/jetson/Desktop/datasets/collects
User=jetson
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

---

## 📞 Contact

Created by [Trung78z](https://github.com/Trung78z)  
For any questions or support, feel free to open an issue.