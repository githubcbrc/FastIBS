#include <iostream>
#include <vector>
#include <string>
#include <utility>    // for std::pair
#include <algorithm>  // for std::sort
#include <filesystem> // for std::filesystem

using namespace std;
namespace fs = std::filesystem;

// Function to get file sizes and sort files by size
vector<pair<uintmax_t, string>> getSortedFilesBySize(const vector<string>& files) {
    vector<pair<uintmax_t, string>> filesWithSize;
    for (const auto &file : files) {
        uintmax_t fileSize = fs::file_size(file);
        filesWithSize.emplace_back(fileSize, file);
    }
    sort(filesWithSize.begin(), filesWithSize.end());
    return filesWithSize;
}


// Function to generate tasks for a single file
vector<pair<string, vector<string>>> generateTasksForFile(const string &file, const vector<string> &targets, int k)
{
    vector<pair<string, vector<string>>> tasks;
    for (int i = 0; i < targets.size(); i += k)
    {
        vector<string> group;
        for (int j = i; j < i + k && j < targets.size(); ++j)
        {
            group.push_back(targets[j]);
        }
        tasks.emplace_back(file, group);
    }
    return tasks;
}

// Main function to generate all tasks
vector<pair<string, vector<string>>> generateTasks(const vector<string> &files, int k)
{
    vector<pair<string, vector<string>>> tasks;
    int n = files.size();

    // Step 1: Sort files by size
    vector<pair<uintmax_t, string>> sortedFiles = getSortedFilesBySize(files);

    // Step 2: Generate tasks
    for (int i = 0; i < n; ++i)
    {
        string file = sortedFiles[i].second;
        vector<string> targets;
        for (int j = i + 1; j < n; ++j)
        {
            targets.emplace_back(sortedFiles[j].second);
        }
        vector<pair<string, vector<string>>> fileTasks = generateTasksForFile(file, targets, k);
        tasks.insert(tasks.end(), fileTasks.begin(), fileTasks.end());
    }

    return tasks;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <folder_path> <k>" << endl;
        return 1;
    }

    string folderPath = argv[1];
    int k = stoi(argv[2]);

    // List all files in the specified folder
    vector<string> files;
    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path().string());
        }
    }

    // Generate tasks
    vector<pair<string, vector<string>>> tasks = generateTasks(files, k);

    // Print tasks
    for (const auto &task : tasks)
    {
        cout << task.first << "\t";
        for (size_t i = 0; i < task.second.size(); ++i)
        {
            cout << task.second[i];
            if (i < task.second.size() - 1)
            {
                cout << "\t";
            }
        }
        cout << endl;
    }

    return 0;
}
