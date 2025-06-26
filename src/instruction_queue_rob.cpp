#include "instruction_queue_rob.hpp"
#include "general.hpp"

instruction_queue_rob::instruction_queue_rob(sc_module_name name, vector<string> inst_q,int rb_sz, nana::listbox &instr):
sc_module(name),
instruct_queue(inst_q.size()),
original_instruct(inst_q),
last_instr(rb_sz),
last_pc(rb_sz),
instructions(instr)
{
    for(unsigned int i = 0 ; i < inst_q.size() ; i++)
    {
        instruct_queue[i].instruction = inst_q[i];
        instruct_queue[i].pc = i;
    }

    SC_THREAD(main);
    sensitive << in;
    dont_initialize();
    SC_METHOD(leitura_rob);
    sensitive << in_rob;
    dont_initialize();
}

void instruction_queue_rob::main()
{
    auto cat = instructions.at(0);
    pc = 0;
    while(1)
    {
        if(pc < instruct_queue.size())
        {
            if(pc)
                cat.at(pc-1).select(false);
            cat.at(pc).select(true,true);
            cat.at(pc).text(ISS,"");
            cat.at(pc).text(EXEC,"");
            cat.at(pc).text(WRITE,"");
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            // Instructions + current_pc + pc_original_instruction
            out->write(instruct_queue[pc].instruction + " " +
                       std::to_string(pc)+ " " + 
                       std::to_string(instruct_queue[pc].pc));
            pc++;
            wait(SC_ZERO_TIME);
            cat.at(pc-1).text(ISS,std::to_string(sc_time_stamp().value() / 1000)); //cat.at(pc-1).text(ISS,"X");
        }
        wait();
    }
}

void instruction_queue_rob::leitura_rob()
{
    string p;
    vector<string> ord;
    unsigned int index;
    int offset;
    in_rob->read(p);
    ord = instruction_split(p);
    index = std::stoi(ord[1])-1; //ROB position
    if(ord[0] == "R") //reverter salto incorreto
    {
        instructions.at(0).at(pc-1).select(false);
        //replace_instructions(last_pc[index]-1,index);
        //pc = last_pc[index]-1;
        
        // Removeu o -1 por conta de um bug: branch instruction era despachada 2 vezes
        replace_instructions(last_pc[index], index);
        pc = last_pc[index];
        
        instruct_queue = last_instr[index]; 
    }
    else if(ord[0] == "S" && ord.size() == 3) //realiza salto (especulado) e armazena informacoes pre-salto
    {
        last_instr[index] = instruct_queue;
        last_pc[index] = pc;
        vector<instr_q> new_instructions_vec;
        offset = std::stoi(ord[2]);
        unsigned int original_pc = instruct_queue[pc-1].pc;
        for(unsigned int i = original_pc+offset ; i < original_instruct.size() ; i++)
            new_instructions_vec.push_back({original_instruct[i],i});
        add_instructions(pc,new_instructions_vec);
    }
    else if(ord[0] == "S") //Não há salto, continua na instrução de baixo (especulado)
    {
        last_pc[index] = pc;
    }
    // else if para tratar JUMP, onde o salto ocorre e não é especulado.
    else if(ord[0] == "J"){
        vector<instr_q> new_instructions_vec;
        offset = std::stoi(ord[2]);
        unsigned int original_pc = instruct_queue[pc-1].pc;
        for(unsigned int i = original_pc + offset; i < original_instruct.size(); i++)
            new_instructions_vec.push_back({original_instruct[i], i});
        add_instructions(pc, new_instructions_vec);
    }
    else //salta atrasado (quando foi predito que nao saltaria)
    {
        vector<instr_q> new_instructions_vec;
        offset = std::stoi(ord[0]);
        instructions.at(0).at(pc-1).select(false);
        pc = last_pc[index];
        unsigned int original_pc = instruct_queue[pc-1].pc;
        for(unsigned int i = original_pc+offset ; i < original_instruct.size() ; i++)
            new_instructions_vec.push_back({original_instruct[i],i});
        add_instructions(pc,new_instructions_vec);
    }
}

void instruction_queue_rob::replace_instructions(unsigned int pos,unsigned int index)
{
    auto cat = instructions.at(0);
    unsigned int sz = instruct_queue.size();
    unsigned int i;
    instructions.auto_draw(false);
    for(i = pos ; i < last_instr[index].size() ; i++)
    {
        if(i < sz)
        {
            instruct_queue[i] = last_instr[index][i];
            cat.at(i).text(ISS,"");
            cat.at(i).text(EXEC,"");
            cat.at(i).text(WRITE,"");
            cat.at(i).text(INSTR,last_instr[index][i].instruction);
        }
        else
        {
            instruct_queue.push_back(last_instr[index][i]);
            instructions.at(0).append(last_instr[index][i].instruction);
        }
    }
    if(i < instruct_queue.size())
    {
        auto item_proxy = instructions.at(0).at(i);
        while(i < instruct_queue.size())
        {
            instruct_queue.pop_back();
            item_proxy = instructions.erase(item_proxy);
        }
    }
    instructions.auto_draw(true);

}

void instruction_queue_rob::add_instructions(unsigned int pos, vector<instr_q> instr_vec)
{
    unsigned int sz = instruct_queue.size();
    auto cat = instructions.at(0);
    unsigned int i = pos;
    instructions.auto_draw(false);
    for(; i < pos + instr_vec.size() ; i++)
    {
        if(i < sz)
        {
            instruct_queue[i] = instr_vec[i-pos];   
            cat.at(i).text(ISS,"");
            cat.at(i).text(EXEC,"");
            cat.at(i).text(WRITE,"");
            cat.at(i).text(INSTR,instr_vec[i-pos].instruction);
        }
        else
        {
            instruct_queue.push_back(instr_vec[i-pos]);
            instructions.at(0).append(instr_vec[i-pos].instruction);
        }
    }
    if(i < instruct_queue.size())
    {
        auto item_proxy = instructions.at(0).at(i);
        while(i < instruct_queue.size())
        {
            instruct_queue.pop_back();
            item_proxy = instructions.erase(item_proxy);
        }
    }
    instructions.auto_draw(true);
}

bool instruction_queue_rob::queue_is_empty(){
    
    return pc == instruct_queue.size();
}

unsigned int instruction_queue_rob::get_instruction_counter() {
    return pc;
}
