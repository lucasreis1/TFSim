import sys

def branch_sort(val):
    return val[1]

if len(sys.argv) == 1:
    print "Uso: adapt_in.py <reg_status> <reg_status_fp> <mem_values> <instruction_queue>"

if len(sys.argv) > 2:
    reg_pos = 1
    fp_pos = 2
    mem_pos = 3
    iq_pos = 4
else:
    iq_pos = 1

output = []
inst_dict = {'DADD':'add','DADDI':'addi','DSUB':'sub','DSUBI':'subi','DDIV':'div','DMUL':'mul','LD':'lw','SD':'sw', 'BEQ': 'beq', 'BNE':'bne','BGTZ':'bgtz','BLTZ':'bltz','BGEZ':'bgez','BLEZ':'blez'}
branch_label = 0

if len(sys.argv) > 2:
###### FP_REG  ######
    output.append('.data\n')
    with open(sys.argv[mem_pos]) as f:
        values = f.read().split(' ')
        for v in values:
            if v == '':
                break
            output.append(hex(int(v)) + '\n')
    i = 0
    with open(sys.argv[fp_pos]) as f:
        values = f.read().split(' ')
        for v in values:
            if v == '':
                break
            output.append('floatValue' + str(i) + ': .float ' + v + '\n')
            i += 1
        output.append('.text\n')
        i = 0
        for v in values:
            if v == '':
                break
            output.append('l.s $f' + str(i) + ',floatValue' + str(i) + '\n')
            i += 1
###### INT_REG ######
    i = 0
    with open(sys.argv[reg_pos]) as f:
        values = f.read().split(' ')
        for v in values:
            if v == '\n':
                break
            output.append("addi $" + str(i) + ",$zero," + v + '\n') 
            i += 1
###### INS_POS ######
branches = []
pos = len(output)
with open(sys.argv[iq_pos]) as f:
    linha = ''
    while True:
        linha = f.readline()
        if linha == '':
            break
        inst = linha.split(' ')[0]
        linha = linha.replace(inst,inst_dict[inst])
        linha = linha.replace('F','$f')
        linha = linha.replace('R','$')
        if linha.startswith('b'):
            labelpos = int(linha.split(',')[-1])
            branches.append(['branchLabel'+str(branch_label)+':',pos+labelpos])
            linha = ','.join(linha.split(',')[:-1]) + ',branchLabel'+str(branch_label)+'\n'
            branch_label+=1
        if '(' in linha:
            split_line = linha.split('(')
            split_line = split_line[0].split(',')
            mem_int = split_line[-1]
            linha = linha.replace(mem_int,hex(0x10010000 + int(mem_int)))
        output.append(linha)
        pos+=1
branches.sort(key=branch_sort)
for i in range(len(branches)):
    branches[i][1] = branches[i][1] + i
    pos = branches[i][1]
    label = branches[i][0]
    output.insert(pos,label + '\n')
with open(sys.argv[iq_pos].split('.')[0] + '_m.asm','w') as f:
    for linha in output:
        f.write(linha)


