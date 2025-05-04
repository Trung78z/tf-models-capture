#!/bin/bash
sudo systemctl stop capture.service
sudo bash -c 'cat > /etc/systemd/system/capture.service' <<EOF
[Unit]
Description=Capture Executable Autostart
After=systemd-user-sessions.service

[Service]
ExecStart=/home/jetson/Desktop/datasets/script.sh
WorkingDirectory=/home/jetson/Desktop/datasets/collects
User=jetson
Restart=on-failure
RestartSec=5
StartLimitIntervalSec=60
StartLimitBurst=3

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
