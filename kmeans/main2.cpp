#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

class Point
{
private:
    int pointId, clusterId;
    int dimensions;
    vector<double> values;

    vector<double> lineToVec(string &line)
    {
        vector<double> values;
        string tmp = "";

        for (int i = 0; i < (int)line.length(); i++)
        {
            if ((48 <= int(line[i]) && int(line[i]) <= 57) || line[i] == '.' || line[i] == '+' || line[i] == '-' || line[i] == 'e')
            {
                tmp += line[i];
            }
            else if (line[i] == ',' || tmp.length() > 0)
            {
                values.push_back(stod(tmp));
                tmp = "";
            }
        }
        if (tmp.length() > 0)
        {
            values.push_back(stod(tmp));
            tmp = "";
        }

        return values;
    }

public:
    Point(int id, string line)
    {
        pointId = id;
        values = lineToVec(line);
        dimensions = values.size();
        clusterId = 0;
    }

    int getDimensions() const { return dimensions; }

    int getCluster() const { return clusterId; }

    int getID() const { return pointId; }

    void setCluster(int val) { clusterId = val; }

    double getVal(int pos) const { return values[pos]; }
};

class Cluster
{
private:
    int clusterId;
    string clusterName;
    vector<double> centroid;
    vector<Point> points;

public:
    Cluster(int clusterId, Point centroid)
    {
        this->clusterId = clusterId;
        for (int i = 0; i < centroid.getDimensions(); i++)
        {
            this->centroid.push_back(centroid.getVal(i));
        }
        this->addPoint(centroid);
    }

    void addPoint(Point p)
    {
        p.setCluster(this->clusterId);
        points.push_back(p);
    }

    bool removePoint(int pointId)
    {
        int size = points.size();

        for (int i = 0; i < size; i++)
        {
            if (points[i].getID() == pointId)
            {
                points.erase(points.begin() + i);
                return true;
            }
        }
        return false;
    }

    void removeAllPoints() { points.clear(); }

    int getId() { return clusterId; }

    Point getPoint(int pos) { return points[pos]; }

    int getSize() { return points.size(); }
    string getClusterName() const { return clusterName; }

    void setClusterName(string name) { this->clusterName = name; }

    double getCentroidByPos(int pos) const { return centroid[pos]; }

    void setCentroidByPos(int pos, double val) { this->centroid[pos] = val; }
};

class KMeans
{
private:
    int K, iters, dimensions, total_points;
    vector<Cluster> clusters;
    string output_dir;

    void clearClusters()
    {
        for (int i = 0; i < K; i++)
        {
            clusters[i].removeAllPoints();
        }
    }

    int getNearestClusterId(Point point)
    {
        double sum = 0.0, min_dist;
        int NearestClusterId;
        if (dimensions == 1)
        {
            min_dist = abs(clusters[0].getCentroidByPos(0) - point.getVal(0));
        }
        else
        {
            for (int i = 0; i < dimensions; i++)
            {
                sum += pow(clusters[0].getCentroidByPos(i) - point.getVal(i), 2.0);
                // sum += abs(clusters[0].getCentroidByPos(i) - point.getVal(i));
            }
            min_dist = sqrt(sum);
        }
        NearestClusterId = clusters[0].getId();

        for (int i = 1; i < K; i++)
        {
            double dist;
            sum = 0.0;

            if (dimensions == 1)
            {
                dist = abs(clusters[i].getCentroidByPos(0) - point.getVal(0));
            }
            else
            {
                for (int j = 0; j < dimensions; j++)
                {
                    sum += pow(clusters[i].getCentroidByPos(j) - point.getVal(j), 2.0);
                    // sum += abs(clusters[i].getCentroidByPos(j) - point.getVal(j));
                }

                dist = sqrt(sum);
                // dist = sum;
            }
            if (dist < min_dist)
            {
                min_dist = dist;
                NearestClusterId = clusters[i].getId();
            }
        }

        return NearestClusterId;
    }

public:
    KMeans(int K, int iterations, string output_dir)
    {
        this->K = K;
        this->iters = iterations;
        this->output_dir = output_dir;
    }

