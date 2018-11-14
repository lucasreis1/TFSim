#include<systemc.h>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<nana/gui.hpp>
#include<nana/gui/widgets/listbox.hpp>
#include<nana/gui/widgets/button.hpp>
#include<nana/gui/widgets/label.hpp>
#include<nana/gui/widgets/menubar.hpp>
#include "top.hpp"

using std::string;
using std::vector;
using std::map;
using std::fstream;

int start(nana::listbox &table, vector<string> &instruction_queue, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &clk)
{
	sc_start();
	return 0;
}

int sc_main(int argc, char *argv[])
{
	using namespace nana;
	std::vector<std::string> columns = {"#","Name","Busy","Op","Vj","Vk","Qj","Qk","A"}; 
	std::vector<int> sizes;
	form fm(API::make_center(1000,600));
	place plc(fm);
	place upper(fm);
	place lower(fm);
	plc.div("<vert<weight=5%><weight = 50% <vert weight = 50% <weight = 60% rst> <instr> ><vert <weight = 50% memor> < <regs> <weight=20%>> > > <weight = 5%> < <gap = 10 btns><weight = 80%> > <clk_c> <weight=30% <rob><weight=45%> >");
	listbox table(fm);
	listbox reg(fm);
	listbox instruct(fm);
	listbox rob(fm);
	//menubar mnbar(fm);
	button botao(fm);
	button clock_control(fm);
	button exit(fm);
	label clock_count(fm);
	grid memory(fm,rectangle(),10,50);
	map<string,int> instruct_time{{"DADD",4},{"DADDI",4},{"DSUB",6},{"DSUBI",6},{"DMUL",10},{"DMULI",10},{"DDIV",16},{"DDIVI",16},{"MEM",2}};
	top top1("top");
	//mnbar.push_back("&Options");
	botao.caption("START");
	clock_control.caption("NEXT CYCLE");
	exit.caption("EXIT");
	plc["rst"] << table;
	plc["btns"] << botao << clock_control << exit;
	plc["memor"] << memory;
	plc["regs"] << reg;
	plc["instr"] << instruct;
	plc["clk_c"] << clock_count;
	plc["rob"] << rob;
	plc.collocate();
	instruct.scheme().item_selected = colors::red;
	//mnbar.at(0).append("a");
	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		table.append_header(columns[i].c_str());
		if(i)
			table.column_at(i).width(45);
		else
			table.column_at(i).width(30);
	}
	columns = {"","Value","Qi"};
	for(unsigned int k = 0 ; k < 2 ; k++)
		for(unsigned int i = 0 ; i < columns.size() ; i++)
			reg.append_header(columns[i].c_str());

	for(unsigned int i = 0 ; i < reg.column_size() ; i++)
		reg.column_at(i).width(60);
	
	auto cat = reg.at(0);
	for(int i = 0 ; i < 32 ;i++)
	{
		string index = std::to_string(i);
		cat.append("R" + index);
		cat.at(i).text(3,"F" + index);
	}

	columns = {"Instruction","Issue","Execute","Write Result"};
	sizes = {150,60,70,100};
	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		instruct.append_header(columns[i]);
		instruct.column_at(i).width(sizes[i]);
	}
	columns = {"Entry","Busy","Instruction","State","Destination","Value"};
	sizes = {45,45,120,120,90,60};
	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		rob.append_header(columns[i]);
		rob.column_at(i).width(sizes[i]);
	}
	ifstream inFile;
	vector<string> instruction_queue;
	string line;
	int value,i = 0;
	float value_fp;
	if(argc < 4)
	{
		cout << "Uso: ./ex.x <lista_instrucoes> <reg_int_valores_iniciais> <reg_fp_valores_iniciais> <memoria_valores_iniciais>" << endl;
		return 1;
	}
	inFile.open(argv[1]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[1] << " nao existe!" << endl;
		return 1;
	}
	auto c = instruct.at(0);
	while(getline(inFile,line))
	{
		instruction_queue.push_back(line);
		c.append(line);
	}
	inFile.close();
	inFile.open(argv[2]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[2] << " nao existe!" << endl;
		return 1;
	}
	i = 0;
	while(inFile >> value)
	{
		cat.at(i).text(1,std::to_string(value));
		cat.at(i).text(2,"0");
		i++;
	}
	inFile.close();

	i = 0;
	inFile.open(argv[3]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[3] << " nao existe!" << endl;
		return 1;
	}
	while(inFile >> value_fp)
	{
		cat.at(i).text(4,std::to_string(value_fp));
		cat.at(i).text(5,"0");
		i++;
	}
	inFile.close();
	inFile.open(argv[4]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[4] << " nao existe!" << endl;
		return 1;
	}
	while(inFile >> value)
		memory.Push(std::to_string(value));
		
	clock_control.enabled(false);
	botao.events().click([&]
	{
		botao.enabled(false);
		clock_control.enabled(true);
		top1.rob_mode(3,2,2,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count,rob);
		sc_start();
	});
	clock_control.events().click([]
	{
		if(sc_is_running())
			sc_start();
	});
	exit.events().click([]
	{
		sc_stop();
		API::exit();
	});
	fm.show();
	exec();
	return 0;
}