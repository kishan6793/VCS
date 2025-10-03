#include "models/tree.hpp"

void Tree::delete_nodes(Node* node) {
    if (node == nullptr) return;
    for (auto& child : node->children) {
        delete_nodes(child.second);
    }
    delete node;
}

void Tree::insert(const IndexEntry& index_entry) {
    const std::string& path = index_entry.filepath;
    std::stringstream ss(path);
    std::string fs_name;
    Node* current = root;

    while (std::getline(ss, fs_name, '/')) {
        if (fs_name.empty()) { continue; }

        if (current->children.find(fs_name) == current->children.end()) {
            current->children[fs_name] = new Node(fs_name);
        }
        current = current->children[fs_name];
    }
    // Set the index entry for the last node
    current->index_entry = index_entry;
}