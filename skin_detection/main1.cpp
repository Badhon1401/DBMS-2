#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <algorithm>

using namespace std;

struct Pixel {
    uint8_t r, g, b;
};

struct ColorHash {
    template <class T1, class T2, class T3>
    std::size_t operator()(const std::tuple<T1, T2, T3>& t) const {
        auto h1 = std::hash<T1>{}(std::get<0>(t));
        auto h2 = std::hash<T2>{}(std::get<1>(t));
        auto h3 = std::hash<T3>{}(std::get<2>(t));
        return h1 ^ h2 ^ h3;
    }
};

class ImageProcessor {
public:
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

private:
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
};

class SkinDetector {
public:
    void processDataset(const string& maskDir, const string& imageDir, int totalImages) {
        unordered_map<std::tuple<int,int,int>, int, ColorHash> skin_rgb_cnt;
        unordered_map<std::tuple<int,int,int>, int, ColorHash> non_skin_rgb_cnt;
        
        int total_skin_color = 0;
        int total_non_skin_color = 0;

        for (int index = 0; index < totalImages; ++index) {
            string index_str = formatIndex(index);
            string mask_path = maskDir + "/" + index_str + ".bmp";
            string image_path = imageDir + "/" + index_str + ".jpg";

            auto mask_pixels = ImageProcessor::readImage(mask_path);
            auto image_pixels = ImageProcessor::readImage(image_path);

            if (!validateImageDimensions(mask_pixels, image_pixels)) {
                cerr << "Image dimensions mismatch for image " << index_str << endl;
                continue;
            }

            processImagePixels(mask_pixels, image_pixels, skin_rgb_cnt, non_skin_rgb_cnt, 
                               total_skin_color, total_non_skin_color);

            cout << "Image " << index << " - DONE!" << endl;
        }

        generateProbabilityFile(skin_rgb_cnt, non_skin_rgb_cnt, total_skin_color, total_non_skin_color);
    }

    void detectSkinInImage(const string& inputImage, const string& outputImage, double threshold = 0.4) {
        vector<vector<vector<double>>> trained_value = loadProbabilityValues();

        auto new_img_pixels = ImageProcessor::readImage(inputImage);

        for (auto& row : new_img_pixels) {
            for (auto& pixel : row) {
                int red = pixel.r;
                int green = pixel.g;
                int blue = pixel.b;

                if (trained_value[red][green][blue] <= threshold) {
                    pixel.r = pixel.g = pixel.b = 255;
                }
            }
        }

        ImageProcessor::writeBMP(outputImage, new_img_pixels);
        cout << "Skin detection complete." << endl;
    }

private:
    string formatIndex(int index) {
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << index;
        return oss.str();
    }

    bool validateImageDimensions(const vector<vector<Pixel>>& mask_pixels, 
                                  const vector<vector<Pixel>>& image_pixels) {
        return mask_pixels.size() == image_pixels.size() && 
               (mask_pixels.empty() || mask_pixels[0].size() == image_pixels[0].size());
    }

    void processImagePixels(const vector<vector<Pixel>>& mask_pixels,
                             const vector<vector<Pixel>>& image_pixels,
                             unordered_map<std::tuple<int,int,int>, int, ColorHash>& skin_rgb_cnt,
                             unordered_map<std::tuple<int,int,int>, int, ColorHash>& non_skin_rgb_cnt,
                             int& total_skin_color,
                             int& total_non_skin_color) {
        for (size_t x = 0; x < image_pixels.size(); ++x) {
            for (size_t y = 0; y < image_pixels[x].size(); ++y) {
                Pixel mask_pixel = mask_pixels[x][y];
                Pixel image_pixel = image_pixels[x][y];

                auto color_key = std::make_tuple(image_pixel.r, image_pixel.g, image_pixel.b);

                if (mask_pixel.b > 250 && mask_pixel.g > 250 && mask_pixel.r > 250) {
                    non_skin_rgb_cnt[color_key]++;
                    total_non_skin_color++;
                } else {
                    skin_rgb_cnt[color_key]++;
                    total_skin_color++;
                }
            }
        }
    }

    void generateProbabilityFile(const unordered_map<std::tuple<int,int,int>, int, ColorHash>& skin_rgb_cnt,
                                  const unordered_map<std::tuple<int,int,int>, int, ColorHash>& non_skin_rgb_cnt,
                                  int total_skin_color,
                                  int total_non_skin_color) {
        ofstream output_file("output.txt");

        vector<vector<vector<double>>> probability_matrix(
            256, vector<vector<double>>(256, vector<double>(256, 0.0))
        );

        for (int r = 0; r < 256; ++r) {
            for (int g = 0; g < 256; ++g) {
                for (int b = 0; b < 256; ++b) {
                    auto color_key = std::make_tuple(r, g, b);
                    
                    auto skin_it = skin_rgb_cnt.find(color_key);
                    auto non_skin_it = non_skin_rgb_cnt.find(color_key);

                    double skin_prob = (skin_it != skin_rgb_cnt.end()) ? 
                        static_cast<double>(skin_it->second) / total_skin_color : 0.0;
                    
                    double non_skin_prob = (non_skin_it != non_skin_rgb_cnt.end()) ? 
                        static_cast<double>(non_skin_it->second) / total_non_skin_color : 0.0;

                    double threshold = (non_skin_prob != 0) ? (skin_prob / non_skin_prob) : 0.0;
                    
                    probability_matrix[r][g][b] = threshold;
                    output_file << threshold << endl;
                }
            }
        }
        output_file.close();
    }

    vector<vector<vector<double>>> loadProbabilityValues() {
        vector<vector<vector<double>>> trained_value(
            256, vector<vector<double>>(256, vector<double>(256, 0.0))
        );

        ifstream input_file("output.txt");
        for (int i = 0; i < 256; ++i) {
            for (int j = 0; j < 256; ++j) {
                for (int k = 0; k < 256; ++k) {
                    input_file >> trained_value[i][j][k];
                }
            }
        }
        input_file.close();

        return trained_value;
    }
};

int main() {
    try {
        SkinDetector detector;
        detector.processDataset("Mask", "ibtd", 555);
        detector.detectSkinInImage("AMla.jpg", "final4.bmp");
    }
    catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}