#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

static std::string sha1(const std::string &message)
{
    // Initialize variables
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    // Pre-processing
    std::string paddedMessage = message;
    paddedMessage += '\x80';
    while ((paddedMessage.size() * 8) % 512 != 448)
    {
        paddedMessage += '\x00';
    }
    uint64_t messageLength = message.size() * 8;
    paddedMessage += std::string(8, '\x00');
    for (int i = 0; i < 8; i++)
    {
        paddedMessage[paddedMessage.size() - 8 + i] = (messageLength >> (56 - i * 8)) & 0xFF;
    }

    // Process message in 512-bit chunks
    for (size_t i = 0; i < paddedMessage.size(); i += 64)
    {
        std::vector<uint32_t> words(80);
        for (int j = 0; j < 16; j++)
        {
            words[j] = (paddedMessage[i + j * 4] << 24) |
                       (paddedMessage[i + j * 4 + 1] << 16) |
                       (paddedMessage[i + j * 4 + 2] << 8) |
                       (paddedMessage[i + j * 4 + 3]);
        }
        for (int j = 16; j < 80; j++)
        {
            words[j] = (words[j - 3] ^ words[j - 8] ^ words[j - 14] ^ words[j - 16]);
            words[j] = (words[j] << 1) | (words[j] >> 31);
        }

        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;

        // Main loop
        for (int j = 0; j < 80; j++)
        {
            uint32_t f, k;
            if (j < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (j < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (j < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + words[j];
            e = d;
            d = c;
            c = (b << 30) | (b >> 2);
            b = a;
            a = temp;
        }

        // Update hash values
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    // Produce final hash value
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    oss << std::setw(8) << h0 << std::setw(8) << h1 << std::setw(8) << h2 << std::setw(8) << h3 << std::setw(8) << h4;

    return oss.str();
}

// int main()
// {
//     std::string message = "";
//     std::string hash = sha1(message);
//     std::cout << "SHA-1 hash of \"" << message << "\" is: " << hash << std::endl;

//     return 0;
// }