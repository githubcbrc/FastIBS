#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <chrono>
#include <tuple>
#include <zlib.h>
#include <cstdlib>
#include <fstream>

#define ZIP_BUFFER_SIZE 4096

using namespace std;

enum class FileType
{
    FASTA,
    FASTQ
};

class ParseResult
{
public:
    vector<string> ids;
    vector<string> sequences;
};


string reverseComplement(const string &kmer)
{
    string revComplement;
    revComplement.reserve(kmer.size());
    for (auto it = kmer.rbegin(); it != kmer.rend(); ++it)
    {
        switch (*it)
        {
        case 'A':
            revComplement += 'T';
            break;
        case 'C':
            revComplement += 'G';
            break;
        case 'G':
            revComplement += 'C';
            break;
        case 'T':
            revComplement += 'A';
            break;           
        default:
            throw invalid_argument("Invalid base in kmer");
        }
    }
    return revComplement;
}

uint64_t convertKmer(const string &kmer)
{
    // Convert a kmer to an integer
    uint64_t result = 1;
    for (char base : kmer)
    {
        result <<= 2; // Shift the result left by 2 bits
        switch (base)
        {
        case 'A':
            result |= 0;
            break;
        case 'C':
            result |= 1;
            break;
        case 'G':
            result |= 2;
            break;
        case 'T':
            result |= 3;
            break;
        default:
            throw invalid_argument("Invalid base");
            return 0;
        }
    }
    return result;
}

string convertIntToKmer(uint64_t kmerInt, int kmerLength)
{
    // Convert an integer to a kmer
    string kmer(kmerLength, 'A'); // Initialize a kmer string with 'A's
    for (int i = kmerLength - 1; i >= 0; --i)
    {
        switch (kmerInt & 3) // Get the last two bits
        {
        case 0:
            kmer[i] = 'A';
            break;
        case 1:
            kmer[i] = 'C';
            break;
        case 2:
            kmer[i] = 'G';
            break;
        case 3:
            kmer[i] = 'T';
            break;
        }
        kmerInt >>= 2; // Shift the kmerInt right by 2 bits
    }
    return kmer;
}

vector<uint64_t> getCanonicalInts(const string &sequence, size_t kmerSize)
{
    vector<uint64_t> kmers;
    for (size_t i = 0; i <= sequence.size() - kmerSize;)
    {
        try
        {
            string kmer = sequence.substr(i, kmerSize);
            string revComplement = reverseComplement(kmer);
            uint64_t canonicalKmer = min(convertKmer(kmer), convertKmer(revComplement));
            kmers.push_back(canonicalKmer);
            i++;
        }
        catch (invalid_argument &e)
        {
            i += kmerSize;
            continue;
        }
    }
    return kmers;
}

vector<string> getCanonicalKmers(string &sequence, int kmerSize)
{
    vector<string> kmers;
    if (sequence.size() < kmerSize)
        return kmers;
    transform(sequence.begin(), sequence.end(), sequence.begin(), ::toupper);
    for (int i = 0; i <= sequence.size() - kmerSize; i++)
    {
        try
        {
            string kmer = sequence.substr(i, kmerSize);
            if (kmer.find_first_not_of("ACGT") != string::npos)
                continue;
            string revComplement = reverseComplement(kmer);
            string canonicalKmer = kmer > revComplement ? revComplement : kmer;
            kmers.push_back(canonicalKmer);
        }
        catch (invalid_argument &e)
        {
            continue;
        }
    }
    return kmers;
}

string decompressGzip(const string &filename)
{
    cout << "Decompressing " << filename << endl;
    gzFile file = gzopen(filename.c_str(), "rb");
    if (!file)
    {
        throw runtime_error("Failed to open gzip file");
    }
    char buffer[ZIP_BUFFER_SIZE];
    stringstream decompressedData;

    while (true)
    {
        int bytesRead = gzread(file, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0)
            break;

        buffer[bytesRead] = '\0';
        decompressedData << buffer;
    }

    gzclose(file);
    return decompressedData.str();
}



ParseResult readSequenceFile(const string &filename, bool compressed = false)
{
    string data;
    if (compressed)
        data = decompressGzip(filename);
    else
    {
        ifstream file(filename);
        if (!file)
            throw runtime_error("Failed to open file");
        stringstream ss;
        ss << file.rdbuf();
        data = ss.str();
    }
    FileType fileType;

    if (!data.empty())
    {
        if (data[0] == '>')
            fileType = FileType::FASTA;
        else if (data[0] == '@')
            fileType = FileType::FASTQ;
        else
            throw runtime_error("Unknown file type");
    }
    else
        throw runtime_error("Empty file");

    cout << "Parsing " << (fileType == FileType::FASTA ? "fasta" : "fastq") << " data\n";
    ParseResult result;
    stringstream ss(data);

    string currentID;
    stringstream currentSeq;
    string line;
    bool isQualityLine = false;
    while (getline(ss, line))
    {
        if (line.empty())
            continue;

        if (fileType == FileType::FASTQ && line[0] == '+')
        {
            isQualityLine = true;
            continue;
        }

        if (isQualityLine)
        {
            isQualityLine = false;
            continue;
        }

        char idChar = (fileType == FileType::FASTA ? '>' : '@');
        if (line[0] == idChar)
        {
            if (!currentSeq.str().empty())
            {
                result.ids.push_back(currentID);
                result.sequences.push_back(currentSeq.str());
                currentSeq.str(string());
            }
            currentID = line.substr(1);
        }
        else
            currentSeq << line;
    }

    if (!currentSeq.str().empty())
    {
        result.ids.push_back(currentID);
        result.sequences.push_back(currentSeq.str());
    }

    return result;
}

void chunkSequences(const vector<string> &ids, const vector<string> &sequences, size_t chunkSize, size_t kmerSize,
                    vector<tuple<string, size_t, size_t, string>> &chunks)
{
    chunks.clear();
    for (size_t i = 0; i < sequences.size(); i++)
    {
        const string &id = ids[i];
        const string &sequence = sequences[i];
        for (size_t i = 0; i < sequence.size(); i += chunkSize - kmerSize)
        {
            size_t end;
            string chunk;
            if (i + chunkSize > sequence.size())
            {
                end = sequence.size();
                chunk = sequence.substr(i);
            }
            else
            {
                end = i + chunkSize;
                chunk = sequence.substr(i, chunkSize);
            }
            chunks.push_back(make_tuple(id, i, end, chunk));
        }
    }
}