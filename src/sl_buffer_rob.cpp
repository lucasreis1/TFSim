#include "sl_buffer_rob.hpp"
#include "general.hpp"

sl_buffer_rob::sl_buffer_rob(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct, nana::listbox::cat_proxy r_ct): 
sc_module(name),
tam(t),
tam_outros(t_outros),
table(lsbox)
{
    string texto;
    ptrs.resize(tam);
    auto cat = table.at(0);
    for(unsigned int i = 0 ; i < tam ; i++)
    {
        texto = "Load" + std::to_string(i+1);
        cat.append({std::to_string(cat.size()+1),texto,"False"});
        ptrs[i] = new res_station_rob(texto.c_str(),i+t_outros,texto,true,instruct_time,cat.at(i+t_outros),ct,r_ct);
        ptrs[i]->in(in_cdb);
        ptrs[i]->out(out_cdb);
        ptrs[i]->out_mem(out_mem);
    }
    SC_THREAD(leitura_issue);
    sensitive << in_issue;
    dont_initialize();
    SC_THREAD(add_rec);
    sensitive << in_adu;
    dont_initialize();
    SC_METHOD(leitura_mem);
    sensitive << in_mem;
    dont_initialize();
    SC_THREAD(leitura_rob);
    sensitive << in_rob;
    dont_initialize();
}

sl_buffer_rob::~sl_buffer_rob()
{
    for(unsigned int i = 0 ; i < tam ; i++)
        delete ptrs[i];
}

void sl_buffer_rob::leitura_issue()
{
    string p;
    vector<string> ord;
    int pos,rob_pos;
    auto cat = table.at(0);
    while(true)
    {
        /* Example input
           LD R6,0(R3) 3 1 4
           instruction - LD R6,0(R3)
           current pc general - 3
           original pc instructions - 1
           rob position - 4
           ord = {"LD", "R6", "0(R3)", "3", "1", "4"} */
        in_issue->nb_read(p);
        ord = instruction_split(p);
        cout << "Issue da instrução " << ord[0] << " no ciclo " << sc_time_stamp() << " para " << ptrs[pos]->type_name << endl << flush;
        pos = busy_check();
        while(pos == -1)
        {
            cout << "Todas as estacoes ocupadas para a instrucao " << p << " no ciclo " << sc_time_stamp() << endl << flush;
            wait(out_mem->default_event());
            pos = busy_check();
        }
        wait(SC_ZERO_TIME);
        in_issue->notify();
        out_issue->write(std::to_string(pos));
        cout << "Instrução " << p << " conseguiu espaço para usar uma estação de reserva em " << sc_time_stamp() << endl << flush;
        // Anteriormente era ord[4]
        // Acréscimo devido à informação adicional (pc_original_instruction)
        // Feita em instruction_queue_rob.cpp
        rob_pos = std::stoi(ord[ord.size() - 1]); // Pode ser ord[5], last position
        ptrs[pos]->op = ord[0];
        ptrs[pos]->instr_pos = std::stoi(ord[3]);
        cat.at(pos+tam_outros).text(OP,ord[0]);
        ptrs[pos]->dest = rob_pos;
        ptrs[pos]->Busy = true;
        cat.at(pos+tam_outros).text(BUSY,"True");
        cout << "Instrucao " << ord[0] << " aguardando o calculo de endereço" << endl << flush;
        wait();
    }
}
void sl_buffer_rob::add_rec()
{
    string p;
    vector<string> ord;
    unsigned int addr,rob_pos,chk;
    auto cat = table.at(0);
    while(true)
    {
        in_adu->read(p);
        ord = instruction_split(p);
        rob_pos = std::stoi(ord[0]);
        addr = std::stoul(ord[1]);
        for(unsigned int i = 0 ; i < tam ; i++)
        {
            if(ptrs[i]->dest == rob_pos && ptrs[i]->isFlushed == false)
            {
                cout << "Instrucao " << ptrs[i]->op << " concluiu o calculo do endereco no ciclo " << sc_time_stamp() << endl << flush;
                ptrs[i]->a = addr;
                cat.at(i+tam_outros).text(A,std::to_string(addr));
                cat.at(i+tam_outros).text(VK,"");
                chk = check_conflict(rob_pos,addr);
                if(!chk)
                    ptrs[i]->exec_event.notify(1,SC_NS);
                else
                    addr_dep[chk].push_back(i);
                break;
            }
        }
        wait();
    }
}
void sl_buffer_rob::leitura_mem()
{
    string p;
    vector<string> ord;
    unsigned int rob_pos;
    in_mem->read(p);
    rob_pos = std::stoi(p);
    if(check_find(rob_pos))
    {
        for(unsigned int i = 0 ; i < addr_dep[rob_pos].size() ; i++)
            ptrs[addr_dep[rob_pos][i]]->exec_event.notify(1,SC_NS);
        addr_dep.erase(rob_pos);
    }
}
void sl_buffer_rob::leitura_rob()
{
    string p;
    auto cat = table.at(0);
    while(true)
    {
        in_rob->nb_read(p);
        if(p == "F")
        {
            in_rob->notify();
            addr_dep.clear();
            for(unsigned int i = 0 ; i < ptrs.size() ; i++)
            {
                auto table_item = cat.at(i+tam_outros);
                if(ptrs[i]->Busy)
                {
                    ptrs[i]->isFlushed = false;
                    ptrs[i]->Busy = false;
                    table_item.text(BUSY,"False");
                    for(unsigned int k = 3 ; k < table_item.columns() ; k++)
                        table_item.text(k,"");
                    //ptrs[i]->isFlushed_event.notify();
                }
            }
        }
        wait();
    }
}

int sl_buffer_rob::busy_check()
{
    for(unsigned int i = 0 ; i < tam ; i++)
        if(ptrs[i]->Busy == false)
            return i;
    return -1;
}

int sl_buffer_rob::check_conflict(unsigned int rob_pos, unsigned int addr)
{
    string res;
    out_rob->write(std::to_string(rob_pos) + ' ' + std::to_string(addr));
    in_rob->read(res);
    return std::stoi(res);
}
bool sl_buffer_rob::check_find(unsigned int i)
{
    return (addr_dep.count(i));
}
