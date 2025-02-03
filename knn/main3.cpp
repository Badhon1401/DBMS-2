#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <string>
#include <queue>
#include <algorithm>
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

    vector<Node *> KnearestNeighbors(const vector<double> &queryPoint, int numberOfNeighbors) const{
   
    priority_queue<pair<double, Node *>> nearestNeighbors;
    
    KnearestNeighborsRecursive(root, queryPoint, 0, nearestNeighbors, numberOfNeighbors);

    vector<Node *> result;

    while (!nearestNeighbors.empty())
    {
        result.push_back(nearestNeighbors.top().second);
        nearestNeighbors.pop();
    }

    reverse(result.begin(), result.end());
    return result;
}

void KnearestNeighborsRecursive(Node *node, const vector<double> &queryPoint, int depth, 
                                priority_queue<pair<double, Node *>> &nearestNeighbors, int numberOfNeighbors) const
{
    if (node == nullptr)
        return;

    double distance = calculateDistance(node->point, queryPoint);

    if (nearestNeighbors.size() < numberOfNeighbors)
    {
        nearestNeighbors.emplace(distance, node);
    }
    else if (distance < nearestNeighbors.top().first)
    {
        nearestNeighbors.pop();
        nearestNeighbors.emplace(distance, node);
    }

    int cd = depth % dimensions;
    Node *nextNode = queryPoint[cd] < node->point[cd] ? node->left : node->right;
    Node *otherNode = queryPoint[cd] < node->point[cd] ? node->right : node->left;

    KnearestNeighborsRecursive(nextNode, queryPoint, depth + 1, nearestNeighbors, numberOfNeighbors);

    if (fabs(queryPoint[cd] - node->point[cd]) < nearestNeighbors.top().first || nearestNeighbors.size() < numberOfNeighbors)
    {
        KnearestNeighborsRecursive(otherNode, queryPoint, depth + 1, nearestNeighbors, numberOfNeighbors);
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

void getKNearestNeighbours(KDTree &tree, size_t dimensions) {
    string input;
    cout << "Enter a query point (comma-separated values): ";
    getline(cin, input);

    vector<double> queryPoint;
    stringstream ss(input);
    string value;

    try {
        while (getline(ss, value, ',')) {
            queryPoint.push_back(stod(value));
        }

        if (queryPoint.size() != dimensions) {
            cerr << "Error: The query point must have exactly " << dimensions << " dimensions.\n";
            return;
        }

        cout << "Enter the number of nearest neighbours you want to find: ";
        int numberOfNearestNeighbours;
        cin >> numberOfNearestNeighbours;

        vector<Node *> nearestNodes = tree.KnearestNeighbors(queryPoint, numberOfNearestNeighbours);

        // sort(nearestNodes.begin(), nearestNodes.end(),
        //      [&queryPoint, &tree](Node *a, Node *b) {
        //          return tree.calculateDistance(a->point, queryPoint) < tree.calculateDistance(b->point, queryPoint);
        //      });

        cout << "Query Point: (";
        for (size_t i = 0; i < queryPoint.size(); i++) {
            cout << queryPoint[i];
            if (i < queryPoint.size() - 1)
                cout << ", ";
        }
        cout << ")\n";

       
        cout << "Nearest Neighbours:\n";
        for (size_t i = 0; i < nearestNodes.size(); i++) {
            Node *node = nearestNodes[i];
            double distance = tree.calculateDistance(node->point, queryPoint);

            cout << "Neighbor " << i + 1 << ": (";
            for (size_t j = 0; j < node->point.size(); j++) {
                cout << node->point[j];
                if (j < node->point.size() - 1)
                    cout << ", ";
            }
            cout << "), Distance: " << distance << "\n";
        }
        cin.get();
    } catch (exception &e) {
        cerr << "Invalid input! Please enter valid comma-separated numerical values.\n";
    }
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
        cout << "5. Get a point's nearest neighbours\n";
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
            getKNearestNeighbours(tree, dimensions);  
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
