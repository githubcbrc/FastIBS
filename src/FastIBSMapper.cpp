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

    if (argc != 4)
    {
        cout << "\nFastIBS - Reference Mapping Tool\n"
             << "---------------------------------\n"
             << "Usage:\n"
             << "  " << argv[0] << " <sourcePath> <referencePath> <resultsFolder>\n\n"
             << "Arguments:\n"
             << "  <sourcePath>     Path to folder containing KMC database files\n"
             << "                   (e.g., /mnt/data/kmc_sets/<dataset_name>)\n\n"
             << "  <referencePath>  Path to folder containing reference genomes in FASTA format\n"
             << "                   (e.g., /mnt/data/reference)\n\n"
             << "  <resultsFolder>  Destination folder for writing mapping result files\n"
             << "                   (e.g., /mnt/data/FastIBS_runs)\n\n"
             << "Notes:\n"
             << "  - All input folders should reside on a mounted data volume.\n"
             << "  - The tool scans <referencePath> for .fasta files and processes them against the KMC base.\n"
             << "  - Output filenames follow the format: <KMC_prefix>_v_<reference_stem>.txt\n"
             << "  - Existing output files will be skipped.\n"
             << "  - Errors during processing are logged to log.txt.\n\n"
             << "Example:\n"
             << "  " << argv[0] << " /mnt/data/kmc_sets/sample1 /mnt/data/reference /mnt/data/FastIBS_runs\n\n"
             << "Output: This tool computes a K-mer mapping for the given references, where each nucleotide position in the reference "
             << "sequences is associated with a count of how many K-mers (of a fixed size, defined by the kmerSize of the KMC source) "
             << "overlap that position and exist in the source KMC database.\n";
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
