#include "sl_buffer.hpp"
#include "general.hpp"

sl_buffer::sl_buffer(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct): 
sc_module(name),
tam(t),
tam_outros(t_outros),
table(lsbox)
{
    string texto;
    ptrs = new res_station*[tam];
    auto cat = table.at(0);
    for(unsigned int i = 0 ; i < tam ; i++)
    {
        texto = "Load" + std::to_string(i+1);
        cat.append({std::to_string(cat.size()+1),texto,"False"});
        ptrs[i] = new res_station(texto.c_str(),i+t_outros+1,texto,true,instruct_time,cat.at(i+t_outros),ct);
        ptrs[i]->in(in_cdb);
        ptrs[i]->out(out_cdb);
        ptrs[i]->out_mem(out_mem);
    }
    SC_THREAD(leitura_issue);
    sensitive << in_issue;
    dont_initialize();
    SC_METHOD(sl_buff_control);
    sensitive << out_mem;
    dont_initialize();
}
sl_buffer::~sl_buffer()
{
    for(unsigned int i = 0 ; i < tam ; i++)
        delete ptrs[i];
    delete ptrs;
}

void sl_buffer::leitura_issue()
{
    string p;
    vector<string> ord,mem_ord;
    int pos;
    int regst;
    float value;
    auto cat = table.at(0);
    while(true)
    {
        in_issue->nb_read(p);
        ord = instruction_split(p);
        pos = busy_check();
        while(pos == -1)
        {
            cout << "Todas as estacoes ocupadas para a instrucao " << p << " no ciclo " << sc_time_stamp() << endl << flush;
            wait(out_mem->default_event());
            pos = busy_check();
            if(pos != -1)
                wait(1,SC_NS);
        }
        in_issue->notify();
        wait(SC_ZERO_TIME);
        cout << "Issue da instrução " << ord[0] << " no ciclo " << sc_time_stamp() << " para " << ptrs[pos]->type_name << endl << flush;
        ptrs[pos]->op = ord[0];
        ptrs[pos]->instr_pos = std::stoi(ord[3]);
        cat.at(pos+tam_outros).text(OP,ord[0]);
        mem_ord = offset_split(ord[2]);
        if(ord[0].at(0) == 'L')
            ask_status(false,ord[1],ptrs[pos]->id);
        else
        {
            regst = ask_status(true,ord[1]);
            if(regst == 0)
            {
                value = ask_value(ord[1]);
                ptrs[pos]->vj = value;
                cat.at(pos+tam_outros).text(VJ,std::to_string((int)value));
            }
            else
            {
                cout << "instruçao " << ord[0] << " aguardando reg " << ord[1] << endl << flush;
                ptrs[pos]->qj = regst;
                cat.at(pos+tam_outros).text(QJ,std::to_string(regst));
            }
        }
        regst = ask_status(true,mem_ord[1]);
        if(regst == 0)
        {
            value = ask_value(mem_ord[1]);
            ptrs[pos]->vk = value;
            cat.at(pos+tam_outros).text(VK,std::to_string((int)value));
        }
        else
        {
            cout << "instruçao " << ord[0] << " aguardando reg " << mem_ord[1] << endl << flush;
            ptrs[pos]->qk = regst;
            cat.at(pos+tam_outros).text(QK,std::to_string(regst));
        }
        ptrs[pos]->a = std::stoi(mem_ord[0]);
        ptrs[pos]->Busy = true;
        cat.at(pos+tam_outros).text(A,mem_ord[0]);
        cat.at(pos+tam_outros).text(BUSY,"True");
        if(sl_buff.empty())
            ptrs[pos]->isFirst = true;
        sl_buff.push_back(ptrs[pos]);
        ptrs[pos]->exec_event.notify(1,SC_NS);
        wait();
    }
}

void sl_buffer::sl_buff_control()
{
    sl_buff.pop_front();
    if(!sl_buff.empty())
    {
        sl_buff[0]->isFirst = true;
        sl_buff[0]->isFirst_event.notify(1,SC_NS);
    }
}

int sl_buffer::busy_check()
{
    for(unsigned int i = 0 ; i < tam ; i++)
        if(ptrs[i]->Busy == false)
            return i;
    return -1;
}

vector<string> sl_buffer::offset_split(string p)
{
    unsigned int i;
    vector<string> ord(2);
    for(i = 0 ; i < p.size() && p[i] != '(';i++)
        ;
    ord[0] = p.substr(0,i);
    ord[1] = p.substr(i+1,p.size()-i-2);
    return ord;
}

unsigned int sl_buffer::ask_status(bool read,string reg,unsigned int pos)
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
float sl_buffer::ask_value(string reg)
{
    string res;
    out_rb->write("R V " + reg);
    in_rb->read(res);
    return std::stof(res);
}
