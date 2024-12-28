#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
using namespace std;

class RecoveryManager
{
public:
    struct LogEntry
    {
        string transaction;
        char variable;
        int oldValue, newValue;

        LogEntry(string trans, char var, int oldVal, int newVal)
            : transaction(trans), variable(var), oldValue(oldVal), newValue(newVal) {}
    };

    vector<LogEntry> logEntries;
    unordered_map<char, int> currentValues;
    set<string> committedTransactions;
    set<string> activeTransactions;

    void readLog(const string &filename)
    {
        ifstream file(filename);
        string line;
        while (getline(file, line))
        {
            processLogLine(line);
        }
        file.close();
    }

    int safeStoi(const string &s)
    {
        try
        {
            return stoi(s);
        }
        catch (const invalid_argument &e)
        {
            cerr << "Error: Invalid integer conversion in log entry: \"" << s << "\"" << endl;
            cerr << "  Cause: Value is not a valid integer." << endl;
            cerr << "  Example: If the log contains 'A B', make sure B is an integer." << endl;
            return 0;
        }
    }

    void processLogLine(const string &line)
    {
        stringstream ss(line);
        string word;

        if (line.find("START") != string::npos)
        {
            ss >> word;
            if (word == "<START") {
                ss >> word;
                word.pop_back();
                activeTransactions.insert(word);
                cout << "Info: Transaction " << word << " started." << endl;
            }
        }
        else if (line.find("COMMIT") != string::npos)
        {
            ss >> word;
            if (word == "<COMMIT") {
                ss >> word;
                word.pop_back();
                committedTransactions.insert(word);
                activeTransactions.erase(word);
                cout << "Info: Transaction " << word << " committed." << endl;
            }
        }
        else if (line.find("ABORT") != string::npos)
        {
            ss >> word;
            if (word == "<ABORT") {
                ss >> word;
                word.pop_back();
                committedTransactions.insert(word);
                activeTransactions.erase(word);
                cout << "Info: Transaction " << word << " aborted." << endl;
            }
        }
        else if (line.find("END CKPT") != string::npos)
        {
            cout << "Info: End of Checkpoint detected." << endl;
        }
        else if (line.find("CKPT") != string::npos)
        {
            cout << "Info: Checkpoint detected." << endl;
            committedTransactions.clear();
        }
        else
        {
            if (line.front() == '<' && line.back() == '>') {
                string logContent = line.substr(1, line.size() - 2);
                stringstream ss(logContent);
                string transToken, varToken, oldValToken, newValToken;

                ss >> transToken >> varToken >> oldValToken >> newValToken;

                if (transToken.empty() || varToken.empty() || oldValToken.empty())
                {
                    cerr << "Parsing Error: Malformed log entry detected: \"" << line << "\"" << endl;
                    return;
                }

                if (transToken[0] != 'T')
                {
                    cerr << "Parsing Error: Log entry does not follow the correct transaction format: \"" << line << "\"" << endl;
                    return;
                }

                string transaction = transToken;
                char variable = varToken[0];
                int oldValue = safeStoi(oldValToken);
                int newValue;
                if (!newValToken.empty()) {
                    newValue = safeStoi(newValToken);
                    logEntries.emplace_back(transaction, variable, oldValue, newValue);
                    cout << "Info: Parsed log entry: " << transaction << " " << variable << " "
                         << oldValue << " " << newValue << endl;
                }
                else {
                    logEntries.emplace_back(transaction, variable, 0, oldValue);
                    cout << "Info: Parsed log entry: " << transaction << " " << variable << " "
                         << oldValue << endl;
                }
            }
            else if (line == "") { return; }
            else
            {
                cerr << "Parsing Error: Log entry does not follow the correct format: \"" << line << "\"" << endl;
            }
        }
    }

    void undo(ofstream &outputFile)
    {
        outputFile << "Undo Phase:" << endl;
        bool hasUndo = false;

        for (auto it = logEntries.rbegin(); it != logEntries.rend(); ++it)
        {
            if (activeTransactions.count(it->transaction))
            {
                currentValues[it->variable] = it->oldValue;
                outputFile << "Undo " << it->transaction << ": " << it->variable << " = " << it->oldValue << endl;
                hasUndo = true;
            }
        }

        if (!hasUndo)
        {
            outputFile << "No active transactions to undo." << endl;
        }
    }

    void redo(ofstream &outputFile)
    {
        outputFile << "Redo Phase:" << endl;
        bool hasRedo = false;

        for (const auto &entry : logEntries)
        {
            if (committedTransactions.count(entry.transaction))
            {
                currentValues[entry.variable] = entry.newValue;
                outputFile << "Redo " << entry.transaction << ": " << entry.variable << " = " << entry.newValue << endl;
                hasRedo = true;
            }
        }

        if (!hasRedo)
        {
            outputFile << "No committed transactions to redo." << endl;
        }
    }

    void writeOutput(const string &outputFilename)
    {
        ofstream outputFile(outputFilename);
        redo(outputFile);
        undo(outputFile);
        outputFile.close();
    }
};

int main()
{
    RecoveryManager manager;
    string inputFile = "transaction_log.txt";
    string outputFile = "recovery_output.txt";

    manager.readLog(inputFile);
    manager.writeOutput(outputFile);

    cout << "Recovery process completed. Check the file " << outputFile << " for details." << endl;
    return 0;
}
