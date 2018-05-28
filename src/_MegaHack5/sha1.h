/*
sha1.hpp - header of
============
SHA-1 in C++
============
100% Public Domain.
Original C Code
-- Steve Reid <steve@edmweb.com>
Small changes to fit into bglibs
-- Bruce Guenter <bruce@untroubled.org>
Translation to simpler C++ Code
-- Volker Grabsch <vog@notjusthosting.com>
Safety fixes
-- Eugene Hopkinson <slowriot at voxelstorm dot com>
*/

#ifndef SHA1_HPP
#define SHA1_HPP


#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

struct validation
{
    const char *ver, *hash;
    size_t size;
};

class SHA1
{
public:
    SHA1();
    void update(const std::string &s);
    void update(std::istream &is);
    std::string final();
    static std::string from_file(const std::string &filename);

private:
    uint32_t digest[5];
    std::string buffer;
    uint64_t transforms;
};

std::string sha1_file(const std::string& file);

std::string get_version(const std::string& path);

#endif /* SHA1_HPP */
