#pragma once
#include "grammar.h"
#include "items.h"
#include "parsing_table.h"
#include "stack.h"
#include "tree.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class LR1Parser {
public:
    explicit LR1Parser(const Grammar& g);

    void build();

    bool parse(const vector<string>& tokens,
               ostream& traceOut = cout);

    const ParseTree& getParseTree() const { return parseTree_; }

    int numStates()   const { return (int)lr1_.states().size(); }
    bool isConflict() const { return table_.hasConflicts; }

    void printItems(ostream& out = cout) const { lr1_.print(out); }
    void printTable(ostream& out = cout) const;

private:
    const Grammar& gram_;
    LR1Items lr1_;
    ParsingTable table_;
    ParseTree parseTree_;
};
