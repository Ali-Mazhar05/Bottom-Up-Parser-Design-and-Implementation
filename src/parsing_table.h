#pragma once
#include "grammar.h"
#include "items.h"
#include <map>
#include <string>
#include <vector>
#include <set>
#include <iostream>

using namespace std;

// ---- Action entry ----
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    ActionType type = ActionType::ERROR;
    int value = -1;      // shift -> state idx; reduce -> production idx

    bool operator==(const Action& o) const {
        return type == o.type && value == o.value;
    }
    bool operator<(const Action& o) const {
        if (type != o.type) return (int)type < (int)o.type;
        return value < o.value;
    }
};

inline string actionStr(const Action& a, const Grammar& g) {
    switch (a.type) {
        case ActionType::SHIFT:  return "s" + to_string(a.value);
        case ActionType::REDUCE: {
            const Production& p = g.productions[a.value];
            string s = "r(" + p.lhs + "->";
            for (auto& sym : p.rhs) s += sym + " ";
            if (!p.rhs.empty()) s.pop_back();
            return s + ")";
        }
        case ActionType::ACCEPT: return "acc";
        default:                 return "";
    }
}

// Conflict record
struct Conflict {
    int state;
    string symbol;
    set<Action> actions;
    string type; // "shift/reduce" or "reduce/reduce"
};

// ---- Parsing Table ----
class ParsingTable {
public:
    // ACTION[state][terminal] = set of Actions (>1 means conflict)
    map<int, map<string, set<Action>>> action;
    // GOTO[state][nonTerminal] = target state
    map<int, map<string, int>> gotoT;

    vector<Conflict> conflicts;
    bool hasConflicts = false;

    void addAction(int state, const string& sym, Action a);
    void addGoto(int state, const string& nt, int target);

    // Resolve: pick shift over reduce, first reduce if reduce/reduce
    Action resolve(int state, const string& sym) const;

    void print(const Grammar& g, ostream& out = cout) const;
    void printConflicts(const Grammar& g, ostream& out = cout) const;
    vector<string> getExpectedSymbols(int state) const;
};

// ---- Builders ----
ParsingTable buildSLR1Table(const Grammar& g, const LR0Items& items);
ParsingTable buildLR1Table (const Grammar& g, const LR1Items& items);
