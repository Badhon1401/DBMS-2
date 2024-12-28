#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>

using namespace std;

// Function to check if a cycle exists in the graph (DFS based) and track cycle path
bool dfs(int node, vector<vector<int>>& graph, vector<int>& visited, vector<int>& recStack, vector<int>& cyclePath) {
    visited[node] = 1;
    recStack[node] = 1;
    cyclePath.push_back(node);

    for (int neighbor : graph[node]) {
        if (!visited[neighbor] && dfs(neighbor, graph, visited, recStack, cyclePath)) {
            return true;
        } else if (recStack[neighbor]) {
            cyclePath.push_back(neighbor);  // Include the start of the cycle in path
            return true;
        }
    }

    recStack[node] = 0;
    cyclePath.pop_back();
    return false;
}

// Function to detect cycles in the graph and show the cycle path if present
bool hasCycle(vector<vector<int>>& graph, int n) {
    vector<int> visited(n, 0);
    vector<int> recStack(n, 0);
    vector<int> cyclePath;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            cyclePath.clear();
            if (dfs(i, graph, visited, recStack, cyclePath)) {
                cout << "\nCycle detected: ";
                for (int j = 0; j < cyclePath.size(); j++) {
                    cout << "T" << cyclePath[j] + 1 << " ";
                }
                cout << endl;
                return true;
            }
        }
    }
    return false;
}

bool isConflictSerializable(vector<vector<string>>& transactions) {
    int numOfTransactions = transactions.size();
    int numOfOperations = transactions[0].size();

   /// vector<vector<int>> graph(numOfTransactions, vector<int>(numOfTransactions, 0));  // Precedence graph
   vector<vector<int>> graph(numOfTransactions);

    for (int tid = 0; tid < numOfTransactions; tid++) {
        for (int operationId = 0; operationId < numOfOperations; operationId++) {
            if (transactions[tid][operationId] == "-") continue;  // Skip idle cycles

            char operation = transactions[tid][operationId][0];    // Read or Write
            char item = transactions[tid][operationId][2];         // Extract data item (A, B, etc.)

            // Detect conflicts for read and write operations
            for (int otherTid = 0; otherTid < numOfTransactions; otherTid++) {
                if (otherTid == tid) continue;

                for (int otherOpId = operationId + 1; otherOpId < numOfOperations; otherOpId++) {
                    if (transactions[otherTid][otherOpId] == "-") continue;

                    char otherOperation = transactions[otherTid][otherOpId][0];
                    char otherItem = transactions[otherTid][otherOpId][2];

                    // Check for conflict and add edge in precedence graph
                    if (item == otherItem) {
                        if ((operation == 'R' && otherOperation == 'W') || (operation == 'W' && otherOperation == 'R') || (operation == 'W' && otherOperation == 'W')) {
                            graph[tid].push_back(otherTid);
                            cout << "There is a precedence from T" << tid + 1 << " to T" << otherTid + 1 << " on item " << item << endl;
                            break;
                        }
                    }
                }
            }
        }
    }
    for(int i=0;i<numOfTransactions;i++){
        for(int j:graph[i]){
            cout<<j;
        }
        cout<<endl;
    }
    // Check for cycles in the precedence graph
    return !hasCycle(graph, numOfTransactions);
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
    cout << "Transactions from input.txt:\n";
    for (int i = 0; i < transactions.size(); i++) {
        cout << "T" << i + 1 << ": ";
        for (const auto& op : transactions[i]) {
            cout << op << " ";
        }
        cout << endl;
    }

    // Check if the schedule is conflict serializable
    if (isConflictSerializable(transactions)) {
        cout << "\nThe schedule is conflict serializable.\n";
    } else {
        cout << "\nThe schedule is NOT conflict serializable (cycle detected).\n";
    }

    return 0;
}
