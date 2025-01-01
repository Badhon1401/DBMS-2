#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <set>
#include <iomanip>
#include <random>
#include <algorithm>
using namespace std;

class Table
{
public:
    vector<string> attrName;
    vector<vector<string>> data;
    vector<vector<string>> attrValueList;

    void extractAttrValue()
    {
        attrValueList.resize(attrName.size());
        for (int j = 0; j < attrName.size(); j++)
        {
            map<string, int> value;
            for (int i = 0; i < data.size(); i++)
            {
                value[data[i][j]] = 1;
            }

            for (auto iter = value.begin(); iter != value.end(); iter++)
            {
                attrValueList[j].push_back(iter->first);
            }
        }
    }
};

class Node
{
public:
    int criteriaAttrIndex;
    string criteriaAttrName;
    string attrValue;
    int treeIndex;
    bool isLeaf;
    string label;
    vector<int> children;

    Node()
    {
        isLeaf = false;
    }
};

class DecisionTree
{
public:
    Table initialTable;
    vector<Node> tree;

    DecisionTree(Table table)
    {
        initialTable = table;
        initialTable.extractAttrValue();

        Node root;
        root.treeIndex = 0;
        tree.push_back(root);
        run(initialTable, 0);
    }

    string guess(vector<string> row)
    {
        string label = "";
        int leafNode = dfs(row, 0);
        if (leafNode == -1)
        {
            return "dfs failed";
        }
        label = tree[leafNode].label;
        return label;
    }

    int chooseHeuristicBranch(int here)
    {
        if (tree[here].children.empty())
        {
            return -1;
        }

        int bestChild = -1;
        int maxDataPoints = -1;

        for (int next : tree[here].children)
        {
            if (next < 0 || next >= tree.size())
            {
                continue;
            }

            if (tree[next].criteriaAttrIndex < 0 || tree[next].criteriaAttrIndex >= initialTable.attrName.size())
            {
                continue;
            }

            int count = 0;
            for (const auto &row : initialTable.data)
            {
                if (tree[next].criteriaAttrIndex >= row.size())
                {
                    continue;
                }

                if (row[tree[next].criteriaAttrIndex] == tree[next].attrValue)
                {
                    count++;
                }
            }

            if (count > maxDataPoints)
            {
                maxDataPoints = count;
                bestChild = next;
            }
        }
        cout<<bestChild<<endl;
        return bestChild;
    }

    int dfs(vector<string> &row, int here)
    {
        if (tree[here].isLeaf)
        {
            return here;
        }

        int criteriaAttrIndex = tree[here].criteriaAttrIndex;

        for (int i = 0; i < tree[here].children.size(); i++)
        {
            int next = tree[here].children[i];

            if (row[criteriaAttrIndex] == tree[next].attrValue)
            {
                return dfs(row, next);
            }
        }

        int heuristicBranch = chooseHeuristicBranch(here);
        if (heuristicBranch == -1)
        {

            return dfs(row, tree[here].children[0]);
        }
        else
        {

            return dfs(row, heuristicBranch);
        }

        return -1;
    }

    void run(Table table, int nodeIndex)
    {
        if (isLeafNode(table))
        {
            tree[nodeIndex].isLeaf = true;
            tree[nodeIndex].label = table.data.back().back();
            return;
        }

        int selectedAttrIndex = getSelectedAttribute(table);
        tree[nodeIndex].criteriaAttrIndex = selectedAttrIndex;

        map<string, vector<int>> attrValueMap;
        for (int i = 0; i < table.data.size(); i++)
        {
            attrValueMap[table.data[i][selectedAttrIndex]].push_back(i);
        }

        pair<string, int> majority = getMajorityLabel(table);
        if ((double)majority.second / table.data.size() > 0.8)
        {
            tree[nodeIndex].isLeaf = true;
            tree[nodeIndex].label = majority.first;
            return;
        }

        for (int i = 0; i < initialTable.attrValueList[selectedAttrIndex].size(); i++)
        {
            string attrValue = initialTable.attrValueList[selectedAttrIndex][i];

            Table nextTable;
            vector<int> candi = attrValueMap[attrValue];
            for (int i = 0; i < candi.size(); i++)
            {
                nextTable.data.push_back(table.data[candi[i]]);
            }

            Node nextNode;
            nextNode.attrValue = attrValue;
            nextNode.treeIndex = (int)tree.size();
            tree[nodeIndex].children.push_back(nextNode.treeIndex);
            tree.push_back(nextNode);

            if (nextTable.data.size() == 0)
            {
                nextNode.isLeaf = true;
                nextNode.label = getMajorityLabel(table).first;
                tree[nextNode.treeIndex] = nextNode;
            }
            else
            {
                run(nextTable, nextNode.treeIndex);
            }
        }
    }

