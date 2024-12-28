#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <set> 

using namespace std;

struct DataRow
{
    vector<string> values;
    string finalColumnValue;
};

struct AggregatedValues
{
    double sum = 0;
    int count = 0;
    double minVal = numeric_limits<double>::max();
    double maxVal = numeric_limits<double>::lowest();

    void addValue(double value)
    {
        sum += value;
        count++;
        minVal = min(minVal, value);
        maxVal = max(maxVal, value);
    }

    double average() const
    {
        return count == 0 ? 0 : sum / count;
    }
};

vector<DataRow> parseInputFile(const string &filename, string &tableName, vector<string> &columnNames, string &finalColumnName)
{
    ifstream inputFile(filename);
    vector<DataRow> data;
    if (!inputFile)
    {
        cerr << "Error opening input file!\n";
        exit(1);
    }

    getline(inputFile, tableName);
    string line;
    getline(inputFile, line);
    stringstream ss(line);
    string columnName;
    while (ss >> columnName)
    {
        columnNames.push_back(columnName);
    }

    finalColumnName = columnNames.back();

    while (getline(inputFile, line))
    {
        stringstream dataStream(line);
        DataRow row;
        string value;
        for (size_t i = 0; i < columnNames.size() - 1; ++i)
        {
            dataStream >> value;
            row.values.push_back(value);
        }
        dataStream >> row.finalColumnValue;
        data.push_back(row);
    }
    inputFile.close();
    return data;
}

void addAggregatedValues(map<string, AggregatedValues> &results, const string &key, double value)
{
    results[key].addValue(value);
}

void writeResultsToFile(const string &filename,
                        const map<string, AggregatedValues> &results,
                        const vector<string> &columnNames)
{
    ofstream outputFile(filename);
    if (!outputFile)
    {
        cerr << "Error opening output file!\n";
        exit(1);
    }

    vector<size_t> columnWidths(columnNames.size(), 0);

    for (size_t i = 0; i < columnNames.size(); ++i)
    {
        columnWidths[i] = columnNames[i].size();
    }

    for (const auto &entry : results)
    {
        stringstream ss(entry.first);
        string value;
        vector<string> values;

        while (getline(ss, value, ','))
        {
            values.push_back(value);
        }

        for (size_t i = 0; i < values.size(); ++i)
        {
            columnWidths[i] = max(columnWidths[i], values[i].size());
        }
    }

    size_t aggColWidth = 10;

    for (size_t i = 0; i < columnNames.size() - 1; ++i)
    {
        outputFile << left << setw(columnWidths[i] + 2) << columnNames[i];
    }
    outputFile << setw(aggColWidth + 2) << "SUM"
               << setw(aggColWidth + 2) << "COUNT"
               << setw(aggColWidth + 2) << "MIN"
               << setw(aggColWidth + 2) << "MAX"
               << "AVG\n";

    for (const auto &entry : results)
    {
        stringstream ss(entry.first);
        string value;
        vector<string> values;

        while (getline(ss, value, ','))
        {
            values.push_back(value);
        }

        for (size_t i = 0; i < values.size(); ++i)
        {
            outputFile << left << setw(columnWidths[i] + 2) << (values[i] == "NULL" ? "NULL" : values[i]);
        }

        const AggregatedValues &agg = entry.second;
        outputFile << setw(aggColWidth + 2) << agg.sum
                   << setw(aggColWidth + 2) << agg.count
                   << setw(aggColWidth + 2) << agg.minVal
                   << setw(aggColWidth + 2) << agg.maxVal
                   << setw(aggColWidth + 2) << agg.average() << "\n";
    }

    outputFile.close();
}

