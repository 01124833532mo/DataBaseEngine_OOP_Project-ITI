#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cctype>
using namespace std;
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static inline string toUpper(const string& s) {
    string r = s;
    for (char& c : r) {
        c = (char)toupper((unsigned char)c);
    }
    return r;
}

static inline string toLower(const string& s) {
    string r = s;
    for (char& c : r)
    {
        c = (char)tolower((unsigned char)c);
    }
    return r;
}

static inline string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start]))
        ++start;

    if (start == s.size()) 
        return "";

    size_t end = s.size() - 1;

    while (end > start && isspace((unsigned char)s[end]))
        --end;

    return s.substr(start, end - start + 1);
}

static inline bool isNumberString(const string& s) {
    if (s.empty()) 
        return false;

    size_t i = 0;
    if (s[0] == '-' && s.size() > 1) 
        i = 1;

    for (; i < s.size(); ++i) {
        if (!isdigit((unsigned char)s[i])) 
            return false;

    }
    return true;
}

static inline void ensure_dir(const string& folder) {
#ifdef _WIN32
    _mkdir(folder.c_str());
#else
    mkdir(folder.c_str(), 0755);
#endif
}

#endif // UTILS_H
