#pragma once
#include "interfaces.hpp"
#include<nana/gui/widgets/listbox.hpp>
#include<vector>
#include<map>

using std::vector;
using std::map;

class res_station_rob: public sc_module
{
public:
    int id;
    unsigned int dest;
    string type_name;
    bool Busy;
    bool isFlushed;
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
    sc_event exec_event,isFlushed_event;
    SC_HAS_PROCESS(res_station_rob);

    res_station_rob(sc_module_name name,int i, string n, bool isMem, map<string,int> inst_map, const nana::listbox::item_proxy item, const nana::listbox::cat_proxy c, const nana::listbox::cat_proxy rgui);
    void exec();
    void leitura();
    void clean_item();

private:
    string p;
    vector<string> ord;
    sc_event val_enc;
    nana::listbox::item_proxy table_item;
    const nana::listbox::cat_proxy instr_queue_gui;
    const nana::listbox::cat_proxy rob_gui;

    void mem_req(bool load,unsigned int addr,int value);
};
