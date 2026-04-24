#include "grammar.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

using namespace std;

// ---------- helpers ----------

static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static vector<string> splitByBar(const string& s) {
    vector<string> parts;
    istringstream ss(s);
    string tok;
    while (getline(ss, tok, '|'))
        parts.push_back(trim(tok));
    return parts;
}

static vector<string> tokenizeRHS(const string& rhs) {
    vector<string> tokens;
    istringstream ss(rhs);
    string tok;
    while (ss >> tok) tokens.push_back(tok);
    return tokens;
}

// ---------- Grammar::tokenizeLine ----------
void Grammar::tokenizeLine(const string& line,
                            string& lhs,
                            vector<vector<string>>& alternatives) {
    // Format:  NonTerminal -> alt1 | alt2 | ...
    size_t arrowPos = line.find("->");
    if (arrowPos == string::npos)
        throw runtime_error("Bad production line (no ->): " + line);

    lhs = trim(line.substr(0, arrowPos));
    string rhsPart = trim(line.substr(arrowPos + 2));

    for (const string& alt : splitByBar(rhsPart)) {
        vector<string> syms = tokenizeRHS(alt);
        if (syms.empty()) syms.push_back("epsilon");
        alternatives.push_back(syms);
    }
}

// ---------- Grammar::readFromFile ----------
bool Grammar::readFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "Cannot open grammar file: " << filename << "\n";
        return false;
    }

    productions.clear();
    nonTerminals.clear();
    terminals.clear();

    bool firstLine = true;
    string line;
    int prodId = 0;

    while (getline(fin, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        string lhs;
        vector<vector<string>> alternatives;
        tokenizeLine(line, lhs, alternatives);

        if (firstLine) {
            startSymbol = lhs;
            firstLine = false;
        }
        nonTerminals.insert(lhs);

        for (auto& rhs : alternatives) {
            Production p;
            p.lhs = lhs;
            p.rhs = rhs;
            p.id  = prodId++;
            productions.push_back(p);
        }
    }

    // Identify terminals: anything in any RHS that is not a non-terminal and not epsilon
    // Two-pass: first collect all NT names, then scan
    for (auto& p : productions)
        for (auto& sym : p.rhs)
            if (sym != "epsilon" && sym != "@")
                if (nonTerminals.find(sym) == nonTerminals.end())
                    terminals.insert(sym);

    terminals.insert("$"); // end-of-input marker

    return true;
}

// ---------- Grammar::augment ----------
void Grammar::augment() {
    augStartSymbol = startSymbol + "Prime";
    // If augStartSymbol already exists, keep appending primes
    while (nonTerminals.count(augStartSymbol))
        augStartSymbol += "Prime";

    Production aug;
    aug.lhs = augStartSymbol;
    aug.rhs = {startSymbol};
    aug.id  = -1; // will be fixed below

    // Insert at front
    productions.insert(productions.begin(), aug);

    // Re-number all productions
    for (int i = 0; i < (int)productions.size(); ++i)
        productions[i].id = i;

    nonTerminals.insert(augStartSymbol);
}

// ---------- FIRST sets ----------
void Grammar::computeFirstForSymbol(const string& sym,
                                    map<string, bool>& inProgress) {
    if (firstSets.count(sym)) return; // already done
    if (inProgress[sym]) return;      // cycle guard
    inProgress[sym] = true;

    set<string>& fset = firstSets[sym];

    if (isTerminal(sym)) { fset.insert(sym); return; }
    if (sym == "epsilon" || sym == "@") { fset.insert("epsilon"); return; }

    for (auto& p : productions) {
        if (p.lhs != sym) continue;
        if (p.rhs.empty() || p.rhs[0] == "epsilon" || p.rhs[0] == "@") {
            fset.insert("epsilon");
            continue;
        }
        // Add FIRST of each symbol in RHS while previous derives epsilon
        bool allEpsilon = true;
        for (auto& s : p.rhs) {
            computeFirstForSymbol(s, inProgress);
            for (auto& t : firstSets[s])
                if (t != "epsilon") fset.insert(t);
            if (!firstSets[s].count("epsilon")) { allEpsilon = false; break; }
        }
        if (allEpsilon) fset.insert("epsilon");
    }
}

