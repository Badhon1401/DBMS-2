#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <algorithm>
#include <iterator>

using namespace std;

// Function to read transactions from a file
void readTransactions(const string& filename, vector<vector<string>>& transactions) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string item;
        vector<string> transaction;

        while (getline(ss, item, ' ')) {  // Split by space
            transaction.push_back(item);
        }
        transactions.push_back(transaction);
    }
    file.close();
}

// Function to generate all combinations of a given size
void generateCombinations(const set<string>& currentItems, int size, set<vector<string>>& combinations) {
    vector<string> items(currentItems.begin(), currentItems.end());
    vector<bool> v(items.size());
    fill(v.end() - size, v.end(), true);  // Create a boolean vector for combinations

    do {
        vector<string> combination;
        for (int i = 0; i < items.size(); ++i) {
            if (v[i]) {
                combination.push_back(items[i]);
            }
        }
        combinations.insert(combination);
    } while (prev_permutation(v.begin(), v.end()));  // Generate next combination
}

// Apriori function to find frequent itemsets
set<vector<string>> apriori(const vector<vector<string>>& transactions, int threshold) {
    unordered_map<string, vector<int>> itemMap;
    set<string> items;

    // Build the item map
    for (int i = 0; i < transactions.size(); ++i) {
        for (const auto& item : transactions[i]) {
            itemMap[item].push_back(i);
            items.insert(item);
        }
    }

    // Store frequent items
    set<string> frequentItems;
    for (const auto& item : items) {
        if (itemMap[item].size() >= threshold) {
            frequentItems.insert(item);
        }
    }

    cout << "Initial Frequent Items: ";
    for (const auto& item : frequentItems) {
        cout << item << " ";
    }
    cout << endl;

    set<vector<string>> result;
    int size = 2;  // Start with pairs

    // Find combinations of increasing size
    while (!frequentItems.empty()) {
        set<vector<string>> combinations;
        generateCombinations(frequentItems, size, combinations);  // Generate combinations of current size

        cout << "Checking combinations of size " << size << "..." << endl;

        set<vector<string>> newFrequentItemsets;  // To store newly found frequent itemsets

        for (const auto& combination : combinations) {
            vector<int> transactionIndices;
            // Find the intersection of transaction indices for the combination
            set<int> firstItemIndices(itemMap[combination[0]].begin(), itemMap[combination[0]].end());
            for (size_t i = 1; i < combination.size(); ++i) {
                set<int> currentItemIndices(itemMap[combination[i]].begin(), itemMap[combination[i]].end());
                vector<int> temp;
                set_intersection(firstItemIndices.begin(), firstItemIndices.end(),
                                 currentItemIndices.begin(), currentItemIndices.end(),
                                 back_inserter(temp));
                firstItemIndices = set<int>(temp.begin(), temp.end());  // Update indices
            }

            // Check if the count meets the threshold
            if (firstItemIndices.size() >= threshold) {
                newFrequentItemsets.insert(combination);
            }
        }

        if (newFrequentItemsets.empty()) {
            break;  // No more frequent itemsets found
        }

        result.insert(newFrequentItemsets.begin(), newFrequentItemsets.end());

        cout << "New Frequent Itemsets of size " << size << ": ";
        for (const auto& itemset : newFrequentItemsets) {
            for (const auto& item : itemset) {
                cout << item << " ";
            }
            cout << "| ";
        }
        cout << endl;

        frequentItems.clear();  // Clear and repopulate frequent items based on new itemsets

        for (const auto& itemset : newFrequentItemsets) {
            for (const auto& item : itemset) {
                frequentItems.insert(item);
            }
        }

        size++;  // Increase the size for the next iteration
    }

    return result;
}

int main() {
    vector<vector<string>> transactions;
    readTransactions("input.txt", transactions);

    int threshold = 2;  // Define your threshold
    set<vector<string>> frequentItemsets = apriori(transactions, threshold);

    // Display the result
    cout << "Final Frequent Itemsets: " << endl;
    for (const auto& itemset : frequentItemsets) {
        for (const auto& item : itemset) {
            cout << item << " ";
        }
        cout << endl;
    }

    return 0;
}
