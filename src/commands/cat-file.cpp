#include "commands/cat-file.hpp"

void CatFileCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs cat-file <flag> <object>");
    utils::write(utils::INFO, "flag  : -e (checks if <object> exits))");
    utils::write(utils::INFO, "flag  : -p (pretty-print <object> content)");
    utils::write(utils::INFO, "flag  : -t (show object type (one of 'blob', 'tree', 'commit')");
    utils::write(utils::EMPTY);
}

void CatFileCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size < 2) {
        const std::string error_msg = "Too few arguments";
        throw std::invalid_argument(error_msg);
    }

    if(args[0] != "-p" && args[0] != "-t" && args[0] != "-e") {
        const std::string error_msg = "Please provide a valid <type>";
        throw std::invalid_argument(error_msg);
    }

    if(!utils::is_valid_hash_syntax(args[1])) {
        const std::string error_msg = "Not a valid object name:" + args[1];
        throw std::invalid_argument(error_msg);
    }
    
    if(args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
}

ObjectContent CatFileCommand::read_and_parse_object(const std::string& obj_hash) {
    if(!utils::is_exist_obj(obj_hash)) {
        const std::string error_msg = "Not a valid object name: " + obj_hash;
        throw std::invalid_argument(error_msg);
    }

    // Get object path
    const std::string obj_path = utils::get_object_path(obj_hash);

    // Read and Uncompress the object
    const std::string decompressed = utils::read_and_decompress(obj_path);

    const size_t null_pos = decompressed.find('\0');
    if(null_pos == std::string::npos) {
        std::string error_msg = "Corrupt object: missing null terminator";
        throw std::runtime_error(error_msg);
    }

    const std::string header = decompressed.substr(0, null_pos);

    const size_t space_pos = header.find(' ');
    if(space_pos == std::string::npos) {
        const std::string error_msg = "Corrupt object: invalid header format";
        throw std::runtime_error(error_msg);
    }

    return {
        .type = header.substr(0, space_pos),
        .content = decompressed.substr(null_pos + 1),
        .size = std::stoul(header.substr(space_pos + 1))
    };
}


void CatFileCommand::search_object(const std::string& obj_hash) {
    try {
        read_and_parse_object(obj_hash);
        utils::write(utils::TRUE, "Object exists:", obj_hash);
    } catch(const std::exception& e) {
        utils::write(utils::FALSE, "Object not found:", obj_hash);
    }
}

void CatFileCommand::print_object(const std::string& obj_hash) {
    ObjectContent obj = read_and_parse_object(obj_hash);
    utils::write(utils::OK, obj.type, obj.size);
    utils::write(utils::CONTENT);
    std::cout << obj.content;
}

std::string CatFileCommand::get_object_type(const std::string& obj_hash) {
    return read_and_parse_object(obj_hash).type;
}

void CatFileCommand::print_object_type(const std::string& obj_hash) {
    ObjectContent obj = read_and_parse_object(obj_hash);
    utils::write(utils::OK, obj.type);
}

void CatFileCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const char flag = args[0][1];
    const std::string obj_hash = args[1];

    switch (flag)
    {
        case 'e':
            search_object(obj_hash);
            break;

        case 'p':
            print_object(obj_hash);
            break;
        
        case 't':
            print_object_type(obj_hash);
            break;
        
        default:
            const std::string error_msg = "cat-file Validation Failed.";
            throw std::logic_error(error_msg);
            break;
    }
}