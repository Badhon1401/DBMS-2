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

private:
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

    void calculateDistances(const vector<vector<double>> &points, const string &outputFilename) const
    {
        ofstream outFile(outputFilename);
        for (const auto &point : points)
        {
            double minDistance = numeric_limits<double>::max();
            calculateDistancesRecursive(root, point, 0, minDistance);
            outFile << "Point: (";
            for (size_t i = 0; i < point.size(); i++)
            {
                outFile << point[i];
                if (i < point.size() - 1)
                    outFile << ", ";
            }
            outFile << ") -> Nearest Distance: " << minDistance << endl;
        }
        outFile.close();
    }

    void calculateDistancesRecursive(Node *node, const vector<double> &targetPoint, int depth, double &minDistance) const
    {
        if (node == nullptr)
            return;

        double distance = calculateDistance(node->point, targetPoint);
        if (distance < minDistance)
            minDistance = distance;

        int cd = depth % dimensions;

        if (targetPoint[cd] < node->point[cd])
        {
            calculateDistancesRecursive(node->left, targetPoint, depth + 1, minDistance);
            if (fabs(targetPoint[cd] - node->point[cd]) < minDistance)
                calculateDistancesRecursive(node->right, targetPoint, depth + 1, minDistance);
        }
        else
        {
            calculateDistancesRecursive(node->right, targetPoint, depth + 1, minDistance);
            if (fabs(targetPoint[cd] - node->point[cd]) < minDistance)
                calculateDistancesRecursive(node->left, targetPoint, depth + 1, minDistance);
        }
    }

    void interactiveQuerySession() const
    {
        cout << "Interactive Query Session:\n";
        while (true)
        {
            cout << "Enter query point (comma-separated, or 'exit' to quit): ";
            string input;
            getline(cin, input);

            if (input == "exit")
                break;

            vector<double> queryPoint;
            stringstream ss(input);
            string value;
            while (getline(ss, value, ','))
            {
                queryPoint.push_back(stod(value));
            }

            double minDistance = numeric_limits<double>::max();
            calculateDistancesRecursive(root, queryPoint, 0, minDistance);

            cout << "Nearest Distance: " << minDistance << endl;
        }
    }
};

int main()
{
    ifstream inFile("input.txt");
    if (!inFile.is_open())
    {
        cerr << "Failed to open input file.\n";
        return 1;
    }

    vector<vector<double>> points;
    vector<vector<double>> queryPoints;
    string line;
    bool isQuerySection = false;

    while (getline(inFile, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Check if this is the query section
        if (line[0] == '#')
        {
            isQuerySection = true;
            continue;
        }

        stringstream ss(line);
        vector<double> point;
        string value;
        while (getline(ss, value, ','))
        {
            point.push_back(stod(value));
        }

        if (isQuerySection)
        {
            queryPoints.push_back(point);
        }
        else
        {
            points.push_back(point);
        }
    }
    inFile.close();

    if (points.empty())
    {
        cerr << "No points found in the input file.\n";
        return 1;
    }

    size_t dimensions = points[0].size();
    KDTree tree(points, dimensions);

    ofstream resultFile("result.txt");
    if (!resultFile.is_open())
    {
        cerr << "Failed to open result file.\n";
        return 1;
    }

    for (const auto& queryPoint : queryPoints)
    {
        vector<double> nearest = tree.nearestNeighbor(queryPoint);

        resultFile << "Query: ";
        for (size_t i = 0; i < queryPoint.size(); ++i)
        {
            resultFile << queryPoint[i] << (i == queryPoint.size() - 1 ? "" : ", ");
        }
        resultFile << "\nNearest Neighbor: ";
        for (size_t i = 0; i < nearest.size(); ++i)
        {
            resultFile << nearest[i] << (i == nearest.size() - 1 ? "" : ", ");
        }
        resultFile << "\n\n";
    }

    resultFile.close();
    cout << "Results written to result.txt\n";

    cout << "\nEnter query points interactively. Type 'exit' to quit.\n";
    while (true)
    {
        cout << "Enter a query point (comma-separated " << dimensions << " values): ";
        string input;
        getline(cin, input);

        if (input == "exit")
        {
            cout << "Exiting interactive session.\n";
            break;
        }

        stringstream ss(input);
        vector<double> queryPoint;
        string value;
        try
        {
            while (getline(ss, value, ','))
            {
                queryPoint.push_back(stod(value));
            }

            if (queryPoint.size() != dimensions)
            {
                cerr << "Error: Query point must have exactly " << dimensions << " dimensions.\n";
                continue;
            }

            vector<double> nearest = tree.nearestNeighbor(queryPoint);

            cout << "Nearest Neighbor: ";
            for (size_t i = 0; i < nearest.size(); ++i)
            {
                cout << nearest[i] << (i == nearest.size() - 1 ? "" : ", ");
            }
            cout << "\n\n";

        }
        catch (exception& e)
        {
            cerr << "Error parsing query point. Ensure it is a comma-separated list of numbers.\n";
        }
    }

    return 0;
}  