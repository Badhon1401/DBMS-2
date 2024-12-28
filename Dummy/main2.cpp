#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>

using namespace std;

// Function to perform DFS and detect cycles
bool dfs(int node, vector<vector<int>>& graph, vector<int>& visited, vector<int>& recStack) {
    visited[node] = 1;
    recStack[node] = 1;

    for (int neighbor : graph[node]) {
        if (!visited[neighbor] && dfs(neighbor, graph, visited, recStack))
            return true;
        else if (recStack[neighbor])
            return true;
    }

    recStack[node] = 0;
    return false;
}

// Function to check for cycles in the dependency graph
bool hasCycle(vector<vector<int>>& graph, int numTransactions) {
    vector<int> visited(numTransactions, 0);
    vector<int> recStack(numTransactions, 0);

    for (int i = 0; i < numTransactions; i++) {
        if (!visited[i] && dfs(i, graph, visited, recStack))
            return true;
    }
    return false;
}

// Function to parse input file and build the dependency graph
bool checkConflictSerializable(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return false;
    }

    string line;
    vector<vector<string>> transactions;
    unordered_map<string, vector<int>> lastWrite; // Map to track last writer for each variable

    // Read and parse each line as a transaction
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string operation;
        vector<string> transaction;
        
        // Read operations in each cycle
        while (ss >> operation) {
            transaction.push_back(operation);
        }
        transactions.push_back(transaction);
    }

    int numTransactions = transactions.size();
    vector<vector<int>> dependencyGraph(numTransactions);

    // Build the dependency graph based on write-write conflicts
    for (int cycle = 0; cycle < transactions[0].size(); cycle++) {
        unordered_map<string, int> writersInCycle;
        
        // Check for writes in the current cycle
        for (int txn = 0; txn < numTransactions; txn++) {
            if (cycle < transactions[txn].size()) {
                string op = transactions[txn][cycle];
                if (op[0] == 'W') {
                    string variable = op.substr(2, op.size() - 3); // Extract the variable from W(X)
                    
                    // Record the writer for this cycle
                    if (writersInCycle.find(variable) != writersInCycle.end()) {
                        int previousTxn = writersInCycle[variable];
                        dependencyGraph[previousTxn].push_back(txn);
                    }
                    writersInCycle[variable] = txn;
                }
            }
        }
    }

    // Check for cycles in the dependency graph
    if (hasCycle(dependencyGraph, numTransactions)) {
        cout << "The schedule is NOT conflict serializable (cycle detected)." << endl;
        return false;
    } else {
        cout << "The schedule is conflict serializable." << endl;
        return true;
    }
}

int main() {
    string filename = "input.txt"; // Input file name
    checkConflictSerializable(filename);
    return 0;
}
