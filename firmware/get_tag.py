
import serial
import time
from pynput import keyboard
import threading

# Configuration
SERIAL_PORT ='/dev/cu.usbserial-02YDYDAP'  # Change this to your serial port
BAUD_RATE = 115200
DELAY_BETWEEN_CHARS = 0.01  # Delay between each character (in seconds)

# Global variable to control the serial reading thread
running = True

def read_serial():
    """Reads from serial port and sends keystrokes"""
    global running

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {SERIAL_PORT}")
        print("Listening for serial data...")

        while running:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8', errors='ignore').strip()
                if data:
                    print(f"Received: {data}")
                    type_text(data)
            time.sleep(0.01)

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        running = False
        if 'ser' in locals():
            ser.close()

def type_text(text):
    """Simulates typing the given text"""
    controller = keyboard.Controller()

    for char in text:
        controller.press(char)
        controller.release(char)
        time.sleep(DELAY_BETWEEN_CHARS)

def main():
    """Main function to start serial reading in a separate thread"""
    serial_thread = threading.Thread(target=read_serial)
    serial_thread.daemon = True
    serial_thread.start()

    try:
        # Keep main thread alive
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping...")
        global running
        running = False
        serial_thread.join()

if __name__ == "__main__":
    main()
