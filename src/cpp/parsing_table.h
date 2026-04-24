#ifndef PARSING_TABLE_H
#define PARSING_TABLE_H

#include <map>
#include <string>

enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    ActionType type;
    int value; // state for shift, rule index for reduce
};

class ParsingTable {
public:
    void addAction(int state, const std::string& symbol, Action action);
    Action getAction(int state, const std::string& symbol);
};

#endif
