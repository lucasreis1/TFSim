#pragma once
#include "interfaces.hpp"
#include "res_station.hpp"
#include<vector>
#include<deque>
#include<nana/gui/widgets/listbox.hpp>

using std::vector;
using std::deque;

class sl_buffer: public sc_module
{
public:
    deque<res_station *> sl_buff;
    sc_port<read_if_f> in_issue;
    sc_port<read_if_f> in_rb;
    sc_port<write_if_f> out_rb;
    sc_port<write_if> out_mem;
    sc_port<read_if> in_cdb;
    sc_port<write_if> out_cdb;
    SC_HAS_PROCESS(sl_buffer);

    sl_buffer(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct);
    ~sl_buffer();
    void leitura_issue();
    void sl_buff_control();

private:
    unsigned int tam;
    unsigned int tam_outros;
    res_station **ptrs;
    nana::listbox &table;

    int busy_check();
    vector<string> offset_split(string p);
    unsigned int ask_status(bool read,string reg,unsigned int pos = 0);
    float ask_value(string reg);
};
