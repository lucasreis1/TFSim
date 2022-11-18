#include "issue_control_rob.hpp"
#include "general.hpp"

issue_control_rob::issue_control_rob(sc_module_name name): sc_module(name)
{
    //Tipo da instrucao define para onde ela sera enviada no fluxo de modulos do SystemC
    //Novas instrucoes devem ser adcionadas manualmente aqui
    res_type = {{"DADD",1},{"DADDI",1},{"DADDU",1},
                {"DADDIU",1},{"DSUB",1},{"DSUBU",1},
                {"DMUL",1},{"DMULU",1},{"DDIV",1},
                {"DDIVU",1},{"SLT", 1},{"SGT", 1},
                {"LD",2},{"SD",3},
                {"BEQ",4},{"BNE",4},{"BGTZ",4},
                {"BLTZ",4},{"BGEZ",4},{"BLEZ",4},
                {"J", 4}};

    SC_THREAD(issue_select);
    sensitive << in;
    dont_initialize();
}

void issue_control_rob::issue_select()
{
    string slb_p;
    while(true)
    {
        in->nb_read(p);
        out_rob->write(p);
        in_rob->read(rob_pos);
        ord = instruction_split(p);
        switch(res_type[ord[0]])
        {
            case 1:
                out_rsv->write(p + ' ' + rob_pos);
                break;
            case 2:
                out_slbuff->write(p + ' ' + rob_pos);
                in_slbuff->read(slb_p);
                out_adu->write(p + ' ' + rob_pos + ' ' +  slb_p);
                break;
            case 3:
                out_adu->write(p + ' ' + rob_pos);
                break;
            case 4:
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

