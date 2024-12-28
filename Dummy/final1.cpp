#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>

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

// Function to generate candidates from frequent itemsets
vector<set<int>> generateCandidates(const vector<set<int>> &prevItemsets)
{
    vector<set<int>> candidates;

    for (size_t i = 0; i < prevItemsets.size(); ++i)
    {
        for (size_t j = i + 1; j < prevItemsets.size(); ++j)
        {
            set<int> unionSet;
            // Manually merge two sets
            for (int item : prevItemsets[i])
            {
                unionSet.insert(item);
            }
            for (int item : prevItemsets[j])
            {
                unionSet.insert(item);
            }
            if (unionSet.size() == prevItemsets[i].size() + 1)
            {
                candidates.push_back(move(unionSet)); // Use move for performance
            }
        }
    }
    return candidates;
}

// Function to create a data cube for counting itemsets
unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> createDataCube(const vector<vector<int>> &transactions,
                                                                      const vector<set<int>> &candidates)
{
    unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> dataCube;

    for (const auto &transaction : transactions)
    {
        for (const auto &candidate : candidates)
        {
            // Manual inclusion check
            bool found = true;
            for (int item : candidate)
            {
                if (find(transaction.begin(), transaction.end(), item) == transaction.end())
                {
                    found = false;
                    break;
                }
            }
            if (found)
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
            frequentItemsets.push_back({pair.first, pair.second}); // Store itemset and count
        }
    }
    return frequentItemsets;
}

// Comparator for sorting pairs based on counts
bool compareByCount(const pair<set<int>, int> &a, const pair<set<int>, int> &b)
{
    return a.second > b.second; // Sort in descending order
}

// Main Apriori algorithm function
vector<vector<pair<set<int>, int>>> apriori(const vector<vector<int>> &transactions, int minSupport)
{
    vector<vector<pair<set<int>, int>>> allFrequentItemsets; // Store different sizes of frequent itemsets

    // Generate initial candidates (1-itemsets)
    unordered_map<int, int> initialCount;
    for (const auto &transaction : transactions)
    {
        for (int item : transaction)
        {
            initialCount[item]++;
        }
    }

    // Filter initial candidates
    vector<pair<set<int>, int>> currentCandidates;
    for (const auto &item : initialCount)
    {
        if (item.second >= minSupport)
        {
            currentCandidates.push_back({{item.first}, item.second}); // Store itemset and count
        }
    }

    allFrequentItemsets.push_back(currentCandidates); // Store size 1 itemsets

    // Iteratively generate frequent itemsets
    int k = 2; // Starting from size 2
    while (!currentCandidates.empty())
    {
        // Extract Itemsets from currentCandidates
        vector<set<int>> newCandidates;
        for (const auto &candidate : currentCandidates)
        {
            newCandidates.push_back(candidate.first);
        }

        newCandidates = generateCandidates(newCandidates);
        auto dataCube = createDataCube(transactions, newCandidates);
        currentCandidates = filterCandidates(dataCube, minSupport);

        // Sort candidates by count and add to overall list
        sort(currentCandidates.begin(), currentCandidates.end(), compareByCount);
        if (!currentCandidates.empty())
        {
            allFrequentItemsets.push_back(currentCandidates); // Store the frequent itemsets of size k
        }

        k++;
    }

    return allFrequentItemsets;
}

// Utility function to print frequent itemsets with their counts
void printFrequentItemsets(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets)
{
    for (size_t size = 0; size < allFrequentItemsets.size(); ++size)
    {
        cout << "Frequent Itemsets of Size " << size + 1 << ":\n";
        for (const auto &itemset : allFrequentItemsets[size])
        {
            cout << "{ ";
            for (int item : itemset.first)
            {
                cout << item << " ";
            }
            cout << "} : " << itemset.second << "\n"; // Print count
        }
        cout << endl;
    }
}

int main()
{
    // Sample transactions
    vector<vector<int>> transactions = {
        {1, 2, 3},
        {2, 3, 4},
        {1, 2},
        {1, 3},
        {2, 3},
        {1, 2, 3}};

    int minSupport = 2; // Adjusted for testing purposes

    // Run Apriori algorithm using data cube method
    vector<vector<pair<set<int>, int>>> allFrequentItemsets = apriori(transactions, minSupport);

    // Print frequent itemsets
    printFrequentItemsets(allFrequentItemsets);

    return 0;
}
