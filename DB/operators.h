#ifndef OPERATORS_H
#define OPERATORS_H

#include <vector>
#include <string>
#include <memory>
#include "db.h"
using namespace std;
// simple operator interface working with vector<string> rows
class Operator {
public:
    virtual void open() = 0;
    virtual bool next() = 0;
    virtual vector<string> getRow() = 0;
    virtual void updateRow(const vector<string>& newRow) {} // optional
    virtual void close() = 0;
    virtual ~Operator() {}
};

class TableScan : public Operator {
    TableDynamic& table_;
    int idx_;
    vector<string> current_;
public:
    TableScan(TableDynamic& t) : table_(t), idx_(-1) {}
    void open() override { idx_ = -1; table_.load(); }
    bool next() override { ++idx_; if (idx_ < table_.rowCount()) 
    { 
        current_ = table_.getRow(idx_);
        return true;
    } 
    return false; }
    vector<string> getRow() override { return current_; }
    void updateRow(const vector<string>& newRow) override 
    { if (idx_ >= 0 && idx_ < table_.rowCount())
        table_.getRow(idx_) = const_cast<vector<string>&>(newRow); }
    void close() override {}
};

class Filter : public Operator {
    unique_ptr<Operator> child_;
    int colIdx_;
    string op_, val_;
    vector<string> current_;
public:
    Filter(unique_ptr<Operator> child, int colIdx, const string& op, const string& val)
        : child_(move(child)), colIdx_(colIdx), op_(op), val_(val) {}
    void open() override { child_->open(); }
    bool next() override {
        while (child_->next()) {
            auto r = child_->getRow();
            if (colIdx_ < 0 || colIdx_ >= (int)r.size()) continue;
            string cell = r[colIdx_];
            bool ok = false;
            bool cellNum = isNumberString(cell), valNum = isNumberString(val_);
            if (cellNum && valNum) {
                try {
                    long long a = stoll(cell), b = stoll(val_);
                    if (op_ == "=") ok = (a == b);
                    else if (op_ == ">") ok = (a > b);
                    else if (op_ == "<") ok = (a < b);
                }
                catch (...) 
                { 
                    ok = false; 
                }
            }
            else {
                if (op_ == "=") ok = (cell == val_);
                else ok = false;
            }
            if (ok) { current_ = r; return true; }
        }
        return false;
    }
    vector<string> getRow() override { return current_; }
    void updateRow(const vector<string>& newRow) override { child_->updateRow(newRow); }
    void close() override { child_->close(); }
};

class Projection : public Operator {
    unique_ptr<Operator> child_;
    vector<int> idxs_;
    vector<string> current_;
public:
    Projection(unique_ptr<Operator> child, const vector<int>& idxs) : child_(move(child)), idxs_(idxs) {}
    void open() override { child_->open(); }
    bool next() override {
        if (!child_->next()) return false;
        auto r = child_->getRow();
        vector<string> out; out.reserve(idxs_.size());
        for (int i : idxs_) out.push_back((i >= 0 && i < (int)r.size()) ? r[i] : string());
        current_ = out; return true;
    }
    vector<string> getRow() override { return current_; }
    void updateRow(const vector<string>& newRow) override { child_->updateRow(newRow); }
    void close() override { child_->close(); }
};

#endif // OPERATORS_H