    pair<string, int> getMajorityLabel(Table table)
    {
        string majorLabel = "";
        int majorCount = 0;

        map<string, int> labelCount;
        for (int i = 0; i < table.data.size(); i++)
        {
            labelCount[table.data[i].back()]++;

            if (labelCount[table.data[i].back()] > majorCount)
            {
                majorCount = labelCount[table.data[i].back()];
                majorLabel = table.data[i].back();
            }
        }

        return {majorLabel, majorCount};
    }

    bool isLeafNode(Table table)
    {
        for (int i = 1; i < table.data.size(); i++)
        {
            if (table.data[0].back() != table.data[i].back())
            {
                return false;
            }
        }
        return true;
    }

    int getSelectedAttribute(Table table)
    {
        int maxAttrIndex = -1;
        double maxAttrValue = 0.0;

        for (int i = 0; i < initialTable.attrName.size() - 1; i++)
        {
            if (maxAttrValue < getGainRatio(table, i))
            {
                maxAttrValue = getGainRatio(table, i);
                maxAttrIndex = i;
            }
        }

        return maxAttrIndex;
    }

    double getGainRatio(Table table, int attrIndex)
    {
        return getGain(table, attrIndex) / getSplitInfoAttrD(table, attrIndex);
    }

    double getInfoD(Table table)
    {
        double ret = 0.0;

        int itemCount = (int)table.data.size();
        map<string, int> labelCount;

        for (int i = 0; i < table.data.size(); i++)
        {
            labelCount[table.data[i].back()]++;
        }

        for (auto iter = labelCount.begin(); iter != labelCount.end(); iter++)
        {
            double p = (double)iter->second / itemCount;

            ret += -1.0 * p * log(p) / log(2);
        }

        return ret;
    }

    double getInfoAttrD(Table table, int attrIndex)
    {
        double ret = 0.0;
        int itemCount = (int)table.data.size();

        map<string, vector<int>> attrValueMap;
        for (int i = 0; i < table.data.size(); i++)
        {
            attrValueMap[table.data[i][attrIndex]].push_back(i);
        }

        for (auto iter = attrValueMap.begin(); iter != attrValueMap.end(); iter++)
        {
            Table nextTable;
            for (int i = 0; i < iter->second.size(); i++)
            {
                nextTable.data.push_back(table.data[iter->second[i]]);
            }
            int nextItemCount = (int)nextTable.data.size();

            ret += (double)nextItemCount / itemCount * getInfoD(nextTable);
        }

        return ret;
    }

    double getGain(Table table, int attrIndex)
    {
        return getInfoD(table) - getInfoAttrD(table, attrIndex);
    }

    double getSplitInfoAttrD(Table table, int attrIndex)
    {
        double ret = 0.0;

        int itemCount = (int)table.data.size();

        map<string, vector<int>> attrValueMap;
        for (int i = 0; i < table.data.size(); i++)
        {
            attrValueMap[table.data[i][attrIndex]].push_back(i);
        }

        for (auto iter = attrValueMap.begin(); iter != attrValueMap.end(); iter++)
        {
            Table nextTable;
            for (int i = 0; i < iter->second.size(); i++)
            {
                nextTable.data.push_back(table.data[iter->second[i]]);
            }
            int nextItemCount = (int)nextTable.data.size();

            double d = (double)nextItemCount / itemCount;
            ret += -1.0 * d * log(d) / log(2);
        }

        return ret;
    }
};

