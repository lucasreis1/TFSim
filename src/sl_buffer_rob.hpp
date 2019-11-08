#pragma once
#include "interfaces.hpp"
#include "res_station_rob.hpp"
#include<vector>
#include<deque>
#include<map>
#include<nana/gui/widgets/listbox.hpp>

using std::vector;
using std::deque;
using std::map;

class sl_buffer_rob: public sc_module
{
public:
    sc_port<read_if_f> in_issue;
    sc_port<write_if_f> out_issue;
    sc_port<read_if> in_mem;
    sc_port<write_if> out_mem;
    sc_port<read_if> in_cdb;
    sc_port<write_if> out_cdb;
    sc_port<read_if> in_adu;
    sc_port<write_if_f> out_rob;
    sc_port<read_if_f> in_rob;

    SC_HAS_PROCESS(sl_buffer_rob);

    sl_buffer_rob(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct, nana::listbox::cat_proxy r_ct);
    ~sl_buffer_rob();
    void leitura_issue();
    void add_rec();
    void leitura_mem();
    void leitura_rob();
private:
    unsigned int tam;
    unsigned int tam_outros;
    vector<res_station_rob *>ptrs;
    nana::listbox &table;
    map<unsigned int,vector<unsigned int> >addr_dep;

    int busy_check();
    int check_conflict(unsigned int rob_pos, unsigned int addr);
    bool check_find(unsigned int i);
};
