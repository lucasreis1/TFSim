#include "general.hpp"
#include "memory.hpp"
#include<vector>

using std::vector;

memory::memory(sc_module_name name, nana::grid &m): sc_module(name), mem(m)
{
    SC_THREAD(leitura_bus);
    sensitive << in;
    dont_initialize();
}

void memory::leitura_bus()
{
    vector<string> ord;
    unsigned int pos;
    string escrita_saida;
    while(1)
    {
        in->read(p);
        ord = instruction_split(p);
        pos = std::stoi(ord[1]);
        if(pos%4)
        {
            cerr << "Endereço " << pos << " não é múltiplo de quatro!" << endl;
            sc_stop();
            nana::API::exit();
        }
        pos/=4;
        if(ord[0] == "L")
        {
            cout << "Instrucao terminada com resultado " << mem.Get(pos) << " para escrever na estaçao de reserva " << ord[2] << endl << flush;
            escrita_saida = ord[2] + ' ' + mem.Get(pos);
            out->write(escrita_saida);
        }
        else
        {
            mem.Set(pos,ord[2]);
        }
        wait();
    }
}
