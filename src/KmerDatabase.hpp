#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <set>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <mutex>
#include <filesystem>

#include <boost/progress.hpp>

#include "thread_pool.hpp"
#include "Window.hpp"
#include "Utils.hpp"
#include "../KMC/kmc_api/kmc_file.h"

#define CHUNK_SIZE 1000000 // defines the size of each sequence chunk when processing files

mutex m;

class KmerDatabase
{
public:
    uint getKmerSize() const
    {
        return kmerSize;
    }

    void printKMCInfo()
    {
        std::cout << "********** KMC Info **********\n";
        std::cout << "K-mer length: " << KMCInfo.kmer_length << '\n';
        std::cout << "Mode: " << KMCInfo.mode << '\n';
        std::cout << "Counter size: " << KMCInfo.counter_size << '\n';
        std::cout << "LUT prefix length: " << KMCInfo.lut_prefix_length << '\n';
        std::cout << "Signature length: " << KMCInfo.signature_len << '\n';
        std::cout << "Min count: " << KMCInfo.min_count << '\n';
        std::cout << "Max count: " << KMCInfo.max_count << '\n';
        std::cout << "Both strands: " << KMCInfo.both_strands << '\n';
        std::cout << "Total k-mers: " << KMCInfo.total_kmers << '\n';
    }

    KmerDatabase(string sourcePath, bool listing = false) : sourcePath(sourcePath)
    {
        if (!listing && !KMCDatabase.OpenForRA(sourcePath.c_str()))
        {
            std::cerr << "Error: Could not open KMC database\n";
            exit(1);
        }
        else if (listing && !KMCDatabase.OpenForListing(sourcePath.c_str()))
        {
            std::cerr << "Error: Could not open KMC database\n";
            exit(1);
        }

        KMCDatabase.Info(KMCInfo);
        kmerSize = KMCInfo.kmer_length;
        // printKMCInfo(KMCInfo);
    }

    bool isKmer(CKmerAPI &kmer)
    {
        return KMCDatabase.IsKmer(kmer);
    }

    bool readNextKmer(CKmerAPI &kmer)
    {
        uint32 count;
        return KMCDatabase.ReadNextKmer(kmer, count);
    }

    bool restartListing()
    {
        return KMCDatabase.RestartListing();
    }

    Window getStatsFromSequence(const tuple<string, size_t, size_t, string> &window)
    {
        Window stats;
        stats.id = get<0>(window);
        stats.start = get<1>(window);
        stats.end = get<2>(window);
        auto sequence = get<3>(window);

        auto kmers = getCanonicalKmers(sequence, kmerSize);
        stats.totalKmers = kmers.size();

        int gapSize = 0;
        CKmerAPI KMCKmer;
        for (const auto &kmer : kmers)
        {
            KMCKmer.from_string(kmer);
            if (!KMCDatabase.IsKmer(KMCKmer))
            {
                gapSize += 1;
            }
            else
            {
                stats.observedKmers += 1;
                if (gapSize > 0)
                {
                    stats.addVariationOriginal(gapSize, int(kmerSize));
                    gapSize = 0;
                }
            }
        }
        if (gapSize > 0)
            stats.addVariationOriginal(gapSize, kmerSize);

        return stats;
    }


    pair<string,string> getMappingFromSequence(const string &id, string sequence)
    {
        auto seqSize = sequence.size();
        vector<short> mapping(seqSize, 0);
        for (size_t i = 0; i <= seqSize - kmerSize; i++)
        {
            string kmer = sequence.substr(i, kmerSize);
            transform(kmer.begin(), kmer.end(), kmer.begin(), ::toupper);
            if (kmer.find_first_not_of("ACGT") != string::npos)
                continue;
            string revComplement = reverseComplement(kmer);
            string canonicalKmer = kmer > revComplement ? revComplement : kmer;
            CKmerAPI KMCKmer;
            KMCKmer.from_string(canonicalKmer);            
            if (KMCDatabase.IsKmer(KMCKmer))
            {
                for (size_t j = 0; j < kmerSize; j++)
                {
                    mapping[i + j] += 1;
                }
            }
        }

        string mappingStr;
        for (auto m : mapping)
        {
            mappingStr += to_string(m) + ",";
        }

        if (!mappingStr.empty())
        {
            mappingStr.pop_back();
        }

        return {id, mappingStr};
    }

