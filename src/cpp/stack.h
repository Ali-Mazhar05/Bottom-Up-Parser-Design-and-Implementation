#ifndef STACK_H
#define STACK_H

#include <stack>
#include <string>

class ParserStack {
private:
    std::stack<int> states;
    std::stack<std::string> symbols;
public:
    void push(int state, const std::string& symbol);
    void pop(int n);
    int topState();
};

#endif