    vector<Cluster> run(vector<Point> &all_points, vector<string> const colors)
    {
        total_points = all_points.size();
        dimensions = all_points[0].getDimensions();

        vector<int> used_pointIds;

        for (int i = 1; i <= K; i++)
        {
            while (true)
            {
                int index = rand() % total_points;

                if (find(used_pointIds.begin(), used_pointIds.end(), index) ==
                    used_pointIds.end())
                {
                    used_pointIds.push_back(index);
                    all_points[index].setCluster(i);
                    Cluster cluster(i, all_points[index]);
                    clusters.push_back(cluster);
                    break;
                }
            }
        }
        cout << "Clusters initialized = " << clusters.size() << endl
             << endl;

        cout << "Running K-Means Clustering.." << endl;

        int iter = 1;
        while (true)
        {
            cout << "Iter - " << iter << "/" << iters << endl;
            bool done = true;

            for (int i = 0; i < total_points; i++)
            {
                int currentClusterId = all_points[i].getCluster();
                int nearestClusterId = getNearestClusterId(all_points[i]);
                if (currentClusterId != nearestClusterId)
                {
                    all_points[i].setCluster(nearestClusterId);

                    done = false;
                }
            }

            clearClusters();

            for (int i = 0; i < total_points; i++)
            {
                clusters[all_points[i].getCluster() - 1].addPoint(all_points[i]);
            }

            for (int i = 0; i < K; i++)
            {
                int ClusterSize = clusters[i].getSize();
                if (ClusterSize > 0)
                {
                    for (int j = 0; j < dimensions; j++)
                    {
                        double sum = 0.0;
                        for (int p = 0; p < ClusterSize; p++)
                        {
                            sum += clusters[i].getPoint(p).getVal(j);
                        }
                        clusters[i].setCentroidByPos(j, sum / ClusterSize);
                    }
                }
            }

            if (done || iter >= iters)
            {
                cout << "Clustering completed in iteration: " << iter << endl;
                break;
            }
            iter++;
        }

        ofstream pointsFile;
        pointsFile.open(output_dir + "/" + to_string(K) + "-points.txt", ios::out);

        for (int i = 0; i < total_points; i++)
        {
            pointsFile << all_points[i].getCluster() << endl;
        }

        pointsFile.close();

        ofstream outfile;
        outfile.open(output_dir + "/" + to_string(K) + "-clusters.txt");
        if (outfile.is_open())
        {
            for (int i = 0; i < K; i++)
            {
                cout << "Cluster " << clusters[i].getId() - 1 << " centroid : ";
                for (int j = 0; j < dimensions - 1; j++)
                {
                    cout << clusters[i].getCentroidByPos(j) << " , ";    // Output to console
                    outfile << clusters[i].getCentroidByPos(j) << " , "; // Output to file
                }
                cout << clusters[i].getCentroidByPos(dimensions - 1);    // Output to console
                outfile << clusters[i].getCentroidByPos(dimensions - 1); // Output to file
                cout << endl;
                outfile << endl;
            }
            outfile.close();
        }
        else
        {
            cout << "Error: Unable to write to clusters.txt";
        }
        return clusters;
    }
};

string getCountryNameFromColors(const vector<string> &colors)
{

    unordered_map<string, string> colorToCountry = {
        {"RED,WHITE", "POLAND"},
        {"GREEN,WHITE,RED", "ITALY"},
        {"GREEN,RED,GREEN", "BANGLADESH"},
        {"RED,WHITE,BLUE,RED", "UNITED STATES"},
        {"WHITE,BLUE,YELLOW", "ARGENTINA"},
        {"YELLOW,WHITE,GREEN", "INDIA"},
        {"GREEN,WHITE,RED,GREEN", "MEXICO"},
        {"RED,WHITE,BLUE,WHITE", "UK"},
        {"RED,WHITE,BLUE", "FRANCE"},
        {"BLUE,BLUE,BLUE", "XYZ"}};

    string key = "";
    for (size_t i = 0; i < colors.size(); ++i)
    {
        key += colors[i];
        if (i < colors.size() - 1)
        {
            key += ",";
        }
    }

    if (colorToCountry.find(key) != colorToCountry.end())
    {
        return colorToCountry[key];
    }

    return "Country not found";
}

