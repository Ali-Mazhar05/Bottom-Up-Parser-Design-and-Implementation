#include "slr_parser.h"
#include <iomanip>
#include <algorithm>

using namespace std;

SLRParser::SLRParser(const Grammar& g) : gram_(g), lr0_(g) {}

void SLRParser::build() {
    lr0_.build();
    table_ = buildSLR1Table(gram_, lr0_);
}

void SLRParser::printTable(ostream& out) const {
    out << "\n========== SLR(1) Parsing Table ==========\n";
    table_.print(gram_, out);
    table_.printConflicts(gram_, out);
    if (table_.hasConflicts)
        out << "\n[!] Grammar is NOT SLR(1) due to conflicts.\n";
    else
        out << "\n[OK] Grammar is SLR(1).\n";
}

bool SLRParser::parse(const vector<string>& tokens,
                      ostream& traceOut) {
    parseTree_ = ParseTree(); // reset

    // Append end-marker
    vector<string> input = tokens;
    input.push_back("$");

    ParserStack stk;
    // Node stacks for parse tree construction
    vector<shared_ptr<TreeNode>> nodeStack;

    size_t ip = 0; // input pointer
    int step  = 1;

    // Header
    const int SW = 5, ColW = 30, ActW = 35;
    traceOut << "\n========== SLR(1) Parsing Trace ==========\n";
    traceOut << left
             << setw(SW)  << "Step"
             << setw(ColW)<< "Stack"
             << setw(ColW)<< "Input"
             << setw(ActW)<< "Action"
             << "\n"
             << string(SW + ColW * 2 + ActW, '-') << "\n";

    while (true) {
        int s = stk.topState();
        const string& a = input[ip];

        // Build remaining input string
        string inputStr;
        for (size_t i = ip; i < input.size(); ++i) inputStr += input[i] + " ";

        Action act = table_.resolve(s, a);

        // Trace
        traceOut << left
                 << setw(SW)  << step++
                 << setw(ColW)<< stk.toString()
                 << setw(ColW)<< inputStr;

        if (act.type == ActionType::SHIFT) {
            traceOut << "Shift " << act.value << "\n";
            stk.push(a, act.value);
            nodeStack.push_back(make_shared<TreeNode>(a));
            ++ip;

        } else if (act.type == ActionType::REDUCE) {
            const Production& p = gram_.productions[act.value];
            string rhsStr = p.lhs + " -> ";
            for (auto& sym : p.rhs) rhsStr += sym + " ";
            traceOut << "Reduce " << rhsStr << "\n";

            // Build tree node
            auto parent = make_shared<TreeNode>(p.lhs);
            int popCnt = (p.rhs.size() == 1 && gram_.isEpsilon(p.rhs[0])) ? 0 : (int)p.rhs.size();
            // Collect children from nodeStack (reverse order)
            vector<shared_ptr<TreeNode>> children;
            for (int i = 0; i < popCnt && !nodeStack.empty(); ++i) {
                children.push_back(nodeStack.back());
                nodeStack.pop_back();
            }
            reverse(children.begin(), children.end());
            for (auto& ch : children) parent->addChild(ch);
            if (popCnt == 0) parent->addChild(make_shared<TreeNode>("ε"));
            nodeStack.push_back(parent);

            stk.pop(popCnt);
            int t = stk.topState();
            auto gIt = table_.gotoT.find(t);
            if (gIt == table_.gotoT.end() || !gIt->second.count(p.lhs)) {
                traceOut << "Error: No GOTO entry for state " << t
                         << " on " << p.lhs << "\n";
                return false;
            }
            int nextState = gIt->second.at(p.lhs);
            stk.push(p.lhs, nextState);

        } else if (act.type == ActionType::ACCEPT) {
            traceOut << "Accept\n";
            traceOut << "\nResult: String accepted successfully!\n";
            if (!nodeStack.empty()) parseTree_.root = nodeStack.back();
            return true;

        } else {
            traceOut << "Error\n";
            traceOut << "\nResult: String REJECTED. Unexpected symbol '" << a
                     << "' in state I" << s << ".\n";
            
            vector<string> expected = table_.getExpectedSymbols(s);
            if (!expected.empty()) {
                traceOut << "Expected one of: ";
                for (size_t i = 0; i < expected.size(); ++i)
                    traceOut << (i ? ", " : "") << expected[i];
                traceOut << "\n";
            }
            return false;
        }
    }
}
