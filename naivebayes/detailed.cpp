#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>
#include <fstream>
using namespace std;

// Constants for numerical stability and validation
const double EPSILON = 1e-10;
const double PI = 3.14159265358979323846;

// Enum for feature types
enum class FeatureType {
    CATEGORICAL,
    NUMERICAL
};

// Structure to hold feature statistics for numerical features
struct NumericalStats {
    double mean;
    double variance;
    double std_dev;
    
    NumericalStats() : mean(0.0), variance(0.0), std_dev(0.0) {}
};

// Class to handle both categorical and numerical features
class Feature {
private:
    FeatureType type;
    string name;
    map<string, int> categorical_values;  // For categorical features
    NumericalStats numerical_stats;                 // For numerical features
    bool is_initialized;

public:
    Feature(const string& feature_name, FeatureType feature_type) 
        : type(feature_type), name(feature_name), is_initialized(false) {}

    FeatureType getType() const { return type; }
    string getName() const { return name; }
    
    void addCategoricalValue(const string& value) {
        if (type != FeatureType::CATEGORICAL) {
            throw runtime_error("Cannot add categorical value to numerical feature");
        }
        categorical_values[value]++;
    }

    void setNumericalStats(double mean, double variance) {
        if (type != FeatureType::NUMERICAL) {
            throw runtime_error("Cannot set numerical stats for categorical feature");
        }
        numerical_stats.mean = mean;
        numerical_stats.variance = variance;
        numerical_stats.std_dev = sqrt(variance);
        is_initialized = true;
    }

    const map<string, int>& getCategoricalValues() const {
        return categorical_values;
    }

    const NumericalStats& getNumericalStats() const {
        return numerical_stats;
    }
};

// Class to handle dataset operations
class Dataset {
private:
    vector<vector<string>> categorical_data;
    vector<vector<double>> numerical_data;
    vector<string> labels;
    vector<FeatureType> feature_types;
    vector<string> feature_names;

public:
    void addSample(const vector<string>& categorical_features,
                  const vector<double>& numerical_features,
                  const string& label) {
        categorical_data.push_back(categorical_features);
        numerical_data.push_back(numerical_features);
        labels.push_back(label);
    }

    void setFeatureTypes(const vector<FeatureType>& types) {
        feature_types = types;
    }

    void setFeatureNames(const vector<string>& names) {
        feature_names = names;
    }

    // Getters
    const vector<vector<string>>& getCategoricalData() const { return categorical_data; }
    const vector<vector<double>>& getNumericalData() const { return numerical_data; }
    const vector<string>& getLabels() const { return labels; }
    const vector<FeatureType>& getFeatureTypes() const { return feature_types; }
    const vector<string>& getFeatureNames() const { return feature_names; }
    
    size_t size() const { return labels.size(); }
};

class NaiveBayes {
private:
    // Class-related members
    map<string, int> class_counts;
    map<string, double> class_priors;
    int total_samples;

    // Feature-related members
    vector<Feature> features;
    map<string, map<string, map<string, int>>> categorical_counts;
    map<string, vector<NumericalStats>> numerical_stats;

    // Helper functions
    double calculateGaussianProbability(double x, double mean, double std_dev) const {
        double exponent = -0.5 * pow((x - mean) / (std_dev + EPSILON), 2);
        return (1.0 / (std_dev * sqrt(2 * PI))) * exp(exponent);
    }

    void validateFeatures(const vector<string>& categorical_features,
                         const vector<double>& numerical_features) const {
        size_t expected_categorical = count_if(features.begin(), features.end(),
            [](const Feature& f) { return f.getType() == FeatureType::CATEGORICAL; });
        size_t expected_numerical = count_if(features.begin(), features.end(),
            [](const Feature& f) { return f.getType() == FeatureType::NUMERICAL; });

        if (categorical_features.size() != expected_categorical || 
            numerical_features.size() != expected_numerical) {
            throw invalid_argument("Invalid number of features provided");
        }
    }

public:
    NaiveBayes() : total_samples(0) {}

    // Initialize the model with feature information
    void initialize(const vector<pair<string, FeatureType>>& feature_info) {
        features.clear();
        for (const auto& info : feature_info) {
            features.emplace_back(info.first, info.second);
        }
    }

    // Train the model
    void fit(const Dataset& dataset) {
        if (dataset.size() == 0) {
            throw runtime_error("Empty dataset provided for training");
        }

        // Reset counters
        total_samples = 0;
        class_counts.clear();
        class_priors.clear();
        categorical_counts.clear();
        numerical_stats.clear();

        // Calculate class counts and priors
        for (const auto& label : dataset.getLabels()) {
            class_counts[label]++;
            total_samples++;
        }

        for (const auto& count : class_counts) {
            class_priors[count.first] = static_cast<double>(count.second) / total_samples;
        }

        // Process categorical features
        const auto& categorical_data = dataset.getCategoricalData();
        size_t categorical_idx = 0;
        for (size_t i = 0; i < features.size(); i++) {
            if (features[i].getType() == FeatureType::CATEGORICAL) {
                for (size_t j = 0; j < dataset.size(); j++) {
                    const string& value = categorical_data[j][categorical_idx];
                    const string& label = dataset.getLabels()[j];
                    categorical_counts[label][features[i].getName()][value]++;
                }
                categorical_idx++;
            }
        }

        // Process numerical features
        const auto& numerical_data = dataset.getNumericalData();
        for (const auto& class_label : class_counts) {
            vector<NumericalStats> class_stats;
            size_t numerical_idx = 0;

            for (size_t i = 0; i < features.size(); i++) {
                if (features[i].getType() == FeatureType::NUMERICAL) {
                    // Calculate mean
                    double mean = 0.0;
                    int count = 0;
                    for (size_t j = 0; j < dataset.size(); j++) {
                        if (dataset.getLabels()[j] == class_label.first) {
                            mean += numerical_data[j][numerical_idx];
                            count++;
                        }
                    }
                    mean /= count;

                    // Calculate variance
                    double variance = 0.0;
                    for (size_t j = 0; j < dataset.size(); j++) {
                        if (dataset.getLabels()[j] == class_label.first) {
                            double diff = numerical_data[j][numerical_idx] - mean;
                            variance += diff * diff;
                        }
                    }
                    variance /= count;

                    NumericalStats stats;
                    stats.mean = mean;
                    stats.variance = variance;
                    stats.std_dev = sqrt(variance);
                    class_stats.push_back(stats);
                    numerical_idx++;
                }
            }
            numerical_stats[class_label.first] = class_stats;
        }
    }

