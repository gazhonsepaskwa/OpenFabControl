compile:
	 mkdir -p ./build
	./bin/arduino-cli compile --fqbn esp32:esp32:esp32 . --build-path ./build

upload:
	@if [ -z "$(PORT)" ]; then \
		echo "Available ports:"; \
		./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | nl; \
		read -p "Port number: " n; \
		port=$$(./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | sed -n "$$n p"); \
	else \
		port="$(PORT)"; \
	fi; \
	./bin/arduino-cli upload --input-dir ./build -p "$$port" --fqbn esp32:esp32:esp32 .

install_arduinocli_linux_macos:
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
