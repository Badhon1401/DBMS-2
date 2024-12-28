#include <bits/stdc++.h>
using namespace std;

int main() {
   // ifstream file("data.csv");  // Open the CSV file
    ifstream inputFile("input.txt");
    string line;
    vector<vector<string>> transactions;  // 2D vector to store CSV data
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string item;
        vector<string> transaction;
        while (ss >> item) {
            transaction.push_back(item);
        }
        transactions.push_back(transaction);
    }
    inputFile.close();
    // if (!file.is_open()) {
    //     cerr << "Could not open the file!" << endl;
    //     return 1;
    // }
    // int n=5;

    // string line;
    // while (getline(file, line) && n>0) {
    //     stringstream ss(line);
    //     string cell;
    //     vector<string> row;

    //     while (getline(ss, cell, ',')) {  // Splits cells by comma
    //         row.push_back(cell);
    //     }
    //     data.push_back(row);  // Add the row to the main data vector
    //     n--;
    // }

    //file.close();

    unordered_map<string,vector<int>> map;
    set<string> items;
for (int i = 0; i < transactions.size(); ++i) {
    for (const auto& cell : transactions[i]) {
        map[cell].push_back(i);
        items.insert(cell);
    }
}
 int ssz=items.size();
 int threshold=2;
 //pair<int,vector<pair<int,vector<string>>>> data;
 vector<string> current_items(items.begin(), items.end());
 set<string> dummy;
    for(int i=0;i<current_items.size();i++){
        if(map[current_items[i]].size()>=threshold){
            dummy.insert(current_items[i]);
        }
    }

  // Traverse and print the map without structured bindings
    // for (const auto& pair : map) {
    //     const string& key = pair.first;                // Access the key
    //     const vector<int>& vec = pair.second;          // Access the vector

    //     cout << "String: " << key << "\nVector contents: ";
    //     for (const int& value : vec) {
    //         cout << value << " ";
    //     }
    //     cout << "\nLength of vector: " << vec.size() << "\n\n";
    // }
    
    // Access a specific element (e.g., element in row 1, column 2)
// string specificData = data[1][2];

// Print all data
// for (const auto& row : data) {
//     for (const auto& cell : row) {
//         cout << cell << " ";
//     }
//     cout << endl;
// }

    return 0;
}
