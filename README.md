


# Tomasulo-Algorithm-Simulator

Event-driven simulation of the Tomasulo Algorithm using the C++ interface SystemC, with a complete GUI.

The purpose of this project is to ease the teaching of this algorithm, deemed extremely important to the understanding of
out-of-order execution on modern microprocessors.

# Guia de Execução

1) Baixe o [SystemC](http://www.accellera.org/downloads/standards/systemc)
1-1. Extrair systemC e entrar na pasta padrao
1-2. Crie uma pasta build dentro da pasta do SystemC e entre nela
	```
	mkdir build
	cd build
	```
	1-3. Gere os arquivos de compilação usando cmake
	```
	cmake .. -DCMAKE_CXX_STANDARD=14 -DCMAKE_INSTALL_PREFIX=/opt/systemc
	```
	1-4. Compile e instale a biblioteca
	```
	make
	sudo make install
	```
2. Crie uma pasta qualquer para fazer download do Nana.
	2-1. Clone o Nana e acesse sua raiz:
	```
	git clone --single-branch -b hotfix-1.6.2 https://github.com/cnjinhao/nana.git
	cd nana
	```
	2-2. Crie uma pasta qualquer (não pode se chamar build) e acesse-a
	```
	mkdir build2
	cd build2
	```
	2-3. Crie os arquivos de compilação usando cmake
	```
	cmake ..
	```
	2-4. Instale as seguintes bibliotecas necessárias antes de compilar o nana:
	```
	sudo apt install libx11-dev
	sudo apt install libxft-dev
	sudo apt install libasound2-dev
	```
	2-5. Compile e instale o nana usando make
	```
	make
	sudo make install
	```
3. Crie uma pasta qualquer para armazenar esse repositorio
	3-1. Clone usando git
	```
	git clone https://github.com/lucasreis1/Tomasulo-Algorithm-Simulator.git
	```
	3-2. Compile o codigo usando o makefile incluido
	```
	make
	```
	3-3. Para executar o código, são necessários 4 arquivos de texto contendo:
	 -  A lista de instruções a serem executadas
	*  Valores iniciais para os 32 registradores inteiros
	*  Valores iniciais para os 32 registradores de ponto flutuante
	*   Valores iniciais para uma memoria (500 valores)

	3-4. Execute com ./simulador <lista_de_instruçoes> <valores_reg_inteiros> <valores_reg_pf> <valores_mem>
	* O repositório fornece 4 arquivos de teste já preenchidos (fila_instr, reg_status, reg_status_fp, mem_status)
