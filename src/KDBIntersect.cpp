#include <iostream>
#include <string>
#include <filesystem>
#include "KmerDatabase.hpp"

using namespace std;
namespace fs = filesystem;

string findPrefix(const string &path)
{
    for (const auto &entry : filesystem::directory_iterator(path))
    {
        string filename = entry.path().filename().string();
        size_t pos = filename.find(".kmc_");
        if (pos != string::npos)
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

uint getIntersectionSize(KmerDatabase &db1, KmerDatabase &db2)
{
    uint intersectionSize = 0;
    CKmerAPI kmer(db1.getKmerSize());
    while (db1.readNextKmer(kmer))
        if (db2.isKmer(kmer))        
            intersectionSize += 1; 
    return intersectionSize;
}

int main(int argc, char *argv[])
{
    string kDB1Path, kDB2Path, database1, database2;
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <kDB1Path> <kDB2Path>\n";
        return 1;
    }
    else
    {
        kDB1Path = argv[1];
        kDB2Path = argv[2];
    }

    removeTrailingSlash(kDB1Path);
    removeTrailingSlash(kDB2Path);

    kDB1Path += "/" + findPrefix(kDB1Path);
    kDB2Path += "/" + findPrefix(kDB2Path);

    uint64_t size1 = fs::file_size(kDB1Path + ".kmc_pre") / 1024 / 1024;
    uint64_t size2 = fs::file_size(kDB2Path + ".kmc_pre") / 1024 / 1024;

    if (size1 > size2)
        swap(kDB1Path, kDB2Path);

    KmerDatabase db1(kDB1Path, true); // open for listing
    KmerDatabase db2(kDB2Path, false); // open for random access

    if (db1.getKmerSize() != db2.getKmerSize())
    {
        cout << "Error: K-mer lengths do not match" << endl;
        return 1;
    }
    auto intersectionSize = getIntersectionSize(db1, db2);
    cout << intersectionSize << endl;

    return 0;
}