compile:
	 mkdir -p ./build
	./bin/arduino-cli compile --fqbn esp32:esp32:esp32da . --build-path ./build

upload:
	./bin/arduino-cli upload --input-dir ./build -p "$(shell ./bin/arduino-cli board list | head -n 2 | tail -n 1 | awk '{print $$1}')" --fqbn esp32:esp32:esp32da .

install_arduinocli_linux:
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
	./bin/arduino-cli core update-index
	./bin/arduino-cli core install esp32:esp32

install_lib:
	./bin/arduino-cli lib install "Adafruit MCP23017 Arduino Library"
	./bin/arduino-cli lib install "Electronic Cats PN7150"
	./bin/arduino-cli lib update-index
	./bin/arduino-cli lib install "Adafruit GFX Library"
	./bin/arduino-cli lib install "Adafruit ILI9341"
	./bin/arduino-cli lib install "QRCodeGFX"

setup: install_arduinocli_linux install_lib
