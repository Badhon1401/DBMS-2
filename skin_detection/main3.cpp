#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <cstdint>
#include <algorithm>
#include <iomanip>
using namespace std;

// Lightweight image representation
struct Pixel {
    uint8_t r, g, b;
};

// Custom hash function for color tuples
struct ColorHash {
    template <class T1, class T2, class T3>
    size_t operator()(const tuple<T1, T2, T3>& t) const {
        return hash<T1>{}(get<0>(t)) ^ 
               hash<T2>{}(get<1>(t)) ^ 
               hash<T3>{}(get<2>(t));
    }
};

class SkinDetector {
private:
    // Memory-efficient probability tracking
    struct ColorProbability {
        uint64_t skin_count = 0;
        uint64_t non_skin_count = 0;
        double probability = 0.0;
    };

    // Use unordered_map to store only colors that appear in the dataset
    unordered_map<tuple<uint8_t, uint8_t, uint8_t>, ColorProbability, ColorHash> color_probabilities;

    // Utility function to format index with leading zeros
    string formatIndex(int index) {
        ostringstream oss;
        oss << setw(4) << setfill('0') << index;
        return oss.str();
    }

     // Read image with basic error handling
     static vector<vector<Pixel>> readImage(const string& filename) {
        string ext = getFileExtension(filename);
        
        if (ext == "bmp") return readBMP(filename);
        if (ext == "jpg" || ext == "jpeg") return readJPG(filename);
        
        cerr << "Unsupported image format" << endl;
        return {};
    }
     static void writeBMP(const string& filename, const vector<vector<Pixel>>& pixels) {
        ofstream file(filename, std::ios::binary);
        if (!file) {
            cerr << "Cannot create file: " << filename << endl;
            return;
        }

        int width = pixels[0].size();
        int height = pixels.size();
        int padding = (4 - (width * 3) % 4) % 4;

        uint8_t header[54] = {
            'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
            40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0
        };

        *(int*)&header[18] = width;
        *(int*)&header[22] = height;

        file.write(reinterpret_cast<char*>(header), 54);

        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                file.write(reinterpret_cast<const char*>(&pixels[y][x].b), 1);
                file.write(reinterpret_cast<const char*>(&pixels[y][x].g), 1);
                file.write(reinterpret_cast<const char*>(&pixels[y][x].r), 1);
            }
            for (int p = 0; p < padding; ++p) {
                char pad = 0;
                file.write(&pad, 1);
            }
        }
    }
    static string getFileExtension(const string& filename) {
        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos == string::npos) return "";

        string ext = filename.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    static vector<vector<Pixel>> readBMP(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Cannot open BMP file: " << filename << endl;
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

    // Simple JPG reading function (very basic, might not work for all JPGs)
    static vector<vector<Pixel>> readJPG(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Cannot open JPG file: " << filename << endl;
            return {};
        }

        // Basic JPG header detection
        uint8_t marker[2];
        file.read(reinterpret_cast<char*>(marker), 2);
        if (marker[0] != 0xFF || marker[1] != 0xD8) {
            cerr << "Invalid JPG header" << endl;
            return {};
        }

        // Find Start of Frame (SOF) marker to get image dimensions
        int width = 0, height = 0;
        while (true) {
            uint8_t marker[2];
            file.read(reinterpret_cast<char*>(marker), 2);
            
            if (file.eof()) break;

            if (marker[0] != 0xFF) continue;

            if (marker[1] == 0xC0 || marker[1] == 0xC2) {
                // SOF marker found
                uint16_t length;
                file.read(reinterpret_cast<char*>(&length), 2);
                length = (length << 8) | (length >> 8); // Convert to big-endian

                uint8_t precision;
                file.read(reinterpret_cast<char*>(&precision), 1);

                uint16_t h, w;
                file.read(reinterpret_cast<char*>(&h), 2);
                file.read(reinterpret_cast<char*>(&w), 2);
                
                height = (h << 8) | (h >> 8);
                width = (w << 8) | (w >> 8);
                break;
            }
        }

        if (width == 0 || height == 0) {
            cerr << "Could not determine image dimensions" << endl;
            return {};
        }

        // Reset file to beginning
        file.seekg(0, ios::beg);

        // Simplified pixel extraction (very basic, might not work for all JPGs)
        vector<vector<Pixel>> pixels(height, vector<Pixel>(width));
        
        // Basic pixel sampling (this is a very crude approximation)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel pixel;
                file.read(reinterpret_cast<char*>(&pixel.r), 1);
                file.read(reinterpret_cast<char*>(&pixel.g), 1);
                file.read(reinterpret_cast<char*>(&pixel.b), 1);
                pixels[y][x] = pixel;
            }
        }

        return pixels;
    }

    // Process a batch of images
    void processBatch(const string& maskDir, const string& imageDir, int start, int end) {
        for (int index = start; index < end; ++index) {
            string index_str = formatIndex(index);
            string mask_path = maskDir + "/" + index_str + ".bmp";
            string image_path = imageDir + "/" + index_str + ".jpg";

            auto mask_pixels = readImage(mask_path);
            auto image_pixels = readImage(image_path);

            if (mask_pixels.empty() || image_pixels.empty()) {
                cerr << "Skipping image " << index_str << endl;
                continue;
            }

            updateColorProbabilities(mask_pixels, image_pixels);
            cout << "Processed image " << index << endl;
        }
    }

    // Update color probabilities
    void updateColorProbabilities(const vector<vector<Pixel>>& mask_pixels, 
                                  const vector<vector<Pixel>>& image_pixels) {
        for (size_t y = 0; y < image_pixels.size(); ++y) {
            for (size_t x = 0; x < image_pixels[y].size(); ++x) {
                const Pixel& mask_pixel = mask_pixels[y][x];
                const Pixel& image_pixel = image_pixels[y][x];

                auto color_key = make_tuple(image_pixel.r, image_pixel.g, image_pixel.b);

                // White/near-white mask indicates non-skin
                if (mask_pixel.r > 250 && mask_pixel.g > 250 && mask_pixel.b > 250) {
                    color_probabilities[color_key].non_skin_count++;
                } else {
                    color_probabilities[color_key].skin_count++;
                }
            }
        }
    }

    // Calculate probabilities and save to file
    void calculateProbabilities(const string& output_file) {
        ofstream file(output_file);
        if (!file) {
            cerr << "Cannot create output file" << endl;
            return;
        }

        uint64_t total_skin_pixels = 0;
        uint64_t total_non_skin_pixels = 0;

        // First pass: calculate totals
        for (const auto& pair : color_probabilities) {
            const auto& color = pair.first;
            const auto& prob = pair.second;
            total_skin_pixels += prob.skin_count;
            total_non_skin_pixels += prob.non_skin_count;
        }

        // Second pass: calculate probabilities
        for (auto& pair : color_probabilities) {
            auto& color = pair.first;
            auto& prob = pair.second;
            double skin_prob = static_cast<double>(prob.skin_count) / total_skin_pixels;
            double non_skin_prob = static_cast<double>(prob.non_skin_count) / total_non_skin_pixels;

            // Simple probability ratio, avoiding division by zero
            prob.probability = (non_skin_prob > 0) ? (skin_prob / non_skin_prob) : 0.0;
            file << prob.probability << endl;
        }
    }

