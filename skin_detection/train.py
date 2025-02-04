import numpy as np
import cv2
import os

# Count of Skin Colors
skin_rgb_cnt = np.empty(shape=(256, 256, 256))
skin_rgb_cnt.fill(0)

# Count of Non-Skin Colors
non_skin_rgb_cnt = np.empty(shape=(256, 256, 256))
non_skin_rgb_cnt.fill(0)

total_skin_color = 0
total_non_skin_color = 0
total_images = 555
indices = ["%04d" % x for x in range(1000)]

for index in range(total_images): # total image 
    mask_file = "Mask/" + str(indices[index]) + ".bmp"
    actual_file = "ibtd/" + str(indices[index]) + ".jpg"
    
    # Check if the files exist
    if not os.path.exists(mask_file):
        print(f"Warning: Mask file not found - {mask_file}")
        continue  # Skip to next iteration if mask file doesn't exist
    
    if not os.path.exists(actual_file):
        print(f"Warning: Actual file not found - {actual_file}")
        continue  # Skip to next iteration if actual file doesn't exist
    
    # Read the images
    mask_image = cv2.imread(mask_file)
    actual_image = cv2.imread(actual_file)
    
    # Check if the images were successfully loaded
    if mask_image is None:
        print(f"Error: Failed to load Mask image - {mask_file}")
        continue  # Skip to next iteration if mask image fails to load
    
    if actual_image is None:
        print(f"Error: Failed to load Actual image - {actual_file}")
        continue  # Skip to next iteration if actual image fails to load

    # Process the images
    height, width, channel = mask_image.shape

    for x in range(height):
        for y in range(width):
            mask_blue = mask_image[x, y, 0]
            mask_green = mask_image[x, y, 1]
            mask_red = mask_image[x, y, 2]

            blue = actual_image[x, y, 0]
            green = actual_image[x, y, 1]
            red = actual_image[x, y, 2]

            if mask_blue > 250 and mask_green > 250 and mask_red > 250:  # means it's NON-SKIN
                non_skin_rgb_cnt[red, green, blue] += 1
                total_non_skin_color += 1
            else:
                skin_rgb_cnt[red, green, blue] += 1
                total_skin_color += 1

    print(f"Image {index} - DONE!")

# Write the output to the file
with open('output.txt', 'w') as fp:
    for r in range(256):
        for g in range(256):
            for b in range(256):
                skin_prob = skin_rgb_cnt[r, g, b] / total_skin_color               # P(C|S)
                non_skin_prob = non_skin_rgb_cnt[r, g, b] / total_non_skin_color   # P(C|NS)

                if non_skin_rgb_cnt[r, g, b] != 0:
                    threshold = skin_prob / non_skin_prob                          # P(C|S) / P(C|NS)
                else:
                    threshold = 0.0

                fp.write(f"{threshold}\n")

print("All images processed and output written to 'output.txt'.")
