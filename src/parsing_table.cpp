#include "parsing_table.h"
#include <iomanip>
#include <algorithm>

using namespace std;

// ============================================================
//  ParsingTable methods
// ============================================================
void ParsingTable::addAction(int state, const string& sym, Action a) {
    auto& cell = action[state][sym];
    if (!cell.empty() && !cell.count(a)) {
        // Conflict!
        cell.insert(a);
        hasConflicts = true;
        // Build conflict record
        Conflict c;
        c.state  = state;
        c.symbol = sym;
        c.actions = cell;
        bool hasShift  = false, hasReduce = false;
        for (auto& x : cell) {
            if (x.type == ActionType::SHIFT)  hasShift  = true;
            if (x.type == ActionType::REDUCE) hasReduce = true;
        }
        c.type = (hasShift && hasReduce) ? "shift/reduce" : "reduce/reduce";
        // Replace existing conflict record for same (state,sym)
        for (auto& ex : conflicts)
            if (ex.state == state && ex.symbol == sym) { ex = c; return; }
        conflicts.push_back(c);
    } else {
        cell.insert(a);
    }
}

void ParsingTable::addGoto(int state, const string& nt, int target) {
    gotoT[state][nt] = target;
}

Action ParsingTable::resolve(int state, const string& sym) const {
    auto sit = action.find(state);
    if (sit == action.end()) return {ActionType::ERROR, -1};
    auto cit = sit->second.find(sym);
    if (cit == sit->second.end()) return {ActionType::ERROR, -1};
    const auto& cell = cit->second;
    if (cell.empty()) return {ActionType::ERROR, -1};
    // Prefer shift (shift/reduce resolution), else first reduce
    for (auto& a : cell) if (a.type == ActionType::SHIFT) return a;
    for (auto& a : cell) if (a.type == ActionType::ACCEPT) return a;
    return *cell.begin();
}

void ParsingTable::print(const Grammar& g, ostream& out) const {
    // Collect ordered terminals and non-terminals
    vector<string> terms, nts;
    for (auto& t : g.terminals) if (t != "$") terms.push_back(t);
    sort(terms.begin(), terms.end());
    terms.push_back("$");

    for (auto& nt : g.nonTerminals) if (nt != g.augStartSymbol) nts.push_back(nt);
    sort(nts.begin(), nts.end());

    // Determine column width
    const int colW = 14;
    const int stateW = 7;

    // Header
    out << "\n" << string(stateW + (int)(terms.size() + nts.size()) * colW + 4, '=') << "\n";
    out << setw(stateW) << "State";
    out << " | ";
    for (auto& t : terms) out << setw(colW) << t;
    out << " | ";
    for (auto& nt : nts)  out << setw(colW) << nt;
    out << "\n" << string(stateW + (int)(terms.size() + nts.size()) * colW + 4, '-') << "\n";

    // Rows
    int maxState = 0;
    for (auto const& pair : action) maxState = max(maxState, pair.first);
    for (auto const& pair : gotoT)  maxState = max(maxState, pair.first);

    for (int s = 0; s <= maxState; ++s) {
        out << setw(stateW) << ("I" + to_string(s));
        out << " | ";
        for (auto& t : terms) {
            Action a = resolve(s, t);
            out << setw(colW) << (a.type == ActionType::ERROR ? "" : actionStr(a, g));
        }
        out << " | ";
        for (auto& nt : nts) {
            auto sit = gotoT.find(s);
            if (sit != gotoT.end()) {
                auto nit = sit->second.find(nt);
                if (nit != sit->second.end()) { out << setw(colW) << nit->second; continue; }
            }
            out << setw(colW) << "";
        }
        out << "\n";
    }
    out << string(stateW + (int)(terms.size() + nts.size()) * colW + 4, '=') << "\n";
}

