#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

using namespace std;

// Each element on the parser stack holds a state number and
// the grammar symbol that caused the push.
struct StackEntry {
    int state;
    string symbol; // empty for the initial bottom entry
};

class ParserStack {
public:
    ParserStack() { stack_.push_back({0, ""}); }

    void push(const string& sym, int state) {
        stack_.push_back({state, sym});
    }

    // Pop n *symbol/state pairs* (each reduce pops |beta| pairs)
    void pop(int n) {
        if (n > (int)stack_.size() - 1)
            throw runtime_error("Stack underflow during pop");
        stack_.resize(stack_.size() - n);
    }

    int topState() const { return stack_.back().state; }

    void clear() { stack_.clear(); stack_.push_back({0, ""}); }

    // Pretty-print: "0 id 5 + 6 ..."
    string toString() const {
        string s;
        for (auto& e : stack_) {
            if (!e.symbol.empty()) s += e.symbol + " ";
            s += to_string(e.state) + " ";
        }
        if (!s.empty() && s.back() == ' ') s.pop_back();
        return s;
    }

    bool empty() const { return stack_.size() <= 1; }
    size_t size() const { return stack_.size(); }

private:
    vector<StackEntry> stack_;
};
