PORT ?= /dev/ttyACM0
RESET_SCRIPTS_DURING_FLASH ?= false
IMAGE_NAME = scriptkitty-fw
CONTAINER_NAME := $(IMAGE_NAME)-$(shell date +%s)
ARDUINO_CLI_VERSION = 0.22.0

submodules:
	git submodule init
	# git submodule update

build: submodules
	docker build . --file Dockerfile --build-arg ARDUINO_CLI_VERSION=$(ARDUINO_CLI_VERSION) --tag scriptkitty-fw

flash: check-port build
	docker create --name $(CONTAINER_NAME) --device=$(PORT) -t scriptkitty-fw:latest
	docker start $(CONTAINER_NAME)
	docker exec $(CONTAINER_NAME) bash -c \
		'./arduino-cli upload -b esp32:esp32:esp32s2 --port $(PORT) ScriptKitty/ && sleep 2'
ifeq ($(RESET_SCRIPTS_DURING_FLASH), true)
	docker exec $(CONTAINER_NAME) bash -c \
		'python3 -m esptool --after no_reset write_flash 0x110000 default.img'
endif
	docker rm --force $(CONTAINER_NAME)

generate_bin: build
	docker create --name $(CONTAINER_NAME) -t scriptkitty-fw:latest
	docker start $(CONTAINER_NAME)
	docker cp $(CONTAINER_NAME):/app/build .
	docker rm --force $(CONTAINER_NAME)
	cp scriptkitty_fw_bin_template scriptkitty_fw.bin
	dd of=scriptkitty_fw.bin if=build/ScriptKitty-Firmware.ino.bin seek=65536 bs=1 conv=notrunc
	dd of=scriptkitty_fw.bin if=fatfs/default.img seek=1114112 bs=1 conv=notrunc count=3080192

check-port:
	@ls $(PORT) || { echo "Device not found at $(PORT)" && exit 1; }

#-------------------------------------------------------------------------------
# macOS development

MAC_PORT = /dev/cu.usbmodem01
MAC_ARDUINO_CLI_TAR = arduino-cli_$(ARDUINO_CLI_VERSION)_macOS_64bit.tar.gz

arduino-cli: scripts/arduino-cli arduino-cli-esp32-install

arduino-cli-esp32-install:
	sudo ./scripts/arduino-cli core install esp32:esp32 --config-file arduino-cli-mac.yaml

scripts/arduino-cli:
	curl -L --output scripts/$(MAC_ARDUINO_CLI_TAR) https://github.com/arduino/arduino-cli/releases/download/$(ARDUINO_CLI_VERSION)/$(MAC_ARDUINO_CLI_TAR)
	tar -xf scripts/$(MAC_ARDUINO_CLI_TAR) --directory=scripts
	rm scripts/$(MAC_ARDUINO_CLI_TAR)
	rm scripts/LICENSE.txt

check-mac-deps:
	docker info
	which python3
	python3 -E -m esptool version

check-mac-port:
	@ls $(MAC_PORT) || { echo "Device not found at $(MAC_PORT)" && exit 1; }

mac-build: submodules
	sudo docker build . --file Dockerfile --build-arg ARDUINO_CLI_VERSION=$(ARDUINO_CLI_VERSION) --tag scriptkitty-fw

flash-on-mac: check-mac-port check-mac-deps arduino-cli mac-build
	sudo docker create --name $(CONTAINER_NAME) -t scriptkitty-fw:latest
	sudo docker cp $(CONTAINER_NAME):/app/build .
	sudo docker rm --force $(CONTAINER_NAME)
ifeq ($(RESET_SCRIPTS_DURING_FLASH), true)
	python3 -m esptool --after no_reset write_flash 0x110000 fatfs/default.img
endif
	{ sudo ./scripts/arduino-cli upload -b esp32:esp32:esp32s2 --port $(MAC_PORT) --config-file arduino-cli-mac.yaml --input-dir build; } || echo \
		'If you get the error: "esptool.py can not exit the download mode over USB..." this is a bug in arduino-cli and can be ignored'
