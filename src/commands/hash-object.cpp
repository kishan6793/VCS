#include "commands/hash-object.hpp"

void HashObjectCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs hash-object [flag] <file-name>");
    utils::write(utils::INFO, "flag  : -w (Generates SHA-1 hash and write to .vcs/objects/ directory)");
    utils::write(utils::EMPTY);
}

void HashObjectCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();
    
    if((args_size == 0) || (args_size == 1 && args[0] == "-w")) {
        const std::string error_msg = "Too few arguments";
        throw std::invalid_argument(error_msg);
    }
    else if(args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
    else {
        if(args_size == 2 && args[0] != "-w") {
            const std::string error_msg = "Invalid flag. Only '-w' is supported";
            throw std::invalid_argument(error_msg);
        }

        if(!utils::is_file_exist(args.back())) {
            std::string error_msg = "Could not open '" + args.back() + "' for reading: No such file or directory";
            throw std::invalid_argument(error_msg);
        }
    }
}

std::string HashObjectCommand::write_obj(const std::stringstream& buffer, const std::string& type) {
    const std::string content = buffer.str();
    const std::string hash = utils::sha1(content);

    std::string header = type + " " + std::to_string(content.size()) + '\0';
    std::string full_content = header + content;

    std::string obj_path = config::OBJECTS_DIR + hash.substr(0, 2) + "/";
    if(utils::create_directory(obj_path) == utils::DIR_STATUS::ERROR) {
        const std::string error_msg = "Failed to create directory: " + obj_path;
        throw std::runtime_error(error_msg);
    }

    std::string obj_file_path = obj_path + hash.substr(2);
    if (utils::is_file_exist(obj_file_path)) { return hash; }
    
    std::string compressed = utils::compress_zlib(full_content);

    std::ofstream obj_file(obj_file_path, std::ios::binary);
    obj_file.write(compressed.data(), compressed.size());
    obj_file.close();

    return hash;
}

std::string HashObjectCommand::write_object(const std::string& file_path) {
    const std::string content = utils::read_file_content(file_path);

    std::stringstream buffer;
    buffer << content;

    const std::string hash = write_obj(buffer, "blob");  
    return hash;
}

void HashObjectCommand::print_file_hash(const std::string& file_path) {
    std::string content = utils::read_file_content(file_path);
    std::string hash = utils::sha1(content);
    utils::write(utils::OK, hash);
}

void HashObjectCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    if(args.size() == 2) {
        std::string file_path = args[1];
        const std::string hash = write_object(file_path);
        utils::write(utils::OK, hash);
    }
    else if(args.size() == 1) {
        std::string file_path = args[0];
        print_file_hash(file_path);
    }
    else {
        std::string error_msg = "hash-object Validation Failed.";
        throw std::logic_error(error_msg);
    }
}