#ifndef LR1_PARSER_H
#define LR1_PARSER_H

#include "grammar.h"
#include "parsing_table.h"
#include "stack.h"
#include "tree.h"

class LR1Parser {
public:
    LR1Parser(Grammar& g);
    void constructTable();
    bool parse(const std::vector<std::string>& input);
};

#endif
