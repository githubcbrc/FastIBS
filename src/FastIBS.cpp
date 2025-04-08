#include <iostream>
#include <string>
#include <filesystem>
#include "KmerDatabase.hpp"

using namespace std;
namespace fs = filesystem;

std::string getReferenceName(const std::string &filename)
{
    size_t pos = filename.find("_genome");
    if (pos != std::string::npos)
    {
        return filename.substr(0, pos);
    }
    else
    {
        return filename;
    }
}

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
    int windowSize;

    // Parse command line arguments
    if (argc != 5)
    {
        cout << "\nFastIBS - IBS Distance Calculator\n"
             << "----------------------------------\n"
             << "Usage:\n"
             << "  " << argv[0] << " <sourcePath> <referencePath> <resultsFolder> <windowSize>\n\n"
             << "Arguments:\n"
             << "  <sourcePath>     Path to folder with KMC dataset\n"
             << "                   e.g., /mnt/data/kmc_sets/BW_01002\n\n"
             << "  <referencePath>  Path to folder with reference genomes (FASTA)\n"
             << "                   e.g., /mnt/data/reference\n\n"
             << "  <resultsFolder>  Path to folder for storing output results\n"
             << "                   e.g., /mnt/data/FastIBS_runs\n\n"
             << "  <windowSize>     Length of the sequence window for IBS calculation\n"
             << "                   (integer, e.g., 50000)\n\n"
             << "Notes:\n"
             << "  - All folders should be located on a mounted data volume.\n"
             << "  - Reference files can be gzip-compressed.\n\n"
             << "Output:\n"
             << "  A tab-delimited file summarizing IBS distance metrics for each window.\n"
             << "  Columns: seqname, start, end, total_kmers, observed_kmers, variations, kmer_distance\n\n";
        return 1;
    }
    else
    {
        sourcePath = argv[1];
        referencePath = argv[2];
        resultsFolder = argv[3];
        windowSize = stoi(argv[4]);
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
        string refName = entry.path().filename();
        refName = getReferenceName(refName);
        cout << "Processing reference: " << refName << endl;
        auto outPath = resultsFolder + "/" + database + "_v_" + refName + "_" + to_string(windowSize) + ".tsv";
        //check if file exists, if so skip
        if (fs::exists(outPath))
        {
            cout << "File already exists, skipping..." << endl;
            continue;
        }
        try
        {
            db.processReference(refPath, outPath, windowSize);
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