    size_t getTotalLength(const vector<string> &sequences)
    {
        size_t totalLength = 0;
        for (const auto &seq : sequences)
        {
            totalLength += seq.size();
        }
        return totalLength;
    }

    size_t estimateNumChunks(const vector<string> &sequences, size_t chunkSize)
    {
        size_t totalLength = getTotalLength(sequences);
        return totalLength / chunkSize + sequences.size() * 2;
    }

    void processReference(string refPath, string outPath, int windowSize = 50000)
    {
        cout << "Window size: " << windowSize << endl;
        cout << "Kmer size: " << kmerSize << endl;

        cout << "Reading sequences" << endl;
        bool compressed = refPath.find(".gz") != string::npos;
        auto result = readSequenceFile(refPath, compressed);
        auto sequences = result.sequences;
        auto ids = result.ids;
        vector<tuple<string, size_t, size_t, string>> chunks;
        chunks.reserve(estimateNumChunks(sequences, windowSize));
        chunkSequences(ids, sequences, windowSize, kmerSize, chunks);

        cout << "Number of chunks: " << chunks.size() << endl;

        /************************************************************/

        cout << "Calculating stats" << endl;
        vector<Window> statsResult(chunks.size());
        thread_pool pool;
        boost::progress_display progressBar(chunks.size());
        for (size_t i = 0; i < chunks.size(); i++)
        {
            pool.push_task([i, this, &chunks, &statsResult, &progressBar]
                           { 
                            statsResult[i] = getStatsFromSequence(chunks[i]); 
                            ++progressBar; });
        }
        pool.wait_for_tasks();
        cout << endl;

        /************************************************************/

        cout << "Writing stats to file" << endl;
        ofstream statsFile(outPath);
        if (statsFile.is_open())
        {
            statsFile << "seqname\tstart\tend\ttotal_kmers\tobserved_kmers\tvariations\tkmer_distance\n";
            for (size_t i = 0; i < statsResult.size(); i++)
            {
                auto stats = statsResult[i];
                statsFile << stats.id << '\t' << stats.start << '\t' << stats.end << '\t'
                          << stats.totalKmers << '\t' << stats.observedKmers << '\t'
                          << stats.variations << '\t' << stats.kmerDistance << endl;
            }
            statsFile.close();
        }
        else
        {
            cerr << "Unable to open file for writing" << endl;
        }
    }


void produceMapping(string refPath, string outPath)
    {
        cout << "Kmer size: " << kmerSize << endl;

        cout << "Reading sequences" << endl;
        bool compressed = refPath.find(".gz") != string::npos;
        auto result = readSequenceFile(refPath, compressed);
        auto sequences = result.sequences;
        auto ids = result.ids;

        /************************************************************/

        cout << "Calculating mapping" << endl;
        vector<pair<string, string>> mappingResult(sequences.size());
        thread_pool pool;
        boost::progress_display progressBar(sequences.size());
        for (size_t i = 0; i < sequences.size(); i++)
        {
            pool.push_task([i, this, &sequences, &ids, &mappingResult, &progressBar]
                           { 
                            mappingResult[i] = getMappingFromSequence(ids[i], sequences[i]); 
                            ++progressBar; });
        }
        pool.wait_for_tasks();
        cout << endl;

        /************************************************************/

        cout << "Writing mapping to file" << endl;
        ofstream mappingFile(outPath);
        if (mappingFile.is_open())
        {
            for (size_t i = 0; i < mappingResult.size(); i++)
            {
                auto [id, seq] = mappingResult[i];
                mappingFile << id << '\n' << seq << endl;
            }
            mappingFile.close();
        }
        else
        {
            cerr << "Unable to open file for writing" << endl;
        }
    }


private:
    uint kmerSize, chunkSize = CHUNK_SIZE;
    string sourcePath;
    CKMCFile KMCDatabase;
    CKMCFileInfo KMCInfo;
};
