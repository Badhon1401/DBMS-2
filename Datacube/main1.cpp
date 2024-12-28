#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <set>
#include <algorithm>

using namespace std;

struct DataRow
{
    vector<string> values;
    double revenue;
};

vector<DataRow> parseInputFile(const string &filename, string &tableName, vector<string> &columnNames, string &revenueColumn)
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

    revenueColumn = columnNames.back();  // Assume revenue is the last column
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
        dataStream >> row.revenue;
        data.push_back(row);
    }
    inputFile.close();
    return data;
}

vector<pair<string, double>> generateRollup(const vector<DataRow> &data, const vector<string> &columnNames, const string &revenueColumn)
{
    map<string, double> rollupResults;
    double grandTotal = 0;

    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
            key += value + ", ";
        key.pop_back();
        key.pop_back();
        rollupResults[key] += row.revenue;

        for (size_t i = 0; i < row.values.size(); ++i)
        {
            string rollupKey;
            for (size_t j = 0; j < row.values.size(); ++j)
            {
                rollupKey += (i == j ? "NULL" : row.values[j]) + ", ";
            }
            rollupKey.pop_back();
            rollupKey.pop_back();
            rollupResults[rollupKey] += row.revenue;
        }

        grandTotal += row.revenue;
    }
    rollupResults["NULL, NULL"] = grandTotal;

    vector<pair<string, double>> results;
    for (const auto &entry : rollupResults)
        results.push_back(entry);

    return results;
}

vector<pair<string, double>> generateCube(const vector<DataRow> &data, const vector<string> &columnNames, const string &revenueColumn)
{
    map<string, double> cubeResults;
    double grandTotal = 0;

    for (const auto &row : data)
    {
        string key;
        for (const auto &value : row.values)
            key += value + ", ";
        key.pop_back();
        key.pop_back();

        cubeResults[key] += row.revenue;

        size_t numCombinations = 1 << row.values.size();
        for (size_t i = 0; i < numCombinations; ++i)
        {
            string cubeKey;
            for (size_t j = 0; j < row.values.size(); ++j)
            {
                cubeKey += ((i & (1 << j)) ? "NULL" : row.values[j]) + ", ";
            }
            cubeKey.pop_back();
            cubeKey.pop_back();
            cubeResults[cubeKey] += row.revenue;
        }

        grandTotal += row.revenue;
    }
    cubeResults["NULL, NULL"] = grandTotal;

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

    outputFile << columnNames[0] << "\t" << columnNames[1] << "\tTotalRevenue\n";

    vector<pair<string, double>> sortedResults(results.begin(), results.end());
    sort(sortedResults.begin(), sortedResults.end(), [](const pair<string, double> &a, const pair<string, double> &b)
         {
             return a.first != "NULL, NULL" && b.first == "NULL, NULL"; });

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
    string revenueColumn;
    vector<DataRow> data = parseInputFile("input.txt", tableName, columnNames, revenueColumn);

    vector<pair<string, double>> rollupResults = generateRollup(data, columnNames, revenueColumn);
    vector<pair<string, double>> cubeResults = generateCube(data, columnNames, revenueColumn);

    writeResultsToFile("output_rollup.txt", rollupResults, columnNames);
    writeResultsToFile("output_cube.txt", cubeResults, columnNames);

    return 0;
}
