#ifndef HASH_OBJECT_HPP
#define HASH_OBJECT_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <fstream>
#include <sstream>

class HashObjectCommand : public Command {
private:
    void print_file_hash(const std::string& file_path);

public:
    void help() override;
    static std::string write_obj(const std::stringstream& buffer, const std::string& type);
    std::string write_object(const std::string& file_path);
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // HASH_OBJECT_HPP