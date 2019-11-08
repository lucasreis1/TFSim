#include <nana/gui.hpp>
#include "reorder_buffer.hpp"

reorder_buffer::reorder_buffer(sc_module_name name,unsigned int sz,unsigned int pred_size, nana::listbox &gui, nana::listbox::cat_proxy instr_gui): 
sc_module(name),
tam(sz),
preditor(pred_size),
gui_table(gui),
instr_queue_gui(instr_gui)
{
    last_rob = 0;
    branch_instr = {{"BEQ",0},{"BNE",1},{"BGTZ",2},{"BLTZ",3},{"BGEZ",4},{"BLEZ",5}};
    ptrs = new rob_slot*[tam];
    for(unsigned int i = 0 ; i < tam ; i++)
    {
        ptrs[i] = new rob_slot(i+1);
        gui_table.at(0).append({std::to_string(i+1),"False","","","",""});
    }
    SC_THREAD(leitura_issue);
    sensitive << in_issue;
    dont_initialize();
    SC_THREAD(new_rob_head);
    sensitive << new_rob_head_event;
    dont_initialize();
    SC_THREAD(leitura_cdb);
    sensitive << in_cdb;
    dont_initialize();
    SC_THREAD(leitura_adu);
    sensitive << in_adu;
    dont_initialize();
    SC_THREAD(value_check);
    sensitive << in_resv_adu;
    dont_initialize();
    SC_THREAD(check_conflict);
    sensitive << in_slb;
    dont_initialize();
}

reorder_buffer::~reorder_buffer()
{
    for(unsigned int i = 0 ; i < tam ; i++)
        delete ptrs[i];
    delete ptrs;
}

void reorder_buffer::leitura_issue()
{
    string p;
    string inst;
    vector<string> ord;
    int pos,regst;
    float value;
    bool check_value;
    auto cat = gui_table.at(0);
    while(true)
    {
        pos = busy_check();
        if(ptrs[pos]->busy)
        {
            cout << "ROB esta totalmente ocupado" << endl << flush;
            wait(free_rob_event);
        }
        in_issue->read(p);
        out_issue->write(std::to_string(pos+1));
        ord = instruction_split(p);
        inst = p.substr(0,instruction_pos_finder(p));
        cout << "Inserindo instrucao " << p << " no ROB " << pos+1 <<"|" << sc_time_stamp() << endl << flush;
        ptrs[pos]->busy = true;
        cat.at(pos).text(R_BUSY,"True");
        cat.at(pos).text(DESTINATION,"");
        cat.at(pos).text(VALUE,"");
        ptrs[pos]->ready = false;
        ptrs[pos]->instruction = ord[0];
        cat.at(pos).text(INSTRUCTION,inst);
        ptrs[pos]->state = ISSUE;
        cat.at(pos).text(STATE,"Issue");
        ptrs[pos]->instr_pos = std::stoi(ord[ord.size()-1]);
        if(ord[0].at(0) == 'S')
        {
            check_value = false;
            regst = ask_status(true,ord[1]);
            if(regst != 0)
            {
                if(ptrs[regst-1]->ready == true)
                {
                    value = std::stof(cat.at(regst-1).text(VALUE));
                    check_value = true;
                }
            }
            if(regst == 0 || check_value == true)
            {
                if(check_value == false)
                    value = ask_value(true,ord[1]);
                ptrs[pos]->value = value;
                if(ord[1].at(0) != 'F')
                    cat.at(pos).text(VALUE,std::to_string((int)value));
                else
                    cat.at(pos).text(VALUE,std::to_string(value));
                ptrs[pos]->qj = 0;
            }
            else
                ptrs[pos]->qj = regst;
        }
        else if(ord[0].at(0) == 'B')
        {
            ptrs[pos]->destination = ord[1];
            regst = ask_status(true,ord[1]);
            check_value = false;
            if(regst != 0)
            {
                if(ptrs[regst-1]->ready == true)
                {
                    value = std::stof(cat.at(regst-1).text(VALUE));
                    check_value = true;
                }
            }
            if(regst == 0 || check_value == true)
            {
                if(check_value == false)
                    value = ask_value(true,ord[1]);
                ptrs[pos]->vj = value;
            }
            else
                ptrs[pos]->qj = regst;
            if(branch_instr[ord[0]] < 2) //instrucao com 2 operandos (BEQ,BNE)
            {
                regst = ask_status(true,ord[2]);
                check_value = false;
                if(regst != 0)
                {
                    if(ptrs[regst-1]->ready == true)
                    {
                        value = std::stof(cat.at(regst-1).text(VALUE));
                        check_value = true;
                    }
                }
                if(regst == 0 || check_value == true)
                {
                    if(check_value == false)
                        value = ask_value(true,ord[2]);
                    ptrs[pos]->vk = value;
                }
                else
                    ptrs[pos]->qk = regst;
                ptrs[pos]->destination = ord[3];
                cat.at(pos).text(DESTINATION,ord[3]);
            }
            else
            {
                cat.at(pos).text(DESTINATION,ord[2]);
                ptrs[pos]->destination = ord[2];
            }
            ptrs[pos]->prediction = preditor.predict();
            if(preditor.predict())
                out_iq->write("S " + std::to_string(ptrs[pos]->entry) +  ' ' + ptrs[pos]->destination);
            else
                out_iq->write("S " + std::to_string(ptrs[pos]->entry));
            if(ptrs[pos]->qj == 0 && ptrs[pos]->qk == 0)
                ptrs[pos]->ready = true;
        }
        else
        {
            ptrs[pos]->destination = ord[1];
            cat.at(pos).text(DESTINATION,ord[1]);
            if(ord[0].at(0) != 'L')
                wait(resv_read_oper_event);
            ask_status(false,ord[1],pos+1);
        }
        if(rob_buff.empty())
            new_rob_head_event.notify(1,SC_NS);
        rob_buff.push_back(ptrs[pos]);
        wait();
    }
}

