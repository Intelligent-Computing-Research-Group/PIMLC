#include "pimconfig.h"
#include "iofile.h"
#include <cstdio>
#include <fstream>
#include <iostream>


using namespace std;

PIMConf &pcfg = PIMConf::getInstance();

void PIMConf::setGlobalPIMConf(const char *configfile)
{
    PIMConf &ins = getInstance();
    string line;
    fstream infile(configfile);
    if (!infile.is_open()) {
        printf("Config file %s open failed!\n", configfile);
        exit(-1);
    }
    string lhs, rhs;
    while (!infile.eof()) {
        getline(infile, line);
        if (parseline(line, lhs, rhs) == 0 && ins.options.count(lhs)) {
            ins.options[lhs](rhs);
        }
    }

    ins._block_rows = ins._level0_rows;
    ins._block_cols = ins._level0_cols;
    ins._cycle_time = (bigint)(0.0001 + 1000.0/ins._frequency);
    if (ins._latency_roundup) {
        ins._compute_latency = roundup(ins._compute_latency,ins._cycle_time);
        for (int i = 0; i < ins._levels; ++i) {
            ins._read_latency[i] = roundup(ins._read_latency[i],ins._cycle_time);
            ins._write_latency[i] = roundup(ins._write_latency[i],ins._cycle_time);
        }
        ins._load_latency = roundup(ins._load_latency,ins._cycle_time);
    }
    ins._block_rows = ins._level0_rows;
    ins._block_cols = ins._level0_cols;
    ins._block_nums = 1;
    for (int i = 0; i < ins._levels; ++i) {
        ins._block_nums *= ins._level_size[i];
        ins._block_cols *= i<=ins._schedule_level ? ins._level_size[i] : 1;
        ins._maxCopyThread[i] = i==0 ? ins.block_nums/ins._level_size[i] : ins._maxCopyThread[i-1]/ins._level_size[i];
        ins._commWeight[i] = ins._read_latency[i] + ins._write_latency[i];
    }
    ins._commWeight[ins._levels] = ins._load_latency + ins._store_latency;
}

void PIMConf::printPIMConf()
{
    PIMConf &ins = getInstance();
    auto traverse = [&ins](const auto &container) -> char 
    {
        for(int i=0;i<ins._levels;++i) cout << container[i] << ' ';
        return '\n';
    };
    cout << "-------- PIM Setting --------\n";

    cout << "name = " << ins._name << "\n";
    cout << "frequency = " << ins._frequency << "\n";
    cout << "latency-roundup = " << ins._latency_roundup << "\n";
    cout << "cycle-time = " << ins._cycle_time << "\n";
    cout << "levels = " << ins._levels << "\n";
    cout << "level-size = " << traverse(ins._level_size);
    cout << "level-name = " << traverse(ins._level_name);
    cout << "schedule-level = " << ins._schedule_level << "\n";
    cout << "smallest-unit-cols = " << ins._level0_cols << "\n";
    cout << "smallest-unit-rows = " << ins._level0_rows << "\n";

    cout << "block-nums = " << ins._block_nums << "\n";
    cout << "block-cols = " << ins._block_cols << "\n";
    cout << "block-rows = " << ins._block_rows << "\n";

    cout << "compute-latency = " << ins._compute_latency << "\n";
    cout << "compute-energy = " << ins._compute_energy << "\n";
    cout << "read-latency = " << traverse(ins._read_latency);
    cout << "write-latency = " << traverse(ins._write_latency);
    cout << "read-energy = " << traverse(ins._read_energy);
    cout << "write-energy = " << traverse(ins._write_energy);

    cout << "load-latency = " << ins._load_latency << "\n";
    cout << "store-latency = " << ins._store_latency << "\n";
    cout << "load-energy = " << ins._load_energy << "\n";
    cout << "store-energy = " << ins._store_energy << "\n";
    cout << "leackage-energy = " << ins._leackage_energy << "\n";
}


