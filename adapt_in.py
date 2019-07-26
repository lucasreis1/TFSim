import sys
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
inst_dict = {'DADD':'add','DADDI':'addi','DSUB':'sub','DSUBI':'subi','DDIV':'div','DMUL':'mul','LD':'lw','SD':'sw'}

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
            output.append("addi $" + str(i) + ",$zero," + v + '\n') 
            i += 1
###### INS_POS ######
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
        if '(' in linha:
            split_line = linha.split('(')
            split_line = split_line[0].split(',')
            mem_int = split_line[-1]
            linha = linha.replace(mem_int,hex(0x10010000 + 4*int(mem_int)))
        output.append(linha)
with open(sys.argv[iq_pos] + '_m.asm','w') as f:
    for linha in output:
        f.write(linha)
