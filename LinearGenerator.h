#ifndef DS_GOST_LINEARGENERATOR_H
#define DS_GOST_LINEARGENERATOR_H


#include <vector>
#include <cstdint>

class LinearGenerator
{
public:
    LinearGenerator(int_fast64_t x0, int_fast64_t a, int_fast64_t c, int_fast64_t M) :
            a(a), c(c), M(M), cur(x0), seed(x0) {}


    std::vector<double> generate(unsigned long number)
    {
        auto numVec = std::vector<double>(number);
        for (int i = 0; i < number; i++)
        {
            numVec[i] = next();
        }

        return numVec;
    }

    double getSeed()
    {
        return seed;
    }

    void setSeed(int_fast64_t seed)
    {
        cur = seed;
        this->seed = seed;
    }

private:
    double next()
    {
        cur = (a * cur + c) % M;
        return cur;
    }

    int_fast64_t a, c, M;
    int_fast64_t cur;
    int_fast64_t seed;
};


#endif
