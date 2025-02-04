#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <cmath>
using namespace std;
#pragma pack(1) // Ensure no padding between struct members

// BMP Header (14 bytes)
struct BMPHeader
{
    char signature[2]; // BM
    int fileSize;
    short reserved1;
    short reserved2;
    int pixelDataOffset;
};

// DIB Header (40 bytes for BITMAPINFOHEADER)
struct DIBHeader
{
    int headerSize;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    int colorsUsed;
    int importantColors;
};

void splitBMP(const string &filename, int tileWidth, int tileHeight)
{
    ifstream file(filename, ios::binary);

    if (!file)
    {
        cerr << "Cannot open BMP file: " << filename << endl;
        return;
    }

    // Read BMP Header
    BMPHeader bmpHeader;
    file.read(reinterpret_cast<char *>(&bmpHeader), sizeof(BMPHeader));

    // Read DIB Header
    DIBHeader dibHeader;
    file.read(reinterpret_cast<char *>(&dibHeader), sizeof(DIBHeader));

    int width = dibHeader.width;
    int height = dibHeader.height;
    int bytesPerPixel = dibHeader.bitsPerPixel / 8;
    int rowSize = (width * bytesPerPixel + 3) & ~3; // Row size is padded to 4-byte alignment

    // Read Pixel Data
    vector<unsigned char> pixelData(height * rowSize);
    file.seekg(bmpHeader.pixelDataOffset, ios::beg);
    file.read(reinterpret_cast<char *>(pixelData.data()), pixelData.size());
    file.close();

    cout << "BMP Loaded: " << width << "x" << height << " pixels, " << bytesPerPixel << " bytes per pixel.\n";

    // Calculate number of tiles
    int tilesX = width / tileWidth;
    int tilesY = height / tileHeight;

    for (int ty = 0; ty < tilesY; ++ty)
    {
        for (int tx = 0; tx < tilesX; ++tx)
        {
            string outputFilename = "tile_" + to_string(tx) + "_" + to_string(ty) + ".bmp";
            ofstream outFile(outputFilename, ios::binary);

            if (!outFile)
            {
                cerr << "Cannot create file: " << outputFilename << endl;
                continue;
            }

            // Write Headers
            outFile.write(reinterpret_cast<char *>(&bmpHeader), sizeof(BMPHeader));
            outFile.write(reinterpret_cast<char *>(&dibHeader), sizeof(DIBHeader));

            // Write Tile Pixel Data
            for (int y = 0; y < tileHeight; ++y)
            {
                int srcRow = (ty * tileHeight + y);
                int srcOffset = srcRow * rowSize + (tx * tileWidth * bytesPerPixel);
                vector<unsigned char> tileData(tileWidth * bytesPerPixel);
                file.seekg(bmpHeader.pixelDataOffset + srcOffset);
                file.read(reinterpret_cast<char *>(tileData.data()), tileWidth * bytesPerPixel);
                outFile.write(reinterpret_cast<char *>(tileData.data()), tileWidth * bytesPerPixel);
            }

            outFile.close();
            cout << "Created tile: " << outputFilename << endl;
        }
    }
}
// Namespace for better organization and potential future expansion
namespace SkinDetection
{

    // Color quantization to reduce memory footprint
    class ColorQuantizer
    {
    public:
        // Quantize color to reduce unique color representations
        static tuple<uint8_t, uint8_t, uint8_t> quantizeColor(
            uint8_t r, uint8_t g, uint8_t b, int bits = 3)
        {
            return {
                static_cast<uint8_t>(r & ~((1 << bits) - 1)),
                static_cast<uint8_t>(g & ~((1 << bits) - 1)),
                static_cast<uint8_t>(b & ~((1 << bits) - 1))};
        }
    };

    // Enhanced pixel representation with color space conversion capabilities
    struct EnhancedPixel
    {
        uint8_t r, g, b;