void reorder_buffer::new_rob_head() 
{
    unsigned int instr_type;
    bool pred;
    auto cat = gui_table.at(0);
    while(true)
    {
        if(rob_buff.empty())
            wait(new_rob_head_event);
        if(!rob_buff[0]->ready)
            wait(rob_head_value_event);
        rob_buff[0]->state = COMMIT;
        wait(SC_ZERO_TIME);
        cat.at(rob_buff[0]->entry-1).text(STATE,"Commit");
        if(rob_buff[0]->instruction.at(0) == 'S')
            mem_write(std::stoi(rob_buff[0]->destination),rob_buff[0]->value,rob_buff[0]->entry);
        else if(rob_buff[0]->instruction.at(0) == 'B')
        {
            instr_queue_gui.at(rob_buff[0]->instr_pos).text(EXEC,"X");
            instr_queue_gui.at(rob_buff[0]->instr_pos).text(WRITE,"X");
            instr_type = branch_instr[rob_buff[0]->instruction];
            if(instr_type < 2)
                pred = branch(instr_type,rob_buff[0]->vj,rob_buff[0]->vk);
            else
                pred = branch(instr_type,(float)rob_buff[0]->vj);
            if(pred != rob_buff[0]->prediction)
            {
                if(pred)
                    out_iq->write(rob_buff[0]->destination + ' ' + std::to_string(rob_buff[0]->entry));
                else
                    out_iq->write("R " + std::to_string(rob_buff[0]->entry));
                cout << "-----------------LIMPANDO ROB no ciclo " << sc_time_stamp() << " -----------------" << endl << flush;
                _flush(); //Esvazia o ROB
                out_resv_adu->write("F");
                out_slb->write("F");
                out_rb->write("F");
                out_adu->write("F");
            }
            preditor.update_state(pred);
        }
        else
        {
            wait(SC_ZERO_TIME);
            unsigned int regst = ask_status(true,rob_buff[0]->destination);
            ask_value(false,rob_buff[0]->destination,rob_buff[0]->value);
            if(regst == rob_buff[0]->entry)
                ask_status(false,rob_buff[0]->destination,0);
        }
        if(!rob_buff.empty())
        {
            rob_buff[0]->busy = false;
            cat.at(rob_buff[0]->entry-1).text(R_BUSY,"False");
            rob_buff[0]->ready = false;
            rob_buff[0]->destination = "";
            rob_buff[0]->qj = rob_buff[0]->qk = 0;
            cout << "Commit da instrucao " << rob_buff[0]->instruction << " com valor " << rob_buff[0]->value << " no ciclo " << sc_time_stamp() << endl << flush;
            free_rob_event.notify(1,SC_NS);
            rob_buff.pop_front();
        }
        wait(1,SC_NS);
    }
}

