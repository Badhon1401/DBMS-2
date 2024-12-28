#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>
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
        return a == b;
    }
};

vector<set<int>> generateCandidates(const vector<set<int>> &prevItemsets)
{
    unordered_set<set<int>, ItemsetHash, ItemsetEqual> uniqueCandidates;

    for (int i = 0; i < prevItemsets.size(); ++i)
    {
        for (int j = i + 1; j < prevItemsets.size(); ++j)
        {
            set<int> unionSet;
            set_union(prevItemsets[i].begin(), prevItemsets[i].end(),
                      prevItemsets[j].begin(), prevItemsets[j].end(),
                      inserter(unionSet, unionSet.begin()));

            if (unionSet.size() == prevItemsets[i].size() + 1)
            {
                uniqueCandidates.insert(move(unionSet));
            }
        }
    }

    return vector<set<int>>(uniqueCandidates.begin(), uniqueCandidates.end());
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
            if (includes(transactionSet.begin(), transactionSet.end(),
                         candidate.begin(), candidate.end()))
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
            frequentItemsets.emplace_back(pair.first, pair.second);
        }
    }
    return frequentItemsets;
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

    sort(currentCandidates.begin(), currentCandidates.end(),
         [](const pair<set<int>, int> &a, const pair<set<int>, int> &b)
         {
             return a.second > b.second;
         });

    allFrequentItemsets.push_back(currentCandidates);

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

        sort(currentCandidates.begin(), currentCandidates.end(),
             [](const pair<set<int>, int> &a, const pair<set<int>, int> &b)
             {
                 return a.second > b.second;
             });

        if (!currentCandidates.empty())
        {
            allFrequentItemsets.push_back(currentCandidates);
        }
    }

    return allFrequentItemsets;
}

void writeToFileFrequentItemsets(const vector<vector<pair<set<int>, int>>> &allFrequentItemsets, const unordered_map<int, string> &intToItem)
{
    ofstream outputFile("output.txt");
    if (allFrequentItemsets.empty() || all_of(allFrequentItemsets.begin(), allFrequentItemsets.end(),
                                              [](const vector<pair<set<int>, int>> &itemsets)
                                              { return itemsets.empty(); }))
    {
        outputFile << "No itemsets surpassed the minimum count value.\n";
    }
    else
    {
        for (int size = 0; size < allFrequentItemsets.size(); ++size)
        {
            if (allFrequentItemsets[size].empty())
            {
                continue; 
            }
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
    }
    outputFile.close();
}

int main()
{
    int minSupportPercentage = 5;

    ifstream inputFile("input.txt");
    string line;
    vector<vector<string>> tnxns;
    unordered_map<string, int> itemToInt;
    unordered_map<int, string> intToItem;
    int itemCounter = 0, numOfTransaction = 0;

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
    return 0;
}
