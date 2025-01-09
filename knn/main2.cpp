#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <string>
using namespace std;


struct Node
{
    vector<double> point; 
    Node *left;           
    Node *right;         

    Node(const vector<double> &pt) : point(pt), left(nullptr), right(nullptr) {}
};

class KDTree
{
private:
    Node *root;        
    size_t dimensions; 

public:
    KDTree(const vector<vector<double>> &points, size_t dims) : root(nullptr), dimensions(dims)
{
    for (const auto &point : points)
    {
        insert(point);
    }
}


    Node *insertRecursive(Node *node, const vector<double> &point, int depth)
    {
        if (node == nullptr)
            return new Node(point);

        int cd = depth % dimensions; 

        if (point[cd] < node->point[cd])
            node->left = insertRecursive(node->left, point, depth + 1);
        else
            node->right = insertRecursive(node->right, point, depth + 1);

        return node;
    }

    bool searchRecursive(Node *node, const vector<double> &point, int depth) const
    {
        if (node == nullptr)
            return false;

        if (node->point == point)
            return true;

        int cd = depth % dimensions;

        if (point[cd] < node->point[cd])
            return searchRecursive(node->left, point, depth + 1);
        else
            return searchRecursive(node->right, point, depth + 1);
    }

    Node *deleteRecursive(Node *node, const vector<double> &point, int depth)
    {
        if (node == nullptr)
            return nullptr;

        int cd = depth % dimensions;

        if (node->point == point)
        {
            if (node->right != nullptr)
            {
                Node *minNode = findMin(node->right, cd, depth + 1);
                node->point = minNode->point;
                node->right = deleteRecursive(node->right, minNode->point, depth + 1);
            }
            else if (node->left != nullptr)
            {
                Node *minNode = findMin(node->left, cd, depth + 1);
                node->point = minNode->point;
                node->right = deleteRecursive(node->left, minNode->point, depth + 1);
                node->left = nullptr;
            }
            else
            {
                delete node;
                return nullptr;
            }
        }
        else if (point[cd] < node->point[cd])
        {
            node->left = deleteRecursive(node->left, point, depth + 1);
        }
        else
        {
            node->right = deleteRecursive(node->right, point, depth + 1);
        }

        return node;
    }

    Node *findMin(Node *node, int cd, int depth)
    {
        if (node == nullptr)
            return nullptr;

        int currentDim = depth % dimensions;

        if (currentDim == cd)
        {
            if (node->left == nullptr)
                return node;
            return findMin(node->left, cd, depth + 1);
        }

        Node *leftMin = findMin(node->left, cd, depth + 1);
        Node *rightMin = findMin(node->right, cd, depth + 1);

        Node *minNode = node;
        if (leftMin != nullptr && leftMin->point[cd] < minNode->point[cd])
            minNode = leftMin;
        if (rightMin != nullptr && rightMin->point[cd] < minNode->point[cd])
            minNode = rightMin;

        return minNode;
    }

    void printRecursive(Node *node, int depth) const
    {
        if (node == nullptr)
            return;

        for (int i = 0; i < depth; i++)
            cout << "  ";
        cout << "(";
        for (size_t i = 0; i < node->point.size(); i++)
        {
            cout << node->point[i];
            if (i < node->point.size() - 1)
                cout << ", ";
        }
        cout << ")" << endl;

        printRecursive(node->left, depth + 1);
        printRecursive(node->right, depth + 1);
    }

    double calculateDistance(const vector<double> &p1, const vector<double> &p2) const
    {
        double distance = 0.0;
        for (size_t i = 0; i < p1.size(); i++)
        {
            distance += pow(p1[i] - p2[i], 2);
        }
        return sqrt(distance);
    }

public:
    KDTree(size_t dims) : root(nullptr), dimensions(dims) {}

    vector<double> nearestNeighbor(const vector<double> &queryPoint) const
    {
        double minDistance = numeric_limits<double>::max();
        Node *nearest = nullptr;
        nearestNeighborRecursive(root, queryPoint, 0, minDistance, nearest);
        return nearest ? nearest->point : vector<double>();
    }

//private:
    void nearestNeighborRecursive(Node *node, const vector<double> &queryPoint, int depth, double &minDistance, Node *&nearest) const
    {
        if (node == nullptr)
            return;

        double distance = calculateDistance(node->point, queryPoint);
        if (distance < minDistance)
        {
            minDistance = distance;
            nearest = node;
        }

        int cd = depth % dimensions;

        Node *nextNode = queryPoint[cd] < node->point[cd] ? node->left : node->right;
        Node *otherNode = queryPoint[cd] < node->point[cd] ? node->right : node->left;

        nearestNeighborRecursive(nextNode, queryPoint, depth + 1, minDistance, nearest);
        if (fabs(queryPoint[cd] - node->point[cd]) < minDistance)
        {
            nearestNeighborRecursive(otherNode, queryPoint, depth + 1, minDistance, nearest);
        }
    }
    Node* getRoot(){
        return root;
    }

