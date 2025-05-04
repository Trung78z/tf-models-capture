run:
	./build.sh

restart:
	sudo systemctl daemon-reload
	sudo systemctl restart capture.service
	sudo systemctl status capture.service
status:
	sudo systemctl status capture.service
stop:
	sudo systemctl stop capture.service
disable:
	sudo systemctl disable capture.service
enable:
	sudo systemctl enable capture.service
	sudo systemctl status capture.service
clean:
	sudo rm -r collects/*.mp4

install:
	sudo chmod +x ./install.sh
	sudo chmod +x ./build.sh
	sudo chmod +x ./script.sh
	./install.sh