        // Convert RGB to HSV color space for more robust skin detection
        tuple<double, double, double> toHSV() const
        {
            double r_norm = r / 255.0;
            double g_norm = g / 255.0;
            double b_norm = b / 255.0;

            double cmax = max({r_norm, g_norm, b_norm});
            double cmin = min({r_norm, g_norm, b_norm});
            double diff = cmax - cmin;

            double hue = 0.0, sat = 0.0, val = cmax;

            if (diff != 0.0)
            {
                sat = diff / cmax;

                if (cmax == r_norm)
                    hue = (g_norm - b_norm) / diff;
                else if (cmax == g_norm)
                    hue = 2.0 + (b_norm - r_norm) / diff;
                else
                    hue = 4.0 + (r_norm - g_norm) / diff;

                hue *= 60.0;
                if (hue < 0.0)
                    hue += 360.0;
            }

            return {hue, sat, val};
        }
    };

    // Custom hash function for quantized colors
    struct QuantizedColorHash
    {
        size_t operator()(const tuple<uint8_t, uint8_t, uint8_t> &color) const
        {
            uint8_t r = get<0>(color);
            uint8_t g = get<1>(color);
            uint8_t b = get<2>(color);
            return (r << 16) | (g << 8) | b;
        }
    };

    class ImageProcessor
    {
    public:
        // Advanced image reading with multiple format support
        static vector<vector<EnhancedPixel>> readImage(const string &filename)
        {
            string ext = getFileExtension(filename);

            if (ext == "bmp")
                return readBMP(filename);
            if (ext == "jpg" || ext == "jpeg")
                return readJPG(filename);

            throw runtime_error("Unsupported image format: " + ext);
        }

    public:
        // Static method for writing BMP
        static void writeBMP(const string &filename,
                             const vector<vector<EnhancedPixel>> &pixels)
        {
            ofstream file(filename, ios::binary);
            if (!file)
            {
                throw runtime_error("Cannot open BMP file: " + filename);
            }

            int width = pixels[0].size();
            int height = pixels.size();
            int padding = (4 - (width * 3) % 4) % 4;

            uint8_t header[54] = {
                0x42, 0x4D,  // Signature
                0, 0, 0, 0,  // Image file size in bytes
                0, 0, 0, 0,  // Reserved
                54, 0, 0, 0, // Start of pixel array
                40, 0, 0, 0, // Info header size
                0, 0, 0, 0,  // Image width
                0, 0, 0, 0,  // Image height
                1, 0, 24, 0, // Number of color planes and bits per pixel
                0, 0, 0, 0,  // Compression
                0, 0, 0, 0,  // Image size (can be 0 for uncompressed)
                0, 0, 0, 0,  // Horizontal resolution (pixels per meter)
                0, 0, 0, 0,  // Vertical resolution (pixels per meter)
                0, 0, 0, 0,  // Number of colors in color table
                0, 0, 0, 0   // Important color count
            };

            int fileSize = 54 + (3 * width + padding) * height;
            header[2] = fileSize;
            header[3] = fileSize >> 8;
            header[4] = fileSize >> 16;
            header[5] = fileSize >> 24;

            header[18] = width;
            header[19] = width >> 8;
            header[20] = width >> 16;
            header[21] = width >> 24;

            header[22] = height;
            header[23] = height >> 8;
            header[24] = height >> 16;
            header[25] = height >> 24;

            file.write(reinterpret_cast<char *>(header), 54);

            for (int y = height - 1; y >= 0; --y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const EnhancedPixel &pixel = pixels[y][x];
                    file.write(reinterpret_cast<const char *>(&pixel.b), 1);
                    file.write(reinterpret_cast<const char *>(&pixel.g), 1);
                    file.write(reinterpret_cast<const char *>(&pixel.r), 1);
                }
                file.write("\0\0\0", padding);
            }
        }

