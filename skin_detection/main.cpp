#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
using namespace std;
// Simple struct to represent a pixel
struct Pixel {
    uint8_t r, g, b;
};

// Function to read BMP image
vector<vector<Pixel>> readBMP(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        return {};
    }

    // BMP header reading
    uint8_t header[54];
    file.read(reinterpret_cast<char*>(header), 54);

    // Extract image dimensions
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];

    // Calculate padding
    int padding = (4 - (width * 3) % 4) % 4;

    // Create 2D vector to store pixels
    vector<vector<Pixel>> pixels(height, vector<Pixel>(width));

    // Read pixels
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            Pixel pixel;
            file.read(reinterpret_cast<char*>(&pixel.b), 1);
            file.read(reinterpret_cast<char*>(&pixel.g), 1);
            file.read(reinterpret_cast<char*>(&pixel.r), 1);
            pixels[y][x] = pixel;
        }
        // Skip padding
        file.seekg(padding, ios::cur);
    }

    return pixels;
}

// Function to write BMP image
void writeBMP(const string& filename, const vector<vector<Pixel>>& pixels) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot create file: " << filename << endl;
        return;
    }

    int width = pixels[0].size();
    int height = pixels.size();

    // Calculate padding
    int padding = (4 - (width * 3) % 4) % 4;

    // BMP Header
    uint8_t header[54] = {
        'B', 'M',           // BMP signature
        0, 0, 0, 0,         // File size (placeholder)
        0, 0, 0, 0,         // Reserved
        54, 0, 0, 0,        // Pixel data offset
        40, 0, 0, 0,        // DIB header size
        0, 0, 0, 0,         // Image width
        0, 0, 0, 0,         // Image height
        1, 0,               // Color planes
        24, 0,              // Bits per pixel
        0, 0, 0, 0,         // Compression method
        0, 0, 0, 0,         // Image size
        0, 0, 0, 0,         // Horizontal resolution
        0, 0, 0, 0,         // Vertical resolution
        0, 0, 0, 0,         // Color palette
        0, 0, 0, 0          // Important colors
    };

    // Set width and height
    *(int*)&header[18] = width;
    *(int*)&header[22] = height;

    // Write header
    file.write(reinterpret_cast<char*>(header), 54);

    // Write pixels
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            file.write(reinterpret_cast<const char*>(&pixels[y][x].b), 1);
            file.write(reinterpret_cast<const char*>(&pixels[y][x].g), 1);
            file.write(reinterpret_cast<const char*>(&pixels[y][x].r), 1);
        }
        // Write padding
        for (int p = 0; p < padding; ++p) {
            char pad = 0;
            file.write(&pad, 1);
        }
    }
}

int main() {
    const int total_images = 555;
    
    vector<vector<vector<int>>> skin_rgb_cnt(256, vector<vector<int>>(256, vector<int>(256, 0)));
    vector<vector<vector<int>>> non_skin_rgb_cnt(256, vector<vector<int>>(256, vector<int>(256, 0)));
    
    int total_skin_color = 0;
    int total_non_skin_color = 0;

    for (int index = 0; index < total_images; ++index) {
        string mask_path = "Mask/" + to_string(index) + ".bmp";
        string image_path = "ibtd/" + to_string(index) + ".jpg";

        auto mask_pixels = readBMP(mask_path);
        auto image_pixels = readBMP(image_path);

        for (size_t x = 0; x < image_pixels.size(); ++x) {
            for (size_t y = 0; y < image_pixels[x].size(); ++y) {
                Pixel mask_pixel = mask_pixels[x][y];
                Pixel image_pixel = image_pixels[x][y];

                int red = image_pixel.r;
                int green = image_pixel.g;
                int blue = image_pixel.b;

                if (mask_pixel.b > 250 && mask_pixel.g > 250 && mask_pixel.r > 250) {
                    non_skin_rgb_cnt[red][green][blue]++;
                    total_non_skin_color++;
                } else {
                    skin_rgb_cnt[red][green][blue]++;
                    total_skin_color++;
                }
            }
        }
        cout << "Image " << index << " - DONE!" << endl;
    }

    ofstream output_file("output.txt");
    for (int r = 0; r < 256; ++r) {
        for (int g = 0; g < 256; ++g) {
            for (int b = 0; b < 256; ++b) {
                double skin_prob = static_cast<double>(skin_rgb_cnt[r][g][b]) / total_skin_color;
                double non_skin_prob = static_cast<double>(non_skin_rgb_cnt[r][g][b]) / total_non_skin_color;
                
                double threshold = (non_skin_rgb_cnt[r][g][b] != 0) ? (skin_prob / non_skin_prob) : 0.0;
                
                output_file << threshold << endl;
            }
        }
    }
    output_file.close();

    const double T = 0.4;

    vector<vector<vector<double>>> trained_value(256, vector<vector<double>>(256, vector<double>(256, 0.0)));
    ifstream input_file("output.txt");

    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            for (int k = 0; k < 256; ++k) {
                double val;
                input_file >> val;
                trained_value[i][j][k] = val;
            }
        }
    }
    input_file.close();

    auto new_img_pixels = readBMP("AMla.bmp");

    for (size_t x = 0; x < new_img_pixels.size(); ++x) {
        for (size_t y = 0; y < new_img_pixels[x].size(); ++y) {
            Pixel& pixel = new_img_pixels[x][y];
            int red = pixel.r;
            int green = pixel.g;
            int blue = pixel.b;

            if (trained_value[red][green][blue] <= T) {
                pixel.r = 255;
                pixel.g = 255;
                pixel.b = 255;
            }
        }
    }

    writeBMP("final4.bmp", new_img_pixels);
    cout << "Done..." << endl;

    return 0;
}