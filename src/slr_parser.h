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

class SLRParser {
public:
    explicit SLRParser(const Grammar& g);

    // Build LR(0) items and SLR(1) parsing table
    void build();

    // Parse a token sequence; tokens should NOT include "$"
    // Returns true if accepted
    bool parse(const vector<string>& tokens,
               ostream& traceOut = cout);

    // Access parse tree of last accepted input
    const ParseTree& getParseTree() const { return parseTree_; }

    // Getters for stats/comparison
    int numStates()  const { return (int)lr0_.states().size(); }
    bool isConflict() const { return table_.hasConflicts; }

    void printItems(ostream& out = cout) const { lr0_.print(out); }
    void printTable(ostream& out = cout) const;

private:
    const Grammar& gram_;
    LR0Items lr0_;
    ParsingTable table_;
    ParseTree parseTree_;
};
