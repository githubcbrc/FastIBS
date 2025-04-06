#include <iostream>
#include <string>
#include <filesystem>
#include "KmerDatabase.hpp"

using namespace std;
namespace fs = std::filesystem;


std::string findPrefix(const std::string &path)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        std::string filename = entry.path().filename().string();
        size_t pos = filename.find(".kmc_");
        if (pos != std::string::npos)
        {
            return filename.substr(0, pos);
        }
    }
    return "";
}

void removeTrailingSlash(string &path)
{
    if (!path.empty() && path.back() == '/')
    {
        path.pop_back();
    }
}

int main(int argc, char *argv[])
{
    string sourcePath, referencePath, resultsFolder, database;

    // Parse command line arguments
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " <sourcePath(KMC)> <referencePath(fasta)> <resultsFolder>\n";
        return 1;
    }
    else
    {
        sourcePath = argv[1];
        referencePath = argv[2];
        resultsFolder = argv[3];
    }

    removeTrailingSlash(sourcePath);
    removeTrailingSlash(referencePath);
    removeTrailingSlash(resultsFolder);

    database = std::filesystem::path(sourcePath).filename().string();
    cout << "****Database: " << database << endl;
    sourcePath += "/" + findPrefix(sourcePath);

    cout << "Using sourcePath: " << sourcePath << endl;
    cout << "Using referencePath: " << referencePath << endl;
    cout << "Using resultsFolder: " << resultsFolder << endl;

    auto start = chrono::high_resolution_clock::now();
    cout << "Loading KMC database from " << sourcePath << endl;
    KmerDatabase db(sourcePath);
    db.printKMCInfo();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Database loaded in " << elapsed.count() << " seconds" << endl;
    cout << "********************************************" << endl;

    /************************************************************/

    start = chrono::high_resolution_clock::now();
    if (!fs::exists(resultsFolder))
    {
        fs::create_directory(resultsFolder);
    }

    for (const auto &entry : fs::directory_iterator(referencePath))
    {
        // ensure to process only fasta files
        if (entry.path().extension() != ".fasta")
            continue;
        string refPath = entry.path();
        string refName = entry.path().stem().string();
        cout << "Processing reference: " << refName << endl;
        auto outPath = resultsFolder + "/" + database + "_v_" + refName + ".txt";
        //check if file exists, if so skip
        if (fs::exists(outPath))
        {
            cout << "File already exists, skipping..." << endl;
            continue;
        }
        try
        {
            db.produceMapping(refPath, outPath);
        }
        catch (const std::exception &e)
        {
            ofstream logFile("log.txt", ios::app); // Open the log file in append mode
            if (logFile.is_open())
            {
                logFile << "Error processing reference: " << refName << endl;
                logFile << e.what() << endl;
                logFile.close();
            }
            else
            {
                cout << "Unable to open log file." << endl;
            }
            continue;
        }
    }

    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "Reference processed in " << elapsed.count() << " seconds" << endl;

    /************************************************************/

    return 0;
}