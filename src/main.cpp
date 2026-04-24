#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

#include "grammar.h"
#include "items.h"
#include "parsing_table.h"
#include "slr_parser.h"
#include "lr1_parser.h"
#include "tree.h"

using namespace std;

// ---- Utilities ----
static vector<string> tokenizeInput(const string& line) {
    vector<string> tokens;
    istringstream ss(line);
    string tok;
    while (ss >> tok) tokens.push_back(tok);
    return tokens;
}

static void writeToFile(const string& path, const string& content) {
    ofstream out(path);
    if (!out.is_open()) { cerr << "Cannot open output file: " << path << "\n"; return; }
    out << content;
}

static void usage(const char* prog) {
    cerr << "Usage:\n"
              << "  " << prog << " --slr  <grammar_file> <input_file>\n"
              << "  " << prog << " --lr1  <grammar_file> <input_file>\n"
              << "  " << prog << " --both <grammar_file> <input_file>\n"
              << "  " << prog << " --compare <grammar_file>\n";
}

// ---- Run SLR(1) ----
void runSLR(Grammar& g,
            const vector<vector<string>>& inputStrings,
            bool printAll = true) {
    auto t0 = chrono::high_resolution_clock::now();

    SLRParser parser(g);
    parser.build();

    auto t1 = chrono::high_resolution_clock::now();
    double buildMs = chrono::duration<double, milli>(t1 - t0).count();

    // --- Output to files ---
    {
        ostringstream ss;
        parser.printItems(ss);
        writeToFile("output/slr_items.txt", ss.str());
    }
    {
        ostringstream ss;
        parser.printTable(ss);
        writeToFile("output/slr_parsing_table.txt", ss.str());
    }

    if (printAll) {
        parser.printItems(cout);
        parser.printTable(cout);
    }

    cout << "\n[SLR(1)] States: " << parser.numStates()
              << "  Build time: " << fixed << setprecision(2) << buildMs << " ms\n";
    if (parser.isConflict())
        cout << "[SLR(1)] WARNING: Grammar has conflicts!\n";

    // Parse each input string
    ostringstream traceAll, treeAll;
    for (auto& tokens : inputStrings) {
        string inputLine;
        for (auto& t : tokens) inputLine += t + " ";
        cout << "\n--- SLR Parsing: " << inputLine << "---\n";

        ostringstream trace;
        bool accepted = parser.parse(tokens, trace);
        cout << trace.str();
        traceAll << "Input: " << inputLine << "\n" << trace.str() << "\n";

        if (accepted) {
            parser.getParseTree().print(cout);
            ostringstream ts;
            parser.getParseTree().print(ts);
            treeAll << "Input: " << inputLine << "\n" << ts.str() << "\n";
        }
    }
    writeToFile("output/slr_trace.txt", traceAll.str());
    writeToFile("output/parse_trees.txt", treeAll.str());
}

// ---- Run LR(1) ----
void runLR1(Grammar& g,
            const vector<vector<string>>& inputStrings,
            bool printAll = true) {
    auto t0 = chrono::high_resolution_clock::now();

    LR1Parser parser(g);
    parser.build();

    auto t1 = chrono::high_resolution_clock::now();
    double buildMs = chrono::duration<double, milli>(t1 - t0).count();

    {
        ostringstream ss;
        parser.printItems(ss);
        writeToFile("output/lr1_items.txt", ss.str());
    }
    {
        ostringstream ss;
        parser.printTable(ss);
        writeToFile("output/lr1_parsing_table.txt", ss.str());
    }

    if (printAll) {
        parser.printItems(cout);
        parser.printTable(cout);
    }

    cout << "\n[LR(1)] States: " << parser.numStates()
              << "  Build time: " << fixed << setprecision(2) << buildMs << " ms\n";
    if (parser.isConflict())
        cout << "[LR(1)] WARNING: Grammar has conflicts!\n";

    ostringstream traceAll;
    for (auto& tokens : inputStrings) {
        string inputLine;
        for (auto& t : tokens) inputLine += t + " ";
        cout << "\n--- LR(1) Parsing: " << inputLine << "---\n";

        ostringstream trace;
        bool accepted = parser.parse(tokens, trace);
        cout << trace.str();
        traceAll << "Input: " << inputLine << "\n" << trace.str() << "\n";

        if (accepted) parser.getParseTree().print(cout);
    }
    writeToFile("output/lr1_trace.txt", traceAll.str());
}

