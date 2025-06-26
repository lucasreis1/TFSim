# TFSim

Event-driven simulation of the Tomasulo Algorithm using the C++ interface SystemC, with a complete GUI.

The purpose of this project is to ease the teaching of this algorithm, deemed extremely important to the understanding of
out-of-order execution on modern microprocessors.

## Guia de Execução

### Container
Um container é fornecido para compilação e execução da ferramenta com mais facilidade. Execute o comando na raíz do repositório:

`bash ./build_docker.sh`

Sempre que precisar usar o container, utilize:

`docker start tfsim-tfsim-1`

Caso queira inserir valores por linha de comando pelo container, lembre-se inserí-los na pasta `in`

### Manualmente
1. Instale os pacotes necessários para uso da biblioteca Nana
	```apt install -y unzip libx11-dev libxft-dev libasound2-dev libxcursor-dev```
2. Execute o script na raíz do repositório para instalar as dependências do simulador
	`sh get_dep.sh`
3. Execute com `./tfsim`
		

- Para maioria dos benchmarks, arquivos de memória e registradores são carregados juntamente com o conjunto de instruções.
- É possível inserir a lista de instruções e outros valores por linha de comando ou pela barra de opções da interface. Por linha de comando, utilize:
			

		-  '-q' para fila de instruções
		- '-i' para valores de registradores inteiros (32 valores)
		- '-f' para valores de registradores PF (32 valores)
		- '-m' para valores de memória (500 valores inteiros)
		- '-r' para número de unidades funcionais (3 inteiros, um para cada tipo (ADD,MULT,LOAD/STORE))
		- '-l' para tempo de latência para cada instrução (uma linha para cada instrução, do formato <INSTRUÇÃO> <tempo de latência em ciclos>)
		- '-s' indica que o programa execute em modo de especulação por hardware (com reorder buffer)
		* O repositório fornece arquivos de teste já preenchidos na pasta 'in'
        * Também são fornecidos benchmarks para testes básicos (ideais para validação da ferramenta), incluidos em in/benchmarks
* Observações:
	- Caso esteja obtendo erro na compilação do simulador devido a biblioteca stdc++fs, utilize a regra nofs:
		```make nofs```
	- Um vídeo de demonstração da execução do simulador pode ser visto [aqui](https://youtu.be/hleCH6yndPY)


## Instruções suportadas

Sem especulação | Com especulação
---| ---|
DADD | BEQ |
DADDI| BNE |
DSUB | BGTZ |
DSUBI| BLTZ |
DMUL | BGEZ | 
DDIV| BLEZ |
LD| SGT |
SD| SLT |
-- | J |
