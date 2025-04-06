
#include <cstddef>

class Window
{
public:
    std::string id;
    int start;
    int end;
    int totalKmers;
    int observedKmers;
    int variations;
    int kmerDistance;

    Window() : totalKmers(0), observedKmers(0), variations(0), kmerDistance(0) {}

    void addVariation(int gapSize, int kmerSize)
    {
        variations += 1;
        if (gapSize < kmerSize)
            kmerDistance = gapSize;
        else
            kmerDistance = gapSize - (kmerSize - 1);
    }

    void addVariationOriginal(int gapSize, int kmerSize)
    {
        variations += 1;
        int kmerDistance_ = gapSize - (kmerSize - 1);
        if (kmerDistance_ <= 0)
            kmerDistance_ = abs(kmerDistance_ + 1);
        kmerDistance += kmerDistance_; 
    }
};