    void insert(const vector<double> &point)
    {
        root = insertRecursive(root, point, 0);
    }

    bool search(const vector<double> &point) const
    {
        return searchRecursive(root, point, 0);
    }

    void remove(const vector<double> &point)
    {
        root = deleteRecursive(root, point, 0);
    }

    void print() const
    {
        printRecursive(root, 0);
    }

//     void calculateDistances(const vector<vector<double>> &points, const string &outputFilename) const
// {
//     ofstream outFile(outputFilename);
//     for (const auto &point : points)
//     {
//         double minDistance = numeric_limits<double>::max();
//         vector<double> nearestPoint = calculateDistancesRecursive(root, point, 0, minDistance);
        
//         outFile << "Point: (";
//         for (size_t i = 0; i < point.size(); i++)
//         {
//             outFile << point[i];
//             if (i < point.size() - 1)
//                 outFile << ", ";
//         }
//         outFile << ") -> Nearest Point: (";
        
//         for (size_t i = 0; i < nearestPoint.size(); i++)
//         {
//             outFile << nearestPoint[i];
//             if (i < nearestPoint.size() - 1)
//                 outFile << ", ";
//         }
        
//         outFile << ") -> Nearest Distance: " << minDistance << endl;
//     }
//     outFile.close();
// }


//     vector<double> calculateDistancesRecursive(Node *node, const vector<double> &targetPoint, int depth, double &minDistance) const
// {
//     if (node == nullptr)
//         return {};

//     double distance = calculateDistance(node->point, targetPoint);
//     vector<double> nearestPoint = node->point;
    
//     if (distance < minDistance)
//     {
//         minDistance = distance;
//     }

//     int cd = depth % dimensions;

//     if (targetPoint[cd] < node->point[cd])
//     {
//         vector<double> leftNearest = calculateDistancesRecursive(node->left, targetPoint, depth + 1, minDistance);
//         if (!leftNearest.empty()) nearestPoint = leftNearest;

//         if (fabs(targetPoint[cd] - node->point[cd]) < minDistance)
//         {
//             vector<double> rightNearest = calculateDistancesRecursive(node->right, targetPoint, depth + 1, minDistance);
//             if (!rightNearest.empty()) nearestPoint = rightNearest;
//         }
//     }
//     else
//     {
//         vector<double> rightNearest = calculateDistancesRecursive(node->right, targetPoint, depth + 1, minDistance);
//         if (!rightNearest.empty()) nearestPoint = rightNearest;

//         if (fabs(targetPoint[cd] - node->point[cd]) < minDistance)
//         {
//             vector<double> leftNearest = calculateDistancesRecursive(node->left, targetPoint, depth + 1, minDistance);
//             if (!leftNearest.empty()) nearestPoint = leftNearest;
//         }
//     }

//     return nearestPoint;
// }


   
};

void insertNode(KDTree &tree, size_t dimensions) {
    string input;
    cout << "Enter the node to insert (comma-separated values): ";
    getline(cin, input);
    stringstream ss(input);
    vector<double> queryPoint;
    string value;
    try {
        while (getline(ss, value, ',')) {
            queryPoint.push_back(stod(value));
        }

        if (queryPoint.size() != dimensions) {
            cerr << "Error: The input node must have exactly " << dimensions << " dimensions.\n";
            return;
        }

        tree.insert(queryPoint);
        cout << "Node inserted successfully.\n";
    } catch (exception &e) {
        cerr << "Invalid input! Please enter valid comma-separated numerical values.\n";
    }
}

void deleteNode(KDTree &tree, size_t dimensions) {
    string input;
    cout << "Enter the node to delete (comma-separated values): ";
    getline(cin, input);
    stringstream ss(input);
    vector<double> queryPoint;
    string value;
    try {
        while (getline(ss, value, ',')) {
            queryPoint.push_back(stod(value));
        }

        if (queryPoint.size() != dimensions) {
            cerr << "Error: The input node must have exactly " << dimensions << " dimensions.\n";
            return;
        }
        if(tree.search(queryPoint)){
        tree.remove(queryPoint);
        cout << "Node deleted successfully.\n";
        }
        else{
            cout << "Node not found in the tree.\n";
        }
    } catch (exception &e) {
        cerr << "Invalid input! Please enter valid comma-separated numerical values.\n";
    }
}