Table readInput(const string &filename)
{
    ifstream fin(filename);
    if (!fin)
    {
        cout << filename << " file could not be opened\n";
        exit(0);
    }

    Table table;
    string str;
    bool isAttrName = true;

    while (getline(fin, str))
    {
        vector<string> row;
        int pre = 0;
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == ',')
            {
                string col = str.substr(pre, i - pre);
                row.push_back(col);
                pre = i + 1;
            }
        }
        string col = str.substr(pre, str.size() - pre);
        row.push_back(col);

        if (isAttrName)
        {
            table.attrName = row;
            isAttrName = false;
        }
        else
        {
            table.data.push_back(row);
        }
    }
    return table;
}

string joinByTab(const vector<string> &row)
{
    string ret = "";
    for (int i = 0; i < row.size(); i++)
    {
        ret += row[i];
        if (i != row.size() - 1)
        {
            ret += '\t';
        }
    }
    return ret;
}

void splitData(const Table &data, double trainPercentage, Table &trainSet, Table &testSet)
{
    vector<int> indices(data.data.size());
    iota(indices.begin(), indices.end(), 0);

    random_device rd;
    mt19937 gen(rd());
    shuffle(indices.begin(), indices.end(), gen);

    size_t trainSize = static_cast<size_t>(trainPercentage * data.data.size() / 100.0);

    for (size_t i = 0; i < data.data.size(); ++i)
    {
        if (i < trainSize)
        {
            trainSet.data.push_back(data.data[indices[i]]);
        }
        else
        {
            testSet.data.push_back(data.data[indices[i]]);
        }
    }

    trainSet.attrName = data.attrName;
    testSet.attrName = data.attrName;
}

void calculateMetrics(const vector<string> &predictions, const vector<string> &actualLabels, ofstream &statsFile)
{
    map<string, int> truePositives, falsePositives, falseNegatives;
    set<string> uniqueLabels;

    for (const auto &label : actualLabels)
    {
        uniqueLabels.insert(label);
        truePositives[label] = 0;
        falsePositives[label] = 0;
        falseNegatives[label] = 0;
    }

    int totalTestCases = predictions.size();
    for (size_t i = 0; i < predictions.size(); i++)
    {
        const string &predicted = predictions[i];
        const string &actual = actualLabels[i];

        if (predicted == actual)
        {
            truePositives[actual]++;
        }
        else
        {
            falsePositives[predicted]++;
            falseNegatives[actual]++;
        }
    }

    double totalPrecision = 0.0, totalRecall = 0.0, totalF1 = 0.0;
    double overallTP = 0, overallFP = 0, overallFN = 0;

    statsFile << "Metrics per class:\n";
    for (const auto &label : uniqueLabels)
    {
        int TP = truePositives[label];
        int FP = falsePositives[label];
        int FN = falseNegatives[label];
        int classTotal = TP + FN + FP;

        int passedTestCases = TP;
        int failedTestCases = classTotal - passedTestCases;

        overallTP += TP;
        overallFP += FP;
        overallFN += FN;

        double precision = (TP + FP > 0) ? (double)TP / (TP + FP) : 0.0;
        double recall = (TP + FN > 0) ? (double)TP / (TP + FN) : 0.0;
        double f1Score = (precision + recall > 0) ? 2 * (precision * recall) / (precision + recall) : 0.0;

        double d2H = sqrt(pow(1 - recall, 2) + pow(1 - precision, 2));

        statsFile << "Class: " << label << endl;
        statsFile << "  Total Test Cases: " << classTotal << endl;
        statsFile << "  Passed Test Cases: " << passedTestCases << endl;
        statsFile << "  Failed Test Cases: " << failedTestCases << endl;
        statsFile << "  Precision: " << fixed << setprecision(2) << precision * 100 << "%" << endl;
        statsFile << "  Recall: " << fixed << setprecision(2) << recall * 100 << "%" << endl;
        statsFile << "  F1 Score: " << fixed << setprecision(2) << f1Score * 100 << "%" << endl;
        statsFile << "  Distance to Heaven (D2H): " << fixed << setprecision(2) << d2H << endl;

        totalPrecision += precision;
        totalRecall += recall;
        totalF1 += f1Score;
    }

    double macroPrecision = totalPrecision / uniqueLabels.size();
    double macroRecall = totalRecall / uniqueLabels.size();
    double macroF1 = totalF1 / uniqueLabels.size();

    statsFile << "\nMacro-Averaged Metrics:\n";
    statsFile << "  Precision: " << fixed << setprecision(2) << macroPrecision * 100 << "%" << endl;
    statsFile << "  Recall: " << fixed << setprecision(2) << macroRecall * 100 << "%" << endl;
    statsFile << "  F1 Score: " << fixed << setprecision(2) << macroF1 * 100 << "%" << endl;

    double overallPrecision = (overallTP + overallFP > 0) ? (double)overallTP / (overallTP + overallFP) : 0.0;
    double overallRecall = (overallTP + overallFN > 0) ? (double)overallTP / (overallTP + overallFN) : 0.0;
    double overallF1 = (overallPrecision + overallRecall > 0) ? 2 * (overallPrecision * overallRecall) / (overallPrecision + overallRecall) : 0.0;
    double overallD2H = sqrt(pow(1 - overallRecall, 2) + pow(1 - overallPrecision, 2));

    statsFile << "\nOverall Metrics:\n";
    statsFile << "  Precision: " << fixed << setprecision(2) << overallPrecision * 100 << "%" << endl;
    statsFile << "  Recall: " << fixed << setprecision(2) << overallRecall * 100 << "%" << endl;
    statsFile << "  F1 Score: " << fixed << setprecision(2) << overallF1 * 100 << "%" << endl;
    statsFile << "  Distance to Heaven (D2H): " << fixed << setprecision(2) << overallD2H << endl;
}