PIMConf::PIMConf()
{
    _name = "PIM";
    _frequency = 1.0;
    _cycle_time = 0;
    _latency_roundup = true;
    _block_cols = 0;
    _block_rows = 0;
    _block_nums = 0;
    _levels = 0;

    _read_latency[0] = 0;
    _write_latency[0] = 0;
    _read_energy[0] = 0.0;
    _write_energy[0] = 0.0;
    for (int i = 0; i < MAXPIMLEVEL; ++i) {
        _level_size[i] = 0;
        _level_name[i] = i?"Level-"+to_string(i):"Block";
        _read_latency[i] = 0;
        _write_latency[i] = 0;
        _read_energy[i] = 0.0;
        _write_energy[i] = 0.0;
    }


    _load_latency = 0;
    _store_latency = 0;
    _load_energy = 0.0;
    _store_energy = 0.0;
    _compute_latency = 0;
    _compute_energy = 0.0;

    _leackage_energy = 0.0;
    initFunctionMap();
}

static inline void getstr(const string &s, string &res, int &t) {
    SKIPSTRINGSPACES(s,t);
    res = "";
    size_t len = s.length();
    while (t<len && s[t]!='\0' && s[t]!=' ' && s[t]!='\t' && s[t]!='\r' && s[t]!='\n') {
        res += s[t++];
    }
};

void PIMConf::initFunctionMap()
{

    options["name"] = [&](string &s) { this->_name = removespace(s); };
    options["frequency"] = [&](string &s) { this->_frequency= stod(s); };
    options["latency-roundup"] = [&](string &s) { this->_latency_roundup= (bool)(stoi(s)); };
    options["levels"] = [&](string &s) { this->_levels= stoi(s); };
    options["level-size"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len) {
            if (t < len) {
                int num = 0;
                do {
                    num = num*10 + (s[t++]-'0');
                } while (t<len && s[t]<='9' && s[t]>='0');
                this->_level_size[level++] = num;
            }
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["level-name"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len && s[t]!= '\0') {
            if (t < len && s[t]!= '\0') {
                this->_level_name[level] = "";
                do {
                    this->_level_name[level] += s[t++];
                } while (t<len && s[t]!=' ' && s[t]!='\t' && s[t]!= '\0' && s[t]!='\r' && s[t]!='\n');
                ++level;
            }
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["schedule-level"] = [&](string &s) { this->_schedule_level= stoi(s); };
    options["smallest-unit-cols"] = [&](string &s) { this->_level0_cols= stoi(s); };
    options["smallest-unit-rows"] = [&](string &s) { this->_level0_rows= stoi(s); };
    options["compute-latency"] = [&](string &s) { this->_compute_latency = stoll(s); };
    options["compute-energy"] = [&](string &s) { this->_compute_energy = stod(s); };
    options["read-latency"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len && s[t]!= '\0') {
            string num;
            getstr(s, num, t);
            _read_latency[level++] = stoll(num);
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["write-latency"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len && s[t]!= '\0') {
            string num;
            getstr(s, num, t);
            _write_latency[level++] = stoll(num);
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["read-energy"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len && s[t]!= '\0') {
            string num;
            getstr(s, num, t);
            _read_energy[level++] = stod(num);
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["write-energy"] = [&](string &s) {
        size_t len = s.length();
        int t = 0, level = 0;
        SKIPSTRINGSPACES(s,t);
        while (t < len && s[t]!= '\0') {
            string num;
            getstr(s, num, t);
            _write_energy[level++] = stod(num);
            SKIPSTRINGSPACES(s,t);
        }
    };
    options["load-latency"] = [&](string &s) { this->_load_latency = stoll(s); };
    options["store-latency"] = [&](string &s) { this->_store_latency = stoll(s); };
    options["load-energy"] = [&](string &s) { this->_load_energy = stod(s); };
    options["store-energy"] = [&](string &s) { this->_store_energy = stod(s); };
    options["leackage-energy"] = [&](string &s) { this->_leackage_energy = stod(s); };
}