void Grammar::computeFirst() {
    firstSets.clear();
    // Seed terminals
    for (auto& t : terminals) firstSets[t].insert(t);
    firstSets["epsilon"].insert("epsilon");
    firstSets["$"].insert("$");

    // Fixed-point over non-terminals
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& nt : nonTerminals) {
            size_t before = firstSets[nt].size();
            for (auto& p : productions) {
                if (p.lhs != nt) continue;
                if (p.rhs.empty() || p.rhs[0] == "epsilon" || p.rhs[0] == "@") {
                    firstSets[nt].insert("epsilon");
                    continue;
                }
                bool allEps = true;
                for (auto& sym : p.rhs) {
                    // Ensure entry exists
                    if (isTerminal(sym) || sym == "epsilon" || sym == "@")
                        firstSets[sym].insert(sym == "@" ? "epsilon" : sym);
                    for (auto& t : firstSets[sym])
                        if (t != "epsilon") firstSets[nt].insert(t);
                    if (!firstSets[sym].count("epsilon")) { allEps = false; break; }
                }
                if (allEps) firstSets[nt].insert("epsilon");
            }
            if (firstSets[nt].size() != before) changed = true;
        }
    }
}

// FIRST of a sequence starting at 'start', with extra lookahead appended
set<string> Grammar::firstOfSequence(const vector<string>& seq,
                                                size_t start,
                                                const string& lookahead) const {
    set<string> result;
    bool allEps = true;
    for (size_t i = start; i < seq.size(); ++i) {
        const string& sym = seq[i];
        auto it = firstSets.find(sym);
        if (it == firstSets.end()) { allEps = false; break; }
        for (auto& t : it->second)
            if (t != "epsilon") result.insert(t);
        if (!it->second.count("epsilon")) { allEps = false; break; }
    }
    if (allEps && !lookahead.empty()) result.insert(lookahead);
    if (allEps && lookahead.empty())  result.insert("epsilon");
    return result;
}

// ---------- FOLLOW sets ----------
void Grammar::computeFollow() {
    followSets.clear();
    for (auto& nt : nonTerminals) followSets[nt]; // init empty
    followSets[augStartSymbol].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p : productions) {
            for (size_t i = 0; i < p.rhs.size(); ++i) {
                const string& sym = p.rhs[i];
                if (!isNonTerminal(sym)) continue;

                size_t before = followSets[sym].size();
                // FIRST of rest of RHS
                auto fRest = firstOfSequence(p.rhs, i + 1);
                for (auto& t : fRest)
                    if (t != "epsilon") followSets[sym].insert(t);
                // If rest can derive epsilon, add FOLLOW(lhs)
                if (fRest.count("epsilon") || (i + 1 == p.rhs.size()))
                    for (auto& t : followSets[p.lhs])
                        followSets[sym].insert(t);

                if (followSets[sym].size() != before) changed = true;
            }
        }
    }
}

// ---------- predicates ----------
bool Grammar::isTerminal(const string& sym) const {
    return terminals.count(sym) > 0;
}
bool Grammar::isNonTerminal(const string& sym) const {
    return nonTerminals.count(sym) > 0;
}
bool Grammar::isEpsilon(const string& sym) const {
    return sym == "epsilon" || sym == "@";
}

// ---------- print ----------
void Grammar::printAugmented(ostream& out) const {
    out << "\n========== Augmented Grammar ==========\n";
    for (auto& p : productions) {
        out << "  [" << p.id << "] " << p.lhs << " -> ";
        if (p.rhs.empty()) { out << "epsilon"; }
        else for (size_t i = 0; i < p.rhs.size(); ++i)
            out << (i ? " " : "") << p.rhs[i];
        out << "\n";
    }
    out << "\nTerminals: ";
    for (auto& t : terminals) out << t << " ";
    out << "\nNon-terminals: ";
    for (auto& nt : nonTerminals) out << nt << " ";
    out << "\n";
}
