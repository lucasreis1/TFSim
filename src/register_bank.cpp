#include "register_bank.hpp"
#include "general.hpp"
#include<vector>

using std::vector;

register_bank::register_bank(sc_module_name name,nana::listbox &regs):sc_module(name), registers(regs)
{
    SC_THREAD(le_bus);
    sensitive << in;
    dont_initialize();
    SC_METHOD(le_cdb);
    sensitive << in_cdb;
    dont_initialize();
}

void register_bank::le_bus()
{
    vector<string> ord;
    unsigned int index;
    string p;
    bool fp;
    auto cat = registers.at(0);
    while(true)
    {
        in->read(p);
        ord = instruction_split(p);
        index = std::stoi(ord[2].substr(1,ord[2].size()-1));
        if(ord[2].at(0) == 'F')
            fp = true;
        else
            fp = false;
        if(ord[0] == "R")
        {
            if(ord[1] == "S")
            {
                if(fp)
                    out->nb_write(cat.at(index).text(FQ));
                else
                    out->nb_write(cat.at(index).text(IQ));
            }
            else
            {
                if(fp)
                    out->nb_write(cat.at(index).text(FVALUE));
                else
                    out->nb_write(cat.at(index).text(IVALUE));
            }
        }
        else
        {
            if(ord[1] == "S")
            {
                if(fp)
                    cat.at(index).text(FQ,ord[3]);
                else
                    cat.at(index).text(IQ,ord[3]);
            }
            else
            {
                if(fp)
                    cat.at(index).text(FVALUE,ord[3]);
                else
                    cat.at(index).text(IVALUE,std::to_string(std::stoi(ord[3])));
            }
        }
        wait();
    }
}

void register_bank::le_cdb()
{
    string p;
    vector<string> ord;
    in_cdb->read(p);
    ord = instruction_split(p);
    auto cat = registers.at(0);
    for(unsigned int i = 0 ; i < 32 ; i++)
    {
        if(cat.at(i).text(FQ) == ord[0])
        {
            cat.at(i).text(FQ,"0");
            cat.at(i).text(FVALUE,ord[1]);
            cout << "Valor de F" << i << " atualizado para " << ord[1] << endl << flush;
        }
        if(cat.at(i).text(IQ) == ord[0])
        {
            cat.at(i).text(IQ,"0");
            cat.at(i).text(IVALUE,std::to_string(std::stoi(ord[1])));
            cout << "Valor de R" << i << " atualizado para " << std::stoi(ord[1]) << endl << flush;
        }
    }
}
