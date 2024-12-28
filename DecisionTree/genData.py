import random
import csv

# Attributes and their possible values
outlook = ["Sunny", "Overcast", "Rain"]
temperature = ["Hot", "Mild", "Cool"]
humidity = ["High", "Normal"]
wind = ["Weak", "Strong"]
labels = ["Yes", "No"]

# Function to generate a dataset
def generate_dataset(file_name, rows):
    with open(file_name, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        writer.writerow(["Outlook", "Temperature", "Humidity", "Wind", "Label"])
        
        for _ in range(rows):
            # Randomly pick one value from each attribute
            data_row = [
                random.choice(outlook),
                random.choice(temperature),
                random.choice(humidity),
                random.choice(wind),
                random.choice(labels)
            ]
            writer.writerow(data_row)

# Generate 1000 rows of data
generate_dataset("train.txt", 500)
print("Dataset generated: decision_tree_dataset.csv")