void kFoldCrossValidation(const Table &data, int k, ofstream &statsFile)
{
    vector<int> indices(data.data.size());
    iota(indices.begin(), indices.end(), 0);

    random_device rd;
    mt19937 gen(rd());
    shuffle(indices.begin(), indices.end(), gen);

    size_t foldSize = data.data.size() / k;

    statsFile << "K-Fold Cross Validation Results:\n";

    for (int fold = 0; fold < k; ++fold)
    {
        Table trainSet, testSet;

        size_t testStart = fold * foldSize;
        size_t testEnd = (fold == k - 1) ? data.data.size() : testStart + foldSize;

        for (size_t i = 0; i < data.data.size(); ++i)
        {
            if (i >= testStart && i < testEnd)
            {
                testSet.data.push_back(data.data[indices[i]]);
            }
            else
            {
                trainSet.data.push_back(data.data[indices[i]]);
            }
        }

        trainSet.attrName = data.attrName;
        testSet.attrName = data.attrName;

        DecisionTree decisionTree(trainSet);

        vector<string> results;
        vector<string> actualLabels;
        for (const auto &row : testSet.data)
        {
            string prediction = decisionTree.guess(row);
            results.push_back(prediction);
            actualLabels.push_back(row.back());
        }

        statsFile << "\nFold " << fold + 1 << " Metrics:\n";
        calculateMetrics(results, actualLabels, statsFile);
    }

    statsFile << "\nK-Fold Cross Validation Completed.\n";
}

int main()
{

    const string trainFileName = "winequality-white.csv";
    Table data = readInput(trainFileName);
    ofstream kf_statsFile("k_fold_test_stats.txt");
    int k;
    cout << "Enter the value of K for K-Fold Cross Validation: ";
    cin >> k;
    if (k > 1 && k <= data.data.size())
    {
        kFoldCrossValidation(data, k, kf_statsFile);
    }
    else
    {
        cout << "Invalid K value. Skipping K-Fold Cross Validation.\n";
    }
    cout << "Execution Completed";
    return 0;
}