    private:
        // Detailed BMP reading with comprehensive error handling
        static vector<vector<EnhancedPixel>> readBMP(const string &filename)
        {
            ifstream file(filename, ios::binary);
            if (!file)
            {
                throw runtime_error("Cannot open BMP file: " + filename);
            }

            // Robust BMP header parsing
            uint8_t header[54];
            file.read(reinterpret_cast<char *>(header), 54);

            int width = *(int *)&header[18];
            int height = *(int *)&header[22];

            if (width <= 0 || height <= 0)
            {
                throw runtime_error("Invalid BMP dimensions");
            }

            int padding = (4 - (width * 3) % 4) % 4;
            vector<vector<EnhancedPixel>> pixels(height, vector<EnhancedPixel>(width));

            for (int y = height - 1; y >= 0; --y)
            {
                for (int x = 0; x < width; ++x)
                {
                    EnhancedPixel pixel;
                    file.read(reinterpret_cast<char *>(&pixel.b), 1);
                    file.read(reinterpret_cast<char *>(&pixel.g), 1);
                    file.read(reinterpret_cast<char *>(&pixel.r), 1);
                    pixels[y][x] = pixel;
                }
                file.seekg(padding, ios::cur);
            }

            return pixels;
        }

        // Advanced JPG reading with more robust parsing
        static vector<vector<EnhancedPixel>> readJPG(const string &filename)
        {
            ifstream file(filename, ios::binary);
            if (!file)
            {
                throw runtime_error("Cannot open JPG file: " + filename);
            }

            // Advanced JPG header validation
            uint8_t marker[2];
            file.read(reinterpret_cast<char *>(marker), 2);
            if (marker[0] != 0xFF || marker[1] != 0xD8)
            {
                throw runtime_error("Invalid JPG header");
            }

            int width = 0, height = 0;
            while (true)
            {
                file.read(reinterpret_cast<char *>(marker), 2);

                if (file.eof())
                    break;

                if (marker[0] != 0xFF)
                    continue;

                if (marker[1] == 0xC0 || marker[1] == 0xC2)
                {
                    uint16_t length;
                    file.read(reinterpret_cast<char *>(&length), 2);
                    length = (length << 8) | (length >> 8);

                    uint8_t precision;
                    file.read(reinterpret_cast<char *>(&precision), 1);

                    uint16_t h, w;
                    file.read(reinterpret_cast<char *>(&h), 2);
                    file.read(reinterpret_cast<char *>(&w), 2);

                    height = (h << 8) | (h >> 8);
                    width = (w << 8) | (w >> 8);
                    break;
                }
            }

            if (width == 0 || height == 0)
            {
                throw runtime_error("Could not determine image dimensions");
            }

            // Reset file and read pixels
            file.seekg(0, ios::beg);
            vector<vector<EnhancedPixel>> pixels(height, vector<EnhancedPixel>(width));

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    EnhancedPixel pixel;
                    file.read(reinterpret_cast<char *>(&pixel.r), 1);
                    file.read(reinterpret_cast<char *>(&pixel.g), 1);
                    file.read(reinterpret_cast<char *>(&pixel.b), 1);
                    pixels[y][x] = pixel;
                }
            }

            return pixels;
        }

        // Utility function to extract file extension
        static string getFileExtension(const string &filename)
        {
            size_t dot_pos = filename.find_last_of('.');
            if (dot_pos == string::npos)
                return "";

            string ext = filename.substr(dot_pos + 1);
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            return ext;
        }
    };

    class SkinDetector
    {
    public:
        // Memory-optimized probability tracking
        struct ColorProbability
        {
            uint64_t skin_count = 0;
            uint64_t non_skin_count = 0;
            double probability = 0.0;
        };

        // Advanced dataset processing method
        void processDataset(const string &maskDir,
                            const string &imageDir,
                            int totalImages,
                            int batchSize = 1)
        {
            for (int start = 0; start < totalImages; start += batchSize)
            {
                int end = min(start + batchSize, totalImages);
                processBatch(maskDir, imageDir, start, end, 4, 4); // Assuming tileWidth and tileHeight are 8

                // Periodic memory management
                if (start % (batchSize * 2) == 0)
                {
                    saveIntermediateProbabilities();
                    color_probabilities.clear();
                }
            }

            calculateProbabilities("skin_probabilities.txt");
        }

        // Enhanced skin detection with multiple strategies
        void detectSkin(const string &inputImage,
                        const string &outputImage,
                        double threshold = 0.4)
        {
            auto image_pixels = ImageProcessor::readImage(inputImage);

            for (auto &row : image_pixels)
            {
                for (auto &pixel : row)
                {
                    // Use quantized color for efficient lookup
                    auto color_key = ColorQuantizer::quantizeColor(pixel.r, pixel.g, pixel.b);

                    // Probability-based skin detection with HSV enhancement
                    if (color_probabilities.count(color_key))
                    {
                        double hue, sat, val;
                        tie(hue, sat, val) = pixel.toHSV();

                        // Advanced skin tone criteria
                        bool isSkinTone = (color_probabilities[color_key].probability <= threshold &&
                                           (hue >= 0 && hue <= 50) &&    // Typical skin hue range
                                           (sat >= 0.1 && sat <= 0.6) && // Skin saturation
                                           (val >= 0.2 && val <= 0.9)    // Skin value/brightness
                        );

                        if (!isSkinTone)
                        {
                            pixel.r = pixel.g = pixel.b = 255; // White out non-skin
                        }
                    }
                }
            }

            // Write processed image
            ImageProcessor::writeBMP(outputImage, image_pixels);
        }

    private:
        // Color probability tracking with quantization
        unordered_map<
            tuple<uint8_t, uint8_t, uint8_t>,
            ColorProbability,
            QuantizedColorHash>
            color_probabilities;

        // Batch processing with enhanced error handling
        void processBatch(const string &maskDir,
                  const string &imageDir,
                  int start, int end,
                  int tileWidth, int tileHeight)
{
    for (int index = start; index < end; ++index)
    {
        string index_str = formatIndex(index);
        string mask_path = maskDir + "/" + index_str + ".bmp";
        string image_path = imageDir + "/" + index_str + ".jpg";

        cout << "Processing:\n"
                  << "Mask: " << mask_path << "\n"
                  << "Image: " << image_path << endl;

        try
        {
            // Read the original image once
            auto original_image_pixels = ImageProcessor::readImage(image_path);
            int image_height = original_image_pixels.size();
            int image_width = original_image_pixels[0].size();

            // Split mask into tiles (creates tile files)
            splitBMP(mask_path, tileWidth, tileHeight);

            // Process each tile
            auto mask_pixels = ImageProcessor::readImage(mask_path);
            int maskHeight = mask_pixels.size();
            int maskWidth = mask_pixels[0].size();
            int tilesX = maskWidth / tileWidth;
            int tilesY = maskHeight / tileHeight;

            for (int ty = 0; ty < tilesY; ++ty)
            {
                for (int tx = 0; tx < tilesX; ++tx)
                {
                    string tile_mask_path = "tile_" + to_string(tx) + "_" + to_string(ty) + ".bmp";
                    auto mask_tile = ImageProcessor::readImage(tile_mask_path);

                    // Calculate tile region in original image
                    int startX = tx * tileWidth;
                    int startY = ty * tileHeight;

                    // Extract image tile from original image data
                    vector<vector<EnhancedPixel>> image_tile(tileHeight, vector<EnhancedPixel>(tileWidth));
                    for (int y = 0; y < tileHeight; ++y)
                    {
                        for (int x = 0; x < tileWidth; ++x)
                        {
                            if (startY + y < image_height && startX + x < image_width)
                            {
                                image_tile[y][x] = original_image_pixels[startY + y][startX + x];
                            }
                        }
                    }

                    // Update probabilities using the mask tile and image tile
                    updateColorProbabilities(mask_tile, image_tile);

                    // Cleanup to free memory
                    mask_tile.clear();
                    mask_tile.shrink_to_fit();
                    image_tile.clear();
                    image_tile.shrink_to_fit();
                }
            }

            // Cleanup original image data
            original_image_pixels.clear();
            original_image_pixels.shrink_to_fit();
        }
        catch (const bad_alloc &e)
        {
            cerr << "Memory error: " << e.what() << "\nReduce batch size or image dimensions." << endl;
        }
        catch (const exception &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
}

        // Color probability update with advanced tracking
        void updateColorProbabilities(
            const vector<vector<EnhancedPixel>> &mask_pixels,
            const vector<vector<EnhancedPixel>> &image_pixels)
        {

            for (size_t y = 0; y < image_pixels.size(); ++y)
            {
                for (size_t x = 0; x < image_pixels[y].size(); ++x)
                {
                    const auto &mask_pixel = mask_pixels[y][x];
                    const auto &image_pixel = image_pixels[y][x];

                    // Quantize color for memory efficiency
                    auto color_key = ColorQuantizer::quantizeColor(
                        image_pixel.r, image_pixel.g, image_pixel.b);

                    // Advanced mask classification
                    if (mask_pixel.r > 250 &&
                        mask_pixel.g > 250 &&
                        mask_pixel.b > 250)
                    {
                        color_probabilities[color_key].non_skin_count++;
                    }
                    else
                    {
                        color_probabilities[color_key].skin_count++;
                    }
                }
            }
        }

        // Probability calculation with advanced statistical methods
        void calculateProbabilities(const string &output_file)
        {
            ofstream file(output_file);
            if (!file)
            {
                throw runtime_error("Cannot create output file");
            }

            uint64_t total_skin_pixels = 0;
            uint64_t total_non_skin_pixels = 0;

            // First pass: calculate totals
            for (const auto &entry : color_probabilities)
            {
                const auto &color = entry.first;
                const auto &prob = entry.second;
                total_skin_pixels += prob.skin_count;
                total_non_skin_pixels += prob.non_skin_count;
            }

            // Bayesian probability calculation
            for (auto &entry : color_probabilities)
            {
                auto &color = entry.first;
                auto &prob = entry.second;
                double skin_prob = static_cast<double>(prob.skin_count) / total_skin_pixels;
                double non_skin_prob = static_cast<double>(prob.non_skin_count) / total_non_skin_pixels;

                // Advanced probability ratio with bias correction
                prob.probability = (non_skin_prob > 0)
                                       ? (skin_prob / (skin_prob + non_skin_prob))
                                       : 0.0;

                file << prob.probability << endl;
            }
        }

        // Utility function to save intermediate results
        void saveIntermediateProbabilities()
        {
            ofstream file("intermediate_probabilities.txt");
            for (auto it = color_probabilities.begin(); it != color_probabilities.end(); ++it)
            {
                const auto &color = it->first;
                auto &prob = it->second;
                file << get<0>(color) << ","
                     << get<1>(color) << ","
                     << get<2, uint8_t, uint8_t, uint8_t>(color) << ","
                     << prob.probability << endl;
            }
        }

        // Utility function for consistent index formatting
        static string formatIndex(int index)
        {
            ostringstream oss;
            oss << setw(4) << setfill('0') << index;
            return oss.str();
        }

    public:
        // Static method for writing BMP (could be moved to ImageProcessor)
        static void writeBMP(const string &filename, const vector<vector<EnhancedPixel>> &pixels)
        {
            ofstream file(filename, ios::binary);
            if (!file)
            {
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
                0, 0, 0, 0};

            *(int *)&header[18] = width;
            *(int *)&header[22] = height;

            file.write(reinterpret_cast<char *>(header), 54);

            for (int y = height - 1; y >= 0; --y)
            {
                for (int x = 0; x < width; ++x)
                {
                    file.write(reinterpret_cast<const char *>(&pixels[y][x].b), 1);
                    file.write(reinterpret_cast<const char *>(&pixels[y][x].g), 1);
                    file.write(reinterpret_cast<const char *>(&pixels[y][x].r), 1);
                }
                for (int p = 0; p < padding; ++p)
                {
                    char pad = 0;
                    file.write(&pad, 1);
                }
            }
        }
    };

} // namespace SkinDetection

int main()
{
    try
    {
        SkinDetection::SkinDetector detector;

        // Process dataset with enhanced memory management
        detector.processDataset("Mask", "ibtd", 2, 1);

        // Detect skin in a specific image with advanced techniques
        detector.detectSkin("AMla.jpg", "final4.bmp");
    } // end of try block
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
} // end of main function
