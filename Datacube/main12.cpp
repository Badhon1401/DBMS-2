#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

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

bool findAggregationType(const vector<int> &aggregationTypes, int type)
{
    for (size_t i = 0; i < aggregationTypes.size(); ++i)
    {
        if (aggregationTypes[i] == type)
        {
            return true;
        }
    }
    return false;
}

void printAggregationResults(const map<string, AggregatedValues> &aggregationResults, const vector<string> &columnNames)
{
    size_t aggColWidth = 10;

    for (size_t i = 0; i < columnNames.size(); ++i)
    {
        cout << left << setw(aggColWidth) << columnNames[i];
    }
    cout << setw(aggColWidth) << "SUM"
         << setw(aggColWidth) << "COUNT"
         << setw(aggColWidth) << "MIN"
         << setw(aggColWidth) << "MAX"
         << "AVG\n";

    for (const auto &entry : aggregationResults)
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
            cout << left << setw(aggColWidth) << (values[i] == "NULL" ? "NULL" : values[i]);
        }

        const AggregatedValues &agg = entry.second;
        cout << setw(aggColWidth) << agg.sum
             << setw(aggColWidth) << agg.count
             << setw(aggColWidth) << agg.minVal
             << setw(aggColWidth) << agg.maxVal
             << setw(aggColWidth) << agg.average() << "\n";
    }
}

void interactiveAggregationQuery(const map<string, AggregatedValues> &rollupMap,
                                 const map<string, AggregatedValues> &cubeMap,
                                 const vector<string> &columnNames)
{
    while (true)
    {
        system("cls");

        cout << "Welcome to the Aggregation Query system!\n";
        cout << "Available columns: \n";
        for (size_t i = 0; i < columnNames.size()-1; ++i)
        {
            cout << (i + 1) << ". " << columnNames[i] << "\n";
        }

        cout << "Please enter the column you want to query by (or type 'exit' to quit): ";
        string columnInput;
        getline(cin, columnInput);

        if (columnInput == "exit")
        {
            cout << "Exiting the query system.\n";
            break;
        }

        int columnIndex = -1;
        try
        {
            columnIndex = stoi(columnInput) - 1;
        }
        catch (...)
        {
            cout << "Invalid input! Please enter a valid column number.\n";
            continue;
        }

        if (columnIndex < 0 || columnIndex >= columnNames.size())
        {
            cout << "Invalid column number. Please enter a number between 1 and " << columnNames.size() << ".\n";
            continue;
        }

        cout << "Which aggregation would you like to perform? (Choose multiple if needed, separated by commas):\n";
        cout << "1. SUM\n2. COUNT\n3. MIN\n4. MAX\n5. AVG\n";
        cout << "Enter the numbers of the aggregation types (e.g., 1,3,5): ";
        string aggregationChoice;
        getline(cin, aggregationChoice);

        vector<int> aggregationTypes;
        stringstream ss(aggregationChoice);
        string aggType;
        while (getline(ss, aggType, ','))
        {
            try
            {
                int agg = stoi(aggType);
                if (agg >= 1 && agg <= 5)
                {
                    aggregationTypes.push_back(agg);
                }
                else
                {
                    cout << "Invalid aggregation type: " << agg << ". Valid options are 1-5.\n";
                }
            }
            catch (...)
            {
                cout << "Invalid aggregation input! Please enter numbers between 1 and 5.\n";
                continue;
            }
        }

        map<string, AggregatedValues> selectedResults;
        if (columnIndex == 0)
        {
            selectedResults = rollupMap;
        }
        else if (columnIndex == 1)
        {
            selectedResults = cubeMap;
        }

        map<string, AggregatedValues> aggregationResults;
        for (const auto &entry : selectedResults)
        {
            const AggregatedValues &agg = entry.second;

            AggregatedValues result = {};
            if (findAggregationType(aggregationTypes, 1))
            {
                result.sum = agg.sum;
            }
            if (findAggregationType(aggregationTypes, 2))
            {
                result.count = agg.count;
            }
            if (findAggregationType(aggregationTypes, 3))
            {
                result.minVal = agg.minVal;
            }
            if (findAggregationType(aggregationTypes, 4))
            {
                result.maxVal = agg.maxVal;
            }
            if (findAggregationType(aggregationTypes, 5))
            {
                result.sum = agg.average();
            }

            aggregationResults[entry.first] = result;
        }

        printAggregationResults(aggregationResults, columnNames);

        cout << "\nDo you want to make another query? (y/n): ";
        string again;
        getline(cin, again);

        if (again != "y" && again != "Y")
        {
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

    vector<DataRow> data = parseInputFile("input.txt",tableName, columnNames, finalColumnName);

    vector<pair<string, AggregatedValues>> rollupResults = generateRollup(data, columnNames, finalColumnName);
    vector<pair<string, AggregatedValues>> cubeResults = generateCube(data, columnNames, finalColumnName);

    map<string, AggregatedValues> rollupMap(rollupResults.begin(), rollupResults.end());
    map<string, AggregatedValues> cubeMap(cubeResults.begin(), cubeResults.end());

    writeResultsToFile("output_rollup.txt", rollupMap, columnNames);
    writeResultsToFile("output_cube.txt", cubeMap, columnNames);

    interactiveAggregationQuery(rollupMap, cubeMap, columnNames);

    return 0;
}