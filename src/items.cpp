#include "items.h"
#include <algorithm>
#include <iomanip>

using namespace std;

// ============================================================
//  Helpers
// ============================================================
static string rhsToString(const Production& p, int dotPos) {
    string s = p.lhs + " -> ";
    for (int i = 0; i <= (int)p.rhs.size(); ++i) {
        if (i == dotPos) s += "• ";
        if (i < (int)p.rhs.size()) s += p.rhs[i] + " ";
    }
    return s;
}

// ============================================================
//  LR(0)
// ============================================================
string LR0Items::symbolAfterDot(const LR0Item& item) const {
    const Production& p = gram.productions[item.prodId];
    if (item.dotPos >= (int)p.rhs.size()) return "";
    const string& sym = p.rhs[item.dotPos];
    if (gram.isEpsilon(sym)) return "";
    return sym;
}

bool LR0Items::dotAtEnd(const LR0Item& item) const {
    const Production& p = gram.productions[item.prodId];
    return item.dotPos >= (int)p.rhs.size() ||
           (p.rhs.size() == 1 && gram.isEpsilon(p.rhs[0]));
}

ItemSet0 LR0Items::closure(const ItemSet0& items) const {
    ItemSet0 result = items;
    bool changed = true;
    while (changed) {
        changed = false;
        ItemSet0 toAdd;
        for (auto& item : result) {
            string sym = symbolAfterDot(item);
            if (sym.empty() || !gram.isNonTerminal(sym)) continue;
            for (int i = 0; i < (int)gram.productions.size(); ++i) {
                if (gram.productions[i].lhs != sym) continue;
                LR0Item ni{i, 0};
                if (!result.count(ni)) toAdd.insert(ni);
            }
        }
        for (auto& ni : toAdd)
            if (result.insert(ni).second) changed = true;
    }
    return result;
}

ItemSet0 LR0Items::gotoOp(const ItemSet0& items, const string& sym) const {
    ItemSet0 kernel;
    for (auto& item : items) {
        if (symbolAfterDot(item) == sym)
            kernel.insert({item.prodId, item.dotPos + 1});
    }
    if (kernel.empty()) return {};
    return closure(kernel);
}

void LR0Items::build() {
    itemSets.clear();
    gotoMap.clear();

    // Find augmented start production index
    int startProd = -1;
    for (int i = 0; i < (int)gram.productions.size(); ++i)
        if (gram.productions[i].lhs == gram.augStartSymbol) { startProd = i; break; }

    ItemSet0 i0 = closure({{startProd, 0}});
    itemSets.push_back(i0);

    // Collect all grammar symbols
    set<string> allSymbols;
    for (auto& t  : gram.terminals)    allSymbols.insert(t);
    for (auto& nt : gram.nonTerminals) allSymbols.insert(nt);
    allSymbols.erase("$"); // handle separately in table

    size_t idx = 0;
    while (idx < itemSets.size()) {
        for (auto& sym : allSymbols) {
            ItemSet0 g = gotoOp(itemSets[idx], sym);
            if (g.empty()) continue;
            // Check if already present
            int found = -1;
            for (int j = 0; j < (int)itemSets.size(); ++j)
                if (itemSets[j] == g) { found = j; break; }
            if (found == -1) {
                found = (int)itemSets.size();
                itemSets.push_back(g);
            }
            gotoMap[{(int)idx, sym}] = found;
        }
        ++idx;
    }
}

void LR0Items::print(ostream& out) const {
    out << "\n========== LR(0) Canonical Collection ==========\n";
    out << "Total states: " << itemSets.size() << "\n\n";
    for (int i = 0; i < (int)itemSets.size(); ++i) {
        out << "I" << i << ":\n";
        for (auto& item : itemSets[i]) {
            const Production& p = gram.productions[item.prodId];
            out << "  " << rhsToString(p, item.dotPos) << "\n";
        }
        out << "\n";
    }
}

// ============================================================
//  LR(1)
// ============================================================
string LR1Items::symbolAfterDot(const LR1Item& item) const {
    const Production& p = gram.productions[item.prodId];
    if (item.dotPos >= (int)p.rhs.size()) return "";
    const string& sym = p.rhs[item.dotPos];
    if (gram.isEpsilon(sym)) return "";
    return sym;
}

bool LR1Items::dotAtEnd(const LR1Item& item) const {
    const Production& p = gram.productions[item.prodId];
    return item.dotPos >= (int)p.rhs.size() ||
           (p.rhs.size() == 1 && gram.isEpsilon(p.rhs[0]));
}

ItemSet1 LR1Items::closure(const ItemSet1& items) const {
    ItemSet1 result = items;
    bool changed = true;
    while (changed) {
        changed = false;
        ItemSet1 toAdd;
        for (auto& item : result) {
            string B = symbolAfterDot(item);
            if (B.empty() || !gram.isNonTerminal(B)) continue;

            const Production& p = gram.productions[item.prodId];
            // beta = symbols after B in this item's RHS
            vector<string> beta(p.rhs.begin() + item.dotPos + 1, p.rhs.end());

            // FIRST(beta a)
            auto lookaheads = gram.firstOfSequence(beta, 0, item.lookahead);
            lookaheads.erase("epsilon");

            for (int i = 0; i < (int)gram.productions.size(); ++i) {
                if (gram.productions[i].lhs != B) continue;
                for (auto& la : lookaheads) {
                    LR1Item ni{i, 0, la};
                    if (!result.count(ni)) toAdd.insert(ni);
                }
            }
        }
        for (auto& ni : toAdd)
            if (result.insert(ni).second) changed = true;
    }
    return result;
}

ItemSet1 LR1Items::gotoOp(const ItemSet1& items, const string& sym) const {
    ItemSet1 kernel;
    for (auto& item : items) {
        if (symbolAfterDot(item) == sym)
            kernel.insert({item.prodId, item.dotPos + 1, item.lookahead});
    }
    if (kernel.empty()) return {};
    return closure(kernel);
}

void LR1Items::build() {
    itemSets.clear();
    gotoMap.clear();

    int startProd = -1;
    for (int i = 0; i < (int)gram.productions.size(); ++i)
        if (gram.productions[i].lhs == gram.augStartSymbol) { startProd = i; break; }

    ItemSet1 i0 = closure({{startProd, 0, "$"}});
    itemSets.push_back(i0);

    set<string> allSymbols;
    for (auto& t  : gram.terminals)    allSymbols.insert(t);
    for (auto& nt : gram.nonTerminals) allSymbols.insert(nt);

    size_t idx = 0;
    while (idx < itemSets.size()) {
        for (auto& sym : allSymbols) {
            ItemSet1 g = gotoOp(itemSets[idx], sym);
            if (g.empty()) continue;
            int found = -1;
            for (int j = 0; j < (int)itemSets.size(); ++j)
                if (itemSets[j] == g) { found = j; break; }
            if (found == -1) {
                found = (int)itemSets.size();
                itemSets.push_back(g);
            }
            gotoMap[{(int)idx, sym}] = found;
        }
        ++idx;
    }
}

void LR1Items::print(ostream& out) const {
    out << "\n========== LR(1) Canonical Collection ==========\n";
    out << "Total states: " << itemSets.size() << "\n\n";
    for (int i = 0; i < (int)itemSets.size(); ++i) {
        out << "I" << i << ":\n";
        for (auto& item : itemSets[i]) {
            const Production& p = gram.productions[item.prodId];
            out << "  [" << rhsToString(p, item.dotPos) << ", " << item.lookahead << "]\n";
        }
        out << "\n";
    }
}