void reorder_buffer::leitura_cdb()
{
    unsigned int index;
    float value;
    auto cat = gui_table.at(0);
    string p;
    vector<string> ord;
    while(true)
    {
        in_cdb->read(p);
        ord = instruction_split(p);
        index = std::stoi(ord[0]);
        value = std::stof(ord[1]);
        check_dependencies(index,value);
        if(ptrs[index-1]->busy)
        {
            ptrs[index-1]->ready = true;
            ptrs[index-1]->value = value;
            if(ptrs[index-1]->destination.at(0) != 'F')
                cat.at(index-1).text(VALUE,std::to_string((int)value));
            else
                cat.at(index-1).text(VALUE,std::to_string(value));
            ptrs[index-1]->state = WRITE;
            cat.at(index-1).text(STATE,"Write Result");
            if(rob_buff[0]->entry == index)
                rob_head_value_event.notify(1,SC_NS);
        }
        wait();
    }
}

void reorder_buffer::leitura_adu()
{
    string p;
    vector<string> ord;
    unsigned int index;
    auto cat = gui_table.at(0);
    while(true)
    {
        in_adu->read(p);
        ord = instruction_split(p);
        index = std::stoi(ord[0]);
        ptrs[index-1]->destination = ord[1];
        wait(SC_ZERO_TIME);
        cat.at(index-1).text(DESTINATION,ord[1]);
        if(ptrs[index-1]->qj == 0)
        {
            ptrs[index-1]->ready = true;
            cat.at(index-1).text(STATE,"Write Result");
            instr_queue_gui.at(ptrs[index-1]->instr_pos).text(WRITE,"X");
        }
        if(rob_buff[0]->entry == index && ptrs[index-1]->ready)
            rob_head_value_event.notify(1,SC_NS); 
        wait();
    }
}
void reorder_buffer::check_conflict()
{
    string p;
    unsigned int rob_pos,last_st;
    vector<string> ord;
    while(true)
    {
        last_st = 0;
        in_slb->nb_read(p);
        if(p != "F")
        {
            in_slb->notify();
            ord = instruction_split(p);
            rob_pos = std::stoi(ord[0]);
            for(unsigned int i = 0 ; i < rob_pos-1 ; i++)
                if( ptrs[i]->instruction.at(0) == 'S' && ptrs[i]->busy && ptrs[i]->destination == ord[1])
                    last_st = i+1;
            out_slb->write(std::to_string(last_st));
        }
        wait();
    }
}

