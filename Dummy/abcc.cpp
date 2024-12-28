#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>
#include <functional> // For std::hash

using namespace std;

// Type alias for better readability
using Itemset = set<int>;
using Transaction = vector<Itemset>;

// Custom hash function for std::set<int>
struct ItemsetHash {
    size_t operator()(const Itemset& itemset) const {
        size_t hash = 0;
        for (int item : itemset) {
            hash ^= hash_function(item); // XOR with the hash of each item
        }
        return hash;
    }
    
private:
    hash<int> hash_function; // Using the standard hash for integers
};

// Custom equality function for std::set<int>
struct ItemsetEqual {
    bool operator()(const Itemset& a, const Itemset& b) const {
        return a == b; // Direct comparison works since sets are unordered
    }
};

// Function to generate candidates from frequent itemsets
vector<Itemset> generateCandidates(const vector<Itemset>& prevItemsets) {
    vector<Itemset> candidates;

    for (size_t i = 0; i < prevItemsets.size(); ++i) {
        for (size_t j = i + 1; j < prevItemsets.size(); ++j) {
            Itemset unionSet;
            set_union(prevItemsets[i].begin(), prevItemsets[i].end(),
                       prevItemsets[j].begin(), prevItemsets[j].end(),
                       inserter(unionSet, unionSet.begin()));
            if (unionSet.size() == prevItemsets[i].size() + 1) {
                candidates.push_back(move(unionSet)); // Use move for performance
            }
        }
    }
    return candidates;
}

// Function to create a data cube for counting itemsets
unordered_map<Itemset, int, ItemsetHash, ItemsetEqual> createDataCube(const vector<Itemset>& transactions, 
                                                                       const vector<Itemset>& candidates) {
    unordered_map<Itemset, int, ItemsetHash, ItemsetEqual> dataCube;

    for (const auto& transaction : transactions) {
        for (const auto& candidate : candidates) {
            if (includes(transaction.begin(), transaction.end(), candidate.begin(), candidate.end())) {
                dataCube[candidate]++;
            }
        }
    }
    return dataCube;
}

// Function to filter candidates based on minimum support
vector<Itemset> filterCandidates(const unordered_map<Itemset, int, ItemsetHash, ItemsetEqual>& dataCube, 
                                 int minSupport) {
    vector<Itemset> frequentItemsets;
    for (const auto& pair : dataCube) {
        if (pair.second >= minSupport) {
            frequentItemsets.push_back(move(pair.first)); // Use move for performance
        }
    }
    return frequentItemsets;
}

// Main Apriori algorithm function
vector<Itemset> apriori(const vector<Itemset>& transactions, int minSupport) {
    vector<Itemset> frequentItemsets;
    vector<Itemset> currentCandidates;

    // Generate initial candidates (1-itemsets)
    unordered_map<int, int> initialCount;
    for (const auto& transaction : transactions) {
        for (int item : transaction) {
            initialCount[item]++;
        }
    }

    // Filter initial candidates
    for (const auto& item : initialCount) {
        if (item.second >= minSupport) {
            frequentItemsets.push_back({item.first});
        }
    }

    currentCandidates = frequentItemsets;

    // Iteratively generate frequent itemsets
    int k = 2; // Starting from size 2
    while (!currentCandidates.empty()) {
        vector<Itemset> newCandidates = generateCandidates(currentCandidates);
        auto dataCube = createDataCube(transactions, newCandidates);
        currentCandidates = filterCandidates(dataCube, minSupport);

        // Add new frequent itemsets to the overall list
        frequentItemsets.insert(frequentItemsets.end(), currentCandidates.begin(), currentCandidates.end());
        
        k++;
    }

    return frequentItemsets;
}

// Utility function to print frequent itemsets
void printFrequentItemsets(const vector<Itemset>& frequentItemsets) {
    cout << "Frequent Itemsets:\n";
    for (const auto& itemset : frequentItemsets) {
        cout << "{ ";
        for (int item : itemset) {
            cout << item << " ";
        }
        cout << "}\n";
    }
}

int main() {
    // Sample transactions
    vector<Itemset> transactions = {
        {1, 2, 3},
        {2, 3, 4},
        {1, 2},
        {1, 3},
        {2, 3},
        {1, 2, 3}
    };

    int minSupport = 4;

    // Run Apriori algorithm using data cube method
    vector<Itemset> frequentItemsets = apriori(transactions, minSupport);

    // Print frequent itemsets
    printFrequentItemsets(frequentItemsets);

    return 0;
}
