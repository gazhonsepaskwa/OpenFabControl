compile:
	 mkdir -p ./build
	./bin/arduino-cli compile --libraries ./libraries --fqbn esp32:esp32:esp32 . --build-path ./build

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

nvs_reset:
	@$(MAKE) compile; \
	if [ -z "$(PORT)" ]; then \
		echo "Available ports:"; \
		./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | nl; \
		read -p "Port number: " n; \
		port=$$(./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | sed -n "$$n p"); \
	else \
		port="$(PORT)"; \
	fi; \
	if [ -z "$$port" ]; then \
		echo "ERROR: no port selected"; \
		exit 1; \
	fi; \
	nvs_line=$$(awk -F',' 'BEGIN{IGNORECASE=1} $$1 ~ /^[[:space:]]*nvs[[:space:]]*$$/ {print; exit}' ./build/partitions.csv); \
	if [ -z "$$nvs_line" ]; then \
		echo "ERROR: could not find NVS partition in ./build/partitions.csv"; \
		exit 1; \
	fi; \
	nvs_offset=$$(echo "$$nvs_line" | awk -F',' '{gsub(/[[:space:]]/,"",$$4); print $$4}'); \
	nvs_size=$$(echo "$$nvs_line"   | awk -F',' '{gsub(/[[:space:]]/,"",$$5); print $$5}'); \
	if [ -z "$$nvs_offset" ] || [ -z "$$nvs_size" ]; then \
		echo "ERROR: failed to parse NVS offset/size from: $$nvs_line"; \
		exit 1; \
	fi; \
	esptool_path=$$(./bin/arduino-cli compile --libraries ./libraries --fqbn esp32:esp32:esp32 . --build-path ./build --show-properties | awk -F= '$$1=="tools.esptool_py.path"{print $$2; exit}'); \
	esptool_cmd=$$(./bin/arduino-cli compile --libraries ./libraries --fqbn esp32:esp32:esp32 . --build-path ./build --show-properties | awk -F= '$$1=="tools.esptool_py.cmd"{print $$2; exit}'); \
	if [ -z "$$esptool_path" ] || [ -z "$$esptool_cmd" ]; then \
		echo "ERROR: could not locate esptool via arduino-cli --show-properties"; \
		exit 1; \
	fi; \
	echo "Erasing NVS at $$nvs_offset size $$nvs_size on $$port"; \
	"$$esptool_path/$$esptool_cmd" --chip esp32 --port "$$port" --baud 921600 --before default-reset --after hard-reset erase-region "$$nvs_offset" "$$nvs_size"; \
	$(MAKE) upload PORT="$$port"

nvs_erase:
	@$(MAKE) compile; \
	if [ -z "$(PORT)" ]; then \
		echo "Available ports:"; \
		./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | nl; \
		read -p "Port number: " n; \
		port=$$(./bin/arduino-cli board list | tail -n +2 | awk '{print $$1}' | sed -n "$$n p"); \
	else \
		port="$(PORT)"; \
	fi; \
	if [ -z "$$port" ]; then \
		echo "ERROR: no port selected"; \
		exit 1; \
	fi; \
	nvs_line=$$(awk -F',' 'BEGIN{IGNORECASE=1} $$1 ~ /^[[:space:]]*nvs[[:space:]]*$$/ {print; exit}' ./build/partitions.csv); \
	if [ -z "$$nvs_line" ]; then \
		echo "ERROR: could not find NVS partition in ./build/partitions.csv"; \
		exit 1; \
	fi; \
	nvs_offset=$$(echo "$$nvs_line" | awk -F',' '{gsub(/[[:space:]]/,"",$$4); print $$4}'); \
	nvs_size=$$(echo "$$nvs_line"   | awk -F',' '{gsub(/[[:space:]]/,"",$$5); print $$5}'); \
	if [ -z "$$nvs_offset" ] || [ -z "$$nvs_size" ]; then \
		echo "ERROR: failed to parse NVS offset/size from: $$nvs_line"; \
		exit 1; \
	fi; \
	esptool_path=$$(./bin/arduino-cli compile --libraries ./libraries --fqbn esp32:esp32:esp32 . --build-path ./build --show-properties | awk -F= '$$1=="tools.esptool_py.path"{print $$2; exit}'); \
	esptool_cmd=$$(./bin/arduino-cli compile --libraries ./libraries --fqbn esp32:esp32:esp32 . --build-path ./build --show-properties | awk -F= '$$1=="tools.esptool_py.cmd"{print $$2; exit}'); \
	if [ -z "$$esptool_path" ] || [ -z "$$esptool_cmd" ]; then \
		echo "ERROR: could not locate esptool via arduino-cli --show-properties"; \
		exit 1; \
	fi; \
	echo "Erasing NVS at $$nvs_offset size $$nvs_size on $$port"; \
	"$$esptool_path/$$esptool_cmd" --chip esp32 --port "$$port" --baud 921600 --before default-reset --after hard-reset erase-region "$$nvs_offset" "$$nvs_size"

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
	./bin/arduino-cli lib install "ArduinoJson"

setup: install_arduinocli_linux install_lib
