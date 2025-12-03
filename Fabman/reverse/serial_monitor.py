import serial
import sys

def serial_monitor(port, baud_rate):
    """
    Continuous serial monitor that prints received data, ignoring carriage returns and other control characters.

    Args:
        port (str): The serial port to monitor (e.g., "/dev/cu.usbserial-02CT6FD6").
        baud_rate (int): The baud rate of the serial port.
    """

    try:
        ser = serial.Serial(port, baud_rate)
        print(f"Connected to {port} at {baud_rate} baud.")

        while True:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore')  # Read one line
                    
                    # Remove carriage returns and other control characters
                    line = line.replace('\r', '').replace('\x07', '').replace('\x08', '')

                    print(line, end='')  # Print without adding a newline character
                except UnicodeDecodeError:
                    print("Error decoding data", end='')
            else:
                time.sleep(0.1) #wait a little before checking again
    except serial.SerialException as e:
        print(f"Error connecting to {port}: {e}")
    except KeyboardInterrupt:
        print("\nExiting serial monitor.")
    finally:
        if 'ser' in locals():  #Make sure ser exists
            ser.close()



if __name__ == "__main__":
    import time

    if len(sys.argv) != 3:
        print("Usage: python serial_monitor.py <port> <baud_rate>")
        sys.exit(1)

    port = sys.argv[1]
    try:
        baud_rate = int(sys.argv[2])
    except ValueError:
        print("Baud rate must be an integer.")
        sys.exit(1)

    serial_monitor(port, baud_rate)