int reorder_buffer::busy_check()
{
    unsigned int ret = last_rob;
    last_rob = (last_rob+1)%tam;
    return ret;
}
unsigned int reorder_buffer::ask_status(bool read,string reg,unsigned int pos)
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
float reorder_buffer::ask_value(bool read,string reg,float value)
{
    string res;
    if(read)
    {
        out_rb->write("R V " + reg);
        in_rb->read(res);
        return std::stof(res);
    }
    else
        out_rb->write("W V " + reg + ' ' + std::to_string(value));
    return 0;
}
void reorder_buffer::mem_write(unsigned int addr,float value,unsigned int rob_pos)
{
    out_mem->write("S " + std::to_string(addr) + ' ' + std::to_string(value) + ' ' + std::to_string(rob_pos));
}
void reorder_buffer::check_dependencies(unsigned int index, float value)
{
    auto cat = gui_table.at(0);
    for(unsigned int i = 0 ; i < tam ; i++)
    {
        if(ptrs[i]->busy && ptrs[i]->instruction.at(0) == 'S')
        {
            if(ptrs[i]->qj == index)
            {
                ptrs[i]->value = value;
                cat.at(i).text(VALUE,std::to_string((int)value));
                ptrs[i]->qj = 0;
                if(ptrs[i]->destination != "")
                {
                    cat.at(i).text(STATE,"Write Result");
                    instr_queue_gui.at(ptrs[i]->instr_pos).text(WRITE,"X");
                    ptrs[i]->ready = true;
                }
                if(rob_buff[0]->entry == index && ptrs[i]->ready)
                    rob_head_value_event.notify(1,SC_NS);
            }
        }
        else if(ptrs[i]->busy && ptrs[i]->instruction.at(0) == 'B')
        {
            if(ptrs[i]->qj == index)
            {
                ptrs[i]->vj = value;
                ptrs[i]->qj = 0;
            }
            if(ptrs[i]->qk == index)
            {
                ptrs[i]->vk = value;
                ptrs[i]->qk = 0;
            }
            if(ptrs[i]->qj == 0 && ptrs[i]->qk == 0)
                ptrs[i]->ready = true;
            if(rob_buff[0]->entry == index && ptrs[i]->ready)
                rob_head_value_event.notify(1,SC_NS);
        }
    }
}
void reorder_buffer::value_check()
{
    string p,value;
    auto cat = gui_table.at(0);
    while(true)
    {
        in_resv_adu->read(p);
        if(p == "N")
            resv_read_oper_event.notify();
        if(p != "N")
        {
            int index = std::stoi(p);
            if(ptrs[index-1]->ready)
                out_resv_adu->write(cat.at(index-1).text(VALUE));
            else
                out_resv_adu->write("EMPTY");
        }
        wait();
    }
}
void reorder_buffer::_flush()
{
    auto cat = gui_table.at(0);
    rob_buff.clear();
    last_rob = 0;
    for(unsigned int i = 0 ; i < tam ; i++)
    {
        ptrs[i]->busy = false;
        ptrs[i]->ready = false;
        ptrs[i]->destination = "";
        ptrs[i]->qj = ptrs[i]->qk = 0;
        cat.at(i).text(R_BUSY,"False");
        cat.at(i).text(INSTRUCTION,"");
        cat.at(i).text(STATE,"");
        cat.at(i).text(DESTINATION,"");
        cat.at(i).text(VALUE,"");
    }
}
bool reorder_buffer::branch(int optype,int rs,int rt)
{
    switch(optype)
    {
        case 0:
            if(rs == rt)
                return true;
            return false;
        case 1:
            if(rs != rt)
                return true;
            return false;
        default:
            cerr << "Erro inesperado, abortando..." << endl << flush;
            sc_stop();
            nana::API::exit();
            exit(1);
    }
}
bool reorder_buffer::branch(int optype,float value)
{
    switch(optype)
    {
        case 2:
            if(value > 0)
                return true;
            return false;
        case 3:
            if(value < 0)
                return true;
            return false;
        case 4:
            if(value >= 0)
                return true;
            return false;
        case 5:
            if(value <= 0)
                return true;
            return false;
        default:
            cerr << "Erro inesperado, abortando..." << endl << flush;
            sc_stop();
            nana::API::exit();
            exit(1);
    }
}

int reorder_buffer::instruction_pos_finder(string p)
{
    for(unsigned int i = p.size() -1; i >= 0 ;i--)
    {
        if(p[i] == ' ')
            return i;
    }
    return -1;
}
