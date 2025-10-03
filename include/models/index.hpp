#ifndef INDEX_HPP
#define INDEX_HPP

#include <string>
#include <ctime>

struct IndexEntry {
    std::string filepath;
    std::string hash;
    std::string size;
    std::string mode;
    std::time_t mtime;

    IndexEntry() {}

    IndexEntry(std::string filepath, std::string hash, std::string size, std::string mode, std::time_t mtime) : filepath(filepath), hash(hash), size(size), mode(mode), mtime(mtime) {}
};

#endif // INDEX_HPP
