#include <iostream>
#include "md5.h"
#include "Signature.h"
#include "InputParser.h"

int main(int argc, char **argv)
{
    InputParser input(argc, argv);

    const std::string filename = input.getCmdOption("-f");
    const std::string password = input.getCmdOption("-p");

    std::string fileHash = md5file(filename.c_str());
    std::string passHash = md5(password);

    mpz_t file_t;
    mpz_t pass_t;

    mpz_init_set_str(file_t, fileHash.c_str(), 16);
    mpz_init_set_str(pass_t, password.c_str(), 16);

    std::cout << "File hash: " << fileHash << std::endl;
    std::cout << "Pass hash: " << passHash << std::endl;


    Signature sig(fileHash);
    sig.attributes.generate(pass_t);
    sig.make();
    sig.writeToFile(std::string(filename + ".sg"));

    Signature fileSig;
    fileSig.readFromFile(std::string(filename + ".sg"));
    fileSig.verify(sig.attributes.p, sig.attributes.q,
                   sig.attributes.a, sig.attributes.y, file_t);

    return 0;
}