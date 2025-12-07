#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include "utils.h"
using namespace std;

class Parse {
    string cmd_;
    string table_;
    vector<string> cols_;      // CREATE: name:TYPE  | SELECT: col list
    vector<string> vals_;      // INSERT values
    string whereCol_, whereOp_, whereVal_;
    string setCol_, setVal_;
    bool valid_ = false;

public:
    Parse() {

    }

    void reset() {

        cmd_.clear();
        table_.clear();
        cols_.clear();
        vals_.clear();
        whereCol_.clear();
        whereOp_.clear();
        whereVal_.clear();
        setCol_.clear();
        setVal_.clear();
        valid_ = false;

    }

    static vector<string> tokenize(const string& q)
    {
        // simple tokenizer to cut by space and special chars
        vector<string> tok;
        string current;
        string s = q;
        if (!s.empty() && s.back() == ';') // remove trailing semicolon
            s.pop_back();

        for (size_t i = 0; i < s.size(); ++i) {
            char ch = s[i];

            if (ch == '(' || ch == ')' || ch == ',' || ch == '=' || ch == '<' || ch == '>' || ch == '!') // special chars
            {
                if (!current.empty())
                {
                    tok.push_back(current);
                    current.clear();
                }

                if ((ch == '<' || ch == '>' || ch == '!' || ch == '=') && i + 1 < s.size() && s[i + 1] == '=')
                {
                    tok.push_back(string(1, ch) + "=");
                    ++i;
                }
                else
                {
                    tok.push_back(string(1, ch));
                }
            }
            else if (isspace((unsigned char)ch)) // space 
            {
                if (!current.empty())
                {
                    tok.push_back(current);
                    current.clear();
                }
            }
            else current.push_back(ch);
        }

        if (!current.empty()) // last token
            tok.push_back(current);

        return tok;
    }

    void parse(const string& query)
    {
        reset();

        auto tok = tokenize(query);

        if (tok.empty())
            return;

        string first = toUpper(tok[0]);

        if (first == "CREATE")
            parseCreate(tok);

        else if (first == "INSERT")
            parseInsert(tok);

        else if (first == "SELECT")
            parseSelect(tok);

        else if (first == "UPDATE")
            parseUpdate(tok);

        else if (first == "DELETE")
            parseDelete(tok);

        else if (first == "DROP")
            parseDrop(tok);
    }

    void parseCreate(const vector<string>& t)
    {
        // CREATE TABLE name ( col TYPE, col TYPE )
        if (t.size() < 5)
            return;

        if (toUpper(t[1]) != "TABLE")
            return;

        table_ = toLower(trim(t[2]));

        int open = -1, close = -1;
        for (size_t i = 3; i < t.size(); ++i)
            if (t[i] == "(")
            {
                open = (int)i;
                break;
            }

        for (int i = (int)t.size() - 1; i >= 0; --i)
            if (t[i] == ")")
            {
                close = i;
                break;
            }

        if (open < 0 || close < 0 || close <= open)
            return;

        vector<string> parts;

        for (int i = open + 1; i < close; ++i)
            if (t[i] != ",")
                parts.push_back(t[i]);

        if (parts.size() % 2 != 0)
            return;

        cols_.clear();

        for (size_t i = 0; i < parts.size(); i += 2)
        {
            string name = toLower(trim(parts[i]));
            string type = toUpper(trim(parts[i + 1]));

            if (type != "INT" && type != "STRING")  // default to STRING
                type = "STRING";

            cols_.push_back(name + ":" + type);
        }
        valid_ = true;

        cmd_ = "CREATE";
    }

    void parseInsert(const vector<string>& t)
    {
        // INSERT INTO table VALUES (val1,val2)
        if (t.size() < 4)
            return;

        if (toUpper(t[1]) != "INTO")
            return;

        table_ = toLower(trim(t[2]));
        int vpos = -1;

        for (size_t i = 3; i < t.size(); ++i)
            if (toUpper(t[i]) == "VALUES")
            {
                vpos = (int)i;
                break;
            }

        if (vpos < 0)
            return;
        int open = -1, close = -1;

        for (size_t i = vpos + 1; i < t.size(); ++i)
            if (t[i] == "(")
            {
                open = (int)i;
                break;
            }
        for (size_t i = open + 1; i < t.size(); ++i)
            if (t[i] == ")")
            {
                close = (int)i;
                break;
            }
        if (open < 0 || close < 0)
            return;
        vals_.clear();

        for (int i = open + 1; i < close; ++i) if (t[i] != ",") vals_.push_back(trim(t[i]));
        valid_ = true; cmd_ = "INSERT";
    }


    void parseSelect(const vector<string>& t)
    {
        if (t.size() < 4)
            return;

        int from = -1;

        for (size_t i = 1; i < t.size(); ++i)
            if (toUpper(t[i]) == "FROM")
            {
                from = (int)i;
                break;
            }

        if (from < 0 || from + 1 >= (int)t.size())
            return;

        cols_.clear();

        for (int i = 1; i < from; ++i)
            if (t[i] != "," && t[i] != "(" && t[i] != ")")
                cols_.push_back(toLower(trim(t[i])));

        table_ = toLower(trim(t[from + 1]));

        if (from + 2 < (int)t.size() && toUpper(t[from + 2]) == "WHERE")
        {
            if (from + 5 <= (int)t.size() - 1)
            {
                whereCol_ = toLower(trim(t[from + 3]));
                whereOp_ = trim(t[from + 4]);
                whereVal_ = trim(t[from + 5]);
                valid_ = true; cmd_ = "SELECT"; return;
            }
            else return;
        }
        valid_ = true; cmd_ = "SELECT";
    }

    void parseUpdate(const vector<string>& t)
    {
        if (t.size() < 5)
            return;

        table_ = toLower(trim(t[1]));

        if (toUpper(t[2]) != "SET")
            return;
        if (t.size() < 6)
            return;

        setCol_ = toLower(trim(t[3]));
        if (t[4] != "=")
            return;

        setVal_ = trim(t[5]);

        if ((int)t.size() > 6 && toUpper(t[6]) == "WHERE")
        {
            if ((int)t.size() >= 10) {
                whereCol_ = toLower(trim(t[7]));
                whereOp_ = trim(t[8]);
                whereVal_ = trim(t[9]);
            }
            else
                return;
        }
        valid_ = true;

        cmd_ = "UPDATE";
    }

    void parseDelete(const vector<string>& t) {
        if (t.size() < 3)
            return;

        if (toUpper(t[1]) != "FROM")
            return;

        table_ = toLower(trim(t[2]));

        if ((int)t.size() > 3 && toUpper(t[3]) == "WHERE")
        {
            if ((int)t.size() >= 6)
            {
                whereCol_ = toLower(trim(t[4]));
                whereOp_ = trim(t[5]);
                whereVal_ = trim(t[6]);
            }
            else
                return;
        }
        valid_ = true;
        cmd_ = "DELETE";
    }

    void parseDrop(const vector<string>& t)
    {
        if (t.size() < 3)
            return;

        if (toUpper(t[1]) != "TABLE")
            return;

        table_ = toLower(trim(t[2]));
        valid_ = true;
        cmd_ = "DROP";
    }

    // getters
    string cmd() const { return cmd_; }
    string table() const { return table_; }
    vector<string> columns() const { return cols_; }
    vector<string> values() const { return vals_; }
    bool valid() const { return valid_; }
    string whereCol() const { return whereCol_; }
    string whereOp() const { return whereOp_; }
    string whereVal() const { return whereVal_; }
    string setCol() const { return setCol_; }
    string setVal() const { return setVal_; }
};

#endif // PARSER_H