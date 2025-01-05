#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <set>
#include <iomanip>
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
    int chooseHeuristicBranch(int here)
    {
        int bestChild = -1;
        int maxDataPoints = -1;

        for (int next : tree[here].children)
        {
            int count = 0;
            for (const auto &row : initialTable.data)
            {
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

        return bestChild;
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
        if (!tree[here].children.empty())
        {
            int heuristicBranch = tree[here].children[0]; // You could refine this heuristic.
            return dfsOnQuery(row, heuristicBranch);
        }

        return -1;
    }

    string guessOnQuery(vector<string> row)
    {
        string label = "";
        int leafNode = dfsOnQuery(row, 0);
        if (leafNode == -1)
        {
            return "dfs failed";
        }
        label = tree[leafNode].label;
        return label;
    }

    int dfsOnQuery(vector<string> &row, int here)
    {
        if (tree[here].isLeaf)
        {
            return here;
        }

        int criteriaAttrIndex = tree[here].criteriaAttrIndex;
        cout << initialTable.attrName[criteriaAttrIndex] << ": " << row[criteriaAttrIndex] << " -> ";

        for (int i = 0; i < tree[here].children.size(); i++)
        {
            int next = tree[here].children[i];

            if (row[criteriaAttrIndex] == tree[next].attrValue)
            {
                return dfsOnQuery(row, next);
            }
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
        return getGain(table, attrIndex)/ getSplitInfoAttrD(table, attrIndex);
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

// Function to read input from the file and parse it
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

// Function to join a vector of strings by tab
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

// Function to write output to a file
void printOutput(const string &filename, const Table &testTable, const vector<string> &results)
{
    ofstream fout(filename);
    if (!fout)
    {
        cout << filename << " file could not be opened\n";
        exit(0);
    }

    fout << joinByTab(testTable.attrName) << "(Actual)    " << testTable.attrName.back() + "(Output)" << endl;
    for (size_t i = 0; i < testTable.data.size(); i++)
    {
        vector<string> row = testTable.data[i];
        row.push_back(results[i]);
        fout << joinByTab(row) << endl;
    }
}

void interactiveQuerySession(DecisionTree &decisionTree, const vector<string> &attrNames, const Table &trainTable)
{
    while (true)
    {
        system("cls"); // Clear the screen before showing the options again
        cout << "Interactive Decision Tree Query System\n";

        vector<string> queryRow;
        // vector<string> selectedCategories; // To store the categories chosen by the user

        // Loop through each attribute and get valid input from the user
        for (size_t attrIdx = 0; attrIdx < attrNames.size() - 1; ++attrIdx)
        {
            const string &attribute = attrNames[attrIdx];
            set<string> validOptions;

            // Collect valid options from the training data
            for (const auto &row : trainTable.data)
            {
                validOptions.insert(row[attrIdx]);
            }

            string userInput;
            while (true)
            {
                // Display the options with numbers
                cout << "\n"
                     << attribute << " (Choose one of the following options):\n";
                int optionNumber = 1;
                for (const auto &option : validOptions)
                {
                    cout << optionNumber++ << ". " << option << "\n";
                }

                cout << "\n"
                     << attribute << ": ";
                getline(cin, userInput);

                // Validate user input (checking if input is a number within range)
                try
                {
                    int selectedOption = stoi(userInput);
                    if (selectedOption >= 1 && selectedOption <= validOptions.size())
                    {
                        // Find the corresponding option (1-based index)
                        auto it = validOptions.begin();
                        advance(it, selectedOption - 1); // Move iterator to the selected option
                        queryRow.push_back(*it);
                        //  selectedCategories.push_back(*it); // Store the selected category
                        break;
                    }
                    else
                    {
                        cout << "\nInvalid input! Please choose a valid option.\n";
                    }
                }
                catch (...)
                {
                    cout << "\nInvalid input! Please choose a valid option.\n";
                }
            }
        }

        // // Show selected categories for each attribute
        // cout << "\nYou have selected the following options:\n";
        // for (size_t i = 0; i < attrNames.size(); ++i)
        // {
        //     cout << attrNames[i] << ": " << selectedCategories[i] << "\n";
        // }

        // Make a prediction using the decision tree
        string result = decisionTree.guessOnQuery(queryRow);
        cout << "\nPrediction: " << trainTable.attrName.back() << " : " << result << "\n";

        // Ask if the user wants to query again
        cout << "\nWould you like to query again? (y/n): ";
        string again;
        getline(cin, again);
        if (again != "y" && again != "Y")
        {
            cout << "Exiting the query system.\n";
            break;
        }
    }
}

// Function to split data into training and testing sets linearly
void splitData(const Table &data, double trainPercentage, Table &trainSet, Table &testSet)
{
    // Calculate number of rows for training data
    size_t trainSize = static_cast<size_t>(trainPercentage * data.data.size() / 100.0);

    // Split data
    for (size_t i = 0; i < data.data.size(); ++i)
    {
        if (i < trainSize)
        {
            trainSet.data.push_back(data.data[i]);
        }
        else
        {
            testSet.data.push_back(data.data[i]);
        }
    }

    // Copy attribute names
    trainSet.attrName = data.attrName;
    testSet.attrName = data.attrName;
}

int main()
{
    // Input file name
    const string trainFileName = "iris.csv";

    // Load data from file
    Table data = readInput(trainFileName);

    // Ask for training percentage and validate input
    double trainPercentage;
    while (true)
    {
        cout << "Enter the percentage of data to be used for training (0-100): ";
        cin >> trainPercentage;

        // Validate input
        if (trainPercentage >= 0 && trainPercentage <= 100)
        {
            break;
        }
        else
        {
            cout << "Invalid input! Please enter a percentage between 0 and 100.\n";
        }
    }

    // Split data into training and testing sets
    Table trainSet, testSet;
    splitData(data, trainPercentage, trainSet, testSet);

    // Initialize Decision Tree with training data
    DecisionTree decisionTree(trainSet);

    // Get predictions for test data
    vector<string> results;
    int passedTests = 0;
    int failedTests = 0;
    for (const auto &row : testSet.data)
    {
        string prediction = decisionTree.guess(row);
        string actual = row.back(); // Assuming last column is the label
        results.push_back(prediction);

        if (prediction == actual)
        {
            passedTests++;
        }
        else
        {
            failedTests++;
        }
    }

    // Write test results to output file
    printOutput("result.txt", testSet, results);

    // Write test statistics
    ofstream statsFile("test_stats.txt");
    double hitRatio = (double)passedTests / (passedTests + failedTests);
    statsFile << "Passed Tests: " << passedTests << endl;
    statsFile << "Failed Tests: " << failedTests << endl;
    statsFile << "Hit Ratio: " << fixed << setprecision(2) << hitRatio * 100 << "%" << endl;

    cout << "Test results have been written to 'result.txt' and statistics to 'test_stats.txt'.\n";

    // Start interactive query session
    interactiveQuerySession(decisionTree, testSet.attrName, trainSet);

    return 0;
}