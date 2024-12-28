#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;

struct DataRow {
    vector<string> values;
    string finalColumnValue;
};

struct AggregatedValues {
    double sum = 0;
    int count = 0;
    double minVal = numeric_limits<double>::max();
    double maxVal = numeric_limits<double>::lowest();

    void addValue(double value) {
        sum += value;
        count++;
        minVal = min(minVal, value);
        maxVal = max(maxVal, value);
    }

    double average() const {
        return count == 0 ? 0 : sum / count;
    }
};

vector<DataRow> parseInputFile(const string &filename, string &tableName, vector<string> &columnNames, string &finalColumnName) {
    ifstream inputFile(filename);
    vector<DataRow> data;
    if (!inputFile) {
        cerr << "Error opening input file!\n";
        exit(1);
    }

    getline(inputFile, tableName);
    string line;
    getline(inputFile, line);
    stringstream ss(line);
    string columnName;
    while (ss >> columnName) {
        columnNames.push_back(columnName);
    }

    finalColumnName = columnNames.back();

    while (getline(inputFile, line)) {
        stringstream dataStream(line);
        DataRow row;
        string value;
        for (size_t i = 0; i < columnNames.size() - 1; ++i) {
            dataStream >> value;
            row.values.push_back(value);
        }
        dataStream >> row.finalColumnValue;
        data.push_back(row);
    }
    inputFile.close();
    return data;
}

void addAggregatedValues(map<string, AggregatedValues> &results, const string &key, double value) {
    results[key].addValue(value);
}

void writeResultsToFile(const string &filename, const map<string, AggregatedValues> &results, const vector<string> &columnNames) {
    ofstream outputFile(filename);
    if (!outputFile) {
        cerr << "Error opening output file!\n";
        exit(1);
    }

    outputFile << left << setw(10) << columnNames[0] << setw(10) << columnNames[1] 
               << setw(8) << "SUM" << setw(8) << "COUNT" << setw(8) << "MIN" 
               << setw(8) << "MAX" << "AVG\n";

    for (const auto &entry : results) {
        stringstream ss(entry.first);
        string value;
        vector<string> values;
        while (getline(ss, value, ',')) {
            values.push_back(value);
        }

        for (const auto &val : values) {
            outputFile << left << setw(10) << (val == "NULL" ? "NULL" : val);
        }

        const AggregatedValues &agg = entry.second;
        outputFile << setw(8) << agg.sum << setw(8) << agg.count 
                   << setw(8) << agg.minVal << setw(8) << agg.maxVal 
                   << agg.average() << "\n";
    }

    outputFile.close();
}

vector<pair<string, AggregatedValues>> generateRollup(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, AggregatedValues> rollupResults;

    // Step 1: Sum up all direct data entries
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

    // Step 2: Perform intermediate rollup aggregations
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

        // Aggregate intermediate results without duplication
        for (const auto &entry : intermediateResults)
        {
            rollupResults[entry.first].sum += entry.second.sum;
            rollupResults[entry.first].count += entry.second.count;
            rollupResults[entry.first].minVal = min(rollupResults[entry.first].minVal, entry.second.minVal);
            rollupResults[entry.first].maxVal = max(rollupResults[entry.first].maxVal, entry.second.maxVal);
        }
    }

    // Add grand total once, not as a summation of rollups
    double grandTotal = 0;
    for (const auto &entry : data)
    {
        grandTotal += stod(entry.finalColumnValue);
    }
    rollupResults["NULL, NULL"].addValue(grandTotal);

    return vector<pair<string, AggregatedValues>>(rollupResults.begin(), rollupResults.end());
}

vector<pair<string, AggregatedValues>> generateCube(const vector<DataRow> &data, const vector<string> &columnNames, const string &finalColumnName)
{
    map<string, AggregatedValues> cubeResults;
    AggregatedValues grandTotalValues;

    // Step 1: Sum all base data entries (no over-counting)
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
        
        // Accumulate grand total values
        grandTotalValues.sum += stod(row.finalColumnValue);
        ++grandTotalValues.count;
        grandTotalValues.minVal = min(grandTotalValues.minVal, stod(row.finalColumnValue));
        grandTotalValues.maxVal = max(grandTotalValues.maxVal, stod(row.finalColumnValue));
    }

    // Step 2: Create higher-level NULL combinations (don't double-count already included combinations)
    size_t numColumns = columnNames.size();
    for (const auto &row : data)
    {
        // Generate combinations using bit masking (but don't generate combinations that already exist)
        size_t numCombinations = 1 << numColumns;
        for (size_t i = 1; i < numCombinations; ++i)
        {
            string cubeKey;
            for (size_t j = 0; j < numColumns; ++j)
            {
                // Only add NULL for specific levels of the hierarchy
                cubeKey += ((i & (1 << j)) ? "NULL" : row.values[j]) + ", ";
            }
            cubeKey.pop_back();
            cubeKey.pop_back();
            addAggregatedValues(cubeResults, cubeKey, stod(row.finalColumnValue));
        }
    }

    // Step 3: Add grand total to the cube result only once
    cubeResults["NULL, NULL"] = grandTotalValues;

    // Return cube results as a vector of pairs
    return vector<pair<string, AggregatedValues>>(cubeResults.begin(), cubeResults.end());
}


int main() {
    string tableName;
    vector<string> columnNames;
    string finalColumnName;
    vector<DataRow> data = parseInputFile("input.txt", tableName, columnNames, finalColumnName);

    vector<pair<string, AggregatedValues>> rollupResults = generateRollup(data, columnNames, finalColumnName);
    vector<pair<string, AggregatedValues>> cubeResults = generateCube(data, columnNames, finalColumnName);

    map<string, AggregatedValues> rollupMap(rollupResults.begin(), rollupResults.end());
    map<string, AggregatedValues> cubeMap(cubeResults.begin(), cubeResults.end());

    writeResultsToFile("output_rollup.txt", rollupMap, columnNames);
    writeResultsToFile("output_cube.txt", cubeMap, columnNames);

    return 0;
}
