#include "res_station_rob.hpp"
#include "general.hpp"


res_station_rob::res_station_rob(sc_module_name name,int i, string n,bool isMem, map<string,int> inst_map, const nana::listbox::item_proxy item, const nana::listbox::cat_proxy c, const nana::listbox::cat_proxy rgui):
sc_module(name),
id(i),
type_name(n),
isMemory(isMem),
instruct_time(inst_map),
table_item(item),
instr_queue_gui(c),
rob_gui(rgui)
{
    Busy = isFlushed = false;
    vj = vk = qj = qk = a = 0;
    SC_THREAD(exec);
    sensitive << exec_event;
    dont_initialize();
    SC_METHOD(leitura);
    sensitive << in;
    dont_initialize();
}

void res_station_rob::exec()
{
    while(true)
    {
        //Enquanto houver dependencia de valor em outra RS, espere
        while(qj || qk)
            wait(val_enc | isFlushed_event);
        wait(SC_ZERO_TIME);
        wait(SC_ZERO_TIME);
        if(!isFlushed)
        {
            float res = 0;
            cout << "Execuçao da instruçao " << op << " iniciada no ciclo " << sc_time_stamp() << " em " << name() << endl << flush;
            if(!isMemory){ //Se for store ou load, ja foi setado pelo address_unit
                if(instr_pos < instr_queue_gui.size())
                    instr_queue_gui.at(instr_pos).text(EXEC,std::to_string(sc_time_stamp().value() / 1000)); //text(EXEC,"X");
            }
            rob_gui.at(dest-1).text(STATE,"Execute");
            if(op.substr(0,4) == "DADD")
                res = vj + vk;
            else if(op.substr(0,4) == "DSUB")
                res = vj - vk;
            else if(op.substr(0,4) == "DMUL")
                res = vj*vk;
            else if(op.substr(0,4) == "DDIV")
            {
                if(vk)
                    res = vj/vk;
                else
                    cout << "Divisao por 0, instrucao ignorada!" << endl;
            }
            else if(isMemory)
            {
                a += vk;
                table_item->text(A,std::to_string(a));
                table_item->text(VK,"");
            }
            else if(op.substr(0,3) == "SLT")
            {
                if(vj < vk){
                    res = 1;
                } else{
                    res = 0;
                }
            }
            else if(op.substr(0,3) == "SGT"){
                if(vj > vk){
                    res = 1;
                }
                else{
                    res = 0;
                }
            }
            if(!isMemory)
            {
                wait(sc_time(instruct_time[op],SC_NS),isFlushed_event);
                wait(SC_ZERO_TIME);
                if(!isFlushed)
                {
                    string escrita_saida,rs;
                    if(fp)
                        rs = std::to_string(res);
                    else
                        rs = std::to_string((int)res);
                    escrita_saida = std::to_string(dest) + ' ' + rs;
                    cout << "Instrucao " << op << " completada no ciclo " << sc_time_stamp() << " em " << name() << " com resultado " << res << endl << flush;
                    out->write(escrita_saida);
                }
            }
            else if(!isFlushed)
            {
                if(op.at(0) == 'L')
                    mem_req(true,a,dest);
                else
                {
                    mem_req(false,a,vj);
                    cout << "Instrucao " << op << " completada no ciclo " << sc_time_stamp() << " em " << name() << " gravando na posicao de memoria " << a << " o resultado " << vj << endl << flush;
                }
                a = 0;
            }
        }
        wait(SC_ZERO_TIME);
        if(!isFlushed){
            if(instr_pos < instr_queue_gui.size())
                instr_queue_gui.at(instr_pos).text(WRITE,std::to_string(sc_time_stamp().value() / 1000)); //text(WRITE,"X");
        }
        
        Busy = isFlushed = false;
        cout << "estacao " << id << " liberada no ciclo " << sc_time_stamp() << endl << flush;
        clean_item(); //Limpa a tabela na interface grafica
        wait();
    }
}

void res_station_rob::leitura()
{
    in->read(p);
    if(qj || qk)
    {
        int rs_source;
        in->read(p);
        ord = instruction_split(p);
        rs_source = std::stoi(ord[0]);
        if(qj == rs_source)
        {
            qj = 0;
            vj = std::stoi(ord[1]);
            table_item->text(VJ,ord[1]);
            table_item->text(QJ,"");
            cout << "Instrucao " << op << " conseguiu o valor " << vj << " da RS_" << rs_source << endl << flush; 
            val_enc.notify(1,SC_NS);
        }
        if(qk == rs_source)
        {
            qk = 0;
            vk = std::stoi(ord[1]);
            table_item->text(VK,ord[1]);
            table_item->text(QK,"");
            cout << "Instrucao " << op << " conseguiu o valor " << vk << " da RS_" << rs_source << endl << flush; 
            val_enc.notify(1,SC_NS);
        }
    }
}

void res_station_rob::clean_item()
{
    for(unsigned i = 2 ; i < table_item->columns(); i++)
        table_item->text(i,"");
    table_item->text(BUSY,"False");
}

void res_station_rob::mem_req(bool load,unsigned int addr,int value)
{
    string escrita_saida;
    string temp = std::to_string(addr) + ' ' + std::to_string(value);
    if(load)
        escrita_saida = "L " + temp;
    else
        escrita_saida = "S " + temp;
    out_mem->write(escrita_saida);
}
