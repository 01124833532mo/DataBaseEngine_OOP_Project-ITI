#include <iostream>
#include <vector>
#include <functional>
#include "utils.h"
#include "parser.h"
#include "db.h"

using namespace std;

static Catalog CATALOG;


TableSchema schemaFromCreateCols(const vector<string>& cols) {
    TableSchema s;
    for (int i = 0; i < cols.size(); i++) {
        string ct = cols[i];
        size_t pos = ct.find(':');

        if (pos == string::npos) {
            s.names.push_back(toLower(trim(ct)));
            s.types.push_back("STRING");
        }
        else {
            string n = toLower(trim(ct.substr(0, pos)));
            string tp = toUpper(trim(ct.substr(pos + 1)));

            if (tp != "INT" && tp != "STRING")
                tp = "STRING";

            s.names.push_back(n);
            s.types.push_back(tp);
        }
    }
    return s;
}


bool validateInsert(const TableSchema& schema,vector<string>& vals, string& err) {
    if (schema.names.size() != vals.size()) {
        err = "column count mismatch";
        return false;
    }
    for (int i = 0; i < vals.size(); i++) {
        if (schema.types[i] == "INT" && !isNumberString(vals[i])) {
            err = "value '" + vals[i] + "' not INT for column " + schema.names[i];
            return false;
        }
    }
    return true;
}


bool checkWhere(const vector<string>& row, int colIdx,string& op,string& val) {

    if (colIdx < 0 || colIdx >= (int)row.size())
        return false;

    string cell = row[colIdx];
    bool cellNum = isNumberString(cell);
    bool valNum = isNumberString(val);

    if (cellNum && valNum) {
        long long a = stoll(cell);
        long long b = stoll(val);
        if (op == "=") return a == b;
        if (op == "!=") return a != b;
        if (op == ">") return a > b;
        if (op == "<=") return a <= b;
        if (op == ">=") return a >= b;
    }
    else {
        if (op == "=") return cell == val;
    }
    return false;
}


