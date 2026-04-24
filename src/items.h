#pragma once
#include "grammar.h"
#include <vector>
#include <set>
#include <map>
#include <string>

using namespace std;

// ===== LR(0) Item =====
struct LR0Item {
    int prodId;   // index into Grammar::productions
    int dotPos;   // dot position (0 = before first symbol)

    bool operator<(const LR0Item& o) const {
        if (prodId != o.prodId) return prodId < o.prodId;
        return dotPos < o.dotPos;
    }
    bool operator==(const LR0Item& o) const {
        return prodId == o.prodId && dotPos == o.dotPos;
    }
};

// ===== LR(1) Item =====
struct LR1Item {
    int prodId;
    int dotPos;
    string lookahead; // terminal or "$"

    bool operator<(const LR1Item& o) const {
        if (prodId   != o.prodId)   return prodId   < o.prodId;
        if (dotPos   != o.dotPos)   return dotPos   < o.dotPos;
        return lookahead < o.lookahead;
    }
    bool operator==(const LR1Item& o) const {
        return prodId == o.prodId && dotPos == o.dotPos && lookahead == o.lookahead;
    }
};

using ItemSet0 = set<LR0Item>;
using ItemSet1 = set<LR1Item>;

// ===== LR(0) canonical collection =====
class LR0Items {
public:
    explicit LR0Items(const Grammar& g) : gram(g) {}

    void build();

    const vector<ItemSet0>& states() const { return itemSets; }
    const map<pair<int,string>, int>& gotoTable() const { return gotoMap; }

    void print(ostream& out = cout) const;

private:
    const Grammar& gram;
    vector<ItemSet0> itemSets;
    map<pair<int,string>, int> gotoMap; // (stateIdx, symbol) -> stateIdx

    ItemSet0 closure(const ItemSet0& items) const;
    ItemSet0 gotoOp(const ItemSet0& items, const string& sym) const;

    string symbolAfterDot(const LR0Item& item) const;
    bool dotAtEnd(const LR0Item& item) const;
};

// ===== LR(1) canonical collection =====
class LR1Items {
public:
    explicit LR1Items(const Grammar& g) : gram(g) {}

    void build();

    const vector<ItemSet1>& states() const { return itemSets; }
    const map<pair<int,string>, int>& gotoTable() const { return gotoMap; }

    void print(ostream& out = cout) const;

private:
    const Grammar& gram;
    vector<ItemSet1> itemSets;
    map<pair<int,string>, int> gotoMap;

    ItemSet1 closure(const ItemSet1& items) const;
    ItemSet1 gotoOp(const ItemSet1& items, const string& sym) const;

    string symbolAfterDot(const LR1Item& item) const;
    bool dotAtEnd(const LR1Item& item) const;
};