// ---- Comparison ----
void runComparison(Grammar& g) {
    auto t0 = chrono::high_resolution_clock::now();
    SLRParser slr(g); slr.build();
    auto t1 = chrono::high_resolution_clock::now();
    double slrMs = chrono::duration<double, milli>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    LR1Parser lr1(g); lr1.build();
    t1 = chrono::high_resolution_clock::now();
    double lr1Ms = chrono::duration<double, milli>(t1 - t0).count();

    ostringstream cmp;
    cmp << "========== Parser Comparison ==========\n\n";
    cmp << left << setw(30) << "Metric"
        << setw(15) << "SLR(1)"
        << setw(15) << "LR(1)" << "\n";
    cmp << string(60, '-') << "\n";
    cmp << setw(30) << "Number of states"
        << setw(15) << slr.numStates()
        << setw(15) << lr1.numStates() << "\n";
    cmp << setw(30) << "Build time (ms)"
        << setw(15) << fixed << setprecision(2) << slrMs
        << setw(15) << lr1Ms << "\n";
    cmp << setw(30) << "Has conflicts"
        << setw(15) << (slr.isConflict() ? "Yes" : "No")
        << setw(15) << (lr1.isConflict() ? "Yes" : "No") << "\n";

    cmp << "\nConclusion:\n";
    if (!slr.isConflict() && !lr1.isConflict())
        cmp << "  Both parsers handle this grammar. SLR(1) uses fewer states.\n";
    else if (slr.isConflict() && !lr1.isConflict())
        cmp << "  SLR(1) has conflicts but LR(1) resolves them.\n"
            << "  This grammar is LR(1) but NOT SLR(1).\n";
    else if (!slr.isConflict() && lr1.isConflict())
        cmp << "  Unexpected: SLR(1) clean but LR(1) has conflicts.\n";
    else
        cmp << "  Both parsers have conflicts. Grammar may be ambiguous.\n";

    cout << cmp.str();
    writeToFile("output/comparison.txt", cmp.str());
}

// ---- main ----
int main(int argc, char* argv[]) {
    if (argc < 3) { usage(argv[0]); return 1; }

    string mode        = argv[1];
    string grammarFile = argv[2];

    Grammar g;
    if (!g.readFromFile(grammarFile)) return 1;
    g.augment();
    g.computeFirst();
    g.computeFollow();

    // Write augmented grammar
    {
        ostringstream ss;
        g.printAugmented(ss);
        writeToFile("output/augmented_grammar.txt", ss.str());
        cout << ss.str();
    }

    // Read input strings (if provided)
    vector<vector<string>> inputs;
    if (argc >= 4) {
        string inputFile = argv[3];
        ifstream fin(inputFile);
        if (!fin.is_open()) {
            cerr << "Cannot open input file: " << inputFile << "\n";
        } else {
            string line;
            while (getline(fin, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                if (line.empty() || line[0] == '#') continue;
                inputs.push_back(tokenizeInput(line));
            }
        }
    }
    if (inputs.empty())
        inputs.push_back({}); // at least one empty run

    if (mode == "--slr") {
        runSLR(g, inputs);
    } else if (mode == "--lr1") {
        runLR1(g, inputs);
    } else if (mode == "--both") {
        runSLR(g, inputs);
        runLR1(g, inputs);
        runComparison(g);
    } else if (mode == "--compare") {
        runComparison(g);
    } else {
        usage(argv[0]);
        return 1;
    }

    return 0;
}
