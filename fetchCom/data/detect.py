import csv
import math
import matplotlib.pyplot as plt
file_path = 'tail9.csv'

class Detector:
    def __init__(self):
        self.x = [0]
        self.ylog = [0]
        self.y = [0]
        self.filtred = [0]

    def detect(self, data):
        if len(data) > 1 and float(data[1])/1000000 > self.x[-1] and '.' in data[0]: 
            self.x.append(float(data[1])/1000000)
            self.ylog.append((float(data[0])))
        # self.filtred.append(self.filtred[-1]*0.8 + self.ylog[-1]*0.2)

    def draw(self):
        fig, ax = plt.subplots()

        # Plot the data
        ax.plot(self.x, self.ylog)
        # ax.plot(self.x, self.filtred)

        # Set labels and title
        ax.set_xlabel('X-axis')
        ax.set_ylabel('Y-axis')
        ax.set_title('Line Plot')
        
        plt.grid()
        # Show the plot
        plt.show()
        



def main():
    detector = Detector()
    try:
        with open(file_path, 'r') as file:
            reader = csv.reader(file)
            for row in reader:
                # Process each row of data
                detector.detect(row)
    except FileNotFoundError:
        print("File not found.")

    detector.draw()


if __name__ == "__main__":
    main()