vector<pair<string, AggregatedValues>> generateRollup(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, AggregatedValues> rollupResults;

    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
        {
            key += value + ", ";
        }
        key.pop_back();
        key.pop_back();
        addAggregatedValues(rollupResults, key, stod(row.finalColumnValue));
    }

    for (size_t i = 0; i < columnNames.size() - 1; ++i)
    {
        map<string, AggregatedValues> intermediateResults;
        for (const auto &row : data)
        {
            string rollupKey;
            for (size_t j = 0; j < row.values.size(); ++j)
            {
                rollupKey += (j >= (row.values.size() - 1 - i) ? "NULL" : row.values[j]) + ", ";
            }
            rollupKey.pop_back();
            rollupKey.pop_back();
            addAggregatedValues(intermediateResults, rollupKey, stod(row.finalColumnValue));
        }

        for (const auto &entry : intermediateResults)
        {
            if (rollupResults.find(entry.first) == rollupResults.end())
            {
                rollupResults[entry.first] = entry.second;
            }
            else
            {
                rollupResults[entry.first].sum += entry.second.sum;
                rollupResults[entry.first].count += entry.second.count;
                rollupResults[entry.first].minVal = min(rollupResults[entry.first].minVal, entry.second.minVal);
                rollupResults[entry.first].maxVal = max(rollupResults[entry.first].maxVal, entry.second.maxVal);
            }
        }
    }

    AggregatedValues grandTotalValues;
    for (const auto &entry : data)
    {
        grandTotalValues.sum += stod(entry.finalColumnValue);
        ++grandTotalValues.count;
        grandTotalValues.minVal = min(grandTotalValues.minVal, stod(entry.finalColumnValue));
        grandTotalValues.maxVal = max(grandTotalValues.maxVal, stod(entry.finalColumnValue));
    }

    return vector<pair<string, AggregatedValues>>(rollupResults.begin(), rollupResults.end());
}

vector<pair<string, AggregatedValues>> generateCube(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, AggregatedValues> cubeResults;
    double grandTotal = 0;
    size_t numColumns = columnNames.size();

    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
        {
            key += value + ", ";
        }
        key.pop_back();
        key.pop_back();
        addAggregatedValues(cubeResults, key, stod(row.finalColumnValue));
        grandTotal += stod(row.finalColumnValue);
    }

    size_t numCombinations = 1 << (numColumns - 1);
    for (const auto &row : data)
    {
        for (size_t i = 1; i < numCombinations; ++i)
        {
            string cubeKey;
            for (size_t j = 0; j < numColumns - 1; ++j)
            {
                cubeKey += ((i & (1 << j)) ? "NULL" : row.values[j]) + ", ";
            }
            cubeKey.pop_back();
            cubeKey.pop_back();
            addAggregatedValues(cubeResults, cubeKey, stod(row.finalColumnValue));
        }
    }

    return vector<pair<string, AggregatedValues>>(cubeResults.begin(), cubeResults.end());
}

bool findAggregationType(const vector<int>& aggregationTypes, int type) {
    for (int aggType : aggregationTypes) {
        if (aggType == type) return true;
    }
    return false;
}

string trim(const string& str) {
    size_t start = 0;
    while (start < str.size() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n' || str[end - 1] == '\r')) {
        --end;
    }
    return str.substr(start, end - start);
}

void printAggregationResults(const map<string, AggregatedValues>& aggregationResults, const vector<string>& columnNames, const vector<int>& selectedAggregations, const vector<int>& selectedColumns) {
    size_t aggColWidth = 10;

    for (int colIndex : selectedColumns) {
        cout << left << setw(aggColWidth) << columnNames[colIndex];
    }

    if (findAggregationType(selectedAggregations, 1)) cout << setw(aggColWidth) << "SUM";
    if (findAggregationType(selectedAggregations, 2)) cout << setw(aggColWidth) << "COUNT";
    if (findAggregationType(selectedAggregations, 3)) cout << setw(aggColWidth) << "MIN";
    if (findAggregationType(selectedAggregations, 4)) cout << setw(aggColWidth) << "MAX";
    if (findAggregationType(selectedAggregations, 5)) cout << setw(aggColWidth) << "AVG";
    cout << "\n";

    for (const auto& entry : aggregationResults) {
        stringstream ss(entry.first);
        string value;
        vector<string> values;

        while (getline(ss, value, ',')) {
            values.push_back(trim(value)); // Trim spaces from each value before adding to the vector
        }

        bool isValidRow = true;
        int numOfColumns = values.size();
        for (size_t i = 0; i < numOfColumns; ++i) {
            bool isSelected = false;
            for (int colIndex : selectedColumns) {
                if (i == (colIndex)) {
                    isSelected = true;
                    break;
                }
            }

            if (isSelected) {
                if (values[i] == "NULL") {
                    isValidRow = false;
                    break;
                }
            } else {
                if (values[i] != "NULL") {
                    isValidRow = false;
                    break;
                }
            }
        }

        if (isValidRow) {
            cout<<"fjdj";
            for (int colIndex : selectedColumns) {
                cout << left << setw(aggColWidth) << values[colIndex];
            }

            const AggregatedValues& agg = entry.second;
            if (findAggregationType(selectedAggregations, 1)) cout << setw(aggColWidth) << agg.sum;
            if (findAggregationType(selectedAggregations, 2)) cout << setw(aggColWidth) << agg.count;
            if (findAggregationType(selectedAggregations, 3)) cout << setw(aggColWidth) << agg.minVal;
            if (findAggregationType(selectedAggregations, 4)) cout << setw(aggColWidth) << agg.maxVal;
            if (findAggregationType(selectedAggregations, 5)) cout << setw(aggColWidth) << agg.average();
            cout << "\n";
        }
    }
}