void ParsingTable::printConflicts(const Grammar& g, ostream& out) const {
    if (conflicts.empty()) {
        out << "No conflicts found. Grammar is valid.\n";
        return;
    }
    out << "\n===== Conflicts Detected =====\n";
    for (auto& c : conflicts) {
        out << c.type << " conflict at state I" << c.state
            << " on symbol '" << c.symbol << "':\n";
        for (auto& a : c.actions)
            out << "  -> " << actionStr(a, g) << "\n";
    }
}

vector<string> ParsingTable::getExpectedSymbols(int state) const {
    vector<string> expected;
    auto it = action.find(state);
    if (it != action.end()) {
        for (auto const& pair : it->second) {
            if (!pair.second.empty()) expected.push_back(pair.first);
        }
    }
    return expected;
}

// ============================================================
//  SLR(1) table builder
// ============================================================
ParsingTable buildSLR1Table(const Grammar& g, const LR0Items& lr0) {
    ParsingTable table;
    const auto& states  = lr0.states();
    const auto& gotoMap = lr0.gotoTable();

    for (int i = 0; i < (int)states.size(); ++i) {
        for (auto& item : states[i]) {
            const Production& p = g.productions[item.prodId];
            bool atEnd = (item.dotPos >= (int)p.rhs.size()) ||
                         (p.rhs.size() == 1 && g.isEpsilon(p.rhs[0]));

            if (!atEnd) {
                // Symbol after dot
                const string& sym = p.rhs[item.dotPos];
                if (g.isEpsilon(sym)) continue;

                auto it = gotoMap.find({i, sym});
                if (it == gotoMap.end()) continue;
                int j = it->second;

                if (g.isTerminal(sym)) {
                    table.addAction(i, sym, {ActionType::SHIFT, j});
                } else {
                    table.addGoto(i, sym, j);
                }
            } else {
                // Reduce
                if (p.lhs == g.augStartSymbol) {
                    // S' -> S •  => accept
                    table.addAction(i, "$", {ActionType::ACCEPT, -1});
                } else {
                    // Reduce on FOLLOW(lhs)
                    auto fit = g.followSets.find(p.lhs);
                    if (fit == g.followSets.end()) continue;
                    for (auto& la : fit->second)
                        table.addAction(i, la, {ActionType::REDUCE, item.prodId});
                }
            }
        }
        // GOTO entries for non-terminals
        for (auto& nt : g.nonTerminals) {
            auto it = gotoMap.find({i, nt});
            if (it != gotoMap.end())
                table.addGoto(i, nt, it->second);
        }
    }
    return table;
}

// ============================================================
//  LR(1) table builder
// ============================================================
ParsingTable buildLR1Table(const Grammar& g, const LR1Items& lr1) {
    ParsingTable table;
    const auto& states  = lr1.states();
    const auto& gotoMap = lr1.gotoTable();

    for (int i = 0; i < (int)states.size(); ++i) {
        for (auto& item : states[i]) {
            const Production& p = g.productions[item.prodId];
            bool atEnd = (item.dotPos >= (int)p.rhs.size()) ||
                         (p.rhs.size() == 1 && g.isEpsilon(p.rhs[0]));

            if (!atEnd) {
                const string& sym = p.rhs[item.dotPos];
                if (g.isEpsilon(sym)) continue;

                auto it = gotoMap.find({i, sym});
                if (it == gotoMap.end()) continue;
                int j = it->second;

                if (g.isTerminal(sym)) {
                    table.addAction(i, sym, {ActionType::SHIFT, j});
                } else {
                    table.addGoto(i, sym, j);
                }
            } else {
                if (p.lhs == g.augStartSymbol && item.lookahead == "$") {
                    table.addAction(i, "$", {ActionType::ACCEPT, -1});
                } else if (p.lhs != g.augStartSymbol) {
                    // Reduce ONLY on the specific lookahead
                    table.addAction(i, item.lookahead, {ActionType::REDUCE, item.prodId});
                }
            }
        }
        // GOTO entries
        for (auto& nt : g.nonTerminals) {
            auto it = gotoMap.find({i, nt});
            if (it != gotoMap.end())
                table.addGoto(i, nt, it->second);
        }
    }
    return table;
}
