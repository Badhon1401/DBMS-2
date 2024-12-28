#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
using namespace std;

void online(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets, const unordered_map<int, string> &intToItem)
{
    unordered_map<string, int> itemToInt;
    for (const auto &pair : intToItem) {
        itemToInt[pair.second] = pair.first;
    }

    vector<int> cartItems;
    set<int> currentSelections;
    cout << "Welcome to the online shopping system! Enter items to add them to your cart.\n";
    cout << "Available items: \n";

    // Display available items initially, sorted by most frequent itemsets
    for (const auto &frequentSet : allFrequentItemsets) {
        for (const auto &itemset : frequentSet) {
            cout << "{ ";
            for (int item : itemset.first) {
                cout << intToItem.at(item) << " ";
            }
            cout << "} with support: " << itemset.second << "\n";
        }
    }

    string inputItem;
    while (true) {
        cout << "\nEnter an item to add to your cart (or enter 0 to exit): ";
        cin >> inputItem;

        if (inputItem == "0") {
            break; // Exit the loop if the user enters 0
        }

        // Check if the item is available
        if (itemToInt.find(inputItem) == itemToInt.end()) {
            cout << "Item '" << inputItem << "' is not available. Please try again.\n";
            continue;
        }

        // Add item to cart
        int itemId = itemToInt[inputItem];
        cartItems.push_back(itemId);
        currentSelections.insert(itemId);
        cout << "Added '" << inputItem << "' to your cart.\n";

        // Display current cart items
        cout << "Current Cart Items: { ";
        for (int item : cartItems) {
            cout << intToItem.at(item) << " ";
        }
        cout << "}\n";

        // Generate new suggestions based on current selections
        cout << "Suggested items based on your cart:\n";
        bool suggestionFound = false;
        for (const auto &frequentSet : allFrequentItemsets) {
            for (const auto &itemset : frequentSet) {
                bool allPresent = true;
                for (int item : itemset.first) {
                    if (currentSelections.find(item) == currentSelections.end()) {
                        allPresent = false;
                        break;
                    }
                }
                if (allPresent) {
                    cout << "{ ";
                    for (int item : itemset.first) {
                        if (currentSelections.find(item) == currentSelections.end()) {
                            cout << intToItem.at(item) << " ";
                        }
                    }
                    cout << "} with support: " << itemset.second << "\n";
                    suggestionFound = true;
                }
            }
        }

        if (!suggestionFound) {
            cout << "No more suggestions available based on your current cart.\n";
        }
    }

    // Display final cart contents
    cout << "\nThank you for shopping! Your final cart contains:\n";
    cout << "{ ";
    for (int item : cartItems) {
        cout << intToItem.at(item) << " ";
    }
    cout << "}\n";
}


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

int main() {
    int minSupport = 2;

    // Load data and map items
    ifstream inputFile("input.txt");
    string line;
    vector<vector<string>> tnxns;
    unordered_map<string, int> itemToInt;
    unordered_map<int, string> intToItem;
    int itemCounter = 0;

    // Read transactions and map items to integers
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

    // Run Apriori algorithm
    vector<vector<pair<set<int>, int>>> allFrequentItemsets = apriori(transactions, minSupport);
    writeToFileFrequentItemsets(allFrequentItemsets,intToItem);
    online(allFrequentItemsets,intToItem);
    return 0;
}
