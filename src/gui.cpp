#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<nana/gui.hpp>
#include "gui.hpp"
#include "top.hpp"

using namespace nana;

char const* str_spec = "<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <rst> <weight = 1%> <weight = 20% regs> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <rob> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >";

char const* str_nospec = "<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <weight = 1%> <rst> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <weight = 29% regs> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >";

// Mostra mensagem na interface grafica
void show_message(string message_title, string message)
{
    msgbox msg(message_title);
    msg << message;
    msg.show();
}
// Organiza a interface de acordo com o modo (com ou sem especulacao)
void set_spec(nana::place &plc, bool is_spec)
{
    if(is_spec)
        plc.div(str_spec);
    else
        plc.div(str_nospec);
    plc.collocate();
}


bool add_instructions(ifstream &File,vector<string> &queue, nana::listbox &instruction_gui)
{
    if(!File.is_open())
        return false;
    if(queue.size())
    {
        queue.clear();
        instruction_gui.clear(0);   
    }
    auto inst_gui_cat = instruction_gui.at(0);
    string line;
    while(getline(File,line))
    {
        if(line.rfind("//", 0) == string::npos) //ignora linhas que começam com "//"
        {
            queue.push_back(line);
            inst_gui_cat.append(line);
        }
    }
    File.close();
    return true;
}