void searchNode(KDTree &tree, size_t dimensions) {
    string input;
    cout << "Enter the node to search for (comma-separated values): ";
    getline(cin, input);
    stringstream ss(input);
    vector<double> queryPoint;
    string value;
    try {
        while (getline(ss, value, ',')) {
            queryPoint.push_back(stod(value));
        }

        if (queryPoint.size() != dimensions) {
            cerr << "Error: The input node must have exactly " << dimensions << " dimensions.\n";
            return;
        }

        if (tree.search(queryPoint)) {
            cout << "Node found in the tree.\n";
        } else {
            cout << "Node not found in the tree.\n";
        }
    } catch (exception &e) {
        cerr << "Invalid input! Please enter valid comma-separated numerical values.\n";
    }
}

void printTree(KDTree &tree) {
    cout << "Printing the tree:\n";
    tree.print();
}

void getNearestNeighbour(KDTree &tree, size_t dimensions) {
    string input;
    cout << "Enter a query point (comma-separated values): ";
    getline(cin, input);
    stringstream ss(input);
    vector<double> queryPoint;
    string value;
    try {
        while (getline(ss, value, ',')) {
            queryPoint.push_back(stod(value));
        }

        if (queryPoint.size() != dimensions) {
            cerr << "Error: The query point must have exactly " << dimensions << " dimensions.\n";
            return;
        }

        
        vector<double> nearestPoint=tree.nearestNeighbor(queryPoint);
        // Write the result to the output file
    cout << "Query Point: (";
    for (size_t i = 0; i < queryPoint.size(); i++) {
        cout << queryPoint[i];
        if (i < queryPoint.size() - 1)
            cout << ", ";
    }
    cout << "), Nearest Point: (";
    
    // Print the nearestPoint values to the file
    for (size_t i = 0; i < nearestPoint.size(); i++) {
        cout << nearestPoint[i];
        if (i < nearestPoint.size() - 1) {
            cout << ", ";
        }
    }
    cout << "),  Nearest Distance: " << tree.calculateDistance(nearestPoint,queryPoint)<< endl;
    cin.get();
    } catch (exception &e) {
        cerr << "Invalid input! Please enter valid comma-separated numerical values.\n";
    }
}

void intializeTree(){
    
}


int main() {
    ifstream inFile("input.txt");
    if (!inFile.is_open()) {
        cerr << "Failed to open input file.\n";
        return 1;
    }

    vector<vector<double>> points;
    vector<vector<double>> queryPoints;
    string line;
    bool isQuerySection = false;

    while (getline(inFile, line)) {
        if (line.empty())
            continue;

        if (line[0] == '#') {
            isQuerySection = true;
            continue;
        }

        stringstream ss(line);
        vector<double> point;
        string value;
        while (getline(ss, value, ',')) {
            point.push_back(stod(value));
        }

        if (isQuerySection) {
            queryPoints.push_back(point);
        } else {
            points.push_back(point);
        }
    }
    inFile.close();

    if (points.empty()) {
        cerr << "No points found in the input file.\n";
        return 1;
    }

    size_t dimensions = points[0].size();
    KDTree tree(points, dimensions);

    
    ofstream outFile("results.txt");
    if (!outFile.is_open()) {
        cerr << "Failed to open output file.\n";
        return 1;
    }

 
outFile << "Training Phase Results:\n";
for (const auto& queryPoint : queryPoints) {
    if (queryPoint.size() != dimensions) {
        cerr << "Error: Query point dimension mismatch.\n";
        continue;
    }

    vector<double> nearestPoint = tree.nearestNeighbor(queryPoint);

   
    outFile << "Query Point: (";
    for (size_t i = 0; i < queryPoint.size(); i++) {
        outFile << queryPoint[i];
        if (i < queryPoint.size() - 1)
            outFile << ", ";
    }
    outFile  << "), Nearest Point: (";
    
  
    for (size_t i = 0; i < nearestPoint.size(); i++) {
        outFile << nearestPoint[i];
        if (i < nearestPoint.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "), Nearest Distance: " << tree.calculateDistance(nearestPoint,queryPoint) << endl;
}

outFile << "\nEnd of Training Phase\n";


    while (true) {
        system("cls");
        cout << "\nInteractive Query Session:\n";
        cout << "1. Insert a node\n";
        cout << "2. Delete a node\n";
        cout << "3. Search a node\n";
        cout << "4. Print the tree\n";
        cout << "5. Get a point's nearest neighbour\n";
        cout << "6. Exit\n";
        cout << "Choose an option (1-6): ";

        int option;
        cin >> option;
        cin.ignore(); 
        

        if (option == 1) {
            insertNode(tree, dimensions);  
        } else if (option == 2) {
            deleteNode(tree, dimensions);  
        } else if (option == 3) {
            searchNode(tree, dimensions); 
        } else if (option == 4) {
            printTree(tree);  
        } else if (option == 5) {
            getNearestNeighbour(tree, dimensions);  
        } else if (option == 6) {
            cout << "Exiting interactive session.\n";
            return 0;
        } else {
            cout << "Invalid option! Please choose a number between 1 and 6.\n";
        }
        cin.get();
    }

    return 0;
}



