#ifndef DS_GOST_SIGNATURE_H
#define DS_GOST_SIGNATURE_H

#include <gmp.h>
#include <string>
#include <gmpxx.h>

class ConGen
{
public:
    ConGen(long x0, long a, long M) :
            a(a), M(M), cur(x0) {}

    double next()
    {
        cur = (a * cur + 34213) % M;
        return cur * 1. / M;
    }

private:
    long long a, M;
    long long cur;
};

class Signature
{
public:
    class Attributes
    {
        friend class Signature;
    public:
        Attributes() = default;


        void generate()
        {

            mpz_init_set_str(p, "EE8172AE 8996608F B69359B8 9EB82A69"
                    "854510E2 977A4D63 BC97322C E5DC3386"
                    "EA0A12B3 43E9190F 23177539 84583978"
                    "6BB0C345 D165976E F2195EC9 B1C379E3", 16);

            mpz_init_set_str(q, "98915E7E C8265EDF CDA31E88 F24809DD"
                    "B064BDC7 285DD50D 7289F0AC 6F49DD2D", 16);

            mpz_init_set_str(a, "9E960315 00C8774A 869582D4 AFDE2127"
                    "AFAD2538 B4B6270A 6F7C8837 B50D50F2"
                    "06755984 A49E5093 04D648BE 2AB5AAB1"
                    "8EBE2CD4 6AC3D849 5B142AA6 CE23E21C", 16);

            keys();
        }

        mpz_t p;
        mpz_t q;
        mpz_t a;
        mpz_t y; // public key

    protected:
        void procedureA(long num)
        {
            long s = num * 2;
            mpz_class x0 = 4099;
            ConGen(x0.get_si(), 19381, 65536);
            mpz_class y0 = x0;

            for (int i = 0; i < s; i++)
            {
                // TODO: procedures
            }
        }
        void keys()
        {
            // TODO: GET THIS FROM PASSWORD OR GENERATE
            mpz_init_set_str(x, "30363145 38303830 34363045 42353244"
                    "35324234 31413237 38324331 38443046", 16);

            // y = (a ^ x) % p;
            mpz_init(y);
            mpz_powm_sec(y, a, x, p);
        }
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

    void make()
    {
        mpz_t h;
        mpz_t k;
        gmp_randstate_t rand;

        // If hash mod q = 0 then set it to 1
        (mpz_divisible_p(hash, attributes.q)) ? mpz_init_set_ui(h, 1u) : mpz_init_set(h, hash);


        mpz_init(k);
        gmp_randinit_mt(rand);
        gmp_randseed_ui(rand, 0x3CD13B26u);

        // Get random k
        mpz_urandomm(k, rand, attributes.q);

        // r = ((a ^ k) % p) % q;
        mpz_powm_sec(r, attributes.a, k, attributes.p);
        mpz_mod(r, r, attributes.q);

        // s = (r * x + k * h) % q;
        mpz_mul(s, r, attributes.x);
        mpz_addmul(s, k, h);
        mpz_mod(s, s, attributes.q);

        gmp_randclear(rand);
        mpz_clear(k);
        mpz_clear(h);
    }

    void writeToFile(const std::string &filepath)
    {
        FILE *f = fopen(filepath.c_str(), "wb");
        mpz_out_raw(f, r);
        mpz_out_raw(f, s);
        fclose(f);
    }

    void readFromFile(const std::string &filepath)
    {
        FILE *f = fopen(filepath.c_str(), "rb");
        mpz_inp_raw(r, f);
        mpz_inp_raw(s, f);
        fclose(f);
    }

    bool verify(const mpz_t p, const mpz_t q, const mpz_t a, const mpz_t y, const mpz_t hash)
    {
        if (mpz_sgn(s) <= 0 ||
            mpz_sgn(r) <= 0 ||
            mpz_cmp(s, q) >= 0 ||
            mpz_cmp(r, q) >= 0)
        {
            return false;
        }

        mpz_t h;
        mpz_t v;
        mpz_t z1;
        mpz_t z2;
        mpz_t u;

        // If hash mod q = 0 then set it to 1
        (mpz_divisible_p(hash, attributes.q)) ? mpz_init_set_ui(h, 1u) : mpz_init_set(h, hash);

        mpz_init(v);
        mpz_init(z1);
        mpz_init(z2);
        mpz_init(u);

        // v = (h ^ (q - 2)) % q;
        mpz_sub_ui(v, q, 2u);
        mpz_powm_sec(v, h, v, q);

        // z1 = (s * v) % q;
        mpz_mul(z1, s, v);
        mpz_mod(z1, z1, q);

        // z2 = ((q - r) * v) % q;
        mpz_sub(z2, q, r);
        mpz_mul(z2, z2, v);
        mpz_mod(z2, z2, q);

        // u = (((a ^ z1) * (y ^ z2)) % p) % q;
        mpz_powm_sec(z1, a, z1, p);
        mpz_powm_sec(z2, y, z2, p);
        mpz_mul(u, z1, z2);
        mpz_mod(u, u, p);
        mpz_mod(u, u, q);

        bool res = (mpz_cmp(r, u) == 0);

        mpz_clear(u);
        mpz_clear(z2);
        mpz_clear(z1);
        mpz_clear(v);
        mpz_clear(h);

        if (res)
        {
            printf("OK");
        } else
        {
            printf("FAIL");
        }

        return res;
    }

private:
    mpz_t r;
    mpz_t s;
    mpz_t hash;

};


#endif
