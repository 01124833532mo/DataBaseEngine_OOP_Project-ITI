#include <iostream>
#include <vector>
#include <string>
using namespace std;

// =====================
// Data Types
// =====================
enum DataType { INT, STRING };

// =====================
// Value class
// =====================
class Value {
public:
    DataType type;
    int intVal;
    string strVal;

    Value() : type(INT), intVal(0) {}
    Value(int v) : type(INT), intVal(v) {}
    Value(string s) : type(STRING), strVal(s) {}

    string toString() {
        if (type == INT) return to_string(intVal);
        return strVal;
    }
};

// =====================
// Column class
// =====================
class Column {
public:
    string name;
    DataType type;
    Column(string n, DataType t) : name(n), type(t) {}
};

// =====================
// Row class
// =====================
class Row {
public:
    vector<Value> values;
    Row() {}
    Row(vector<Value> v) : values(v) {}
};

// =====================
// Table class
// =====================
class Table {
public:
    string name;
    vector<Column> columns;
    vector<Row> rows;

    Table(string n) : name(n) {}

    void addColumn(string colName, DataType type) {
        columns.push_back(Column(colName, type));
    }

    void insertRow(vector<Value> vals) {
        if (vals.size() != columns.size()) {
            cout << "Error: row size mismatch\n";
            return;
        }
        rows.push_back(Row(vals));
    }

    int columnIndex(string colName) {
        for (int i = 0; i < columns.size(); i++)
            if (columns[i].name == colName) return i;
        return -1;
    }

    void printTable() {
        for (auto& c : columns) cout << c.name << "\t";
        cout << "\n-------------------\n";
        for (auto& r : rows) {
            for (auto& v : r.values) cout << v.toString() << "\t";
            cout << "\n";
        }
    }
};

// global table
Table students("students");

// =====================
// Operator base class
// =====================
class Operator {
public:
    virtual void open() = 0;
    virtual bool next() = 0;
    virtual Row getRow() = 0;
    virtual void updateRow(const Row& newRow) {}
    virtual void close() = 0;
    virtual ~Operator() {}
};

// =====================
// TableScan operator
// =====================
class TableScan : public Operator {
    Table& table;
    int idx;
public:
    TableScan(Table& t) : table(t), idx(-1) {}
    void open() override { idx = -1; }
    bool next() override 
     {
        return ++idx < table.rows.size(); 
    }
    Row getRow() override 
    { 
        return table.rows[idx];
    }
    void updateRow(const Row& newRow) override { table.rows[idx] = newRow; }

    int columnIndex(string colName) 
    {   
        int val = table.columnIndex(colName);
        return val; 
    }

    void close() override {}
};

// =====================
// Filter operator
// =====================
class Filter : public Operator {
    Operator* child;
    int colIdx;
    string op;
    string val;
    Row current;
public:
    Filter(Operator* child, Table& t, string colName, string op, string val)
        : child(child), op(op), val(val) {
        colIdx =child->columnIndex(colName);
        if (colIdx == -1) throw runtime_error("Filter: column not found");
    }

    void open() override { child->open(); }
    bool next() override {
        while (child->next()) {
            Row r = child->getRow();
            Value v = r.values[colIdx];
            bool match = false;
            if (v.type == INT) {
                int x = v.intVal;
                int y = stoi(val);
                if (op == "=") match = (x == y);
                else if (op == ">") match = (x > y);
                else if (op == "<") match = (x < y);
            }
            else {
                if (op== "=") match = (v.strVal == val);
            }
            if (match) { current = r; return true; }
        }
        return false;
    }
    Row getRow() override { return current; }
    void updateRow(const Row& newRow) override { child->updateRow(newRow); }
    void close() override { child->close(); }
};

// =====================
// Projection operator
// =====================
class Projection : public Operator {
    Operator* child;
    Table& table;
    vector<int> colIdx;
    Row current;
public:
    Projection(Operator* child, Table& t, vector<string> cols) : child(child), table(t) {
        for (auto& c : cols) {
            int idx = t.columnIndex(c);
            if (idx == -1) throw runtime_error("Projection: column not found");
            colIdx.push_back(idx);
        }
    }

    void open() override { child->open(); }
    bool next() override {
        if (!child->next()) return false;
        Row r = child->getRow();
        vector<Value> out;
        for (int idx : colIdx) out.push_back(r.values[idx]);
        current = Row(out);
        return true;
    }
    Row getRow() override { return current; }
    void close() override { child->close(); }
};

// =====================
// Update operator
// =====================
class UpdateOperator : public Operator {
    Operator* child;
    int targetIdx;
    Value newVal;
public:
    UpdateOperator(Operator* child, Table& t, string targetCol, Value newVal)
        : child(child), newVal(newVal) {
        targetIdx = t.columnIndex(targetCol);
        if (targetIdx == -1) throw runtime_error("Update: column not found");
    }

    void open() override { child->open(); }
    bool next() override {
        while (child->next()) {
            Row r = child->getRow();
            r.values[targetIdx] = newVal;
            child->updateRow(r);
        }
        return false;
    }
    Row getRow() override { return Row(); }
    void close() override { child->close(); }
};


// =========================
// Parsed Query Structure
// =========================
struct ParsedQuery {
    string type; // SELECT or UPDATE
    vector<string> select_list; // for SELECT
    string from_table;
    string where_left;
    string where_op;
    string where_right;
    Row update_values; // for UPDATE
};

// =========================
// Build Plan based on Parser
// =========================
Operator* buildPlan(const ParsedQuery& q, Table& table) {
    Operator* root = new TableScan(table);

    if (q.where_left != "")
        root = new Filter(root, q.where_left, q.where_op, q.where_right);

    if (q.type == "SELECT" && !q.select_list.empty())
        root = new Projection(root, q.select_list);

    if (q.type == "UPDATE")
        root = new UpdateOperator(root, q.update_values);

    return root;
}



// =====================
// MAIN
// =====================
int main() {
    // 1) CREATE TABLE
    students.addColumn("name", STRING);
    students.addColumn("age", INT);

    // 2) INSERT
    students.insertRow({ Value("Ali"), Value(18) });
    students.insertRow({ Value("Omar"), Value(22) });
    students.insertRow({ Value("Sara"), Value(25) });
    students.insertRow({ Value("Mona"), Value(19) });

    // 3) SELECT name, age WHERE age > 20
    TableScan scan(students);
    Filter filt(&scan, students, "age", ">", "20");
    Projection proj(&filt, students, { "name","age" });
    proj.open();
    cout << "\n== SELECT Result ==\n";
    while (proj.next()) {
        Row r = proj.getRow();
        for (auto& v : r.values) cout << v.toString() << "\t";
        cout << "\n";
    }
    proj.close();

    // 4) UPDATE age = 30 WHERE name = "Sara"
    TableScan scan2(students);
    Filter filt2(&scan2, students, "name", "=", "Sara");
    UpdateOperator upd(&filt2, students, "age", Value(30));
    upd.open();
    upd.next();
    upd.close();

    // 5) PRINT TABLE
    cout << "\n== Table after UPDATE ==\n";
    students.printTable();

    return 0;
}
