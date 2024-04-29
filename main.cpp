#include <iostream>
#include "sha1.hpp"
#include "sig.hpp"

int main()
{
    const std::string password = "passworddasdasdasda";
    const std::string msg = "helloworlddsdasdasdadasda";

    std::string hash = sha1(msg);
    std::string passHash = sha1(password);

    mpz_t file_t;
    mpz_t pass_t;

    mpz_init_set_str(file_t, hash.c_str(), 16);
    mpz_init_set_str(pass_t, passHash.c_str(), 16);

    // std::cout << "File hash: " << hash << std::endl;
    // std::cout << "Pass hash: " << passHash << std::endl;

    Signature sig(hash);
    // gmp_printf("My variable: %Zd\n", pass_t);
    sig.attributes.generate(pass_t);
    sig.make();
    // sig.writeToFile(std::string(filename + ".sg"));

    Signature fileSig;
    // fileSig.readFromFile(std::string(filename + ".sg"));
    sig.verify(sig.attributes.p, sig.attributes.q,
               sig.attributes.a, sig.attributes.y, file_t);

    return 0;
}