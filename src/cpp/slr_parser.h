#ifndef SLR_PARSER_H
#define SLR_PARSER_H

#include "grammar.h"
#include "parsing_table.h"
#include "stack.h"
#include "tree.h"

class SLRParser {
public:
    SLRParser(Grammar& g);
    void constructTable();
    bool parse(const std::vector<std::string>& input);
};

#endif
