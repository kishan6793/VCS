#ifndef CAT_FILE_HPP
#define CAT_FILE_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <fstream>
#include <sstream>

struct ObjectContent {
    std::string type;
    std::string content;
    size_t size;
};

class CatFileCommand : public Command {
private:
    void search_object(const std::string& obj_hash);
    void print_object(const std::string& obj_hash);
    void print_object_type(const std::string& obj_hash);
    ObjectContent read_and_parse_object(const std::string& obj_hash);

public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
    std::string get_object_type(const std::string& obj_hash);
};

#endif // CAT_FILE_HPP