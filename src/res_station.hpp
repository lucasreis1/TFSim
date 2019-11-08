#pragma once
#include "interfaces.hpp"
#include<nana/gui/widgets/listbox.hpp>
#include<vector>
#include<map>

using std::vector;
using std::map;

class res_station: public sc_module
{
public:
    int id;
    string type_name;
    bool Busy;
    bool isFirst;
    bool isMemory;
    bool fp;
    string op;
    float vj,vk;
    int qj,qk;
    unsigned int a;
    unsigned int instr_pos;
    map<string,int> instruct_time;
    sc_port<write_if> out;
    sc_port<read_if> in;
    sc_port<write_if> out_mem;
    sc_event exec_event;
    sc_event isFirst_event;
    SC_HAS_PROCESS(res_station);

    res_station(sc_module_name name,int i, string n,bool isMem,  map<string,int> inst_map, const nana::listbox::item_proxy item, const nana::listbox::cat_proxy c);
    void exec();
    void leitura();
    void clean_item();

private:
    string p;
    vector<string> ord;
    sc_event val_enc;
    nana::listbox::item_proxy table_item;
    const nana::listbox::cat_proxy cat;

    void mem_req(bool load,unsigned int addr,int value);
};