void interactiveAggregationQuery(const map<string, AggregatedValues>& aggregationData, const vector<string>& columnNames) {
    while (true) {
        system("cls");

        cout << "Welcome to the Aggregation Query system!\n";
        cout << "Available columns: \n";
        for (size_t i = 0; i < columnNames.size()-1; ++i) {
            cout << (i + 1) << ". " << columnNames[i] << "\n";
        }

        cout << "Please enter the columns you want to query by (e.g., 1,2,4 or type 'exit' to quit, 0 for grand total): ";
        string columnInput;
        getline(cin, columnInput);

        if (columnInput == "exit") {
            cout << "Exiting the query system.\n";
            break;
        }

        vector<int> selectedColumns;
        stringstream columnStream(columnInput);
        string colStr;
        while (getline(columnStream, colStr, ',')) {
            try {
                int colIndex = stoi(colStr) - 1;
                if (colIndex >= 0 && colIndex < columnNames.size()) {
                    selectedColumns.push_back(colIndex-1);
                } else if (colStr == "0") {
                    // for (int i = 0; i < columnNames.size(); ++i) {
                    //     selectedColumns.push_back(i);
                    // }
                    break;
                } else {
                    cout << "Invalid column index: " << colStr << ". Please enter valid column indices.\n";
                }
            } catch (...) {
                cout << "Invalid input! Please enter valid numbers for columns.\n";
                continue;
            }
        }

        if (selectedColumns.empty()) {
            cout << "No valid columns selected. Please try again.\n";
            continue;
        }

        cout << "Which aggregation would you like to perform? (Choose multiple if needed, separated by commas):\n";
        cout << "1. SUM\n2. COUNT\n3. MIN\n4. MAX\n5. AVG\n";
        cout << "Enter the numbers of the aggregation types (e.g., 1,3,5): ";
        string aggregationChoice;
        getline(cin, aggregationChoice);

        vector<int> aggregationTypes;
        stringstream aggStream(aggregationChoice);
        string aggType;
        while (getline(aggStream, aggType, ',')) {
            try {
                int agg = stoi(aggType);
                if (agg >= 1 && agg <= 5) {
                    aggregationTypes.push_back(agg);
                } else {
                    cout << "Invalid aggregation type: " << agg << ". Valid options are 1-5.\n";
                }
            } catch (...) {
                cout << "Invalid aggregation input! Please enter numbers between 1 and 5.\n";
                continue;
            }
        }

        printAggregationResults(aggregationData, columnNames, aggregationTypes, selectedColumns);

        cout << "\nDo you want to make another query? (y/n): ";
        string again;
        getline(cin, again);

        if (again != "y" && again != "Y") {
            cout << "Exiting the query system.\n";
            break;
        }
    }
}

int main()
{
    string tableName;
    vector<string> columnNames;
    string finalColumnName;

    // Assuming parseInputFile and generateRollup/generateCube functions exist and work as expected
    vector<DataRow> data = parseInputFile("input.txt", tableName, columnNames, finalColumnName);

    // Generate rollup and cube results
    vector<pair<string, AggregatedValues>> rollupResults = generateRollup(data, columnNames, finalColumnName);
    vector<pair<string, AggregatedValues>> cubeResults = generateCube(data, columnNames, finalColumnName);

    // Convert results to maps for easy querying
    map<string, AggregatedValues> rollupMap(rollupResults.begin(), rollupResults.end());
    map<string, AggregatedValues> cubeMap(cubeResults.begin(), cubeResults.end());

    // Interactive querying for rollupMap
    interactiveAggregationQuery(cubeMap, columnNames);

    return 0;
}