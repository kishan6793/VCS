#include "commands/ls-tree.hpp"

void LsTreeCommand::help() 
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs ls-tree <tree-hash>");
    utils::write(utils::EMPTY);
}

void LsTreeCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size > 1) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }

    if(CatFileCommand().get_object_type(args[0]) != "tree") {
        std::string error_msg = "Not a valid tree hash: " + args[0];
        throw std::invalid_argument(error_msg);
    }
}

void LsTreeCommand::print_tree(const std::string& tree_hash) {
    std::string tree_path = utils::get_object_path(tree_hash);
    std::string tree_content = utils::decompress_zlib(utils::read_file_content(tree_path));

    std::istringstream iss(tree_content);
    std::string line;

    std::getline(iss, line);
    std::istringstream iss_header(line);
    std::string type, size;
    iss_header >> type >> size;

    if(type != "tree") {
        const std::string error_msg = "Invalid tree object: " + tree_hash;
        throw std::invalid_argument(error_msg);
    }

    utils::write(utils::OK);
    utils::write(utils::CONTENT, type, size);

    while (std::getline(iss, line)) {
        std::istringstream iss_line(line);
        std::string mode, type, hash, mtime, file_size, file_name;
        iss_line >> mode >> type >> hash >> mtime >> file_size >> file_name;

        int len = (size.size() - file_size.size());
        file_size += std::string(len, ' ');

        utils::write(utils::CONTENT, mode, type, hash, mtime, file_size, file_name);
    }
}

void LsTreeCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const std::string& tree_hash = args[0];
    print_tree(tree_hash);
}