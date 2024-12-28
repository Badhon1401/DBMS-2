#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>

using namespace std;

// Function to check if a cycle exists in the graph (DFS based)
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

// Function to detect cycles in the graph
bool hasCycle(vector<vector<int>>& graph, int n) {
    vector<int> visited(n, 0);
    vector<int> recStack(n, 0);

    for (int i = 0; i < n; i++) {
        if (!visited[i] && dfs(i, graph, visited, recStack))
            return true;
    }

    return false;
}

bool isConflictSerializable(vector<vector<string>>& transactions) {
  //  map<char, int> lastRead, lastWrite;  
    int numOfTransactions = transactions.size();
    int numOfOperations = transactions[0].size();
    
    vector<vector<int>> graph(numOfTransactions);  // Precedence graph
    
    for (int tid = 0; tid < numOfTransactions; tid++) {
        for (const auto& trans : transactions[tid]) {
            if (trans == "-") continue;  // Skip idle cycles

            char operation = trans[0];  // Read or Write
            char item = trans[2];        // Data item (A, B, etc.)

            if (operation == 'R') {
                // If another transaction wrote to this item before, create a dependency
                if (lastWrite.find(item) != lastWrite.end() && lastWrite[item] != tid) {
                    graph[lastWrite[item]].push_back(tid);  // Edge from last writer to this reader
                }
                lastRead[item] = tid;
                break;
            } else if (operation == 'W') {
                // If another transaction wrote to or read from this item before, create dependencies
                if (lastWrite.find(item) != lastWrite.end() && lastWrite[item] != tid) {
                    graph[lastWrite[item]].push_back(tid);  // Edge from last writer to this writer
                }
                if (lastRead.find(item) != lastRead.end() && lastRead[item] != tid) {
                    graph[lastRead[item]].push_back(tid);  // Edge from last reader to this writer
                }
                lastWrite[item] = tid;
                break;
            }
        }
    }

    // Check for cycles in the precedence graph
    return !hasCycle(graph, n);
}

// Main function to read transactions from a file and process them
int main() {
    ifstream inputFile("input.txt");

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open input file." << endl;
        return 1;
    }

    string line;
    vector<vector<string>> transactions;
    
    // Reading the transactions from input file
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string operation;
        vector<string> transaction;
        while (ss >> operation) {
            transaction.push_back(operation);
        }
        transactions.push_back(transaction);
    }

    inputFile.close();

    // Print transaction information
    // cout << "Transactions from input.txt:\n";
    // for (int i = 0; i < transactions.size(); i++) {
    //     cout << "T" << i + 1 << ": ";
    //     for (const auto& op : transactions[i]) {
    //         cout << op << " ";
    //     }
    //     cout << endl;
    // }

    // Check if the schedule is conflict serializable
    if (isConflictSerializable(transactions)) {
        cout << "\nThe schedule is conflict serializable.\n";
    } else {
        cout << "\nThe schedule is NOT conflict serializable (cycle detected).\n";
    }

    return 0;
}
