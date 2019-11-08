#include "general.hpp"

vector<string> instruction_split(string p)
{
    vector<string> ord;
    unsigned int i,last_pos;
    last_pos = 0;
    for(i = 0 ; i < p.size() ; i++)
        if(p[i] == ' ' || p[i] == ',')
        {
            ord.push_back(p.substr(last_pos,i-last_pos));
            last_pos = i+1;
        }
    ord.push_back(p.substr(last_pos,p.size()-last_pos));
    return ord;
}
