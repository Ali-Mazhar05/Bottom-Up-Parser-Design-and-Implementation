#ifndef ITEMS_H
#define ITEMS_H

#include <vector>
#include <string>

struct Item {
    int ruleIndex;
    int dotPosition;
    // For LR(1), add lookahead
    std::string lookahead;
};

class Items {
public:
    // Methods for computing closures and transitions
};

#endif
