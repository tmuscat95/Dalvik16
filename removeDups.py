fo = open("Interpreter/opcodes.h","r")

lines = fo.readlines()

fo.close()

uniquelines = set(lines)

lines = list(uniquelines)

fo = open("Interpreter/opcodes.h","w")

fo.write("#ifndef OPCODES_H\n#define OPCODES_H\n\n")
fo.writelines(lines)
fo.write("\n#endif")

