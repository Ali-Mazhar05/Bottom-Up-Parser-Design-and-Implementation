#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

using namespace std;

struct Production {
    string lhs;
    vector<string> rhs; // symbols; empty means epsilon
    int id; // unique production index after augmentation
};

class Grammar {
public:
    vector<Production> productions;
    string startSymbol;          // original start
    string augStartSymbol;       // S' (augmented)
    set<string> terminals;
    set<string> nonTerminals;

    // FIRST and FOLLOW sets
    map<string, set<string>> firstSets;
    map<string, set<string>> followSets;

    bool readFromFile(const string& filename);
    void augment();
    void computeFirst();
    void computeFollow();

    // FIRST of a sequence of symbols + extra lookahead
    set<string> firstOfSequence(const vector<string>& seq, size_t start,
                                          const string& lookahead = "") const;

    bool isTerminal(const string& sym) const;
    bool isNonTerminal(const string& sym) const;
    bool isEpsilon(const string& sym) const;

    void printAugmented(ostream& out = cout) const;

private:
    void tokenizeLine(const string& line,
                      string& lhs,
                      vector<vector<string>>& alternatives);
    void computeFirstForSymbol(const string& sym,
                               map<string, bool>& inProgress);
};
