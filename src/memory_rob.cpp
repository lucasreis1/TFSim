#include "general.hpp"
#include "memory_rob.hpp"
#include<vector>

using std::vector;

memory_rob::memory_rob(sc_module_name name, nana::grid &m): sc_module(name), mem(m)
{
    SC_THREAD(leitura_bus);
    sensitive << in;
    dont_initialize();
}

void memory_rob::leitura_bus()
{
    vector<string> ord;
    unsigned int pos;
    string escrita_saida;
    while(true)
    {
        in->read(p);
        ord = instruction_split(p);
        pos = std::stoi(ord[1]);

        /*if(pos%4)
        {
            cerr << "Endereço " << pos << " não é múltiplo de quatro!" << endl;
            sc_stop();
            nana::API::exit();
        }
        pos/=4;*/

        if(ord[0] == "L")
        {
            escrita_saida = ord[2] + ' ' + mem.Get(pos);
            out->write(escrita_saida);
        }
        else
        {
            wait(SC_ZERO_TIME);
            mem.Set(pos,std::to_string((int)std::stoi(ord[2])));
            out_slb->write(ord[3]);
        }
        wait();
    }
}
