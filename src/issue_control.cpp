#include "issue_control.hpp"
#include "general.hpp"

issue_control::issue_control(sc_module_name name): sc_module(name)
{
    //Tipo da instrucao define para onde ela sera enviada no fluxo de modulos do SystemC
    //Novas instrucoes devem ser adcionadas manualmente aqui
    res_type = {{"DADD",1},{"DADDI",1},{"DADDU",1},{"DADDIU",1},{"DSUB",1},{"DSUBU",1},{"DMUL",1},{"DMULU",1},{"DDIV",1},{"DDIVU",1},{"LD",2},{"SD",2}};
    SC_THREAD(issue_select);
    sensitive << in;
    dont_initialize();
}

void issue_control::issue_select()
{
    while(true)
    {
        in->nb_read(p);
        ord = instruction_split(p);
        switch(res_type[ord[0]])
        {
            case 1:
                out_rsv->write(p);
                break;
            case 2:
                out_slbuff->write(p);
                break;
            default:
                cerr << "Instruçao nao suportada!" << endl << flush;
                sc_stop();
                exit(1);
        }
        in->notify();
        wait();
    }
}

