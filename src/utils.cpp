#include "utils.hpp"

namespace utils {

    void clear_screen() {
        #ifdef _WIN32
        system("cls");
        #else
        std::cout << "\033[2J\033[H";
        #endif
    }

    bool is_directory_exist(const std::string& path) {
        return fs::exists(path) && fs::is_directory(path);
    }
    
    bool is_file_exist(const std::string& path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }

    DIR_STATUS create_directory(const std::string& path) {
        if (is_directory_exist(path)) {
            return DIR_STATUS::ALREADY_EXIST;
        }

        if (!fs::create_directories(path)) {
            return DIR_STATUS::ERROR;
        }

        return DIR_STATUS::CREATED;
    }

    FILE_STATUS create_file(const std::string& path, const std::string& content) {
        if (is_file_exist(path)) {
            return FILE_STATUS::ALREADY_EXIST;
        }

        std::ofstream file(path);
        if (!file) {
            return FILE_STATUS::ERROR;
        }

        if (!content.empty()) {
            file << content;
        }
        
        file.close();
        return FILE_STATUS::CREATED;
    }

    std::string get_current_path() {
        return fs::current_path().string();
    }  
    
    void create_vcs_structure() {

        const std::vector<std::string> dirs = {
            config::VCS_DIR,
            config::OBJECTS_DIR,
            config::REFS_DIR,
            config::REFS_HEAD_DIR,
            config::LOG_DIR,
            config::LOG_REFS_DIR,
            config::LOG_REFS_HEAD_DIR
        };

        for(const std::string& dir : dirs) {
            if (utils::create_directory(dir) == utils::DIR_STATUS::ERROR) {
                const std::string error_msg = "Failed to create directory: " + dir;
                throw std::runtime_error(error_msg);
            }
        }

        // Create '.vcs/refs/stash' file
        if(create_file(config::STASH, std::string(40, '0')) == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create 'stash' file";
            throw std::runtime_error(error_msg);
        }

        // Create '.vcs/logs/refs/stash' file
        if(create_file(config::LOG_STASH, "") == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create 'stash' log file";
            throw std::runtime_error(error_msg);
        }

        // Create '.vcsignore' file
        if(create_file(config::VCS_IGNORE, "main.out") == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create '.vcsignore' file";
            throw std::runtime_error(error_msg);
        }

        // Create 'master' branch file
        if(create_file(config::REFS_HEAD_DIR + "master", std::string(40, '0')) == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create 'master' branch file.";
            throw std::runtime_error(error_msg);
        }

        // Create 'master' branch log file
        if(create_file(config::LOG_REFS_HEAD_DIR + "master", "") == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create 'master' branch log file.";
            throw std::runtime_error(error_msg);
        }

        // Create HEAD file
        const std::string head_content = "ref: refs/heads/master";
        if(create_file(config::HEAD_FILE, head_content) == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create HEAD file.";
            throw std::runtime_error(error_msg);
        }

        // Create index file
        if(create_file(config::INDEX_FILE, "") == utils::FILE_STATUS::ERROR) {
            const std::string error_msg = "Failed to create index file.";
            throw std::runtime_error(error_msg);
        }
    }

    std::string get_object_path(const std::string& obj_hash) {
        if(!is_valid_hash_syntax(obj_hash)) {
            const std::string error_msg = "Invalid object hash: " + obj_hash;
            throw std::invalid_argument(error_msg);
        }

        return config::OBJECTS_DIR + obj_hash.substr(0, 2) + "/" + obj_hash.substr(2);
    }

    bool is_exist_obj(const std::string& obj_hash) {
        const std::string obj_path = get_object_path(obj_hash);
        return is_file_exist(obj_path);
    }

    std::string read_and_decompress(const std::string& obj_path) {
        std::ifstream file(obj_path, std::ios::binary);

        if (!file) {
            std::string error_msg = "Failed to open object file: " + obj_path;
            throw std::runtime_error(error_msg);
        }

        const std::string data(std::istreambuf_iterator<char>(file), {});
        return decompress_zlib(data);
    }

