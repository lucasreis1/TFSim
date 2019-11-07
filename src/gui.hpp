#pragma once

#include<nana/gui.hpp>
#include<nana/gui/widgets/listbox.hpp>
#include<nana/gui/filebox.hpp>
#include<fstream>
#include<vector>
#include<string>

extern const char* str_spec; 

extern const char*  str_nospec; 


void set_spec(nana::place &plc, bool is_spec);
bool add_instructions(std::ifstream &File,std::vector<std::string> &queue, nana::listbox &instruction_gui);
void show_message(std::string message_title, std::string message);
