#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
using namespace std;

struct ItemsetHash
{
    hash<int> hash_function;

    int operator()(const set<int> &itemset) const
    {
        int hash = 0;
        for (int item : itemset)
        {
            hash ^= hash_function(item);
        }
        return hash;
    }
};

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

set<int> mergeSets(const set<int> &a, const set<int> &b)
{
    set<int> unionSet = a;
    for (int item : b)
    {
        unionSet.insert(item);
    }
    return unionSet;
}

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
                uniqueCandidates.insert(move(unionSet));
            }
        }
    }

    return vector<set<int>>(uniqueCandidates.begin(), uniqueCandidates.end());
}

bool isSubset(const set<int> &transactionSet, const set<int> &candidate)
{
    for (int item : candidate)
    {
        if (transactionSet.find(item) == transactionSet.end())
            return false;
    }
    return true;
}

unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> createItemsetFrequencyMapper(const vector<vector<int>> &transactions,
                                                                                     const vector<set<int>> &candidates)
{
    unordered_map<set<int>, int, ItemsetHash, ItemsetEqual> dataCube;

    for (const auto &transaction : transactions)
    {
        set<int> transactionSet(transaction.begin(), transaction.end());

        for (const auto &candidate : candidates)
        {
            if (isSubset(transactionSet, candidate))
            {
                dataCube[candidate]++;
            }
        }
    }
    return dataCube;
}

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

void sortIems(vector<pair<set<int>, int>> &candidates)
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
        swap(candidates[i], candidates[maxIdx]);
    }
}

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
    sortIems(currentCandidates);

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
        auto dataCube = createItemsetFrequencyMapper(transactions, newCandidates);
        currentCandidates = filterCandidates(dataCube, minSupport);

        sortIems(currentCandidates);
        if (!currentCandidates.empty())
        {
            allFrequentItemsets.push_back(currentCandidates);
        }

        k++;
    }

    return allFrequentItemsets;
}

void writeToFileFrequentItemsets(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets, const unordered_map<int, string> &intToItem)
{
    ofstream outputFile("output.txt");

    bool hasFrequentItemsets = false;

    for (const auto &itemsets : allFrequentItemsets)
    {
        if (!itemsets.empty())
        {
            hasFrequentItemsets = true;
            break;
        }
    }

    if (!hasFrequentItemsets)
    {
        outputFile << "No itemsets surpassed the minimum count value.\n";
    }
    else
    {
        for (int size = 0; size < allFrequentItemsets.size(); ++size)
        {
            const auto &itemsets = allFrequentItemsets[size];
            if (itemsets.empty())
            {
                continue;
            }
            outputFile << "Frequent Itemsets of Size " << size + 1 << ":\n";
            for (const auto &itemset : itemsets)
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
    }
    outputFile.close();
}

void online(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets,
            const unordered_map<int, string> &intToItem,
            const unordered_map<string, int> &itemToInt)
{

    int suggestionLimit = 5;
    int totalNoOfItems = intToItem.size();
    set<int> selectedItems;
    cout << "Top Suggested Items:\n";

    for (const auto &itemset : allFrequentItemsets[0])
    {
        int suggestions = 0;
        for (int item : itemset.first)
        {
            if (suggestions >= suggestionLimit)
                break;
            cout << intToItem.at(item) << " ";
            suggestions++;
        }
    }
    cout << "\n";

    while (selectedItems.size() < totalNoOfItems)
    {
        cout << "\nSelect an item name (or enter 0 to exit, or 1 to list available items): ";
        string inputItem;
        cin >> inputItem;

        if (inputItem == "0")
            break;

        if (inputItem == "1")
        {
            cout << "Available Items (not yet selected):\n";
            for (const auto &pair : intToItem)
            {
                if (selectedItems.find(pair.first) == selectedItems.end())
                {
                    cout << pair.second << " ";
                }
            }
            cout << "\n";
            continue;
        }
        if (itemToInt.find(inputItem) == itemToInt.end())
        {
            cout << "Item '" << inputItem << "' is not available. Please try again.\n";
            continue;
        }

        int itemId = itemToInt.at(inputItem);
        if (selectedItems.find(itemId) != selectedItems.end())
        {
            cout << "Item '" << inputItem << "' already selected. Please select a different item.\n";
            continue;
        }

        selectedItems.insert(itemId);
        cout << "Added '" << inputItem << "' to your selected items.\n";

        cout << "Selected Items: { ";
        for (int id : selectedItems)
        {
            cout << intToItem.at(id) << " ";
        }
        cout << "}\n";

        cout << "Top " << suggestionLimit << " Suggested Items based on your selection:\n";
        set<int> suggestions;

        for (const auto &itemsetList : allFrequentItemsets)
        {
            for (const auto &itemset : itemsetList)
            {
                bool matchFound = false;
                for (int selectedItem : selectedItems)
                {
                    if (itemset.first.find(selectedItem) != itemset.first.end())
                    {
                        matchFound = true;
                        break;
                    }
                }

                if (matchFound)
                {
                    for (int item : itemset.first)
                    {
                        if (selectedItems.find(item) == selectedItems.end())
                        {
                            suggestions.insert(item);
                        }
                    }
                }

                if (suggestions.size() >= suggestionLimit)
                    break;
            }
            if (suggestions.size() >= suggestionLimit)
                break;
        }

        if (suggestions.size() < suggestionLimit)
        {
            for (const auto &pair : intToItem)
            {
                if (selectedItems.find(pair.first) == selectedItems.end() && suggestions.size() < suggestionLimit)
                {
                    suggestions.insert(pair.first);
                }
            }
        }

        if (suggestions.empty())
        {
            cout << "No more suggestions available. You can choose items by yourself.\n";
        }
        else
        {
            cout << "Suggestions: ";
            for (int item : suggestions)
            {
                cout << intToItem.at(item) << " ";
            }
            cout << "\n";
        }
    }

    cout << "\nThank you! Your final selected items:\n{ ";
    for (int item : selectedItems)
    {
        cout << intToItem.at(item) << " ";
    }
    cout << "}\n";
}

int main()
{
    int minSupportPercentage = 60;

    ifstream inputFile("input.txt");
    string line;
    vector<vector<string>> tnxns;
    unordered_map<string, int> itemToInt;
    unordered_map<int, string> intToItem;
    int itemCounter = 0;
    int numOfTransaction = 0;

    while (getline(inputFile, line))
    {
        stringstream ss(line);
        string item;
        vector<string> transaction;
        while (ss >> item)
        {
            transaction.push_back(item);
            if (itemToInt.find(item) == itemToInt.end())
            {
                itemToInt[item] = itemCounter;
                intToItem[itemCounter] = item;
                itemCounter++;
            }
        }
        numOfTransaction++;
        tnxns.push_back(transaction);
    }
    inputFile.close();

    vector<vector<int>> transactions;
    for (const auto &transaction : tnxns)
    {
        vector<int> intTransaction;
        for (const auto &item : transaction)
        {
            intTransaction.push_back(itemToInt[item]);
        }
        transactions.push_back(intTransaction);
    }
    int minSupport = (minSupportPercentage * numOfTransaction) / 100;
    vector<vector<pair<set<int>, int>>> allFrequentItemsets = apriori(transactions, minSupport);
    writeToFileFrequentItemsets(allFrequentItemsets, intToItem);
    online(allFrequentItemsets, intToItem, itemToInt);
    return 0;
}
