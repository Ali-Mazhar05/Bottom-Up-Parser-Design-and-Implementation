#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

struct TreeNode {
    string label;
    vector<shared_ptr<TreeNode>> children;

    explicit TreeNode(const string& lbl) : label(lbl) {}

    void addChild(shared_ptr<TreeNode> child) {
        children.push_back(child);
    }
};

class ParseTree {
public:
    shared_ptr<TreeNode> root;

    // Print tree with indentation
    void print(ostream& out = cout) const {
        if (!root) { out << "(empty tree)\n"; return; }
        out << "\n========== Parse Tree ==========\n";
        printNode(out, root, "", true);
        out << "\n";
    }

private:
    void printNode(ostream& out,
                   const shared_ptr<TreeNode>& node,
                   const string& prefix,
                   bool isLast) const {
        out << prefix << (isLast ? "└── " : "├── ") << node->label << "\n";
        const string childPrefix = prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < node->children.size(); ++i)
            printNode(out, node->children[i], childPrefix, i + 1 == node->children.size());
    }
};
