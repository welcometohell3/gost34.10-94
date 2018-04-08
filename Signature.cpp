#include "Signature.h"

void Signature::make()
{
    mpz_t h;
    mpz_t k;
    gmp_randstate_t rand;

    // If hash mod q = 0 then set it to 1
    (mpz_divisible_p(hash, attributes.q)) ? mpz_init_set_ui(h, 1u) : mpz_init_set(h, hash);

    mpz_init(k);
    gmp_randinit_mt(rand);
    gmp_randseed_ui(rand, attributes.g.getSeed());

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

bool Signature::verify(const __mpz_struct *p, const __mpz_struct *q, const __mpz_struct *a, const __mpz_struct *y,
                       const __mpz_struct *hash)
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

void Signature::readFromFile(const std::string &filepath)
{
    FILE *f = fopen(filepath.c_str(), "rb");
    mpz_inp_raw(r, f);
    mpz_inp_raw(s, f);
    fclose(f);
}

void Signature::writeToFile(const std::string &filepath)
{
    printf("Signature: \n");
    gmp_printf("r: %Zd \n", r);
    gmp_printf("s: %Zd \n", s);

    FILE *f = fopen(filepath.c_str(), "wb");

    mpz_out_raw(f, r);
    mpz_out_raw(f, s);
    fclose(f);
}

std::vector<mpz_t> Signature::Attributes::generatePrimes512()
{
    gmp_randstate_t rand;
    gmp_randinit_mt(rand);
    gmp_randseed_ui(rand, g.getSeed());
    int bitLength = 512;
    std::vector<int> t;
    t.push_back(bitLength);
    int index = 0;
    while (t[index] >= 33)
    {
        auto value = (int) floor(t[index] / 2);
        t.push_back(value);
        index++;
    }
    std::vector<mpz_t> primes(t.size());

    for (int i = 0; i < primes.size(); i++)
    {
        mpz_init(primes[i]);
        mpz_set_ui(primes[i], 0u);
    }
    mpz_t bits;
    mpz_init(bits);

    mpz_rrandomb(bits, rand, 32);
    mpz_nextprime(primes[index], bits);

    int m = index - 1;
    bool flag = true;
    do
    {
        int r = (int) std::ceil(t[m] / 32);
        mpz_t n;
        mpz_t k;
        mpz_init_set_ui(n, 0u);
        mpz_init_set_ui(k, 0u);
        do
        {
            if (flag)
            {

                std::vector<double> y = g.generate(r);
                mpz_t sum;
                mpz_init_set_ui(sum, 0u);

                for (int i = 0; i < r - 1; i++)
                {
                    mpz_t tmp;
                    mpz_init(tmp);
                    mpz_t y_tmp;

                    mpz_init_set_d(y_tmp, y[i]);
                    mpz_t two32;
                    mpz_init(two32);
                    mpz_ui_pow_ui(two32, 2, 32);

                    mpz_addmul(sum, y_tmp, two32);

                    mpz_clear(tmp);
                    mpz_clear(y_tmp);
                    mpz_clear(two32);
                }

                mpz_t prevSeed;
                auto sd = g.getSeed();
                mpz_init_set_d(prevSeed, g.getSeed());

                mpz_add(sum, sum, prevSeed);
                g.setSeed(y[r - 1]);

                mpz_t tmp1;
                mpz_init(tmp1);
                mpz_ui_pow_ui(tmp1, 2, t[m] - 1);
                mpz_cdiv_q(tmp1, tmp1, primes[m + 1]);

                mpz_t tmp2;
                mpz_init_set_ui(tmp2, 0u);
                mpz_ui_pow_ui(tmp2, 2, t[m] - 1);
                mpz_mul(tmp2, tmp2, sum);

                mpz_t two32r;
                mpz_init(two32r);
                mpz_ui_pow_ui(two32r, 2, 32 * r);
                mpz_mul(two32r, two32r, primes[m + 1]);

                mpz_fdiv_q(tmp2, tmp2, two32r);

                mpz_add(n, tmp1, tmp2);

                if (!mpz_even_p(n))
                {
                    mpz_add_ui(n, n, 1u);
                }

                mpz_set_ui(k, 0);

                mpz_clear(sum);
                mpz_clear(prevSeed);
                mpz_clear(tmp1);
                mpz_clear(tmp2);
                mpz_clear(two32r);
            }

            mpz_t nplusk;
            mpz_init(nplusk);
            mpz_add(nplusk, n, k);
            mpz_mul(primes[m], primes[m + 1], nplusk);
            mpz_add_ui(primes[m], primes[m], 1u);

            mpz_t twot;
            mpz_init(twot);
            mpz_ui_pow_ui(twot, 2, t[m]);

            if (mpz_cmp(primes[m], twot) == 1)
            {
                flag = true;
                continue;
            }

            mpz_t tmp;
            mpz_init(tmp);

            mpz_t tmp2;
            mpz_init(tmp2);

            mpz_clear(nplusk);
            mpz_init(nplusk);
            mpz_add(nplusk, n, k);
            mpz_mul(tmp, primes[m + 1], nplusk);
            mpz_set(tmp2, nplusk);

            mpz_t two;
            mpz_init_set_ui(two, 2u);

            mpz_powm(tmp, two, tmp, primes[m]);
            mpz_powm(tmp2, two, tmp2, primes[m]);

            mpz_t one;
            mpz_init_set_ui(one, 1u);

            if ((mpz_cmp(tmp, one) != 0) || (mpz_cmp(tmp2, one) == 0))
            {
                flag = false;
                mpz_add_ui(k, k, 2);
            } else
            {
                flag = true;
                mpz_clear(nplusk);
                mpz_clear(twot);
                mpz_clear(tmp);
                mpz_clear(tmp2);
                mpz_clear(two);
                mpz_clear(one);
                break;
            }
            mpz_clear(nplusk);
            mpz_clear(twot);
            mpz_clear(tmp);
            mpz_clear(tmp2);
            mpz_clear(two);
            mpz_clear(one);
        } while (true);
        m--;
    } while (m >= 0);
    gmp_randclear(rand);
    return primes;
}

std::vector<mpz_t> Signature::Attributes::generatePrimes1024()
{

    int bitLength = 1024;

    mpz_t q;
    mpz_init_set(q, generatePrimes512()[1]);
    mpz_t Q;
    mpz_init_set(Q, generatePrimes512()[0]);

    mpz_t p;
    mpz_init(p);

    mpz_t n;
    mpz_t k;
    mpz_init_set_ui(n, 0u);
    mpz_init_set_ui(k, 0u);
    bool flag = true;
    do
    {
        if (flag)
        {
            std::vector<double> y = g.generate(32);

            mpz_t sum;
            mpz_init_set_ui(sum, 0u);
            for (int i = 0; i < y.size() - 1; i++)
            {
                mpz_t tmp;
                mpz_init(tmp);
                mpz_t y_tmp;

                mpz_init_set_d(y_tmp, y[i]);
                mpz_t two32;
                mpz_init(two32);
                mpz_ui_pow_ui(two32, 2, 32);

                mpz_addmul(sum, y_tmp, two32);

                mpz_clear(tmp);
                mpz_clear(y_tmp);
                mpz_clear(two32);
            }

            mpz_t prevSeed;
            auto sd = g.getSeed();
            mpz_init_set_d(prevSeed, g.getSeed());

            mpz_add(sum, sum, prevSeed);
            g.setSeed(y[y.size() - 1]);

            mpz_t tmp1;
            mpz_init(tmp1);
            mpz_ui_pow_ui(tmp1, 2, bitLength - 1);
            mpz_t qmulQ;
            mpz_init(qmulQ);
            mpz_mul(qmulQ, q, Q);
            mpz_cdiv_q(tmp1, tmp1, qmulQ);

            mpz_t tmp2;
            mpz_init_set_ui(tmp2, 0u);
            mpz_ui_pow_ui(tmp2, 2, bitLength - 1);
            mpz_mul(tmp2, tmp2, sum);

            mpz_t two32r;
            mpz_init(two32r);
            mpz_ui_pow_ui(two32r, 2, bitLength);
            mpz_mul(two32r, two32r, qmulQ);

            mpz_fdiv_q(tmp2, tmp2, two32r);

            mpz_add(n, tmp1, tmp2);

            if (!mpz_even_p(n))
            {
                mpz_add_ui(n, n, 1u);
            }

            mpz_set_ui(k, 0);

            mpz_clear(sum);
            mpz_clear(prevSeed);
            mpz_clear(tmp1);
            mpz_clear(tmp2);
            mpz_clear(two32r);
            mpz_clear(qmulQ);
        }
        mpz_t qmulQ;
        mpz_init(qmulQ);
        mpz_mul(qmulQ, q, Q);

        mpz_t nplusk;
        mpz_init(nplusk);
        mpz_add(nplusk, n, k);
        mpz_mul(p, qmulQ, nplusk);
        mpz_add_ui(p, p, 1u);

        mpz_t twot;
        mpz_init(twot);
        mpz_ui_pow_ui(twot, 2, bitLength);

        if (mpz_cmp(p, twot) == 1)
        {
            flag = true;
            continue;
        }

        mpz_t tmp;
        mpz_init(tmp);

        mpz_t tmp2;
        mpz_init(tmp2);

        mpz_clear(nplusk);
        mpz_init(nplusk);
        mpz_add(nplusk, n, k);
        mpz_mul(tmp, qmulQ, nplusk);
        mpz_set(tmp2, nplusk);

        mpz_t two;
        mpz_init_set_ui(two, 2u);

        mpz_powm(tmp, two, tmp, p);
        mpz_powm(tmp2, two, tmp2, p);

        mpz_t one;
        mpz_init_set_ui(one, 1u);
        int res1 = mpz_cmp(tmp, one);
        if ((mpz_cmp(tmp, one) != 0) || (mpz_cmp(tmp2, one) == 0))
        {
            flag = false;
            mpz_add_ui(k, k, 2);
        } else
        {
            mpz_clear(nplusk);
            mpz_clear(twot);
            mpz_clear(tmp);
            mpz_clear(tmp2);
            mpz_clear(two);
            mpz_clear(one);
            break;
        }
        mpz_clear(nplusk);
        mpz_clear(twot);
        mpz_clear(tmp);
        mpz_clear(tmp2);
        mpz_clear(two);
        mpz_clear(one);
    } while (true);

    mpz_clear(Q);
    std::vector<mpz_t> res(2);
    mpz_set(res[0], q);
    mpz_set(res[1], p);
    gmp_printf("q: %Zd \n", q);
    gmp_printf("p: %Zd \n", p);
    mpz_set(this->q, q);
    mpz_set(this->p, p);
    return res;
}

void Signature::Attributes::generateA()
{
    int bitLength = 1024;
    gmp_randstate_t rand;
    gmp_randinit_mt(rand);
    gmp_randseed_ui(rand, g.getSeed());
    mpz_t d;
    mpz_t f;

    mpz_init(d);
    mpz_init(f);
    do
    {
        mpz_rrandomb(d, rand, bitLength);
        mpz_t tmp;
        mpz_init(tmp);
        mpz_sub_ui(tmp, p, 1u);
        mpz_div(tmp, tmp, q);
        mpz_powm(f, d, tmp, p);

    } while (mpz_cmp_ui(f, 1u) == 0);
    mpz_set(a, f);
    gmp_printf("a: %Zd \n", a);
    gmp_randclear(rand);
    mpz_clear(d);
    mpz_clear(f);
}

void Signature::Attributes::keys(const mpz_t pass)
{
    mpz_set(x, pass);

    // y = (a ^ x) % p;
    mpz_init(y);
    mpz_powm_sec(y, a, x, p);
}

void Signature::Attributes::generate(const __mpz_struct *pass, bool random)
{
    if (random)
    {
        generatePrimes1024();
        generateA();
    } else
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
    }

    keys(pass);
    unsigned long long seed = 0;
    mpz_export(&seed, nullptr, -1, sizeof seed, 0, 0, pass);
    g.setSeed(seed);
}
