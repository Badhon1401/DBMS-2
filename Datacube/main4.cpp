#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

using namespace std;

struct DataRow
{
    vector<string> values;
    string finalColumnValue; // to store the final column's value
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

    finalColumnName = columnNames.back();  // Assume the last column is the final column (revenue or otherwise)

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
        dataStream >> row.finalColumnValue;  // Store the final column value dynamically
        data.push_back(row);
    }
    inputFile.close();
    return data;
}

void Sort(vector<pair<string, double>> &results)
{
    for (size_t i = 0; i < results.size(); ++i)
    {
        for (size_t j = i + 1; j < results.size(); ++j)
        {
            // Ensure "NULL, NULL" is last
            bool swapCondition = (results[i].first == "NULL, NULL") || 
                                 (results[j].first != "NULL, NULL" && results[i].first > results[j].first);
            if (swapCondition)
            {
                swap(results[i], results[j]);
            }
        }
    }
}

vector<pair<string, double>> generateRollup(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, double> rollupResults;
    double grandTotal = 0;

    // Base-level aggregation (no NULLs)
    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
            key += value + ", ";
        key.pop_back();
        key.pop_back();  // Remove trailing comma and space
        rollupResults[key] += stod(row.finalColumnValue);
        grandTotal += stod(row.finalColumnValue);
    }

    // Generate rollup levels progressively by replacing suffixes (rightmost columns) with NULL
    for (size_t i = 0; i < columnNames.size() - 1; ++i)
    {
        map<string, double> intermediateResults; // Temporary map for current level aggregation
        for (const auto &row : data)
        {
            string rollupKey;
            for (size_t j = 0; j < row.values.size(); ++j)
            {
                // Replace values starting from the rightmost columns
                rollupKey += (j >= (row.values.size() - 1 - i) ? "NULL" : row.values[j]) + ", ";
            }
            rollupKey.pop_back();
            rollupKey.pop_back();  // Remove trailing comma and space
            intermediateResults[rollupKey] += stod(row.finalColumnValue);
        }

        // Merge intermediate results with rollupResults
        for (const auto &entry : intermediateResults)
        {
            rollupResults[entry.first] += entry.second;
        }
    }

    // Add grand total entry
    rollupResults["NULL, NULL"] = grandTotal;

    // Convert to vector
    vector<pair<string, double>> results;
    for (const auto &entry : rollupResults)
        results.push_back(entry);

    return results;
}


vector<pair<string, double>> generateCube(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, double> cubeResults;
    double grandTotal = 0;

    // Base-level aggregation (no NULLs)
    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
            key += value + ", ";
        key.pop_back();
        key.pop_back();
        cubeResults[key] += stod(row.finalColumnValue);
        grandTotal += stod(row.finalColumnValue);
    }

    // Generate all combinations of NULLs
    size_t numColumns = columnNames.size() - 1; // Exclude final column
    for (const auto &row : data)
    {
        size_t numCombinations = 1 << numColumns; // 2^numColumns combinations
        for (size_t i = 1; i < numCombinations; ++i) // Start from 1 to avoid all non-NULLs
        {
            string cubeKey;
            for (size_t j = 0; j < numColumns; ++j)
            {
                cubeKey += ((i & (1 << j)) ? "NULL" : row.values[j]) + ", ";
            }
            cubeKey.pop_back();
            cubeKey.pop_back();
            cubeResults[cubeKey] += stod(row.finalColumnValue);
        }
    }

    // Add grand total entry
    cubeResults["NULL, NULL"] = grandTotal;

    // Convert to vector
    vector<pair<string, double>> results;
    for (const auto &entry : cubeResults)
        results.push_back(entry);

    return results;
}

void writeResultsToFile(const string &filename, const vector<pair<string, double>> &results, const vector<string> &columnNames)
{
    ofstream outputFile(filename);
    if (!outputFile)
    {
        cerr << "Error opening output file!\n";
        exit(1);
    }

    outputFile << columnNames[0] << "\t" << columnNames[1] << "\t" << columnNames.back() << "\n";
   
    vector<pair<string, double>> sortedResults(results.begin(), results.end());
    Sort(sortedResults);

    for (const auto &entry : sortedResults)
    {
        stringstream ss(entry.first);
        string value;
        vector<string> values;
        while (getline(ss, value, ','))
            values.push_back(value);

        for (size_t i = 0; i < values.size(); ++i)
        {
            if (values[i] == "NULL")
                values[i] = "NULL";
            outputFile << values[i] << "\t";
        }

        outputFile << entry.second << "\n";
    }

    outputFile.close();
}

int main()
{
    string tableName;
    vector<string> columnNames;
    string finalColumnName;
    vector<DataRow> data = parseInputFile("input.txt", tableName, columnNames, finalColumnName);

    vector<pair<string, double>> rollupResults = generateRollup(data, columnNames, finalColumnName);
    vector<pair<string, double>> cubeResults = generateCube(data, columnNames, finalColumnName);

    writeResultsToFile("output_rollup.txt", rollupResults, columnNames);
    writeResultsToFile("output_cube.txt", cubeResults, columnNames);

    return 0;
}
