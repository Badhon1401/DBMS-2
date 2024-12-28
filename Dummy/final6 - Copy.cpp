#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
using namespace std;

// Custom hash function for std::set<int>
struct ItemsetHash
{
    hash<int> hash_function;

    // Overloaded operator to compute hash
    int operator()(const set<int> &itemset) const
    {
        int hash = 0;
        for (int item : itemset)
        {
            hash ^= hash_function(item); // Use the member variable for hashing
        }
        return hash;
    }
};

// Custom equality function for std::set<int>
struct ItemsetEqual
{
    bool operator()(const set<int> &a, const set<int> &b) const
    {
        if (a.size() != b.size())
            return false;
        auto it1 = a.begin();
        auto it2 = b.begin();
        while (it1 != a.end())
        {
            if (*it1 != *it2)
                return false;
            ++it1;
            ++it2;
        }
        return true;
    }
};

// Manual union function to merge two sets
set<int> mergeSets(const set<int> &a, const set<int> &b)
{
    set<int> unionSet = a;
    for (int item : b)
    {
        unionSet.insert(item); // Insert each item from b into unionSet
    }
    return unionSet;
}

// Function to generate candidates from frequent itemsets
vector<set<int>> generateCandidates(const vector<set<int>> &prevItemsets)
{
    unordered_set<set<int>, ItemsetHash, ItemsetEqual> uniqueCandidates;

    for (int i = 0; i < prevItemsets.size(); ++i)
    {
        for (int j = i + 1; j < prevItemsets.size(); ++j)
        {
            set<int> unionSet = mergeSets(prevItemsets[i], prevItemsets[j]);

            if (unionSet.size() == prevItemsets[i].size() + 1)
            {
                uniqueCandidates.insert(move(unionSet)); // Add to unique set
            }
        }
    }

    return vector<set<int>>(uniqueCandidates.begin(), uniqueCandidates.end());
}

// Manual subset check to see if candidate is a subset of transactionSet
bool isSubset(const set<int> &transactionSet, const set<int> &candidate)
{
    for (int item : candidate)
    {
        if (transactionSet.find(item) == transactionSet.end())
            return false;
    }
    return true;
}

// Function to create data cube by counting candidate occurrences
unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> createDataCube(const vector<vector<int>> &transactions,
                                                                       const vector<set<int>> &candidates)
{
    unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> dataCube;

    for (const auto &transaction : transactions)
    {
        set<int> transactionSet(transaction.begin(), transaction.end());

        for (const auto &candidate : candidates)
        {
            if (isSubset(transactionSet, candidate)) // Use manual subset check
            {
                dataCube[candidate]++; // Increment the count for the candidate
            }
        }
    }
    return dataCube;
}

// Function to filter candidates based on minimum support
vector<pair<set<int>, int>> filterCandidates(const unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> &dataCube,
                                             int minSupport)
{
    vector<pair<set<int>, int>> frequentItemsets;
    for (const auto &pair : dataCube)
    {
        if (pair.second >= minSupport)
        {
            frequentItemsets.push_back({pair.first, pair.second});
        }
    }
    return frequentItemsets;
}

// Manual selection sort to sort candidates by count in descending order
void manualSortByCount(vector<pair<set<int>, int>> &candidates)
{
    for (int i = 0; i < candidates.size(); ++i)
    {
        int maxIdx = i;
        for (int j = i + 1; j < candidates.size(); ++j)
        {
            if (candidates[j].second > candidates[maxIdx].second)
            {
                maxIdx = j;
            }
        }
        swap(candidates[i], candidates[maxIdx]); // Place the largest found at the current position
    }
}

// Main Apriori algorithm function
vector<vector<pair<set<int>, int>>> apriori(const vector<vector<int>> &transactions, int minSupport)
{
    vector<vector<pair<set<int>, int>>> allFrequentItemsets;

    unordered_map<int, int> initialCount;
    for (const auto &transaction : transactions)
    {
        for (int item : transaction)
        {
            initialCount[item]++;
        }
    }

    vector<pair<set<int>, int>> currentCandidates;
    for (const auto &item : initialCount)
    {
        if (item.second >= minSupport)
        {
            currentCandidates.push_back({{item.first}, item.second});
        }
    }
    manualSortByCount(currentCandidates);

    allFrequentItemsets.push_back(currentCandidates);

    int k = 2;
    while (!currentCandidates.empty())
    {
        vector<set<int>> newCandidates;
        for (const auto &candidate : currentCandidates)
        {
            newCandidates.push_back(candidate.first);
        }

        newCandidates = generateCandidates(newCandidates);
        auto dataCube = createDataCube(transactions, newCandidates);
        currentCandidates = filterCandidates(dataCube, minSupport);

        // Sort candidates by count manually
        manualSortByCount(currentCandidates);
        if (!currentCandidates.empty())
        {
            allFrequentItemsets.push_back(currentCandidates);
        }

        k++;
    }

    return allFrequentItemsets;
}

