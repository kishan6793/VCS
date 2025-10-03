#ifndef TREE_HPP
#define TREE_HPP

#include "models/index.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

class Node {
public:
    std::string fs_name;
    std::map<std::string, Node*> children; // {children_name, address}, {children_name, address}
    IndexEntry index_entry;

    Node(const std::string name) : fs_name(name){}
};

class Tree {
private:
    void delete_nodes(Node* node);

public:
    Node* root;

    Tree() : root(new Node(".")) {}

    ~Tree() { delete_nodes(root); }

    void insert(const IndexEntry& index_entry);  
};

#endif // TREE_HPP
