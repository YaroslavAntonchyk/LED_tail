import serial
import csv

# Configure serial port
port = "COM5"  # Modify this to match the correct port
baud_rate = 115200  # Modify this to match the correct baud rate

# Open the serial port
ser = serial.Serial(port, baud_rate)

# Create a CSV file to store the data
csv_filename = "data1.csv"

# Create a CSV writer object
csv_file = open(csv_filename, "w", newline="")
csv_writer = csv.writer(csv_file)

# Read data from the serial port and write to CSV
try:
    while True:
        # Read a line of data from the serial port
        try:
            data = ser.readline().decode().strip()
        except:
            pass
        # Split the data into fields (if needed)
        # fields = data.split(",")  # Modify this if the data has multiple fields
        
        # Write the data to the CSV file
        csv_writer.writerow([data])  # Modify this if the data has multiple fields
        
        # Flush the CSV file buffer
        csv_file.flush()
        
        # Print the data to the console
        print(data)
        
except KeyboardInterrupt:
    # Close the CSV file and serial port on program exit
    csv_file.close()
    ser.close()
