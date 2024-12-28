#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <algorithm>
using namespace std;

// TreeNode structure to represent the Decision Tree
struct TreeNode {
    int attribute = -1; // Index of the attribute used for splitting
    unordered_map<int, TreeNode*> children; // Children nodes
    int result = -1; // Final result for a leaf node
    bool isLeaf = false; // Is this node a leaf
};

// Function to calculate entropy
double calculateEntropy(const vector<vector<int>>& data, int targetColumn) {
    unordered_map<int, int> frequency;
    for (const auto& row : data) {
        frequency[row[targetColumn]]++;
    }

    double entropy = 0.0;
    int total = data.size();
    for (const auto& pair : frequency) {
        double probability = static_cast<double>(pair.second) / total;
        entropy -= probability * log2(probability);
    }
    return entropy;
}

// Function to split data based on an attribute
unordered_map<int, vector<vector<int>>> splitData(
    const vector<vector<int>>& data, int attribute) {
    unordered_map<int, vector<vector<int>>> subsets;
    for (const auto& row : data) {
        subsets[row[attribute]].push_back(row);
    }
    return subsets;
}

// Find the best attribute to split data
int findBestAttribute(const vector<vector<int>>& data, int targetColumn, const vector<bool>& usedAttributes) {
    double baseEntropy = calculateEntropy(data, targetColumn);
    int bestAttribute = -1;
    double maxGain = 0;

    for (size_t i = 0; i < data[0].size(); ++i) {
        if (i == targetColumn || usedAttributes[i]) continue;

        auto subsets = splitData(data, i);
        double weightedEntropy = 0.0;
        for (const auto& pair : subsets) {
            double subsetEntropy = calculateEntropy(pair.second, targetColumn);
            weightedEntropy += (static_cast<double>(pair.second.size()) / data.size()) * subsetEntropy;
        }

        double gain = baseEntropy - weightedEntropy;
        if (gain > maxGain) {
            maxGain = gain;
            bestAttribute = i;
        }
    }
    return bestAttribute;
}

// Build the decision tree
TreeNode* buildTree(const vector<vector<int>>& data, int targetColumn, vector<bool> usedAttributes) {
    TreeNode* node = new TreeNode;

    // Check if all results are the same
    unordered_map<int, int> frequency;
    for (const auto& row : data) {
        frequency[row[targetColumn]]++;
    }

    if (frequency.size() == 1) {
        node->isLeaf = true;
        node->result = data[0][targetColumn];
        return node;
    }

    // Find the best attribute to split
    int bestAttribute = findBestAttribute(data, targetColumn, usedAttributes);
    if (bestAttribute == -1) {
        node->isLeaf = true;
        node->result = max_element(frequency.begin(), frequency.end(),
                                   [](const auto& a, const auto& b) { return a.second < b.second; })->first;
        return node;
    }

    node->attribute = bestAttribute;
    usedAttributes[bestAttribute] = true;

    auto subsets = splitData(data, bestAttribute);
    for (const auto& pair : subsets) {
        node->children[pair.first] = buildTree(pair.second, targetColumn, usedAttributes);
    }
    return node;
}

// Predict the result for a given input using the tree
int predict(TreeNode* node, const vector<int>& input) {
    while (!node->isLeaf) {
        int attribute = node->attribute;
        int value = input[attribute];
        if (node->children.find(value) != node->children.end()) {
            node = node->children[value];
        } else {
            cerr << "Input value not found in the decision tree.\n";
            return -1; // Error code
        }
    }
    return node->result;
}

// Utility function to read the dataset
vector<vector<int>> readDataset(const string& filename, unordered_map<int, unordered_map<string, int>>& attributeMappings) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open file " << filename << "\n";
        exit(1);
    }

    vector<vector<int>> data;
    string line;
    int rowNumber = 0;

    while (getline(file, line)) {
        istringstream ss(line);
        string value;
        vector<int> row;

        int colNumber = 0;
        while (ss >> value) {
            if (attributeMappings[colNumber].find(value) == attributeMappings[colNumber].end()) {
                attributeMappings[colNumber][value] = attributeMappings[colNumber].size();
            }
            row.push_back(attributeMappings[colNumber][value]);
            colNumber++;
        }
        data.push_back(row);
        rowNumber++;
    }
    file.close();
    return data;
}

int main() {
    unordered_map<int, unordered_map<string, int>> attributeMappings;
    string filename = "data.txt";

    // Read the dataset
    auto data = readDataset(filename, attributeMappings);

    int targetColumn = data[0].size() - 1; // Target column is the last one
    vector<bool> usedAttributes(data[0].size(), false);

    // Build the decision tree
    TreeNode* root = buildTree(data, targetColumn, usedAttributes);

    cout << "Decision Tree built successfully. Let's make predictions!\n";

    while (true) {
        vector<int> input(data[0].size() - 1);

        // Gather input dynamically for each attribute
        for (size_t i = 0; i < input.size(); ++i) {
            cout << "Enter value for attribute " << i + 1 << ": ";
            cin >> input[i];
        }

        // Predict the result using the input
        int result = predict(root, input);
        if (result != -1) {
            cout << "Predicted result: " << result << "\n";
        } else {
            cout << "Prediction failed. Ensure input values are valid.\n";
        }

        // Check if the user wants to continue
        cout << "Do you want to make another prediction? (y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    return 0;
}