void executeParse(const Parse& p) {
    string cmd = p.cmd();

    if (cmd == "CREATE") {
       
        string tname = p.table();

        if (CATALOG.has(tname))//
        {
            cout << "Error: table already exists\n";
            return;
        }

        TableSchema s = schemaFromCreateCols(p.columns());

        if (s.names.empty()) 
        {
            cout << "CREATE: no columns\n";
            return; 
        }

        bool ok = CATALOG.create(tname, s);

        cout << (ok ? "[OK] Created " + tname : "Failed to create") << "\n";
        return;
    }


    if (cmd == "INSERT") {
        string tname = p.table();
        if (!CATALOG.has(tname))
        { 
            cout << "INSERT: table not found\n"; 
            return;
        }

        TableDynamic* t = CATALOG.get(tname);
        vector<string> vals = p.values();
        string err;

        if (!validateInsert(t->schema(), vals, err)) {
            cout << "INSERT validation: " << err << "\n";
            return;
        }

        t->insertRow(vals);
        cout << "[OK] Inserted into " << tname << "\n";
        return;
    }


    if (cmd == "SELECT") {
        string tname = p.table();
        if (!CATALOG.has(tname)) 
        { 
            cout << "SELECT: table not found\n";
            return; 
        }

        TableDynamic* t = CATALOG.get(tname);
        t->load();

        vector<string> sel = p.columns();
        vector<int> selIdx;

        //*
        if (sel.size() == 1 && sel[0] == "*") {
            for (int i = 0; i < (int)t->schema().names.size(); i++)
                selIdx.push_back(i);
        }
        else {// col name
            for (int i = 0; i < (int)sel.size(); i++) {
                int idx = t->columnIndex(sel[i]);
                if (idx < 0) { cout << "SELECT: unknown column " << sel[i] << "\n"; return; }
                selIdx.push_back(idx);
            }
        }

        //where 
        int whereIdx = -1;
        string whereVal, whereOp;
        if (!p.whereCol().empty()) {
            whereIdx = t->columnIndex(p.whereCol());
            if (whereIdx < 0)
            { 
                cout << "SELECT: unknown WHERE column\n";
                return;
            }
            whereVal = p.whereVal();
            whereOp = p.whereOp();
        }

        //print col name
        for (int i = 0; i < (int)selIdx.size(); i++)
            cout << t->schema().names[selIdx[i]] << "\t";
        cout << "\n--------------------------------\n";

        //print data
        for (int r = 0; r < t->rowCount(); r++) {
            const vector<string>& row = t->getRow(r);

            if (whereIdx != -1 && !checkWhere(row, whereIdx, whereOp, whereVal))
                continue;

            for (int i = 0; i < (int)selIdx.size(); i++)
                cout << row[selIdx[i]] << "\t";

            cout << "\n";
        }
        return;
    }


    if (cmd == "UPDATE") {
        string tname = p.table();
        if (!CATALOG.has(tname)) 
        { 
            cout << "UPDATE: table not found\n";
            return;
        }

        TableDynamic* t = CATALOG.get(tname);
        t->load();


        string setCol = p.setCol();
        string setVal = p.setVal();
        int targetIdx = t->columnIndex(setCol);

        if (targetIdx < 0)
        { 
            cout << "UPDATE: unknown column\n"; return;
        }

        if (t->schema().types[targetIdx] == "INT" && !isNumberString(setVal)) {
            cout << "UPDATE: type mismatch\n"; 
            return;
        }

        int whereIdx = -1;
        string whereVal, whereOp;
        if (!p.whereCol().empty()) {
            whereIdx = t->columnIndex(p.whereCol());
            if (whereIdx < 0)
            {
                cout << "UPDATE: unknown WHERE col\n"; return;
            }
            whereVal = p.whereVal();
            whereOp = p.whereOp();
        }

        int changed = 0;
        for (int r = 0; r < t->rowCount(); r++)
        {
            vector<string> row = t->getRow(r);
            if (whereIdx == -1 || checkWhere(row, whereIdx, whereOp, whereVal)) {
                row[targetIdx] = setVal;
                t->getRow(r) = row;
                changed++;
            }
        }

        if (changed > 0) t->save();

        cout << "[OK] UPDATE changed: " << changed << "\n";
        return;
    }


    if (cmd == "DELETE") {
        string tname = p.table();
        if (!CATALOG.has(tname)) 
        {
            cout << "DELETE: table not found\n"; 
            return;
        }

        TableDynamic* t = CATALOG.get(tname);
        t->load();

        int whereIdx = -1;
        string whereVal, whereOp;
        if (!p.whereCol().empty()) {
            whereIdx = t->columnIndex(p.whereCol());
            if (whereIdx < 0) 
            {
                cout << "DELETE: unknown WHERE col\n";
                return;
            }
            whereVal = p.whereVal();
            whereOp = p.whereOp();
        }

        int removed = 0;

        for (int r = t->rowCount() - 1; r >= 0; r--) {
            vector<string> row = t->getRow(r);
            if (whereIdx == -1 || checkWhere(row, whereIdx, whereOp, whereVal)) {
                t->deleteRows(row);
                removed++;
            }
        }

        if (removed > 0) t->save();

        cout << "[OK] DELETE removed: " << removed << "\n";
        return;
    }
    if (cmd == "DROP") {
        if (!p.valid()) {
            cout << "DROP: syntax error\n";
            return;
        }

        if (CATALOG.drop(p.table())) {
            cout << "Table '" << p.table() << "' dropped successfully.\n";
        }
        else {
            cout << "DROP: table '" << p.table() << "' not found or already dropped.\n";
        }
        return;
    }



    cout << "Unsupported command: " << cmd << "\n";
}

int main() {
    cout << "Mini Dynamic DB Engine\n"
        << "--------------------------------------------------------\n"
        << "  CREATE TABLE t (name STRING, age INT)\n"
        << "  INSERT INTO t VALUES (Ali,25)\n"
        << "  SELECT name,age FROM t WHERE age > 20\n"
        << "  UPDATE t SET age = 30 WHERE name = Ali\n"
        << "  DELETE FROM t WHERE age < 18\n"
        << "  DROP TABLE t\n"
        << "  EXIT to exit from program\n"
        << "--------------------------------------------------------";

    while (true) {
        cout << "\nSQL> ";
        string line;
        if (!getline(cin, line)) 
            break;

        if (line.empty())
            continue;

        string up = toUpper(line);
        if (up == "EXIT")
            break;

        Parse p;
        p.parse(line);
        if (!p.valid()) {
            std::cout << "Invalid query or unsupported format. Examples:\n"
                << "  CREATE TABLE t (name STRING, age INT)\n"
                << "  INSERT INTO t VALUES (Ali,25)\n"
                << "  SELECT name,age FROM t WHERE age > 20\n"
                << "  UPDATE t SET age = 30 WHERE name = Ali\n"
                << "  DELETE FROM t WHERE age < 18\n"
                << "  DROP TABLE t\n";
            
            continue;
        }
        try { executeParse(p); }
        catch (const exception& ex) 
        { 
            cout << "Error: " << ex.what() << "\n";
        }
    }

    cout << "Bye\n";
    return 0;
}
