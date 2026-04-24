#include "stack.h"

void ParserStack::push(int state, const std::string& symbol) {
    states.push(state);
    symbols.push(symbol);
}

void ParserStack::pop(int n) {
    for (int i = 0; i < n; ++i) {
        if (!states.empty()) states.pop();
        if (!symbols.empty()) symbols.pop();
    }
}

int ParserStack::topState() {
    return states.empty() ? -1 : states.top();
}