    bool is_valid_hash_syntax(const std::string& hash) {
        const int hash_size = hash.size();

        if(hash_size != 40) {
            return false;
        }

        for(int i = 0; i < hash_size; ++i) {
            if(!std::isxdigit(hash[i])) {
                return false;
            }
        }

        return true;
    }
    
    std::string sha1(const std::string& input) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

        std::stringstream ss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    std::string read_file_content(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        
        if (!file) {
            std::string error_msg = "Failed to open file: " + filePath;
            throw std::runtime_error(error_msg);
        }

        // Seek to end to get size, then read into string
        file.seekg(0, std::ios::end);
        std::size_t size = file.tellg();
        std::string content(size, '\0');
        file.seekg(0);
        file.read(&content[0], size);

        return content;
    }

    std::string compress_zlib(const std::string& input) {
        z_stream zs{};
        deflateInit(&zs, Z_BEST_COMPRESSION);

        zs.next_in = (Bytef*)input.data();
        zs.avail_in = input.size();

        int ret;
        char outbuffer[32768];
        std::string outstring;

        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);
            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            throw std::runtime_error("Failed to compress data with zlib");
        }

        return outstring;
    }

    std::string decompress_zlib(const std::string& compressed_data) {
        if(compressed_data.empty()) return compressed_data;
        
        z_stream zs{};
        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data.data()));
        zs.avail_in = compressed_data.size();

        if (inflateInit(&zs) != Z_OK) {
            throw std::runtime_error("inflateInit failed while decompressing.");
        }

        std::string decompressed;
        char outbuffer[32768];

        int ret;
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);

            if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&zs);
                throw std::runtime_error("inflate failed while decompressing.");
            }

            decompressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);

        } while (ret != Z_STREAM_END);

        inflateEnd(&zs);
        return decompressed;
    }

    std::time_t get_current_timestamp() {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }

    std::string get_file_mode(const std::string& path) {
        auto perms = fs::status(path).permissions();
        std::string mode;
        if (fs::is_symlink(path)) {
            mode = "120000"; // symlink
        } else if ((perms & fs::perms::owner_exec) != fs::perms::none) {
            mode = "100755"; // executable
        } else {
            mode = "100644"; // regular file 
        }

        return mode;
    }

    std::time_t get_mtime(const std::string& path) {
        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        return std::chrono::system_clock::to_time_t(sctp);
    }

    std::string extract_ref_path(const std::string& line) {
        std::string delimiter = ": ";
        size_t pos = line.find(delimiter);

        if (pos == std::string::npos) return ""; 

        std::string path = line.substr(pos + delimiter.length());
        return path;
    }

    std::string extract_ref_branch(const std::string& line) {
        const int size = line.size();
        std::string branch;
        
        int i = size - 1;
        while (i >= 0 && line[i] != '/')
        {
            branch += line[i];
            i--;
        }

        if(i == -1) return "";

        std::reverse(branch.begin(), branch.end());
        return branch;
    }

    std::string get_username() {
        const char* username = std::getenv("USER"); // POSIX systems
        if (!username) {
            username = std::getenv("USERNAME"); // Windows fallback
        }

        if (username) {
            return std::string(username);
        } else {
            throw std::runtime_error("Unable to determine system username.");
        }
    }

    // Returns Unix timestamp as string
    std::string get_unix_timestamp() {
        std::time_t now = std::time(NULL);
        return std::to_string(now);
    }

    std::uintmax_t get_file_size(const std::string& file_path) { 
        return fs::file_size(file_path);
    }

    bool path_exists(const std::string& relative_path) {
        std::filesystem::path full_path = std::filesystem::current_path() / relative_path;
        return std::filesystem::exists(full_path);
    }

    std::string normalizeRelativePath(const std::string& path) 
    {
        if (path.empty() || path[0] == '/') {
            // Absolute or empty path not allowed
            return "";
        }

        std::istringstream ss(path);
        std::string token;
        std::vector<std::string> stack;

        while (std::getline(ss, token, '/')) {
            if (token.empty() || token == ".") { // Ignore empty or current dir
                continue;
            } else if (token == "..") {
                if (stack.empty()) { // Attempt to escape root
                    return "";
                }
                stack.pop_back(); // Go up
            } else { // Valid directory or file
                stack.push_back(token);
            }
        }

        if (stack.empty()) {
            return ".";
        }

        // Reconstruct path
        std::string result;
        for (size_t i = 0; i < stack.size(); ++i) {
            if (i > 0) result += '/';
            result += stack[i];
        }

        return result;
    }

    bool is_valid_branch_name(const std::string& str) {
        int n = str.size();
        for(int i = 0; i < n; i++) {
            if((str[i] >= 'a' && str[i] <= 'z') || ((i > 0) && (i + 1 < n) && str[i] == '-'));
            else return false;
        }
        return true;
    }

    std::string get_current_branch() {
        const std::string head_file_content = utils::read_file_content(config::HEAD_FILE);
        return utils::extract_ref_branch(head_file_content);
    }

    std::string get_commit_hash(const std::string& branch_name) {
        const std::string branch_path = config::REFS_HEAD_DIR + branch_name;
        return utils::read_file_content(branch_path);
    }

    std::set<std::string> load_ignore_list() {
        std::set<std::string> ignore_list;
        ignore_list.insert(".vcs/");

        std::ifstream in(config::VCS_IGNORE);

        if (!in.is_open()) {
            return ignore_list;
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            std::istringstream ss(line);
            std::string path;
            while (ss >> path) {
                if (!path.empty()) {
                    ignore_list.insert(path);
                }
            }
        }
        return ignore_list;
    }

    bool is_path_ignored(const fs::path& path, bool is_directory, const std::set<std::string>& ignore_list) {
        std::string rel_path = fs::relative(path, fs::current_path()).string();

        if (is_directory) { rel_path += '/'; }

        for (const std::string& ignored_path : ignore_list) {
            if(rel_path.find(ignored_path) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    bool is_ignored(const fs::path& path, bool is_directory, const std::set<std::string>& ignore_list) {
        std::string rel_path = fs::relative(path, fs::current_path()).string();

        if (is_directory) { rel_path += '/'; }

        return (ignore_list.find(rel_path) != ignore_list.end()) ? true : false;
    }

    std::string get_tree_hash_from_commit(const std::string& commit_hash) {
        if(commit_hash == std::string(40, '0')) { return ""; }

        const std::string commit_file = utils::get_object_path(commit_hash);
        const std::string commit_content = utils::read_and_decompress(commit_file);
        
        // commit_content example:
        // tree <tree_hash>
        // parent <parent_hash>
        // author ...
        // committer ...
        // message ...
        // We need to extract the tree hash from the first line.

        size_t tree_pos = commit_content.find("tree ");
        if (tree_pos == std::string::npos) { return ""; }

        size_t hash_start = tree_pos + 5;
        size_t hash_end = commit_content.find('\n', hash_start);
        if (hash_end == std::string::npos) { return ""; }

        return commit_content.substr(hash_start, hash_end - hash_start);
    }

    std::vector<std::string> get_all_branches(const std::string& path) {
        std::vector<std::string> branches;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            const std::string& file = entry.path().filename().string();
            branches.push_back(file);
        }

        return branches;
    }

    std::string get_head_commit_hash() {
        const std::string head_file_content = utils::read_file_content(config::HEAD_FILE);
        if (head_file_content.find("ref:") == std::string::npos) {
            return head_file_content; // Detached HEAD state
        }

        size_t pos = head_file_content.find("ref: ");
        if (pos == std::string::npos) {
            const std::string error_msg = "Invalid HEAD file format: " + head_file_content;
            throw std::logic_error(error_msg);
        }

        return utils::get_commit_hash(utils::get_current_branch());
    }

    std::string get_red_text(const std::string& text) {
        return "\033[31m" + text + "\033[0m"; // Red text
    }

    std::string get_green_text(const std::string& text) {
        return "\033[32m" + text + "\033[0m"; // Green text
    }

    std::string get_lightblue_text(const std::string& text) {
        return "\033[94m" + text + "\033[0m"; // Light blue (bright blue) text
    }

    std::string get_blue_text(const std::string& text) {
        return "\033[34m" + text + "\033[0m"; // Blue text
    }

    std::string get_light_green_text(const std::string& text) {
        return "\033[92m" + text + "\033[0m"; // Light green text
    }

    bool is_head_detached() {
        const std::string head_content = read_file_content(config::HEAD_FILE); 
        return (head_content.find("ref: ") == std::string::npos);
    }

    void create_file_from_blob(const std::string& filepath, const std::string& hash, const std::string& mode) {
        fs::path path(filepath);

        // Ensure all parent directories exist
        if (path.has_parent_path()) {
            fs::create_directories(path.parent_path());
        }

        // Read blob content using hash
        std::string blob_raw = utils::read_and_decompress(utils::get_object_path(hash));
        // blob_raw is "blob <size>\0<content>"
        size_t null_pos = blob_raw.find('\0');
        if (null_pos == std::string::npos) {
            const std::string error_msg = "Corrupted blob object: missing null separator";
            throw std::runtime_error(error_msg);
        }
        
        std::string blob_content = blob_raw.substr(null_pos + 1);

        try {
            if (mode == "120000") {
                // Symlink: content is target path
                fs::create_symlink(blob_content, path);
            } 
            else {
                // Write file
                std::ofstream out(path, std::ios::binary);
                out << blob_content;
                out.close();

                // Set file permissions
                if (mode == "100755") {
                    fs::permissions(path,
                        fs::perms::owner_all |
                        fs::perms::group_read |
                        fs::perms::others_read,
                        fs::perm_options::replace);
                } 
                else if (mode == "100644") {
                    fs::permissions(path,
                        fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read |
                        fs::perms::others_read,
                        fs::perm_options::replace);
                }
            }
        } catch (const std::exception& e) {
            const std::string error_msg = "Error creating file " + filepath + ": " + e.what();
            throw std::runtime_error(error_msg);
        }
    }

    void get_lines_from_blob(const std::string& hash, std::vector<std::string>& lines) {
        std::string full = utils::read_and_decompress(utils::get_object_path(hash));

        // Find the first null character that separates the header and content
        size_t pos = full.find('\0');
        if (pos == std::string::npos) return; // malformed blob

        // Extract only the file content (after the null)
        std::string content = full.substr(pos + 1);

        // Split into lines
        std::istringstream content_stream(content);
        std::string line;
        while (std::getline(content_stream, line)) {
            lines.push_back(line);
        }
    }

    bool is_commit_exists_on_branch(const std::string& branch, const std::string& cur_commit_hash) {
        const std::string branch_path = config::LOG_REFS_HEAD_DIR + branch;

        std::ifstream file(branch_path);

        if (!file) {
            const std::string error_msg = "Could not open file: " + branch_path;
            throw std::runtime_error(error_msg);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue; 

            std::istringstream iss(line);
            std::string parent_hash, commit_hash;
            iss >> parent_hash >> commit_hash;

            if (!commit_hash.empty() && cur_commit_hash == commit_hash) return true;
        }

        return false;
    }

    void warning_checkout() {
        utils::write(utils::EMPTY);
        utils::write(utils::WARN, "Switching will clear the current staging area.");
        utils::write(utils::WARN, "Any uncommitted changes will be lost, reverting the working directory back to the state of the last commit.");
        utils::write(utils::WARN, "If you want to save your changes, press 'N' to cancel the switch.");
        utils::write(utils::WARN, "Then, use the commit command to save your current work.");
        utils::write(utils::WARN, "Once committed, you can safely switch.");
        
        std::string response;
        std::cout << utils::WARN << " Are you sure you want to proceed? (Y/N) : "; 
        std::cin >> response;
        
        utils::write(utils::EMPTY);

        if (response != "Y" && response != "y") {
            utils::write(utils::INFO, "Branch switch cancelled. Your current changes are safe.");
            exit(0);
        }

        utils::write(utils::INFO, "Proceeding with branch switch...");
        utils::write(utils::EMPTY);
    }

    void solve(const std::string& tree_hash, std::stringstream& buffer, std::string path) {
        const std::string tree_content = utils::read_and_decompress(utils::get_object_path(tree_hash));

        // ./main.out ls-tree 6725736609ced67ff91c01194131fb5c1e96b795
        // [ OK        ]  
        // [ CONTENT   ]  tree 303 
        // [ CONTENT   ]  100644 blob ee7b91e970e57c6b795e4f25e99f2e9e16d8f3fe 1750696059 31 demo.txt 
        // [ CONTENT   ]  100755 blob c3b26da2c89459d767e20f9d0ca95d14b5e4b4d2 1750759111 2495376 main.out 
        // [ CONTENT   ]  040000 tree 8e4df253a7968ffe2641eae68d8cfce7e0048258 1750696466 65 test1 
        // [ CONTENT   ]  040000 tree 90e2141d7522fc9c83b99c3e50d3f507165ac71d 1750696477 65 test2 

        // index-file
        // <file-path> <sha1-hash> <size> <mode> <mtime>

        std::stringstream ss(tree_content);
        std::string line;
        std::getline(ss, line); // Read the first line (tree 286)

        while (std::getline(ss, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string mode, type, hash, mtime, size, file_name;
            iss >> mode >> type >> hash >> mtime >> size >> file_name;

            if (type == "tree") {
                solve(hash, buffer, file_name + "/"); // Recursive call for sub-trees
            } else if (type == "blob") {
                const std::string filepath = path + file_name;
                buffer << filepath << " " << hash << " " << size << " " << mode << " " << mtime << "\n"; // add to index file
            }
        }
    }

    void clean_working_directory() {
        const fs::path cwd = fs::current_path();
        const std::set<std::string> ignore_list = utils::load_ignore_list();

        for (const auto& entry : fs::directory_iterator(cwd)) {
            const std::string name = entry.path().filename().string();

            // Skip .vcs directory and main.out file
            // This will cause the error for .vcsignore
            if (name == ".vcs" || name == "main.out") continue;

            if(is_path_ignored(entry.path(), entry.is_directory(), ignore_list)) continue;

            try {
                fs::remove_all(entry.path());  // delete file or directory
            } catch (const fs::filesystem_error& e) {
                const std::string error_msg = "Failed to delete " + entry.path().string() + ": " + e.what();
                throw std::runtime_error(error_msg);
            }
        }
    }

    void make_checkout() {
        // Decompress and load existing index
        const std::string decompressed_data = utils::read_and_decompress(config::INDEX_FILE);  
        if (decompressed_data.empty()) return;

        // Tree tree;

        std::istringstream iss(decompressed_data);
        std::string line;
        while (std::getline(iss, line)) {
            std::istringstream line_stream(line);
            std::string filepath, hash, size, mode, mtime;
            line_stream >> filepath >> hash >> size >> mode >> mtime;
            
            create_file_from_blob(filepath, hash, mode); // Create files based on the index
        }
    }

    void get_lines_from_file(const std::string& path, std::vector<std::string>& lines) {
        std::ifstream file(path);
        std::string line;

        while (std::getline(file, line)) {
            lines.push_back(line);
        }
    }

    std::string parse_timestamp(const std::string& timestamp_str) {
        // Convert timestamp string to time_t
        std::time_t timestamp = 0;
        try {
            timestamp = static_cast<std::time_t>(std::stoll(timestamp_str));
        } catch (const std::exception& e) {
            const std::string error_msg = "Invalid timestamp: " + timestamp_str;
            throw VCSException(error_msg);
        }

        // Format timestamp to human-readable string
        std::tm* tm_info = std::localtime(&timestamp);
        char date_str[100];
        std::strftime(date_str, sizeof(date_str), "%a %b %d %H:%M:%S %Y", tm_info);

        return date_str;
    }
}