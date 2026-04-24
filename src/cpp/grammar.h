#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <string>
#include <vector>
#include <map>

class Grammar {
public:
    Grammar();
    void loadFromFile(const std::string& filename);
    void augment();
    // Add grammar related methods
};

#endif
