#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <random>
#include <sstream>
using namespace std;

class NaiveBayesClassifier
{
private:
    unordered_map<int, double> classes;
    unordered_map<int, unordered_map<int, double>> attributesPerClass;
    unordered_map<int, double> totalWordsPerClass;
    int totalUniqueAttributes = 0;

public:
    NaiveBayesClassifier(vector<vector<int>> &data)
    {

        set<int> uniqueAttributes;

        for (auto entry : data)
        {
            if (classes.find(entry[0]) == classes.end())
            {
                classes[entry[0]] = 1;
                attributesPerClass[entry[0]] = {};
                totalWordsPerClass[entry[0]] = 0;
            }
            else
            {
                classes[entry[0]] += 1;
            }

            for (int k = 1; k < entry.size(); k++)
            {
                uniqueAttributes.insert(entry[k]);
                if (attributesPerClass[entry[0]].find(entry[k]) == attributesPerClass[entry[0]].end())
                {
                    attributesPerClass[entry[0]][entry[k]] = 1;
                }
                else
                {
                    attributesPerClass[entry[0]][entry[k]] += 1;
                }
                totalWordsPerClass[entry[0]] += 1;
            }
        }

        totalUniqueAttributes = uniqueAttributes.size();

        for (auto &cls : classes)
        {
            int classLabel = cls.first;

            classes[classLabel] /= data.size();

            for (auto &word : attributesPerClass[classLabel])
            {
                word.second = (word.second + 1.0) /
                              (totalWordsPerClass[classLabel] + totalUniqueAttributes);
            }
        }
    }

    int predict(vector<int> attributes, bool printResults)
    {
        if (attributes.empty())
        {
            cout << "No valid words found in input" << endl;
            return -1;
        }

        double bestScore = -numeric_limits<double>::infinity();
        int bestClass = -1;
        if (printResults)
        {
            cout << "\nProbabilities for each class:" << endl
                 << endl;
        }

        for (auto &cls : classes)
        {
            int classLabel = cls.first;
            double score = log(cls.second);
            if (printResults)
            {
                cout << "Class " << classLabel << " initial log probability: " << score << endl
                     << endl;
            }

            int wordsFound = 0;
            for (int word : attributes)
            {
                if (attributesPerClass[classLabel].find(word) != attributesPerClass[classLabel].end())
                {
                    double wordProb = attributesPerClass[classLabel][word];
                    score += log(wordProb);
                    if (printResults)
                    {
                        cout << "Word ID " << word << " found, prob: " << wordProb << endl;
                    }
                    wordsFound++;
                }
                else
                {
                    double smoothedProb = 1.0 / (totalWordsPerClass[classLabel] + totalUniqueAttributes);
                    score += log(smoothedProb);
                    if (printResults)
                    {
                        cout << "Word ID " << word << " not found, smoothed prob: " << smoothedProb << endl;
                    }
                }
            }

            if (wordsFound == 0)
            {
                score -= 10;
            }
            if (printResults)
            {
                cout << "\nFinal log probability for class " << classLabel << ": " << score << endl
                     << endl;
            }

            if (score > bestScore)
            {
                bestScore = score;
                bestClass = classLabel;
            }
        }
        if (printResults)
        {
            cout << "\nFinal prediction: " << bestClass << " with log probability: " << bestScore << endl;
        }
        return bestClass;
    }
};

void populateData(vector<vector<int>> &data, unordered_map<string, int> &classmap, unordered_map<string, int> &attrimap,
                  const string &filename, const string &classLabel)
{
    ifstream file(filename);
    string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string word;
        vector<int> apair = {classmap[classLabel]};
        while (iss >> word)
        {
            if (attrimap.find(word) == attrimap.end())
            {
                int newIndex = attrimap.size();
                attrimap[word] = newIndex;
            }
            apair.push_back(attrimap[word]);
        }
        data.push_back(apair);
    }
}

void evaluateModel(NaiveBayesClassifier &model,
                   unordered_map<string, int> &classmap,
                   unordered_map<string, int> &attrimap)
{
    vector<vector<int>> testData;

    populateData(testData, classmap, attrimap, "good.test.txt", "good");
    populateData(testData, classmap, attrimap, "bad.test.txt", "bad");
    populateData(testData, classmap, attrimap, "neutral.test.txt", "neutral");

    int correct = 0;
    int total = 0;

    for (const auto &example : testData)
    {

        int trueClass = example[0];

        vector<int> attributes(example.begin() + 1, example.end());

        int predicted = model.predict(attributes, false);

        if (predicted == trueClass)
        {
            correct++;
        }
        total++;
    }

    double accuracy = (double)correct / total;
    cout << "\nTest Results:" << endl;
    cout << "Total examples: " << total << endl;
    cout << "Correct predictions: " << correct << endl;
    cout << "Accuracy: " << (accuracy * 100) << "%" << endl;
}

void testModelInConsole(NaiveBayesClassifier &model, unordered_map<string, int> &attrimap)
{
    string input;
    cout << "\nEnter text to classify (type 'quit' to exit):" << endl;

    while (true)
    {
        cout << "\n> ";
        getline(cin, input);

        if (input == "quit" || input == "exit")
        {
            break;
        }

        istringstream iss(input);
        string word;
        vector<int> attributes;

        while (iss >> word)
        {

            attributes.push_back(attrimap[word]);
        }

        int result = model.predict(attributes, true);
        cout << "Prediction: ";
        if (result == 0)
        {
            cout << "GOOD" << endl;
        }
        else if (result == 1)
        {
            cout << "BAD" << endl;
        }
        else
        {
            cout << "NEUTRAL" << endl;
        }
    }

    cout << "Goodbye!" << endl;
}

int main()
{
    unordered_map<string, int> classmap = {{"good", 0}, {"bad", 1}, {"neutral", 2}};
    unordered_map<string, int> attrimap;
    vector<vector<int>> data;

    populateData(data, classmap, attrimap, "good.txt", "good");
    populateData(data, classmap, attrimap, "bad.txt", "bad");
    populateData(data, classmap, attrimap, "neutral.txt", "neutral");

    random_device rd;
    mt19937 g(rd());
    shuffle(data.begin(), data.end(), g);

    NaiveBayesClassifier mymodel(data); 

    cout << "\nEvaluating model on test data..." << endl;
    evaluateModel(mymodel, classmap, attrimap);
    testModelInConsole(mymodel, attrimap);

    return 0;
}