// Utility function to print frequent itemsets with their counts
void writeToFileFrequentItemsets(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets,const unordered_map<int, string> intToItem)
{
    ofstream outputFile("output.txt");
    for (int size = 0; size < allFrequentItemsets.size(); ++size)
    {
        outputFile << "Frequent Itemsets of Size " << size + 1 << ":\n";
        for (const auto &itemset : allFrequentItemsets[size])
        {
            outputFile << "{ ";
            for (int item : itemset.first)
            {
                outputFile << intToItem.at(item) << " ";
            }
            outputFile << "} : " << itemset.second << "\n";
        }
        outputFile << endl;
    }
    outputFile.close();
}

void online(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets, 
            const unordered_map<int, string> &intToItem, 
            const unordered_map<string, int> &itemToInt) {

    int suggestionLimit = 5;
    set<int> selectedItems;
    cout << "Top Suggested Items:\n";

    // Display initial suggestions based on the first itemset
    for (const auto &itemset : allFrequentItemsets[0]) {
        int suggestions = 0;
        for (int item : itemset.first) {
            if (suggestions >= suggestionLimit) break;
            cout << intToItem.at(item) << " ";
            suggestions++;
        }
    }
    cout << "\n";

    while (selectedItems.size() < suggestionLimit) { // Loop until the limit is reached
        cout << "\nSelect an item name (or enter 0 to exit): ";
        string inputItem;
        cin >> inputItem;

        if (inputItem == "0") break;

        // Check for availability and duplicate selection
        if (itemToInt.find(inputItem) == itemToInt.end()) {
            cout << "Item '" << inputItem << "' is not available. Please try again.\n";
            continue;
        }

        int itemId = itemToInt.at(inputItem);
        if (selectedItems.find(itemId) != selectedItems.end()) {
            cout << "Item '" << inputItem << "' already selected. Please select a different item.\n";
            continue;
        }

        // Add item to selectedItems
        selectedItems.insert(itemId);
        cout << "Added '" << inputItem << "' to your selected items.\n";

        // Display selected items
        cout << "Selected Items: { ";
        for (int id : selectedItems) {
            cout << intToItem.at(id) << " ";
        }
        cout << "}\n";

        // Generate new suggestions
        cout << "Top " << suggestionLimit << " Suggested Items based on your selection:\n";
        set<int> suggestions;

        for (const auto &itemsetList : allFrequentItemsets) {
            for (const auto &itemset : itemsetList) {
                bool matchFound = false;

                // Check if any selected items are in the itemset
                for (int selectedItem : selectedItems) {
                    if (itemset.first.find(selectedItem) != itemset.first.end()) {
                        matchFound = true;
                        break; // No need to check further
                    }
                }

                if (matchFound) {
                    for (int item : itemset.first) {
                        if (selectedItems.find(item) == selectedItems.end()) {
                            suggestions.insert(item);
                        }
                    }
                }

                if (suggestions.size() >= suggestionLimit) break; // Stop if we reached the limit
            }
            if (suggestions.size() >= suggestionLimit) break; // Stop if we reached the limit
        }

        // Display the suggestions
        if (suggestions.empty()) {
            cout << "No more suggestions available.\n";
            break; // Exit if no suggestions are available
        } else {
            cout << "Suggestions: ";
            for (int item : suggestions) {
                cout << intToItem.at(item) << " ";
            }
            cout << "\n";
        }
    }

    // Display final selected items
    cout << "\nThank you! Your final selected items:\n{ ";
    for (int item : selectedItems) {
        cout << intToItem.at(item) << " ";
    }
    cout << "}\n";
}

int main() {
    int minSupportPercentage = 20;

    // Load data and map items
    ifstream inputFile("input.txt");
    string line;
    vector<vector<string>> tnxns;
    unordered_map<string, int> itemToInt;
    unordered_map<int, string> intToItem;
    int itemCounter = 0;

    //Read transactions and map items to integers from txt file
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string item;
        vector<string> transaction;
        while (ss >> item) {
            transaction.push_back(item);
            if (itemToInt.find(item) == itemToInt.end()) {
                itemToInt[item] = itemCounter;
                intToItem[itemCounter] = item;
                itemCounter++;
            }
        }
        tnxns.push_back(transaction);
    }
    inputFile.close();

    // Convert transactions to integer-based vector
    vector<vector<int>> transactions;
    for (const auto& transaction : tnxns) {
        vector<int> intTransaction;
        for (const auto& item : transaction) {
            intTransaction.push_back(itemToInt[item]);
        }
        transactions.push_back(intTransaction);
    }
    int minSupport=(minSupportPercentage/100)*itemCounter;
    // Run Apriori algorithm
    vector<vector<pair<set<int>, int>>> allFrequentItemsets = apriori(transactions, minSupport);
    writeToFileFrequentItemsets(allFrequentItemsets,intToItem);
    online(allFrequentItemsets,intToItem,itemToInt);
    return 0;
}
