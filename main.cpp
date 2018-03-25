#include <iostream>
#include "md5.h"
#include "Signature.h"

int main()
{
    std::string hash = md5file("file");
    mpz_t hash_t;

    mpz_init_set_str(hash_t, hash.c_str(), 16);

    std::cout << hash << std::endl;

    Signature sig(hash);
    sig.attributes.generate();
    sig.make();
    sig.writeToFile("file.sg");

    Signature fileSig;
    fileSig.readFromFile("file.sg");
    fileSig.verify(sig.attributes.p, sig.attributes.q,
                   sig.attributes.a, sig.attributes.y, hash_t);

    return 0;
}