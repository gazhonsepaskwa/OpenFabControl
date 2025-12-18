import serial
import time
import threading

def test_baud_rate(port, baud_rates):
    """Tests a range of baud rates and prints the data received."""
    try:
        ser = serial.Serial(port, baud_rates[0])  # Initialize with the first rate
        time.sleep(2)  # Allow time for initialization
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return

    best_baud_rate = None
    best_data = ""
    
    for baud_rate in baud_rates:
        try:
            ser.baudrate = baud_rate
            print(f"Trying baud rate: {baud_rate}")
            time.sleep(10)  # Allow time for UART to reconfigure
            
            data = ser.readline().decode('utf-8', errors='ignore')
            print(f"Received data at {baud_rate}:\n{data}")

            if "version of ESP-IDF" in data:
                if best_baud_rate is None or baud_rate < best_baud_rate:
                    best_baud_rate = baud_rate
                    best_data = data
                    print(f"Found better baud rate: {best_baud_rate}")
            
            input("Press Enter to continue...") #pause for input

        except serial.SerialException as e:
            print(f"Error at {baud_rate}: {e}")
        except Exception as e:
            print(f"General error at {baud_rate}: {e}")


    print("\nTesting complete.")
    if best_baud_rate:
        print(f"Best baud rate found: {best_baud_rate}")
        print(f"Data received at best baud rate:\n{best_data}")
    else:
        print("No suitable baud rate found.")
    ser.close()



# Define the serial port and baud rates to test
serial_port = "/dev/cu.usbserial-02CT6FD6"
baud_rates = [115200, 115300, 115400, 115500, 115600] # Add more rates if needed


if __name__ == "__main__":
    test_baud_rate(serial_port, baud_rates)

