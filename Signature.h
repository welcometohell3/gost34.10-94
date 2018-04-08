#ifndef DS_GOST_SIGNATURE_H
#define DS_GOST_SIGNATURE_H

#include <gmp.h>
#include <string>
#include <gmpxx.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "LinearGenerator.h"

class Signature
{
public:
    class Attributes
    {
        friend class Signature;

    public:
        Attributes() = default;

        void generate(const mpz_t pass, bool random = true);

        mpz_t p;
        mpz_t q;
        mpz_t a;
        mpz_t y; // public key

        LinearGenerator g{0x3DFC46F1, 97781173, 0xD, (long) pow(2, 32)};
    protected:
        std::vector<mpz_t> generatePrimes512();

        std::vector<mpz_t> generatePrimes1024();

        void generateA();

        void keys(const mpz_t pass);

        mpz_t x; // secret key
    } attributes;

public:
    Signature()
    {
        mpz_init(r);
        mpz_init(s);
    }

    Signature(const std::string &_hash)
    {
        mpz_init(r);
        mpz_init(s);
        mpz_init_set_str(hash, _hash.c_str(), 16);
    }

    ~Signature()
    {
        mpz_clear(r);
        mpz_clear(s);
    }

    void make();

    void writeToFile(const std::string &filepath);

    void readFromFile(const std::string &filepath);

    bool verify(const mpz_t p, const mpz_t q, const mpz_t a, const mpz_t y, const mpz_t hash);

private:
    mpz_t r;
    mpz_t s;
    mpz_t hash;

};

#endif