public:
    // Main processing method with batching
    void processDataset(const string& maskDir, const string& imageDir, int totalImages, int batchSize = 50) {
        for (int start = 0; start < totalImages; start += batchSize) {
            int end = min(start + batchSize, totalImages);
            processBatch(maskDir, imageDir, start, end);
        }

        // Save final probabilities
        calculateProbabilities("skin_probabilities.txt");
    }

    // Skin detection method
    void detectSkin(const string& inputImage, const string& outputImage, double threshold = 0.4) {
        auto image_pixels = readImage(inputImage);

        for (auto& row : image_pixels) {
            for (auto& pixel : row) {
                auto color_key = make_tuple(pixel.r, pixel.g, pixel.b);

                // Check probability and apply skin detection
                if (color_probabilities.count(color_key) && 
                    color_probabilities[color_key].probability <= threshold) {
                    pixel.r = pixel.g = pixel.b = 255;  // White out non-skin
                }
            }
        }

        // Write processed image
        writeBMP(outputImage, image_pixels);
    }
};

int main() {
    try {
        SkinDetector detector;
        
        // Process dataset in manageable batches
        detector.processDataset("Mask", "ibtd", 15, 5);
        
        // Detect skin in a specific image
        detector.detectSkin("AMla.jpg", "final4.bmp");
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}