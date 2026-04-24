#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

struct TreeNode {
    std::string value;
    std::vector<TreeNode*> children;

    TreeNode(const std::string& val) : value(val) {}
};

class ParseTree {
public:
    TreeNode* root;
    ParseTree();
    void print();
};

#endif
