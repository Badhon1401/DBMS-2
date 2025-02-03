#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <random>
#include <sstream>
using namespace std;

class NaiveBayesClassifier {
private:
    unordered_map<int, double> classes;
    unordered_map<int, unordered_map<int, double>> attributesPerClass;
    unordered_map<int, double> totalWordsPerClass;  // Store total words per class
    int totalUniqueAttributes = 0;
    
public:
    NaiveBayesClassifier(vector<vector<int>>& data) {
        // First pass: count classes and attributes
        set<int> uniqueAttributes;
        
        // Initialize counters
        for (auto entry : data) {
            if (classes.find(entry[0]) == classes.end()) {
                classes[entry[0]] = 1;
                attributesPerClass[entry[0]] = {};
                totalWordsPerClass[entry[0]] = 0;
            } else {
                classes[entry[0]] += 1;
            }
            
            // Count words in each class
            for (int k = 1; k < entry.size(); k++) {
                uniqueAttributes.insert(entry[k]);
                if (attributesPerClass[entry[0]].find(entry[k]) == attributesPerClass[entry[0]].end()) {
                    attributesPerClass[entry[0]][entry[k]] = 1;
                } else {
                    attributesPerClass[entry[0]][entry[k]] += 1;
                }
                totalWordsPerClass[entry[0]] += 1;
            }
        }
        
        totalUniqueAttributes = uniqueAttributes.size();
        
        // Calculate probabilities with proper Laplace smoothing
        for (auto& cls : classes) {
            int classLabel = cls.first;
            
            // Convert class counts to probabilities
            classes[classLabel] /= data.size();
            
            // Convert word counts to probabilities with Laplace smoothing
            for (auto& word : attributesPerClass[classLabel]) {
                // Use Laplace smoothing: (count + 1) / (total + |V|)
                word.second = (word.second + 1.0) / 
                            (totalWordsPerClass[classLabel] + totalUniqueAttributes);
            }
        }
    }
    
    int predict(vector<int> attributes) {
        if (attributes.empty()) {
            cout << "No valid words found in input" << endl;
            return -1;  // Return -1 for empty input
        }
        
        double bestScore = -numeric_limits<double>::infinity();
        int bestClass = -1;
        
        cout << "\nProbabilities for each class:" << endl;
        
        for (auto& cls : classes) {
            int classLabel = cls.first;
            // Start with log of class prior probability
            double score = log(cls.second);
            cout << "Class " << classLabel << " initial log probability: " << score << endl;
            
            int wordsFound = 0;
            // Process each word
            for (int word : attributes) {
                if (attributesPerClass[classLabel].find(word) != attributesPerClass[classLabel].end()) {
                    // Word found in this class
                    double wordProb = attributesPerClass[classLabel][word];
                    score += log(wordProb);
                    cout << "Word ID " << word << " found, prob: " << wordProb << endl;
                    wordsFound++;
                } else {
                    // Word not found - use Laplace smoothing
                    double smoothedProb = 1.0 / (totalWordsPerClass[classLabel] + totalUniqueAttributes);
                    score += log(smoothedProb);
                    cout << "Word ID " << word << " not found, smoothed prob: " << smoothedProb << endl;
                }
            }
            
            // Penalize classes where we found fewer matching words
            if (wordsFound == 0) {
                score -= 10;  // Significant penalty for no matches
            }
            
            cout << "Final log probability for class " << classLabel << ": " << score << endl;
            
            if (score > bestScore) {
                bestScore = score;
                bestClass = classLabel;
            }
        }
        
        cout << "\nFinal prediction: " << bestClass << " with log probability: " << bestScore << endl;
        return bestClass;
    }
};

void populateData(vector<vector<int>> &data, unordered_map<string, int> &classmap, unordered_map<string, int> &attrimap,
                  const string &filename, const string &classLabel) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string word;
        vector<int> apair = {classmap[classLabel]};
        while (iss >> word) {
            if (attrimap.find(word) == attrimap.end()) {
                int newIndex = attrimap.size();
                attrimap[word] = newIndex;
            }
            apair.push_back(attrimap[word]);
        }
        data.push_back(apair);
    }
}

// Add this function after populateData but before main()
void evaluateModel(NaiveBayesClassifier& model, 
                  unordered_map<string, int>& classmap,
                  unordered_map<string, int>& attrimap) {
    vector<vector<int>> testData;
    
    // Load test data
    populateData(testData, classmap, attrimap, "good.test.txt", "good");
    populateData(testData, classmap, attrimap, "bad.test.txt", "bad");
    
    int correct = 0;
    int total = 0;
    
    // Test each example
    for (const auto& example : testData) {
        // First element is the true class
        int trueClass = example[0];
        
        // Rest are the attributes
        vector<int> attributes(example.begin() + 1, example.end());
        
        // Get prediction
        int predicted = model.predict(attributes);
        
        if (predicted == trueClass) {
            correct++;
        }
        total++;
        
        // Print prediction details
        // cout << "True class: " << trueClass 
        //      << " Predicted: " << predicted 
        //      << " " << (predicted == trueClass ? "CORRECT" : "WRONG") 
        //      << endl;
    }
    
    // Print accuracy
    double accuracy = (double)correct / total;
    cout << "\nTest Results:" << endl;
    cout << "Total examples: " << total << endl;
    cout << "Correct predictions: " << correct << endl;
    cout << "Accuracy: " << (accuracy * 100) << "%" << endl;
}
// Add this function before main()
void testModelInConsole(NaiveBayesClassifier& model, unordered_map<string, int>& attrimap) {
    string input;
    cout << "\nEnter text to classify (type 'quit' to exit):" << endl;
    
    while (true) {
        cout << "\n> ";
        getline(cin, input);
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        // Convert input string to vector of attributes
        istringstream iss(input);
        string word;
        vector<int> attributes;
        
        while (iss >> word) {
            // Check if word exists in attribute map
            if (attrimap.find(word) != attrimap.end()) {
                attributes.push_back(attrimap[word]);
            }
            // If word not in training data, just skip it
        }
        
        // Make prediction
        int result = model.predict(attributes);
        cout << "Prediction: " << (result == 0 ? "GOOD" : "BAD") << endl;
    }
    
    cout << "Goodbye!" << endl;
}
// Replace the end of main() with this:
int main() {
    unordered_map<string, int> classmap = {{"good", 0}, {"bad", 1}};
    unordered_map<string, int> attrimap;
    vector<vector<int>> data;

    // Populate training data
    populateData(data, classmap, attrimap, "good.txt", "good");
    populateData(data, classmap, attrimap, "bad.txt", "bad");

    // Shuffle the training data
    random_device rd;
    mt19937 g(rd());
    shuffle(data.begin(), data.end(), g);

    // Train model
    NaiveBayesClassifier mymodel(data);
    
    // Evaluate on test data
    cout << "\nEvaluating model on test data..." << endl;
 //   evaluateModel(mymodel, classmap, attrimap);
    testModelInConsole(mymodel, attrimap);

    return 0;
}