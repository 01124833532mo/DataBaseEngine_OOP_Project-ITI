#ifndef DB_H
#define DB_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <functional>
#include <filesystem>
#include "utils.h"

using namespace std;
namespace fs = std::filesystem;

struct TableSchema {
    vector<string> names;
    vector<string> types;
};

class TableDynamic {
    string name_;
    TableSchema schema_;
    vector<vector<string>> rows_;
    string folder_ = "./db/";

    string filepath() const // return full path of table file
    {
        return folder_ + name_ + ".txt";
    }

public:
    TableDynamic()
    {


    };

    TableDynamic(const string& name)
    {
        name_ = name;
        ensureDir();
    }

    void ensureDir() const
    {
        ensure_dir(folder_);
    }
    void setName(const string& n)
    {
        name_ = n; 
        ensureDir();
    }

    const string& name() const
    {
        return name_;
    }

    void setSchema(const TableSchema& s)
    {
        schema_.names.clear();
        schema_.types.clear();

        for (size_t i = 0; i < s.names.size(); ++i) {
            schema_.names.push_back(toLower(s.names[i]));
            schema_.types.push_back(s.types[i]);
        }
    }
    const TableSchema& schema() const
    {
        return schema_;
    }

    int columnIndex(const string& col) const
    {
        string want = toLower(trim(col));

        for (int i = 0; i < (int)schema_.names.size(); ++i) {
            if (schema_.names[i] == want)
                return i;
        }
        return -1;
    }

    void insertRow(const vector<string>& vals)
    {
        rows_.push_back(vals);

        save();
    }
    int rowCount() const
    {
        return (int)rows_.size();
    }

    vector<string>& getRow(int idx)
    {
        return rows_.at(idx);
    }

    int updateRows(function<bool(const vector<string>&)> pred, int targetIdx, const string& newVal) {
        int changed = 0;

        for (auto& row : rows_) {
            if (pred(row)) {
                row[targetIdx] = newVal;
                ++changed;
            }
        }
        if (changed > 0)
            save();

        return changed;
    }

    int deleteRows(vector<string> pred)
    {
        vector<vector<string>> remain;
        int deleted = 0;

        for (auto& row : rows_)
        {
            if (row==pred)
                deleted++;    
            else
                remain.push_back(row);
        }

        rows_.swap(remain);

        if (deleted > 0)
            save();

        return deleted;
    }

    void save() const {
        // ensure file exestence
        ensure_dir(folder_);

        ofstream out(filepath()); // open file to write and save at out

        if (!out.is_open())
            return;

        int coloums = (int)schema_.names.size();

        out << coloums << "\n";
        for (int i = 0; i < coloums; ++i)
            out << schema_.names[i] << " " << schema_.types[i] << "\n";

        for (int i = 0; i < (int)rows_.size(); ++i) {
            out << (i + 1);
            for (auto& v : rows_[i]) out << "," << v;
            out << "\n";
        }
    }

    void load() {
        rows_.clear();
        ifstream in(filepath()); // open file

        if (!in.is_open())
            return;

        int c = 0;

        if (!(in >> c))
            return;

        string line;
        getline(in, line);
        schema_.names.clear();
        schema_.types.clear();

        for (int i = 0; i < c; ++i) { // load colounns
            if (!getline(in, line))
                return;

            stringstream ss(line);
            string name, type;
            ss >> name >> type;

            schema_.names.push_back(toLower(name));
            schema_.types.push_back(type);
        }
        while (getline(in, line))  // load rows
        {
            if (line.empty())
                continue;

            vector<string> fields;
            string cur;

            for (char ch : line) {
                if (ch == ',')
                {
                    fields.push_back(cur);
                    cur.clear();
                }
                else
                    cur.push_back(ch);
            }
            if (!cur.empty())
                fields.push_back(cur);

            if ((int)fields.size() >= 1 + c) {
                vector<string> vals;
                for (int j = 1; j <= c; ++j)
                    vals.push_back(fields[j]);

                rows_.push_back(vals);
            }
        }
    }

    const vector<vector<string>>& rows() const
    {
        return rows_;
    }
};

class Catalog {
    unordered_map<string, TableDynamic> tables_;
    string folder_ = "./db/";

public:
    Catalog() {

    }

    void loadExisting(const string& folder = "./db/") {
        ensure_dir(folder);
    }

    bool has(const string& name) const {
        string key = toLower(trim(name));

        if (tables_.count(key))
            return true;

        string path = folder_ + key + ".txt";
        return fs::exists(path); // check file exsist at folder
    }

    TableDynamic* get(const string& name) {
        string key = toLower(trim(name));

        auto it = tables_.find(key);
        if (it != tables_.end())
            return &it->second;

        string path = folder_ + key + ".txt";
        if (fs::exists(path)) {
            TableDynamic t(key);
            t.load();
            auto [insertedIt, success] = tables_.emplace(key, std::move(t));
            return &insertedIt->second;
        }

        return nullptr;
    }

    bool create(const string& name, const TableSchema& schema) {
        string key = toLower(trim(name));

        if (has(key))
            return false;
        TableDynamic t(key);

        t.setSchema(schema);
        t.save();

        tables_.emplace(key, std::move(t));
        return true;
    }


    vector<string> listTables() const {
        vector<string> r;
        for (auto& p : tables_) r.push_back(p.first);
        return r;
    }

    void registerExisting(const string& name) {  // load specific file if dont exsist at ram
        string key = toLower(trim(name));
        if (!has(key)) {
            TableDynamic t(key);
            t.load();
            tables_.emplace(key, std::move(t));
        }
    }

    bool drop(const string& name) {
        string key = toLower(trim(name));

        tables_.erase(key);

        string path = folder_ + key + ".txt";
        if (fs::exists(path)) {
            fs::remove(path);
            return true;
        }

        return false;
    }

    void registerExistingAll() {
        ensure_dir(folder_);
        for (auto& p : fs::directory_iterator(folder_)) { // loop all files at folder ("db/")
            if (p.path().extension() == ".txt") {
                string name = p.path().stem().string(); // cut file without extention (user.txt -> user)
                string key = toLower(name);
                if (!has(key)) {
                    TableDynamic t(name);
                    t.load();
                    tables_.emplace(key, std::move(t));
                }
            }
        }
    }
};

#endif