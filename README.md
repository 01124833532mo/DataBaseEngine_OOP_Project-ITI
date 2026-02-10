# 🗄️ Database Engine (OOP Project - ITI)

A mini relational database management system (DBMS) built from scratch in C++ using Object-Oriented Programming principles. This project implements core database operations including SQL parsing, query execution, and persistent file-based storage.

![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B&logoColor=white)
![OOP](https://img.shields.io/badge/OOP-Design-blue)
![SQL](https://img.shields.io/badge/SQL-Parser-orange)
![ITI](https://img.shields.io/badge/ITI-Project-green)

## ✨ Features

### 🔍 **SQL Support**
- ✅ **CREATE TABLE**: Define tables with custom schema (INT, STRING types)
- ✅ **INSERT INTO**: Add records to tables with type validation
- ✅ **SELECT**: Query data with column projection and WHERE clauses
- ✅ **UPDATE**: Modify existing records with conditional filtering
- ✅ **DELETE**: Remove records based on conditions
- ✅ **DROP TABLE**: Delete tables and their data

### 🏗️ **Core Components**
- **SQL Parser**: Tokenizes and validates SQL queries
- **Catalog System**: Manages table metadata and persistence
- **Query Executor**: Implements operator-based query execution
- **File Storage**: Persistent storage in human-readable text format
- **Type System**: Support for INT and STRING data types

### 🎯 **Design Patterns**
- **Iterator Pattern**: Operator-based query execution (TableScan, Filter, Projection)
- **Catalog Pattern**: Centralized table management
- **Template Method**: Abstract Operator base class
- **Lazy Loading**: Tables loaded on-demand from disk

## 🏗️ Architecture

### Project Structure

```
DataBaseEngine_OOP_Project-ITI/
├── DB/
│   ├── main.cpp           # Entry point with REPL interface
│   ├── db.h               # Core database classes
│   │   ├── TableSchema    # Table structure definition
│   │   ├── TableDynamic   # Table operations and persistence
│   │   └── Catalog        # Database-wide table management
│   ├── parser.h           # SQL query parser
│   ├── operators.h        # Query execution operators
│   ├── utils.h            # Utility functions (toLower, trim, etc.)
│   └── Source.cpp         # Alternative implementation (demonstration)
├── DB.sln                 # Visual Studio solution
└── db/                    # Database files directory (generated)
    └── *.txt              # Table data files
```

### Class Diagram

```
┌─────────────────┐
│    Catalog      │
│─────────────────│
│ - tables_       │
│ + create()      │
│ + get()         │
│ + drop()        │
│ + has()         │
└────────┬────────┘
         │
         │ manages
         ▼
┌─────────────────┐
│  TableDynamic   │
│─────────────────│
│ - name_         │
│ - schema_       │
│ - rows_         │
│ + insertRow()   │
│ + updateRows()  │
│ + deleteRows()  │
│ + save()        │
│ + load()        │
└────────┬────────┘
         │
         │ uses
         ▼
┌─────────────────┐
│   TableSchema   │
│─────────────────│
│ + names[]       │
│ + types[]       │
└─────────────────┘

┌─────────────────┐
│    Operator     │ (Abstract)
│─────────────────│
│ + open()        │
│ + next()        │
│ + getRow()      │
│ + close()       │
└────────┬────────┘
         │
         ├─────────────────┬─────────────────┐
         │                 │                 │
┌────────▼────────┐ ┌──────▼──────┐ ┌───────▼────────┐
│   TableScan     │ │   Filter    │ │  Projection    │
│─────────────────│ │─────────────│ │────────────────│
│ Scan all rows   │ │ WHERE cond. │ │ Select columns │
└─────────────────┘ └─────────────┘ └────────────────┘
```

## 🚀 Getting Started

### Prerequisites

- **Compiler**: C++17 or higher (for `<filesystem>`)
- **IDE**: Visual Studio 2019/2022 (recommended) or any C++ compiler
- **OS**: Windows, Linux, or macOS

### Compilation

#### Using Visual Studio
```bash
1. Open DB.sln
2. Build Solution (Ctrl+Shift+B)
3. Run (F5 or Ctrl+F5)
```

#### Using g++ (Linux/macOS)
```bash
cd DB
g++ -std=c++17 main.cpp -o db_engine
./db_engine
```

#### Using g++ (Windows)
```bash
cd DB
g++ -std=c++17 main.cpp -o db_engine.exe
db_engine.exe
```

## 💻 Usage

### Interactive SQL Shell

When you run the program, you'll see:

```
Mini Dynamic DB Engine
--------------------------------------------------------
  CREATE TABLE t (name STRING, age INT)
  INSERT INTO t VALUES (Ali,25)
  SELECT name,age FROM t WHERE age > 20
  UPDATE t SET age = 30 WHERE name = Ali
  DELETE FROM t WHERE age < 18
  DROP TABLE t
  EXIT to exit from program
--------------------------------------------------------

SQL>
```

### Example Session

```sql
SQL> CREATE TABLE students (name STRING, age INT, grade STRING)
[OK] Created students

SQL> INSERT INTO students VALUES (Ali, 20, A)
[OK] Inserted 1 row

SQL> INSERT INTO students VALUES (Sara, 22, B)
[OK] Inserted 1 row

SQL> INSERT INTO students VALUES (Omar, 19, A)
[OK] Inserted 1 row

SQL> SELECT name, age FROM students WHERE age > 19
name    age
-------------------
Ali     20
Sara    22

SQL> UPDATE students SET grade = A+ WHERE name = Ali
[OK] Updated 1 row(s)

SQL> DELETE FROM students WHERE age < 20
[OK] Deleted 1 row(s)

SQL> DROP TABLE students
[OK] Dropped table students

SQL> EXIT
```

## 📝 SQL Syntax

### CREATE TABLE
```sql
CREATE TABLE table_name (column1 TYPE, column2 TYPE, ...)
```
- **Types**: `INT`, `STRING`
- **Example**: `CREATE TABLE users (id INT, name STRING)`

### INSERT INTO
```sql
INSERT INTO table_name VALUES (val1, val2, ...)
```
- Values must match column types
- **Example**: `INSERT INTO users VALUES (1, John)`

### SELECT
```sql
SELECT col1, col2 FROM table_name WHERE column op value
```
- **Operators**: `=`, `>`, `<`, `>=`, `<=`, `!=`
- **Example**: `SELECT name FROM users WHERE id > 5`

### UPDATE
```sql
UPDATE table_name SET column = value WHERE column op value
```
- **Example**: `UPDATE users SET name = Jane WHERE id = 1`

### DELETE
```sql
DELETE FROM table_name WHERE column op value
```
- **Example**: `DELETE FROM users WHERE age < 18`

### DROP TABLE
```sql
DROP TABLE table_name
```
- **Example**: `DROP TABLE users`

## 🗂️ File Storage Format

Tables are stored in `./db/` directory as text files:

**Example: `students.txt`**
```
3                          # Number of columns
name STRING                # Column 1: name, type STRING
age INT                    # Column 2: age, type INT
grade STRING               # Column 3: grade, type STRING
1 Ali 20 A                # Row 1 (ID, values...)
2 Sara 22 B               # Row 2
3 Omar 19 A               # Row 3
```

## 🧩 Core Classes

### 1. **Catalog**
Manages all tables in the database:
- `create(name, schema)`: Create new table
- `get(name)`: Retrieve table by name
- `has(name)`: Check if table exists
- `drop(name)`: Delete table
- `registerExistingAll()`: Load all tables from disk

### 2. **TableDynamic**
Represents a single table:
- `insertRow(values)`: Add new row
- `updateRows(predicate, colIdx, newVal)`: Update rows
- `deleteRows(predicate)`: Delete rows
- `save()`: Persist to disk
- `load()`: Load from disk

### 3. **Parse**
SQL query parser:
- `parse(query)`: Tokenize and validate SQL
- `parseCreate()`, `parseInsert()`, `parseSelect()`, etc.
- `valid()`: Check if query is valid

### 4. **Operators** (Query Execution)
- **TableScan**: Iterate over all rows
- **Filter**: Apply WHERE conditions
- **Projection**: Select specific columns

## 🎓 OOP Principles Applied

- ✅ **Encapsulation**: Private members with public interfaces
- ✅ **Abstraction**: Abstract `Operator` class
- ✅ **Inheritance**: Operator subclasses (TableScan, Filter, Projection)
- ✅ **Polymorphism**: Virtual functions in Operator hierarchy
- ✅ **Composition**: Catalog contains TableDynamic instances
- ✅ **RAII**: Automatic file handling with fstream

## 🧪 Testing

### Basic Tests

1. **Table Creation**
```sql
CREATE TABLE test (id INT, name STRING)
```

2. **Data Insertion**
```sql
INSERT INTO test VALUES (1, Alice)
INSERT INTO test VALUES (2, Bob)
```

3. **Query Execution**
```sql
SELECT * FROM test WHERE id > 0
```

4. **Update Operation**
```sql
UPDATE test SET name = Charlie WHERE id = 1
```

5. **Delete Operation**
```sql
DELETE FROM test WHERE id = 2
```

6. **Table Deletion**
```sql
DROP TABLE test
```

## 🔧 Technical Details

### Data Types
- **INT**: Stored as string, converted for numeric operations
- **STRING**: Raw string storage

### Comparison Operators
- Numeric: `=`, `>`, `<`, `>=`, `<=`, `!=`
- String: `=` (exact match)

### File I/O
- Tables auto-save after INSERT, UPDATE, DELETE
- Lazy loading: Tables loaded when first accessed
- Directory auto-creation (`./db/`)

## 🚧 Limitations & Future Enhancements

### Current Limitations
- No JOIN operations
- No aggregate functions (COUNT, SUM, AVG)
- No indexes (linear scan only)
- No transactions or concurrency control
- No NULL values support
- No primary/foreign keys

### Planned Features
- [ ] JOIN support (INNER, LEFT, RIGHT)
- [ ] Aggregate functions
- [ ] B-tree indexing
- [ ] Transaction support (ACID)
- [ ] Multi-threaded query execution
- [ ] Query optimization
- [ ] Network protocol (client-server)
- [ ] More data types (FLOAT, DATE, BOOL)

## 🤝 Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/NewFeature`)
3. Commit your changes (`git commit -m 'Add NewFeature'`)
4. Push to the branch (`git push origin feature/NewFeature`)
5. Open a Pull Request

## 📚 Learning Resources

This project demonstrates:
- SQL parsing and execution
- File-based database storage
- Iterator pattern for query execution
- Object-oriented database design
- C++ STL usage (`vector`, `unordered_map`, `fstream`)
- Modern C++ features (`<filesystem>`, `unique_ptr`)

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

## 👨‍💻 Author

**01124833532mo**
- GitHub: [@01124833532mo](https://github.com/01124833532mo)
- Project: ITI Database Engine Assignment

## 🏆 Acknowledgments

- **ITI (Information Technology Institute)**: Project assignment
- **OOP Course**: Advanced object-oriented programming concepts
- **Database Systems**: Foundational database theory

---

⭐ **Star this repository** if you found it helpful for learning database internals!

💡 **Have questions?** Open an issue or reach out!

📖 **Want to learn more?** Check out the code documentation in header files.

---

**Built with ❤️ using C++ and OOP principles**