void processClusters(const vector<Cluster> &clusters, const unordered_map<string, vector<Point>> &countryPoints)
{
    string outputDir = "output";
    ofstream outfile(outputDir + "/results.txt");

    if (!outfile.is_open())
    {
        cerr << "Error: Unable to open results file for writing." << endl;
        return;
    }
    outfile << "Cluster Predictions:\n";
    for (auto country : countryPoints)
    {
        vector<string> countryColors;
        cout << endl
             << country.first << endl;
        for (Point point : country.second)
        {

            long double dist = std::numeric_limits<double>::infinity();
            int id = 0;
            for (int i = 0; i < clusters.size(); i++)
            {
                long double calc = 0.0;
                for (int j = 0; j < country.second[0].getDimensions(); j++)
                {
                    calc += pow(clusters[i].getCentroidByPos(j) - point.getVal(j), 2.0);
                }
                if (calc < dist)
                {
                    dist = calc;
                    id = i;
                }
            }
            cout << clusters[id].getClusterName() << endl;
            countryColors.push_back(clusters[id].getClusterName());
        }
        outfile << "Country " << country.first << " Predicted Country => " << getCountryNameFromColors(countryColors) << endl;
    }

    outfile.close();
    cout << "Results saved to output/results.txt" << endl;
}

void testing(vector<Cluster> clusters)
{
    unordered_map<string, vector<Point>> countryPoints;
    vector<Point> allPoints;

    string filename = "countries.txt";
    ifstream infile(filename.c_str());
    if (!infile.is_open())
    {
        cerr << "Error: Unable to open countries.txt." << endl;
        return;
    }

    string line;
    int pointId = 1;

    while (getline(infile, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        if (line[0] == '#')
        {
            string country = line.substr(1);
            while (getline(infile, line) && !line.empty() && isdigit(line[0]))
            {
                Point point(pointId++, line);
                allPoints.push_back(point);
                countryPoints[country].push_back(point);
            }
        }
    }

    infile.close();
    cout << "\nData fetched successfully!\n";

    processClusters(clusters, countryPoints);
}
void setClusterName(vector<Cluster> &clusters, const vector<string> &colors)
{

    vector<vector<int>> points = {
        {255, 255, 255}, // White
        {255, 255, 0},   // Yellow
        {255, 0, 0},     // Red
        {0, 255, 0},     // Green
        {0, 0, 255}      // Blue
    };

    if (colors.size() < points.size())
    {
        cerr << "Error: Not enough colors provided for points." << endl;
        return;
    }

    for (size_t i = 0; i < clusters.size(); ++i)
    {
        long double minDist = std::numeric_limits<double>::infinity();
        int bestIndex = -1;

        for (size_t k = 0; k < points.size(); ++k)
        {
            long double dist = 0.0;
            for (int j = 0; j < 3; ++j)
            {
                dist += pow(clusters[i].getCentroidByPos(j) - points[k][j], 2.0);
            }
            if (dist < minDist)
            {
                minDist = dist;
                bestIndex = k;
            }
        }

        if (bestIndex != -1)
        {
            clusters[i].setClusterName(colors[bestIndex]);
        }
    }
}

int main(int argc, char **argv)
{

    string output_dir = "output";

    int K = 0;

    string filename = "col.txt";

    ifstream infile(filename.c_str());

    if (!infile.is_open())
    {
        cout << "Error: Failed to open file." << endl;
        return 1;
    }

    int pointId = 1;
    vector<Point> all_points;
    string line;
    vector<string> colors;

    while (getline(infile, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        if (!line.empty() && (line[0] == '#' || isalpha(line[0])))
        {
            if (line[0] == '#')
                colors.push_back(line.substr(1));
            else
                colors.push_back(line);
            K++;
            continue;
        }

        try
        {
            Point point(pointId++, line);
            all_points.push_back(point);
        }
        catch (exception &e)
        {
            cerr << "Error parsing line: " << line << ". Exception: " << e.what() << endl;
            return 1;
        }
    }

    infile.close();
    cout << "\nData fetched successfully!" << endl
         << endl;

    if ((int)all_points.size() < K)
    {
        cout << "Error: Number of clusters greater than number of points." << endl;
        return 1;
    }

    int iters = 100;

    KMeans kmeans(K, iters, output_dir);
    vector<Cluster> clusters = kmeans.run(all_points, colors);
    setClusterName(clusters, colors);
    for (int i = 0; i < clusters.size(); i++)
    {
        cout << clusters[i].getClusterName() << " " << clusters[i].getCentroidByPos(0) << " " << clusters[i].getCentroidByPos(1) << " " << clusters[i].getCentroidByPos(2) << endl;
    }
    testing(clusters);
}
