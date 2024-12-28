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
    double revenue;
};

vector<DataRow> parseInputFile(const string &filename, string &tableName, vector<string> &columnNames)
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
        columnNames.push_back(columnName);
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

vector<pair<string, double>> generateRollup(const vector<DataRow> &data, const vector<string> &columnNames)
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
                rollupKey += (i == j ? "ALL" : row.values[j]) + ", ";
            }
            rollupKey.pop_back();
            rollupKey.pop_back();
            rollupResults[rollupKey] += row.revenue;
        }
        grandTotal += row.revenue;
    }
    rollupResults["ALL, ALL"] = grandTotal;
    vector<pair<string, double>> results;
    for (const auto &entry : rollupResults)
        results.push_back(entry);
    return results;
}

vector<pair<string, double>> generateCube(const vector<DataRow> &data, const vector<string> &columnNames)
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
                cubeKey += ((i & (1 << j)) ? "ALL" : row.values[j]) + ", ";
            }
            cubeKey.pop_back();
            cubeKey.pop_back();
            cubeResults[cubeKey] += row.revenue;
        }
        grandTotal += row.revenue;
    }
    cubeResults["ALL, ALL"] = grandTotal;
    vector<pair<string, double>> results;
    for (const auto &entry : cubeResults)
        results.push_back(entry);
    return results;
}

void writeResultsToFile(const string &filename, const vector<pair<string, double>> &results)
{
    ofstream outputFile(filename);
    if (!outputFile)
    {
        cerr << "Error opening output file!\n";
        exit(1);
    }
    for (const auto &entry : results)
        outputFile << entry.first << ": " << entry.second << "\n";
    outputFile.close();
}

void interactiveQuery(const vector<pair<string, double>> &rollupResults, const vector<pair<string, double>> &cubeResults, const vector<string> &columnNames)
{
    cout << "\nEnter column names to query aggregated results (e.g., 'Region, Product'). Type 'exit' to quit.\n";
    string input;
    while (true)
    {
        cout << "Query: ";
        getline(cin, input);
        if (input == "exit")
            break;
        bool found = false;
        for (const auto &entry : rollupResults)
        {
            if (entry.first == input)
            {
                cout << "ROLLUP Result - " << entry.first << ": " << entry.second << "\n";
                found = true;
                break;
            }
        }
        for (const auto &entry : cubeResults)
        {
            if (entry.first == input)
            {
                cout << "CUBE Result - " << entry.first << ": " << entry.second << "\n";
                found = true;
                break;
            }
        }
        if (!found)
        {
            cout << "Invalid column name. Available column names are: \n";
            for (const auto &col : columnNames)
                cout << col << " ";
            cout << "\n";
        }
    }
}

int main()
{
    string tableName;
    vector<string> columnNames;
    vector<DataRow> data = parseInputFile("input.txt", tableName, columnNames);
    vector<pair<string, double>> rollupResults = generateRollup(data, columnNames);
    vector<pair<string, double>> cubeResults = generateCube(data, columnNames);
    writeResultsToFile("output_rollup.txt", rollupResults);
    writeResultsToFile("output_cube.txt", cubeResults);
    interactiveQuery(rollupResults, cubeResults, columnNames);
    return 0;
}