    // Predict class for new sample
    string predict(const vector<string>& categorical_features,
                       const vector<double>& numerical_features) const {
        validateFeatures(categorical_features, numerical_features);

        string best_class;
        double best_probability = -numeric_limits<double>::infinity();

        // Calculate probability for each class
        for (const auto& class_pair : class_counts) {
            const string& class_label = class_pair.first;
            
            // Start with log of class prior
            double log_probability = log(class_priors.at(class_label));

            // Add log probabilities for categorical features
            size_t categorical_idx = 0;
            size_t numerical_idx = 0;
            
            for (const auto& feature : features) {
                if (feature.getType() == FeatureType::CATEGORICAL) {
                    const string& value = categorical_features[categorical_idx];
                    int count = 0;
                    if (categorical_counts.at(class_label).at(feature.getName()).count(value)) {
                        count = categorical_counts.at(class_label).at(feature.getName()).at(value);
                    }
                    
                    // Apply Laplace smoothing
                    double probability = (count + 1.0) / 
                        (class_counts.at(class_label) + feature.getCategoricalValues().size());
                    log_probability += log(probability);
                    categorical_idx++;
                } else {
                    // Handle numerical features using Gaussian probability
                    const NumericalStats& stats = numerical_stats.at(class_label)[numerical_idx];
                    double probability = calculateGaussianProbability(
                        numerical_features[numerical_idx],
                        stats.mean,
                        stats.std_dev
                    );
                    log_probability += log(probability + EPSILON);
                    numerical_idx++;
                }
            }

            // Update best class if probability is higher
            if (log_probability > best_probability) {
                best_probability = log_probability;
                best_class = class_label;
            }
        }

        return best_class;
    }

    // Get model information
    string getModelInfo() const {
        stringstream ss;
        ss << "Naive Bayes Model Information:\n";
        ss << "Total samples: " << total_samples << "\n\n";
        
        ss << "Class distribution:\n";
        for (const auto& class_pair : class_counts) {
            ss << class_pair.first << ": " << class_pair.second << " samples ("
               << (100.0 * class_pair.second / total_samples) << "%)\n";
        }
        
        ss << "\nFeature Information:\n";
        for (const auto& feature : features) {
            ss << "- " << feature.getName() << " ("
               << (feature.getType() == FeatureType::CATEGORICAL ? "Categorical" : "Numerical")
               << ")\n";
        }
        
        return ss.str();
    }
};

// Example usage
int main() {
    try {
        // Initialize model with feature information
        NaiveBayes classifier;
        vector<pair<string, FeatureType>> feature_info = {
            {"Weather", FeatureType::CATEGORICAL},
            {"Temperature", FeatureType::NUMERICAL},
            {"Humidity", FeatureType::NUMERICAL}
        };
        classifier.initialize(feature_info);

        // Create and populate dataset
        Dataset dataset;
        dataset.setFeatureTypes({FeatureType::CATEGORICAL, FeatureType::NUMERICAL, FeatureType::NUMERICAL});
        dataset.setFeatureNames({"Weather", "Temperature", "Humidity"});

        // Add training samples: {categorical_features}, {numerical_features}, label
        dataset.addSample({"Sunny"}, {85.0, 85.0}, "Don't Play");
        dataset.addSample({"Sunny"}, {80.0, 90.0}, "Don't Play");
        dataset.addSample({"Overcast"}, {83.0, 78.0}, "Play");
        dataset.addSample({"Rain"}, {70.0, 96.0}, "Play");
        dataset.addSample({"Rain"}, {68.0, 80.0}, "Play");
        dataset.addSample({"Rain"}, {65.0, 70.0}, "Don't Play");
        dataset.addSample({"Overcast"}, {64.0, 65.0}, "Play");
        dataset.addSample({"Sunny"}, {72.0, 95.0}, "Don't Play");
        dataset.addSample({"Sunny"}, {69.0, 70.0}, "Play");
        dataset.addSample({"Rain"}, {75.0, 80.0}, "Play");

        // Train the model
        classifier.fit(dataset);

        // Print model information
        cout << classifier.getModelInfo() << endl;

        // Make predictions
        vector<string> categorical_features = {"Sunny"};
        vector<double> numerical_features = {70.0, 75.0};
        
        string prediction = classifier.predict(categorical_features, numerical_features);
        cout << "Prediction for [Sunny, 70.0, 75.0]: " << prediction << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}