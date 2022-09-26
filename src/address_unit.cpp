#include "address_unit.hpp"
#include "general.hpp"

address_unit::address_unit(sc_module_name name,unsigned int t, nana::listbox::cat_proxy instr_t, nana::listbox::cat_proxy rst_t, int rst_tm):
sc_module(name),
delay_time(t),
instruct_table(instr_t),
res_station_table(rst_t),
rst_tam(rst_tm)
{
    SC_THREAD(leitura_issue);
    sensitive << in_issue;
    dont_initialize();
    SC_THREAD(leitura_cdb);
    sensitive << in_cdb;
    dont_initialize();
    SC_THREAD(addr_issue);
    sensitive << addr_queue_event;
    dont_initialize();
    SC_THREAD(leitura_rob);
    sensitive << in_rob;
    dont_initialize();
}

void address_unit::leitura_issue()
{
    bool store;
    bool check_value;
    int value;
    while(true)
    {
        /* Example input
           LD R6,0(R3) 3 2 4 0
           instruction - LD R6,0(R3)
           current pc general - 3
           original pc instructions - 2
           rob position - 4
           rst_pos - 0
           ord = {"LD", "R6", "0(R3)", "3", "1", "4", "0"} */
        in_issue->read(p);
        ord = instruction_split(p);
        wait(sc_time(1,SC_NS));
        mem_ord = offset_split(ord[2]);
        a = std::stoi(mem_ord[0]);
        instr_pos = std::stoi(ord[3]);
        rob_pos = std::stoi(ord[5]);
        regst = ask_status(true,mem_ord[1]);
        check_value = false;
        if(regst != 0)
        {
            string check = ask_rob_value(std::to_string(regst));
            if(check != "EMPTY")
            {
                value = std::stof(check);
                check_value = true;
            }   
        }
        if(ord[0].at(0) == 'S')
            store = true;
        else
            store = false;
        if(!store)
        {
            // Anteriormente era ord[5]
            // Acréscimo devido à informação adicional (pc_original_instruction)
            // Feita em instruction_queue_rob.cpp
            rst_pos = std::stoi(ord[6]);
            res_station_table.at(rst_pos+rst_tam).text(A,mem_ord[0]);
        }
        else
            rst_pos = -1;
        if(regst == 0 || check_value == true)
        {
            if(check_value == false)
                value = ask_value(mem_ord[1]);
            wait(SC_ZERO_TIME);
            instruct_table.at(instr_pos).text(EXEC,std::to_string(sc_time_stamp().value() / 1000)); //text(EXEC,"X");
            a += value;
            if(store)
            {
                if(addr_queue.empty())
                    addr_queue_event.notify(delay_time,SC_NS);
                addr_queue.push({store,true,regst,rob_pos,instr_pos,rst_pos,a});
            }
            else
            {
                offset_buff.push_back({store,true,regst,rob_pos,instr_pos,rst_pos,a});
                check_loads();
            }
        }
        else
        {
            offset_buff.push_back({store,false,regst,rob_pos,instr_pos,rst_pos,a});
            if(!store)
                res_station_table.at(rst_pos+rst_tam).text(QK,std::to_string(regst));
            cout << "Instrucao " << ord[0] << " aguardando o resultado do ROB " << regst << endl << flush;
        }
        wait();
    }
}

void address_unit::leitura_cdb()
{
    string p_c;
    vector<string> ord_c;
    while(true)
    {
        in_cdb->read(p_c);
        ord_c = instruction_split(p_c);
        bool found = false;
        for(unsigned int i = 0 ; i < offset_buff.size() ; i++)
        {
            if(std::stoi(ord_c[0]) == offset_buff[i].regst)
            {
                offset_buff[i].a+= std::stoi(ord_c[1]);
                wait(SC_ZERO_TIME);
                instruct_table.at(offset_buff[i].instr_pos).text(EXEC,std::to_string(sc_time_stamp().value() / 1000)); //text(EXEC,"X")
                if(offset_buff[i].store)
                {
                    if(addr_queue.empty())
                        addr_queue_event.notify(delay_time,SC_NS);
                    addr_queue.push(offset_buff[i]);
                    cout << "Instrucao no ROB " << offset_buff[i].rob_pos << " obteve o resultado do ROB " << ord_c[0] << endl;
                    offset_buff.erase(offset_buff.begin() + i);
                    i--;
                }
                else
                {
                    res_station_table.at(offset_buff[i].rst_pos+rst_tam).text(QK,"");
                    res_station_table.at(offset_buff[i].rst_pos+rst_tam).text(VK,ord_c[1]);
                    offset_buff[i].addr_calc = true;
                    cout << "Instrucao no ROB " << offset_buff[i].rob_pos << " obteve o resultado do ROB " << ord_c[0] << endl;
                }
                found = true;
            }
        }
        if(found)
            check_loads();
    wait();
    }
}

void address_unit::addr_issue()
{
    addr_node fr;
    while(true)
    {
        while(addr_queue.empty())
            wait(addr_queue_event);
        fr = addr_queue.front();
        if(fr.store)
            out_rob->write(std::to_string(fr.rob_pos) + ' ' + std::to_string(fr.a));
        else
            out_slbuff->write(std::to_string(fr.rob_pos) + ' ' + std::to_string(fr.a));
        addr_queue.pop();
        wait(1,SC_NS);
    }
}

void address_unit::leitura_rob()
{
    string p;
    while(1)
    {
        in_rob->read(p);
        if(p == "F")
        {
            queue<addr_node> empty;
            std::swap(addr_queue,empty);
            offset_buff.clear();
        }
        wait();
    }
}

string address_unit::ask_rob_value(string rob_pos)
{
    string res;
    out_rob_svl->write(rob_pos);
    in_rob_svl->nb_read(res);
    while(res == "F")
        wait(out_rob->default_event());
    in_rob_svl->notify();
    return res;
}

vector<string> address_unit::offset_split(string p)
{
    unsigned int i;
    vector<string> ord(2);
    for(i = 0 ; i < p.size() && p[i] != '(';i++)
        ;
    ord[0] = p.substr(0,i);
    ord[1] = p.substr(i+1,p.size()-i-2);
    return ord;
}

float address_unit::ask_value(string reg)
{
    string res;
    out_rb->write("R V " + reg);
    in_rb->read(res);
    return std::stof(res);
}
unsigned int address_unit::ask_status(bool read,string reg,unsigned int pos)
{
    string res;
    if(read)
    {
        out_rb->write("R S " + reg);
        in_rb->read(res);
        return std::stoi(res);
    }
    else
        out_rb->write("W S " + reg + " " + std::to_string(pos));
    return 0;
}
void address_unit::check_loads()
{
    for(unsigned i = 0 ; i < offset_buff.size() && !offset_buff[i].store ; i++)
        if(offset_buff[i].addr_calc)
        {
            instruct_table.at(offset_buff[i].instr_pos).text(EXEC,std::to_string(sc_time_stamp().value() / 1000)); //text(EXEC,"X")
            if(addr_queue.empty())
                addr_queue_event.notify(delay_time,SC_NS);
            addr_queue.push(offset_buff[i]);
            offset_buff.erase(offset_buff.begin()+i);
            i--;
        }
}
