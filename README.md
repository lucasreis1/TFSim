



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
 - Crie uma pasta qualquer para fazer download do Nana.
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
 - Crie uma pasta qualquer para armazenar esse repositorio
	3-1. Clone usando git
	```	
	git clone https://github.com/lucasreis1/Tomasulo-Algorithm-Simulator.git
	```
	3-2. Compile o codigo usando o makefile incluido
	```
	make
	```

	3-3. Execute com ./simulador
		

	 - Os valores iniciais de registradores e de memória são gerados aleatoriamente para cada execução. É possível alterá-los (assim como inserir a lista de instruções) por linha de comando ou pela barra de opções da interface. Por linha de comando, utilize:
			

		 -  '-q' para fila de instruções
		- '-i' para valores de registradores inteiros (32 valores)
		- '-f' para valores de registradores PF (32 valores)
		- '-m' para valores de memória (500 valores inteiros)
		* O repositório fornece 4 arquivos de teste já preenchidos (fila_instr, reg_status, reg_status_fp, mem_status)
* Observações:
	- Caso esteja obtendo erro na compilação do simulador devido a biblioteca stdc++fs, utilize a regra nofs:
		```make nofs```

