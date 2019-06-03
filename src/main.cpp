#include<systemc.h>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<nana/gui.hpp>
#include<nana/gui/widgets/button.hpp>
#include<nana/gui/widgets/menubar.hpp>
#include<nana/gui/widgets/group.hpp>
#include<nana/gui/widgets/textbox.hpp>
#include<nana/gui/filebox.hpp>
#include "top.hpp"

using std::string;
using std::vector;
using std::map;
using std::fstream;

int sc_main(int argc, char *argv[])
{
	using namespace nana;
	vector<string> columns = {"#","Name","Busy","Op","Vj","Vk","Qj","Qk","A"}; 
	vector<string> instruction_queue;
	int nadd,nmul,nls;
	nadd = 3;
	nmul = nls = 2;
	std::vector<int> sizes;
	bool spec = false;
	bool fila = false;
	ifstream inFile;
	form fm(API::make_center(1000,600));
	place plc(fm);
	place upper(fm);
	place lower(fm);
	listbox table(fm);
	listbox reg(fm);
	listbox instruct(fm);
	listbox rob(fm);
	menubar mnbar(fm);
	button botao(fm);
	button clock_control(fm);
	button exit(fm);
	group clock_group(fm);
	label clock_count(clock_group);
	clock_group.caption("Ciclo de Clock");
	clock_group.div("count");
	grid memory(fm,rectangle(),10,50);
	map<string,int> instruct_time{{"DADD",4},{"DADDI",4},{"DSUB",6},{"DSUBI",6},{"DMUL",10},{"DDIV",16},{"MEM",2}};
	top top1("top");
	mnbar.push_back("&Opções");
	botao.caption("START");
	clock_control.caption("NEXT CYCLE");
	exit.caption("EXIT");
	plc.div("<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <weight = 1%> <rst> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <weight = 29% regs> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >");
	plc["rst"] << table;
	plc["btns"] << botao << clock_control << exit;
	plc["memor"] << memory;
	plc["regs"] << reg;
	plc["rob"] << rob;
	plc["instr"] << instruct;
	plc["clk_c"] << clock_group;
	clock_group["count"] << clock_count;
	clock_group.collocate();
	plc.collocate();
	menu &op = mnbar.at(0);
	op.append("Especulação",[&](menu::item_proxy &ip)
	{
		if(ip.checked())
		{
			plc.div("<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <rst> <weight = 1%> <weight = 20% regs> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <rob> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >");
			plc.collocate();
			spec = true;
		}
		else
		{
			plc.div("<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <weight = 1%> <rst> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <weight = 29% regs> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >");
			plc.collocate();
			spec = false;
		}
	});
	op.check_style(0,menu::checks::highlight);
	op.append("Modificar valores");
	auto sub = op.create_sub_menu(1);
	sub->append("Número de Estações de Reserva",[&](menu::item_proxy ip)
	{
		inputbox ibox(fm,"","Quantidade de Estações de Reserva");
		inputbox::integer add("ADD/SUB",nadd,1,10,1);
		inputbox::integer mul("MUL/DIV",nmul,1,10,1);
		inputbox::integer sl("LOAD/STORE",nls,1,10,1);
		if(ibox.show_modal(add,mul,sl))
		{
			nadd = add.value();
			nmul = mul.value();
			nls = sl.value();
		}
	});
	sub->append("Tempos de latência", [&](menu::item_proxy &ip)
	{
		inputbox ibox(fm,"","Tempos de latência para instruções");
		inputbox::text dadd_t("DADD",std::to_string(instruct_time["DADD"]));
		inputbox::text daddi_t("DADDI",std::to_string(instruct_time["DADDI"]));
		inputbox::text dsub_t("DSUB",std::to_string(instruct_time["DSUB"]));
		inputbox::text dsubi_t("DSUBI",std::to_string(instruct_time["DSUBI"]));
		inputbox::text dmul_t("DMUL",std::to_string(instruct_time["DMUL"]));
		inputbox::text ddiv_t("DDIV",std::to_string(instruct_time["DDIV"]));
		inputbox::text mem_t("Load/Store",std::to_string(instruct_time["MEM"]));
		if(ibox.show_modal(dadd_t,daddi_t,dsub_t,dsubi_t,dmul_t,ddiv_t,mem_t))
		{
			instruct_time["DADD"] = std::stoi(dadd_t.value());
			instruct_time["DADDI"] = std::stoi(daddi_t.value());
			instruct_time["DSUB"] = std::stoi(dsub_t.value());
			instruct_time["DSUBI"] = std::stoi(dsubi_t.value());
			instruct_time["DMUL"] = std::stoi(dmul_t.value());
			instruct_time["DDIV"] = std::stoi(ddiv_t.value());
			instruct_time["MEM"] = std::stoi(mem_t.value());
		}
	});
	sub->append("Fila de instruções", [&](menu::item_proxy &ip)
	{
		filebox fb(0,true);
		inputbox ibox(fm,"Localização do arquivo com a lista de instruções:");
		inputbox::path caminho("",fb);
		if(ibox.show_modal(caminho))
		{
			auto path = caminho.value();
			inFile.open(path);
			if(!inFile.is_open())
			{
				msgbox msg("Arquivo inválido");
				msg << "Não foi possível abrir o arquivo!";
				msg.show();
			}
			else
			{
				fila = true;
				string line;
				auto instr_gui = instruct.at(0);
				while(getline(inFile,line))
				{
					instruction_queue.push_back(line);
					instr_gui.append(line);
				}
				inFile.close();
			}
		}
	});
	sub->append("Valores de registradores inteiros",[&](menu::item_proxy &ip)
	{
		filebox fb(0,true);
		inputbox ibox(fm,"Localização do arquivo de valores de registradores inteiros:");
		inputbox::path caminho("",fb);
		if(ibox.show_modal(caminho))
		{
			auto path = caminho.value();
			inFile.open(path);
			if(!inFile.is_open())
			{
				msgbox msg("Arquivo inválido");
				msg << "Não foi possível abrir o arquivo!";
				msg.show();
			}
			else
			{
				auto reg_gui = reg.at(0);
				int value,i = 0;
				while(inFile >> value && i < 32)
				{
					reg_gui.at(i).text(1,std::to_string(value));
					i++;
				}
				for(; i < 32 ; i++)
					reg_gui.at(i).text(1,"0");
				inFile.close();
			}
		}
	});
	sub->append("Valores de registradores PF",[&](menu::item_proxy &ip)
	{
		filebox fb(0,true);
		inputbox ibox(fm,"Localização do arquivo de valores de registradores PF:");
		inputbox::path caminho("",fb);
		if(ibox.show_modal(caminho))
		{
			auto path = caminho.value();
			inFile.open(path);
			if(!inFile.is_open())
			{
				msgbox msg("Arquivo inválido");
				msg << "Não foi possível abrir o arquivo!";
				msg.show();
			}
			else
			{
				auto reg_gui = reg.at(0);
				int i = 0;
				float value;
				while(inFile >> value && i < 32)
				{
					reg_gui.at(i).text(4,std::to_string(value));
					i++;
				}
				for(; i < 32 ; i++)
					reg_gui.at(i).text(4,"0");
				inFile.close();
			}
		}
	});
	sub->append("Valores de memória",[&](menu::item_proxy &ip)
	{
		filebox fb(0,true);
		inputbox ibox(fm,"Localização do arquivo de valores de memória:");
		inputbox::path caminho("",fb);
		if(ibox.show_modal(caminho))
		{
			auto path = caminho.value();
			inFile.open(path);
			if(!inFile.is_open())
			{
				msgbox msg("Arquivo inválido");
				msg << "Não foi possível abrir o arquivo!";
				msg.show();
			}
			else
			{
				int i = 0;
				int value;
				while(inFile >> value && i < 500)
				{
					memory.Set(i,std::to_string(value));
					i++;
				}
				for(; i < 500 ; i++)
				{
					memory.Set(i,"0");
				}
				inFile.close();
			}
		}
	});
	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		table.append_header(columns[i].c_str());
		if(i)
			table.column_at(i).width(45);
		else
			table.column_at(i).width(30);
	}
	columns = {"","Value","Qi"};
	sizes = {30,60,40};
	for(unsigned int k = 0 ; k < 2 ; k++)
		for(unsigned int i = 0 ; i < columns.size() ; i++)
		{
			reg.append_header(columns[i]);
			reg.column_at(k*columns.size() + i).width(sizes[i]);
		}

	auto reg_gui = reg.at(0);
	for(int i = 0 ; i < 32 ;i++)
	{
		string index = std::to_string(i);
		reg_gui.append("R" + index);
		reg_gui.at(i).text(3,"F" + index);
	}

	columns = {"Instruction","Issue","Execute","Write Result"};
	sizes = {140,60,70,95};
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

	srand(static_cast <unsigned> (time(0)));
	for(int i = 0 ; i < 32 ; i++)
	{
		reg_gui.at(i).text(1,std::to_string(rand()%100));
		reg_gui.at(i).text(2,"0");
		reg_gui.at(i).text(4,std::to_string(static_cast <float> (rand()) / static_cast <float> (RAND_MAX/100.0)));
		reg_gui.at(i).text(5,"0");
	}
	for(int i = 0 ; i < 500 ; i++)
		memory.Push(std::to_string(rand()%100));
	for(int k = 1; k < argc; k+=2)
	{
		int i;
		if (strlen(argv[k]) > 2)
		{
			msgbox msg("Opção inválida");
			msg << "Opção \"" << argv[k] << "\" inválida";
			msg.show();
		}
		else
		{
			char c = argv[k][1];
			switch(c)
			{
				case 'q':
					inFile.open(argv[k+1]);
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						fila = true;
						auto c = instruct.at(0);
						string line;
						while(getline(inFile,line))
						{
							instruction_queue.push_back(line);
							c.append(line);
						}
						inFile.close();
					}
					break;
				case 'i':
					inFile.open(argv[k+1]);
					int value;
					i = 0;
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						while(inFile >> value && i < 32)
						{
							reg_gui.at(i).text(1,std::to_string(value));
							i++;
						}
						for(; i < 32 ; i++)
							reg_gui.at(i).text(1,"0");
						inFile.close();
					}
					break;
				case 'f':
					float value_fp;
					i = 0;
					inFile.open(argv[k+1]);
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						while(inFile >> value_fp && i < 32)
						{
							reg_gui.at(i).text(4,std::to_string(value_fp));
							i++;
						}
						for(; i < 32 ; i++)
							reg_gui.at(i).text(4,"0");
						inFile.close();
					}
					break;
				case 'm':
					inFile.open(argv[k+1]);
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						int value;
						i = 0;
						while(inFile >> value && i < 500)
						{
							memory.Set(i,std::to_string(value));
							i++;
						}
						for(; i < 500 ; i++)
						{
							memory.Set(i,"0");
						}
						inFile.close();
					}
					break;
				case 'r':
					inFile.open(argv[k+1]);
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						int value;
						if(inFile >> value && value <= 10 && value > 0)
							nadd = value;
						if(inFile >> value && value <= 10 && value > 0)
							nmul = value;
						if(inFile >> value && value <= 10 && value > 0)
							nls = value;
						inFile.close();
					}
					break;
				case 's':
					plc.div("<vert <weight = 5%><vert weight=85% < <weight = 1% ><instr> <rst> <weight = 1%> <weight = 20% regs> <weight = 1%> > < <weight = 1%> <memor> <weight = 1%> <rob> <weight = 1%> > > < <weight = 1%> <clk_c weight=15%> <weight=50%> <gap = 10btns> <weight = 1%> > <weight = 2%> >");
					plc.collocate();
					spec = true;
					k--;
					break;
				case 'l':
					inFile.open(argv[k+1]);
					if(!inFile.is_open())
					{
						msgbox msg("Arquivo inválido");
						msg << "Não foi possível abrir o arquivo " << argv[k+1];
						msg.show();
					}
					else
					{
						string inst;
						int value;
						while(inFile >> inst)
						{
							if(inFile >> value && instruct_time.count(inst))
								instruct_time[inst] = value;
						}
					}
					inFile.close();
					break;
				default:
					msgbox msg("Opção inválida");
					msg << "Opção \"" << argv[k] << "\" inválida";
					msg.show();
					break;
			}
		}
	}
	clock_control.enabled(false);
	botao.events().click([&]
	{
		if(fila)
		{
			botao.enabled(false);
			clock_control.enabled(true);
			//Desativa os menus
			op.enabled(0,false);
			op.enabled(1,false);
			for(int i = 0 ; i < 6 ; i++)
				sub->enabled(i,false);
			if(spec)
				top1.rob_mode(nadd,nmul,nls,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count,rob);
			else
				top1.simple_mode(nadd,nmul,nls,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count);
			sc_start();
		}
		else
		{
			msgbox msg("Fila de instruções vazia");
			msg << "A fila de instruções está vazia. Insira um conjunto de instruções para iniciar.";
			msg.show();
		}
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