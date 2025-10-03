#include "commands/add.hpp"

void AddCommand::help() 
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs add [flag] <path>");
    utils::write(utils::INFO, "flag  : -s (status)");
    utils::write(utils::EMPTY);
}

void AddCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if ((args_size == 0) || (args_size == 1 && args[0] == "-s")) {
        const std::string error_msg = "Too few arguments";
        throw std::invalid_argument(error_msg);
    }
    else if (args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
    else {
        if(args_size == 2 && args[0] != "-s") {
            const std::string error_msg = "Invalid flag. Only '-s' is supported";
            throw std::invalid_argument(error_msg);
        }

        const std::string path = utils::normalizeRelativePath(args.back());

        if(path.empty()) {
            const std::string error_msg = "Invalid path: Path must not be empty, absolute, or escape the current directory.";
            throw std::invalid_argument(error_msg);
        }

        if(!utils::path_exists(path)) {
            const std::string error_msg = "Invalid path: '" + path + "' does not exist.";
            throw std::invalid_argument(error_msg);
        }

        args.back() = path;
    }
}

void process_file(const bool is_status_flag, const fs::path& file_path, std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, std::time_t>>& index_map) {
    const std::string file_content = utils::read_file_content(file_path.string());
    const std::string newHash = utils::sha1(file_content);

    auto it = index_map.find(file_path.string());
    if (it != index_map.end()) {
        const auto& [hash, size, mode, mtime] = it->second;
        if (hash == newHash) { return; } // File is unchanged
    }

    std::stringstream buffer;
    buffer << file_content;

    const std::string hash = HashObjectCommand::write_obj(buffer, "blob"); 
    const std::string file_size = std::to_string(fs::file_size(file_path));
    const std::string mode = utils::get_file_mode(file_path.string());
    const std::time_t mtime = utils::get_mtime(file_path.string());
    
    // Store index entry: path, hash, mode, timestamp
    index_map[file_path.string()] = {hash, file_size, mode, mtime};

    if(is_status_flag) utils::write(utils::OK, hash, file_path.string());
};

void iterate_directory(const bool is_status_flag, const fs::path& path, const std::set<std::string>& ignore_list, std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, std::time_t>>& index_map) {
    if (fs::is_regular_file(path)) {
        // Directly process a single file
        if (!utils::is_ignored(path, false, ignore_list)) {
            fs::path rel_path = fs::relative(path, fs::current_path());
            process_file(is_status_flag, rel_path, index_map);
        }
        return;
    }
    
    if (!fs::is_directory(path)) { return; }
    
    for (const auto& entry : fs::directory_iterator(path)) {
        if (utils::is_ignored(entry.path(), entry.is_directory(), ignore_list)) continue;

        if (entry.is_directory()) {
            iterate_directory(is_status_flag, entry.path(), ignore_list, index_map);
        } else {
            fs::path rel_path = fs::relative(entry.path(), fs::current_path());
            process_file(is_status_flag, rel_path, index_map);
        }
    }
}

void AddCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, std::time_t>> index_map;

    // Decompress and load existing index
    const std::string decompressed_data = utils::read_and_decompress(config::INDEX_FILE);  
    const std::set<std::string> ignore_list = utils::load_ignore_list();
    
    if (!decompressed_data.empty()) {
        std::istringstream iss(decompressed_data);
        std::string line;
        while (std::getline(iss, line)) {
            std::istringstream line_stream(line);
            std::string filepath, hash, size, mode;
            std::time_t mtime;
            line_stream >> filepath >> hash >> size >> mode >> mtime;
            if (fs::exists(filepath) && !utils::is_ignored(filepath, false, ignore_list)) {
                index_map[filepath] = {hash, size, mode, mtime};
            }
        }
    }

    const bool is_status_flag = (args.size() == 2 && args[0] == "-s");

    const std::string path = args.back();

    // Process files
    iterate_directory(is_status_flag, path, ignore_list, index_map);

    // Write and compress index
    std::ostringstream oss;
    for (const auto& [filepath, data] : index_map) {
        const auto& [hash, size, mode, mtime] = data;
        oss << filepath << " " << hash << " " << size << " " << mode << " " << mtime << "\n";
    }

    const std::string uncompressed_index_file = oss.str();
    const std::string compressed_index_file = utils::compress_zlib(uncompressed_index_file);
    
    std::ofstream index_file(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    index_file.write(compressed_index_file.data(), compressed_index_file.size());
    index_file.close();
}