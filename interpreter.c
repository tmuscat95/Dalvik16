/* PC Side Interpreter, uses full set of Opcodes, outputs a file containing every opcode used in sequence, to be used by the Microcontroller side interpreter.
A Small Python script removes duplicate opcodes and formats it into a valid header file.

*/
#include <stdlib.h>
#include "interpreter.h"
rDex * p;
Reg * reservedRegs; //used to store method return results
extern FILE * defs;

void _garbageCollector(Instance * inst){
    if(inst->references==0){
        for(u2 j =0;j<inst->classDef->classData->instance_fields_size;j++){
            if(inst->fields[j].isObjRef==1){
                inst->fields[j].ObjectRef->references--;
                _garbageCollector(inst->fields[j].ObjectRef);
            }

        }
    }
}

void garbageCollector(Instance ** instances,u2 insts_count){

    if(instances!=NULL) {
        for (u2 i = 0; i < insts_count; i++) {
            if (instances[i]->references == 0) {
                for (u2 j = 0; j < instances[i]->classDef->classData->instance_fields_size; j++) {
                    if(instances[i]->fields[j].isObjRef==1) {
                        instances[i]->fields[j].ObjectRef->references--;
                        _garbageCollector(instances[i]->fields[j].ObjectRef);
                    }
                }
            }
        }

//        u2 insts_count_old = insts_count;

        for (u2 i = 0; i < insts_count; i++) {
            if (instances[i]->references == 0) {
                free(instances[i]);

            }
        }


        free(instances);
    }
}

void RunMain(){

    u2 i;
    method_id_item * methodIdItem;
    class_data_item * classDataItem;
    encoded_method *  method;
    u1 clinitFound = 0;
    //runs static value initializers (if any) for all classes before running main.
    do {
        clinitFound = 0;
        for (i = 0; i < p->sizes.method_ids_size; i++) {
            if (p->method_id_items[i].is_main == 2) {
                p->method_id_items[i].is_main = 0;
                clinitFound = 1;
                break;
            }
        }

        if(clinitFound){
            methodIdItem = &p->method_id_items[i];
            for(u2 j=0;j<p->sizes.class_defs_size;j++){
                if(p->classDefs[j].class_idx[0]==methodIdItem->class_idx[0]){
                    classDataItem = p->classDefs[j].classData;
                    break;
                }

            }

            method = NULL;
            s4 base = classDataItem->direct_methods[0].method_idx_diff;
            if(p->method_id_items[base].name_idx[0]==methodIdItem->name_idx[0])
                method=classDataItem->direct_methods;
            else {
                for (u2 k = 1; k < classDataItem->direct_methods_size; k++) {
                    if(p->method_id_items[classDataItem->direct_methods[k].method_idx_diff+base].name_idx[0] == methodIdItem->name_idx[0]){
                        method=(classDataItem->direct_methods+k);
                        break;
                    }
                    else
                        base += classDataItem->direct_methods[k].method_idx_diff;

                }
            }

            interpreter(NULL,0,method);
        }
    }while(clinitFound);

    for(i=0;i<p->sizes.method_ids_size;i++){
        if(p->method_id_items[i].is_main==1)
            break;
    }

    methodIdItem = &p->method_id_items[i];

    for(u2 j=0;j<p->sizes.class_defs_size;j++){
        if(p->classDefs[j].class_idx[0]==methodIdItem->class_idx[0]){
            classDataItem = p->classDefs[j].classData;
            break;
        }

    }

    method = NULL;
    u4 base = classDataItem->direct_methods[0].method_idx_diff;
    if(p->method_id_items[base].name_idx[0]==methodIdItem->name_idx[0])
        method=classDataItem->direct_methods;
    else {
        for (u2 k = 1; k < classDataItem->direct_methods_size; k++) {
            if(p->method_id_items[classDataItem->direct_methods[k].method_idx_diff+base].name_idx[0] == methodIdItem->name_idx[0]){
                method=(classDataItem->direct_methods+k);
                break;
            }
            else
                base += classDataItem->direct_methods[k].method_idx_diff;

        }
    }

    interpreter(NULL,0,method); //assumed main runs without arguments
}

float ieee754Float(u4 x){
    union ieee754_float u;
    u.ieee.mantissa = x;
    u.ieee.negative = x >> 31;
    u.ieee.exponent = x >> 23;

    return u.f;
}

u8 doubleieee754(double x){
    union ieee754_double u;
    u.d = x;
    u8 y = u.ieee.mantissa1;
    u8 tmp = u.ieee.mantissa0;
    tmp <<= 32;
    y += tmp;
    tmp = u.ieee.exponent;
    tmp <<= 52;
    y += tmp;
    tmp = u.ieee.negative;
    tmp <<= 63;
    y += tmp;

    return y;

}

u4 floatieee754(float x){
    union ieee754_float u;
    u.f = x;
    u4 y = u.ieee.mantissa;
    u4 tmp = u.ieee.exponent;
    tmp <<=23;
    y += tmp;
    tmp = u.ieee.negative;
    tmp <<= 31;
    y += tmp;

    return y;

}

double ieee754Double(u8 x){
    union ieee754_double u;
    u.ieee.mantissa1 = x;
    u.ieee.mantissa0 =(x >> 32);
    u.ieee.exponent = (x >> 52);
    u.ieee.negative = (x >> 63);

    return u.d;
}

u2 recons16(u1 msb,u1 lsb){
    u2 recons = msb;
    recons = recons << 8;
    recons += lsb;

    return recons;
}

u4 recons32(u2 msb,u2 lsb){
    u4 recons = msb;
    recons = recons << 16;
    recons += lsb;

    return recons;

}

u8 recons64(u4 msb,u4 lsb){
    u8 recons = msb;
    recons = recons << 32;
    recons += lsb;

    return recons;
}

////IMP NEEDS to read in 8 bit words
void interpreter(const u4 *params, u1 paramsNo, encoded_method *methodItem) {
//    FILE * defs = fopen("./Interpreter/opcodes.h","w");
    Instance **instances = NULL; //a list of all Instances of objects, used by garbage collector to implement mark and sweep.
    u2 insts_count = 0;
    u4 *paramsToPass = NULL;
    Reg *registers;
    u2 noOfRegisters = methodItem->methodCode->registers_size[0];
    registers = (Reg *) malloc(noOfRegisters * sizeof(Reg));
    u1 j = 0;
    for (u2 i = noOfRegisters - paramsNo; i < noOfRegisters; i++) {
        registers[i].bits = params[j++];
    }

    u1 *insns = methodItem->methodCode->insns;


    for (u4 i = 0; i < methodItem->methodCode->insns_size[0]; i++) {
        switch (insns[i]) {
//
            case MOVE: {
                fprintf(defs,"#define MOVE 0x%X\n",insns[i]);
                /*In this mode of addressing, only moves from and to the first 16 registers are allowed,
                 thus each register address is in fact a single hexadecimal digit (4 bits) and move instruction of this sort are entirely contained within 16-bits.
                 Instructions of this type thus take the following format: 01YX where 01 is the opcode and Y are X are hex numbers from 0 to F denoting the From and To registers respectively*/
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "From" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "To" register number
                registers[addr.To].bits = registers[addr.From].bits;
//            registers[addr.From].bits = 0;
                break;
            }

            case MOVE16to8: {
                fprintf(defs,"#define MOVE16to8 0x%X\n",insns[i]);

                Address16to8 addr816;
                addr816.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);
                registers[addr816.To].bits = registers[addr816.From].bits;
                registers[addr816.From].bits = 0;
                break;
            }

            case MOVE16to16: {
                fprintf(defs,"#define MOVE16to16 0x%X\n",insns[i]);

                Address16to16 addr1616;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr1616.From = recons16(msb, lsb);

                lsb = insns[++i];
                msb = insns[++i];
                addr1616.From = recons16(msb, lsb);
                registers[addr1616.To].bits = registers[addr1616.From].bits;
                registers[addr1616.From].bits = 0;
                break;
            }
                //default wide moved is 4 bit addressed (first 16 registers)
            case MOVEWIDE: {
                fprintf(defs,"#define MOVEWIDE 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "From" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                registers[addr.To + 1].bits = registers[addr.From + 1].bits;
                break;
            }

            case MOVEWIDE16to8: {
                fprintf(defs,"#define MOVEWIDE16to8 0x%X\n",insns[i]);

                Address16to8 addr816;
                addr816.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);

                registers[addr816.To].bits = registers[addr816.From].bits; //16 LSB
                registers[addr816.To + 1].bits = registers[addr816.From + 1].bits; //16 MSB
                break;
            }

            case MOVEWIDE16to16: {
                fprintf(defs,"#define MOVEWIDE16to16 0x%X\n",insns[i]);

                Address16to16 addr1616;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr1616.To = recons16(msb, lsb);


                lsb = insns[++i];
                msb = insns[++i];
                addr1616.From = recons16(msb, lsb);

                registers[addr1616.To].bits = registers[addr1616.From].bits; //16 LSB
                // registers[addr1616.To].bits= registers[addr1616.From].bits >> 16; //16 MSB
                registers[addr1616.To + 1].bits = registers[addr1616.From + 1].bits;
                break;
            }

            case MOVEOBJECT: //Why is this a different instruciton from MOVE ? Bytecode spec doesn't say.
            {
                fprintf(defs,"#define MOVEOBJECT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                ((Instance *) registers[addr.To].bits)->references++;
//                registers[addr.From].bits = 0;
                break;
            }

            case MOVEOBJECT16to8: {
                fprintf(defs,"#define MOVEOBJECT16to8 0x%X\n",insns[i]);

                Address16to8 addr816;
                addr816.To = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);

                registers[addr816.To].bits = registers[addr816.From].bits;
                ((Instance *) registers[addr816.To].bits)->references++;
                break;
            }

            case MOVEOBJECT16to16: {
                fprintf(defs,"#define MOVEOBJECT16to16 0x%X\n",insns[i]);

                Address16to16 addr1616;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr1616.To = recons16(msb, lsb);

                lsb = insns[++i];
                msb = insns[++i];
                addr1616.From = recons16(msb, lsb);
                registers[addr1616.To].bits = registers[addr1616.From].bits;
                ((Instance *) registers[addr1616.To].bits)->references++;
                break;
            }

            case MOVERESULTOBJECT:
            case MOVERESULTWIDE:
            case MOVERESULT: {
                u1 opcode = insns[i];
                u1 dest = insns[++i];
                registers[dest].bits = reservedRegs[0].bits;

                switch(opcode){
                    case MOVERESULTWIDE:
                        fprintf(defs,"#define MOVERESULTWIDE 0x%X\n",opcode);
                        registers[dest + 1].bits = reservedRegs[1].bits;
                        break;
                    case MOVERESULTOBJECT:
                        fprintf(defs,"#define MOVERESULTOBJECT 0x%X\n",opcode);
                        ((Instance *) registers[dest].bits)->references--;
                        break;
                    default:
                        fprintf(defs,"#define MOVERESULT 0x%X\n",opcode);
                        break;
                }
                free(reservedRegs);

                break;
            }


//            case MOVEEXCEPTION:
//
//                break;

            case RET_VOID:
                fprintf(defs,"#define RET_VOID 0x%X\n",insns[i]);
                goto RETURN;

            case RETOBJ:
            case RETWIDE:
            case RET: {
                u1 opcode = insns[i];
                u2 retValueReg = insns[++i];
                reservedRegs = (Reg *) malloc(sizeof(Reg));
                reservedRegs[0].bits = registers[retValueReg].bits;

                switch(opcode){
                    case RETWIDE:
                        fprintf(defs,"#define RETWIDE 0x%X\n",opcode);
                        reservedRegs[1].bits = registers[retValueReg + 1].bits;
                        break;
                    case RETOBJ:
                        fprintf(defs,"#define RETOBJ 0x%X\n",opcode);
                        ((Instance *) registers[retValueReg].bits)->references++; //prevent object referredto by returned object ref from being Garbage Collected when function ends.
                        break;
                    default:
                        fprintf(defs,"#define RET 0x%X\n",opcode);
                        break;
                }
                goto RETURN;
            }

            case CONSTvAB: //OK
            {
                fprintf(defs,"#define CONSTvAB 0x%X\n",insns[i]);
                NibbleAddress addr;
                u1 byte = insns[++i];
                addr.To = (byte & 0b00001111); //4 LSB ,4 bit register value
                u1 nibConst = byte >> 4;
                if ((nibConst & 0b00001000) == 0) //if MSB (SIGN BIT) is NOT 1, place value s4o register as is;
                    registers[addr.To].bits = nibConst;
                else
                    registers[addr.To].bits = SIGNEX4to32(nibConst);

                break;
            }

            case CONSTvAABBBB://OK
            {
                fprintf(defs,"#define CONSTvAABBBB 0x%X\n",insns[i]);
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                s2 Const = recons16(msb, lsb);
                registers[addr168.To].bits = Const;
                break;
            }
//

            case CONSTvAABBBBBBBB: //OK
            {
                fprintf(defs,"#define CONSTvAABBBBBBBB 0x%X\n",insns[i]);
                Address16to8 addr168;
                addr168.To = insns[++i];


                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 LSB = recons16(lsb2, lsb1);
                u1 msb1 = insns[++i];
                u1 msb2 = insns[++i];
                u2 MSB = recons16(msb2, msb1);

                registers[addr168.To].bits = recons32(MSB, LSB);


                break;
            }

            case CONSTvAA_4B_40: {
                fprintf(defs,"#define CONSTvAA_4B_40 0x%X\n",insns[i]);
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 msb = recons16(lsb2, lsb1);
                registers[addr168.To].bits = recons32(msb, 0);
                break;
            }


            case CONSTWIDEvAABBBB: {
                fprintf(defs,"#define CONSTWIDEvAABBBB 0x%X\n",insns[i]);
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                s2 Const = recons16(msb, lsb);
                if (Const >= 0) {
                    registers[addr168.To].bits = Const;
                    registers[addr168.To + 1].bits = 0;
                } else {
                    registers[addr168.To].bits = Const;
                    registers[addr168.To + 1].bits = x32BITMASK1s;
                    break;
                }
                break;
            }


            case CONSTWIDEvAA_8B: {
                fprintf(defs,"#define CONSTWIDEvAA_8B 0x%X\n",insns[i]);
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 LSB = recons16(lsb2, lsb1);
                u1 msb1 = insns[++i];
                u1 msb2 = insns[++i];
                u2 MSB = recons16(msb2, msb1);
                s4 a = recons32(MSB, LSB);
                registers[addr168.To].bits = a;

                if (a >= 0)
                    registers[addr168.To + 1].bits = 0;
                else
                    registers[addr168.To + 1].bits = x32BITMASK1s;
                break;
            }

            case CONSTWIDEvAA_16B: {
                fprintf(defs,"#define CONSTWIDEvAA_16B 0x%X\n",insns[i]);

                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 LSB = recons16(lsb2, lsb1);
                u1 msb1 = insns[++i];
                u1 msb2 = insns[++i];
                u2 MSB = recons16(msb2, msb1);

                registers[addr168.To].bits = recons32(MSB, LSB);

                lsb1 = insns[++i];
                lsb2 = insns[++i];
                LSB = recons16(lsb2, lsb1);
                msb1 = insns[++i];
                msb2 = insns[++i];
                MSB = recons16(msb2, msb1);
                registers[addr168.To + 1].bits = recons32(MSB, LSB);

                break;
            }

            case CONSTWIDEvAA_4B_120: {
                fprintf(defs,"#define CONSTWIDEvAA_4B_120 0x%X\n",insns[i]);

                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 msb = recons16(lsb2, lsb1);
                registers[addr168.To].bits = 0;
                registers[addr168.To + 1].bits = recons32(msb, 0);
                break;
            }
                //NOTE: Following 3 instructions deal with references to strings/classes not class data/string literals themselves
            case CONSTSTRINGvAA_4B: {
                fprintf(defs,"#define CONSTSTRINGvAA_4B 0x%X\n",insns[i]);

                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                registers[addr168.To].bits = recons16(msb, lsb);
                break;
            }


            case CONSTSTRINGvAA_8B: //ok
            {
                fprintf(defs,"#define CONSTSTRINGvAA_8B 0x%X\n",insns[i]);

                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 LSB = recons16(lsb2, lsb1);
                u1 msb1 = insns[++i];
                u1 msb2 = insns[++i];
                u2 MSB = recons16(msb2, msb1);

                registers[addr168.To].bits = recons32(MSB, LSB);
                registers[addr168.To + 1].bits = 0;
                break;
            }


            case CONSTCLASSvAA_4B://ok
            {
                fprintf(defs,"#define CONSTCLASSvAA_4B 0x%X\n",insns[i]);

                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                registers[addr168.To].bits = recons16(msb, lsb);
                break;
            }

//
//            case MONITOR_ENTER:
//            //Threading
//                break;
//
//            case MONITOR_EXIT:
//            //Threading
//                break;
//
//            case CHECKCAST:
//            //Only function is to throw an exception in event of invalid cast.
//                break;

            case INSTANCE_OF: {
                fprintf(defs,"#define INSTANCE_OF 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //Register Holding Instance address
                addr.To = insns[i]; //Result, 1 or 0

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 typeIndex = recons16(msb, lsb);

                if (((Instance *) registers[addr.From].bits)->classDef->class_idx[0] == typeIndex)
                    registers[addr.To].bits = 1;
                else
                    registers[addr.To].bits = 0;


                break;
            }

            case ARRAY_LENGTH: {
                fprintf(defs,"#define ARRAY_LENGTH 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                Array *array = (Array *) registers[addr.From].bits;
                registers[addr.To].bits = array->size;
                break;
            }

            case NEW_INSTANCE: {
                fprintf(defs,"#define NEW_INSTANCE 0x%X\n",insns[i]);

                u1 newInstanceRefReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 typeIndex = recons16(msb, lsb);

                Instance *instance = (Instance *) malloc(sizeof(Instance));

                u2 j;
                for (j = 0; j < p->sizes.class_defs_size; j++)
                    if (p->classDefs[j].class_idx[0] == typeIndex)
                        break;

                instance->classDef = &p->classDefs[j];
                instance->fields = (field_id_item *) malloc(
                        p->classDefs[j].classData->instance_fields_size * sizeof(field_id_item));
                instance->references = 0;

                registers[newInstanceRefReg].bits = instance;
                insts_count++;
                instances = realloc(instances, insts_count * sizeof(Instance *));
                instances[insts_count - 1] = instance;
                break;
            }

            case NEW_ARRAY: { //ok
                fprintf(defs,"#define NEW_ARRAY 0x%X\n",insns[i]);

                Array *array = (Array *) malloc(sizeof(Array));

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //Register containing Size of array
                addr.To = insns[i];    //Register where pos4er to contents of array to be placed
                u2 typeRef; //index s4o the type array,stored as a literal in the bytecode.
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                array->size = registers[addr.From].bits;

                typeRef = recons16(msb, lsb);

                switch (p->strings[p->type_ids[typeRef]].string_data[1]) {
                    case 'Z':
                        array->array = (u1 *) malloc(U1 * array->size);
                        array->type = 'Z';
                        break;
                    case 'B':
                        array->array = (u1 *) malloc(U1 * array->size);
                        array->type = 'B';
                        break;
                    case 'S':
                        array->array = (s2 *) malloc(U2 * array->size);
                        array->type = 'S';
                        break;
                    case 'C':
                        array->array = (u2 *) malloc(sizeof(u2) * array->size);
                        array->type = 'C';
                        break;
                    case 'I':
                        array->array = (s4 *) malloc(U4 * array->size);
                        array->type = 'I';
                        break;
                    case 'J':
                        array->array = (s8 *) malloc(U8 * array->size);
                        array->type = 'J';
                        break;
                    case 'F':
                        array->array = (float *) malloc(sizeof(float) * array->size);
                        array->type = 'F';
                        break;
                    case 'D':
                        array->array = (double *) malloc(sizeof(double) * array->size);
                        array->type = 'D';
                        break;


                }

                registers[addr.To].bits = array;

            }
                break;

            case FILLED_NEW_ARRAY: {
                fprintf(defs,"#define FILLED_NEW_ARRAY 0x%X\n",insns[i]);

                u1 size = insns[++i];
                size = size >> 4;
                //oddly enough, array size/number of parameters seems to be stored in the 4 msb of the byte following the opcode while the 4 lsb are left unused.
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 typeRef = recons16(msb, lsb);

                reservedRegs[0].bits = (Array *) malloc(sizeof(Array));
                Array *array = (Array *) reservedRegs[0].bits;

                ((Array *) reservedRegs[0].bits)->type = p->strings[p->type_ids[typeRef]].string_data[1];
                ((Array *) reservedRegs[0].bits)->size = size;

                switch (((Array *) reservedRegs[0].bits)->type) {
                    case 'Z':
                        array->array = (u1 *) malloc(U1 * size);

                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *((u1 *) array->array + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'B':
                        array->array = (u1 *) malloc(U1 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *((u1 *) array->array + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'S':
                        array->array = (s2 *) malloc(U2 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s2 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'C':
                        array->array = (u2 *) malloc(sizeof(u2) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((u2 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'I':
                        array->array = (s4 *) malloc(U4 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s4 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'J':
                        array->array = (s8 *) malloc(U8 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s8 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'F':
                        array->array = (float *) malloc(sizeof(float) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((float *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'D':
                        array->array = (double *) malloc(sizeof(double) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((double *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;

                }

                break;
            }


            case FILLED_NEW_ARRAY_RANGE: { //I can't seem to find the Java code to generate this opcode, thus not tested yet
                fprintf(defs,"#define FILLED_NEW_ARRAY_RANGE 0x%X\n",insns[i]);

                Array *array = (Array *) malloc(sizeof(Array));
                u1 size = insns[++i];
                array->size = size;
                u2 typeRef; //index s4o the type array,stored as a literal in the bytecode.
                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                typeRef = recons16(msb, lsb);

                reservedRegs[0].bits = array;


                switch (p->strings[p->type_ids[typeRef]].string_data[1]) {
                    case 'Z':
                        array->array = (u1 *) malloc(U1 * size);

                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *((u1 *) array->array + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'B':
                        array->array = (u1 *) malloc(U1 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *((u1 *) array->array + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'S':
                        array->array = (s2 *) malloc(U2 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s2 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'C':
                        array->array = (u2 *) malloc(sizeof(u2) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((u2 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'I':
                        array->array = (s4 *) malloc(U4 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s4 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'J':
                        array->array = (s8 *) malloc(U8 * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((s8 *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'F':
                        array->array = (float *) malloc(sizeof(float) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((float *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;
                    case 'D':
                        array->array = (double *) malloc(sizeof(double) * size);
                        for (u1 j = 0; i < size; j++) {
                            u2 arrayContentsReg = insns[++i];

                            *(((double *) array->array) + i) = registers[arrayContentsReg].bits;
                        }
                        break;

                }

                break;
            }
/////
            case FILL_ARRAY_DATA: { //ok
                fprintf(defs,"#define FILL_ARRAY_DATA 0x%X\n",insns[i]);

                u4 L = i;
                u2 arrayReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 LSB = recons16(msb, lsb);

                lsb = insns[++i];
                msb = insns[++i];

                u2 MSB = recons16(msb, lsb);

                s4 offsetToTable = recons32(MSB, LSB);
                L += 2 * offsetToTable;
                L += 2; //Skip over table type, always static data array in this case
                u2 bytesPerElement;
                lsb = insns[L];
                msb = insns[++L];
                bytesPerElement = recons16(msb, lsb);

                s4 numberOfElements;
                lsb = insns[++L];
                msb = insns[++L];
                LSB = recons16(msb, lsb);
                lsb = insns[++L];
                msb = insns[++L];
                MSB = recons16(msb, lsb);

                numberOfElements = recons32(MSB, LSB);
                Array *array = (Array *) registers[arrayReg].bits;

                for (u2 e = 0; e < numberOfElements; e++) {
                    switch (bytesPerElement) {
                        case 1:
                            ((u1 *) array->array)[e] = insns[++L];
                            break;
                        case 2: {
                            u1 lsb = insns[++L];
                            u1 msb = insns[++L];
                            ((u2 *) array->array)[e] = recons16(msb, lsb);
                            break;
                        }
                        case 4: {
                            lsb = insns[++L];
                            msb = insns[++L];
                            LSB = recons16(msb, lsb);
                            lsb = insns[++L];
                            msb = insns[++L];
                            MSB = recons16(msb, lsb);
                            ((u4 *) array->array)[e] = recons32(MSB, LSB);
                            break;
                        }
                        case 8: {
                            lsb = insns[++L];
                            msb = insns[++L];
                            LSB = recons16(msb, lsb);
                            lsb = insns[++L];
                            msb = insns[++L];
                            MSB = recons16(msb, lsb);
                            LSB = recons32(MSB, LSB);
                            lsb = insns[++L];
                            msb = insns[++L];
                            LSB = recons16(msb, lsb);
                            lsb = insns[++L];
                            msb = insns[++L];
                            MSB = recons16(msb, lsb);
                            MSB = recons32(MSB, LSB);
                            ((u8 *) array->array)[e] = recons64(MSB, LSB);
                            break;
                        }
                    }
                }


                break;
            }

//            case THROW:
//
//                break;

            case GOTO: {
                fprintf(defs,"#define GOTO 0x%X\n",insns[i]);

                s8 L = i;
                s1 offset = (s1) insns[++i];
                i = L + (2 * offset) - 1; //offset is in words, Dalvik words are 16-bit
                continue;
            }

            case GOTO16: {
                fprintf(defs,"#define GOTO16 0x%X\n",insns[i]);

                s8 L = i;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                s2 offset = (s2) recons16(msb, lsb);
                i = L + (2 * offset) - 1;
                continue;
            }

            case GOTO32: {
                fprintf(defs,"#define GOTO32 0x%X\n",insns[i]);

                s8 L = i;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 LSB = recons16(msb, lsb);
                lsb = insns[++i];
                msb = insns[++i];
                u2 MSB = recons16(msb, lsb);
                s4 offset = recons32(MSB, LSB);
                i = L + (2 * offset) - 1;
                continue;
            }

            case PACKED_SWITCH: { //ok
                fprintf(defs,"#define PACKED_SWITCH 0x%X\n",insns[i]);

                long L = i;
                long J = i;
                u1 testReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 LSB = recons16(msb, lsb);
                lsb = insns[++i];
                msb = insns[++i];
                u2 MSB = recons16(msb, lsb);

                u4 tableOffset = recons32(MSB, LSB);
                L += (2 * tableOffset) + 1;

                u2 numberOfCases;
                lsb = insns[++L];
                msb = insns[++L];
                numberOfCases = recons16(msb, lsb);

                lsb = insns[++L];
                msb = insns[++L];
                LSB = recons16(msb, lsb);
                lsb = insns[++L];
                msb = insns[++L];
                MSB = recons16(msb, lsb);
                u4 elementbase = recons32(MSB, LSB);


                if (registers[testReg].bits > elementbase + numberOfCases || registers[testReg].bits < elementbase)
                    continue; //default case

                for (u2 j = (u2) registers[testReg].bits - elementbase; j > 0; j--)
                    L += 4;

                lsb = insns[++L];
                msb = insns[++L];
                LSB = recons16(msb, lsb);
                lsb = insns[++L];
                msb = insns[++L];
                MSB = recons16(msb, lsb);
                u4 offSet = recons32(MSB, LSB);

                i = J + (2 * offSet) - 1;
                continue;
            }


            case SPARSE_SWITCH: { //ok
                fprintf(defs,"#define SPARSE_SWITCH 0x%X\n",insns[i]);

                long L = i;
                long J = i;
                u1 testReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 LSB = recons16(msb, lsb);
                lsb = insns[++i];
                msb = insns[++i];
                u2 MSB = recons16(msb, lsb);

                u4 tableOffset = recons32(MSB, LSB);
//             long K = i;
                L += (2 * tableOffset) + 1;

                u2 numberOfCases;
                lsb = insns[++L];
                msb = insns[++L];
                numberOfCases = recons16(msb, lsb);


                s4 index = -1;
                for (u2 j = 0; j < numberOfCases; j++) {
                    lsb = insns[++L];
                    msb = insns[++L];
                    LSB = recons16(msb, lsb);
                    lsb = insns[++L];
                    msb = insns[++L];
                    MSB = recons16(msb, lsb);


                    s4 tableConstant = recons32(MSB, LSB);

                    if (tableConstant == registers[testReg].bits)
                        index = j;

                }

                if (index == -1) //if not found
                    continue;


                for (u2 j = index; j > 0; j--)
                    L += 4;

                lsb = insns[++L];
                msb = insns[++L];
                LSB = recons16(msb, lsb);
                lsb = insns[++L];
                msb = insns[++L];
                MSB = recons16(msb, lsb);
                long offSet = recons32(MSB, LSB);

                i = J + (2 * offSet) - 1;
                continue;
            }

            case CMPG_FLOAT:
            case CMPL_FLOAT: { //ok
                if(insns[i]==0x2E)
                    fputs("#define CMPG_FLOAT 0x2E",defs);
                else
                    fputs("#define CMPL_FLOAT 0x2F",defs);


                u1 gOrL = insns[i];
                BinOP binOP;
                binOP.Dest = insns[++i];
                binOP.Op1Addr = insns[++i];
                binOP.Op2Addr = insns[++i];
                float op1 = ieee754Float(registers[binOP.Op1Addr].bits);
                float op2 = ieee754Float(registers[binOP.Op2Addr].bits);

                if (op1 == op2)
                    registers[binOP.Dest].bits = 0;
                else if (op1 > op2)
                    registers[binOP.Dest].bits = 1;
                else if (op1 < op2)
                    registers[binOP.Dest].bits = -1;

//
//                if (op1 == NAN || op2 == NAN){
//                    if (gOrL == CMPL_FLOAT)
//                        registers[binOP.Dest].bits = -1;
//                    else
//                        registers[binOP.Dest].bits = 1;
//
//                }
                break;
            }

            case CMPG_DOUBLE:
            case CMPL_DOUBLE: {
                if(insns[i]==0x2f)
                    fputs("#define CMPL_DOUBLE 0x2F\n",defs);
                else
                    fputs("#define CMPG_DOUBLE 0x30\n",defs);
                u1 gOrL = insns[i];
                BinOP binOP;
                binOP.Dest = insns[++i];
                binOP.Op1Addr = insns[++i];
                binOP.Op2Addr = insns[++i];
                double op1 = ieee754Double(recons64(registers[binOP.Op1Addr + 1].bits, registers[binOP.Op1Addr].bits));
                double op2 = ieee754Double(recons64(registers[binOP.Op2Addr + 1].bits, registers[binOP.Op2Addr].bits));

                if (op1 == op2)
                    registers[binOP.Dest].bits = 0;
                else if (op1 > op2)
                    registers[binOP.Dest].bits = 1;
                else if (op1 < op2)
                    registers[binOP.Dest].bits = -1;


////                if (op1 == NAN || op2 == NAN)
//                    if (gOrL == CMPL_DOUBLE)
//                        registers[binOP.Dest].bits = -1;
//                    else
//                        registers[binOP.Dest].bits = 1;
                break;
            }

            case CMP_LONG: {
                fprintf(defs,"#define CMP_LONG 0x%X\n",insns[i]);
                BinOP binOP;
                binOP.Dest = insns[++i];
                binOP.Op1Addr = insns[++i];
                binOP.Op2Addr = insns[++i];
                long op1 = (long) recons64(registers[binOP.Op1Addr + 1].bits, registers[binOP.Op1Addr].bits);
                long op2 = (long) recons64(registers[binOP.Op2Addr + 1].bits, registers[binOP.Op2Addr].bits);

                if (op1 == op2)
                    registers[binOP.Dest].bits = 0;
                else if (op1 > op2)
                    registers[binOP.Dest].bits = 1;
                else if (op1 < op2)
                    registers[binOP.Dest].bits = -1;

//                if (op1 == NAN || op2 == NAN)
//                    registers[binOP.Dest].bits = 1;

                break;
            }
            case IFLE:
            case IFGT:
            case IFGE:
            case IFLT:
            case IFNE:
            case IFEQ: {
                u1 opcode = insns[i];
                u1 cond = 0;
                u4 I = i;
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                s2 offset = recons16(msb, lsb);

                switch (opcode) {
                    case IFEQ: {
                        fprintf(defs,"#define IFEQ 0x%X\n",opcode);
                        if (registers[addr.From].bits == registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
                    case IFNE: {
                        fprintf(defs,"#define IFNE 0x%X\n",opcode);
                        if (registers[addr.From].bits != registers[addr.To].bits)
                            cond = 1;
                        break;
                    }

                    case IFLT: {
                        fprintf(defs,"#define IFLT 0x%X\n",opcode);
                        if (registers[addr.From].bits > registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
                    case IFGE: {
                        fprintf(defs,"#define IFGE 0x%X\n",opcode);
                        if (registers[addr.From].bits <= registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
                    case IFGT: {
                        fprintf(defs,"#define IFGT 0x%X\n",opcode);
                        if (registers[addr.From].bits < registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
                    case IFLE: {
                        fprintf(defs,"#define IFLE 0x%X\n",opcode);
                        if (registers[addr.From].bits >= registers[addr.To].bits)
                            cond = 1;
                        break;
                    }

                }

                if (cond)
                    i = I + (2 * offset) - 1;

                break;
            }

            case IFLEZ:
            case IFGTZ:
            case IFGEZ:
            case IFLTZ:
            case IFNEZ:
            case IFEQZ: {//ok
                u1 opcode = insns[i];
                u1 cond = 0;
                long I = i;
                u1 reg = insns[++i];
                s4 x = (s4) registers[reg].bits;

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                s2 offset = recons16(msb, lsb);

                switch (opcode) {
                    case IFEQZ:
                        fprintf(defs,"#define IFEQZ 0x%X\n",opcode);

                        if (x == 0)
                            cond = 1;
                        break;
                    case IFNEZ:
                        fprintf(defs,"#define IFNEZ 0x%X\n",opcode);

                        if (x != 0)
                            cond = 1;
                        break;
                    case IFLTZ:
                        fprintf(defs,"#define IFLTZ 0x%X\n",opcode);

                        if (x < 0)
                            cond = 1;
                        break;
                    case IFGEZ:
                        fprintf(defs,"#define IFGEZ 0x%X\n",opcode);

                        if (x >= 0)
                            cond = 1;
                        break;
                    case IFGTZ:
                        fprintf(defs,"#define IFGTZ 0x%X\n",opcode);

                        if (x > 0)
                            cond = 1;
                        break;
                    case IFLEZ:
                        fprintf(defs,"#define IFLEZ 0x%X\n",opcode);

                        if (x <= 0)
                            cond = 1;
                        break;

                }


                if (cond)
                    i = I + (2 * offset) - 1;
                break;
            }

            case AGET:
            case AGET_SHORT:
            case AGET_CHAR:
            case AGET_BOOL:
            case AGET_BYTE:
            case AGET_OBJ:
            case AGET_WIDE:{//32 bit s4eger/float value
                u1 opcode = insns[i];
                u1 destReg = insns[++i];
                u1 arrayReg = insns[++i];
                u1 indexReg = insns[++i];

                Array *array = (Array *) registers[arrayReg].bits;


                u2 index = (u2) registers[indexReg].bits;
                switch(opcode){
                    case AGET:
                        fprintf(defs,"#define AGET 0x%X\n",opcode);

                        registers[destReg].bits = ((s4 *) array->array)[index];
                        break;
                    case AGET_BOOL:
                        fprintf(defs,"#define AGET_BOOL 0x%X\n",opcode);

                        registers[destReg].bits = ((s1 *) array->array)[index];
                        break;
                    case AGET_BYTE:
                        fprintf(defs,"#define AGET_BYTE 0x%X\n",opcode);

                        registers[destReg].bits = ((s1 *) array->array)[index];
                        break;

                    case AGET_OBJ:
                        fprintf(defs,"#define AGET_OBJ 0x%X\n",opcode);

                        registers[destReg].bits = ((s4 *) array->array)[index];
                        break;
                    case AGET_CHAR:
                        fprintf(defs,"#define AGET_CHAR 0x%X\n",opcode);

                        registers[destReg].bits = ((u2 *) array->array)[index];
                        break;
                    case AGET_SHORT:
                        fprintf(defs,"#define AGET_SHORT 0x%X\n",opcode);

                        registers[destReg].bits = ((s2 *) array->array)[index];
                        break;
                    case AGET_WIDE:{
                        fprintf(defs,"#define AGET_WIDE 0x%X\n",opcode);

                        u4 lsb = (u4) ((u8 *) array->array)[index];
                        u8 _msb = ((u8 *) array->array)[index];
                        u4 msb = (_msb >> 32);

                        registers[destReg].bits = lsb; //32 lsb
                        registers[destReg + 1].bits = msb; //32 msb
                        break;
                    }
                }


                break;
            }

            case APUT_SHORT:
            case APUT_CHAR:
            case APUT_BOOL:
            case APUT_BYTE:
            case APUT_OBJ:
            case APUT_WIDE:
            case APUT: {//32 bit s4eger/float value //ok
                u1 opcode = insns[i];
                u1 fromReg = insns[++i];
                u1 arrayReg = insns[++i];
                u1 indexReg = insns[++i];

                Array *array = (Array *) registers[arrayReg].bits;
                u2 index = (u2) registers[indexReg].bits;

                switch(opcode){
                    case APUT:
                        fprintf(defs,"#define APUT 0x%X\n",opcode);

                        ((s4 *) array->array)[index] = registers[fromReg].bits;
                        break;
                    case APUT_WIDE:{
                        fprintf(defs,"#define APUT_WIDE 0x%X\n",opcode);

                        u2 index = (u2) registers[indexReg].bits;
                        u4 lsb = registers[fromReg].bits;
                        u4 msb = registers[fromReg + 1].bits;
                        u8 put = recons64(msb, lsb);

                        ((u8 *) array->array)[index] = put;

                        break;
                    }
                    case APUT_OBJ:
                        fprintf(defs,"#define APUT_OBJ 0x%X\n",opcode);

                        ((u2 *) array->array)[index] = registers[fromReg].bits;
                        break;

                    case APUT_BOOL:
                        fprintf(defs,"#define APUT_BOOL 0x%X\n",opcode);

                        ((s1 *) array->array)[index] = registers[fromReg].bits;
                        break;
                    case APUT_BYTE:
                        fprintf(defs,"#define APUT_BYTE 0x%X\n",opcode);

                        ((s1 *) array->array)[index] = registers[fromReg].bits;
                        break;
                    case APUT_CHAR:
                        fprintf(defs,"#define APUT_CHAR 0x%X\n",opcode);

                        ((u2 *) array->array)[index] = registers[fromReg].bits;
                        break;
                    case APUT_SHORT:
                        fprintf(defs,"#define APUT_SHORT 0x%X\n",opcode);

                        ((s2 *) array->array)[index] = registers[fromReg].bits;
                        break;
                }


                break;
            }

            case IGET_SHORT:
            case IGET_CHAR:
            case IGET_BYTE:
            case IGET_BOOL:
            case IGET_OBJ:
            case IGET_WIDE:
            case IGET: {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //Instance Ref
                addr.To = insns[i]; //Register from

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);

                switch(opcode){
                    case IGET:
                        fprintf(defs,"#define IGET 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Integer;
                        break;
                    case IGET_WIDE:
                        fprintf(defs,"#define IGET_WIDE 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Long;
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Long >> 32;
                        break;
                    case IGET_OBJ:
                        fprintf(defs,"#define IGET_OBJ 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].ObjectRef;
                        break;
                    case IGET_BYTE:
                        fprintf(defs,"#define IGET_BYTE 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Byte;
                        break;
                    case IGET_BOOL:
                        fprintf(defs,"#define IGET_BOOL 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Byte;
                        break;
                    case IGET_CHAR:
                        fprintf(defs,"#define IGET_CHAR 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Char;
                        break;
                    case IGET_SHORT:
                        fprintf(defs,"#define IGET_SHORT 0x%X\n",opcode);

                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Short;
                        break;
                }

                break;
            }

            case IPUT_SHORT:
            case IPUT_CHAR:
            case IPUT_BYTE:
            case IPUT_BOOL:
            case IPUT_OBJ:
            case IPUT_WIDE:
            case IPUT: { //value in reg s4o Instance Ref
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //Instance Ref
                addr.From = insns[i]; //Register from

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);
                switch(opcode){
                    case IPUT_SHORT:
                        fprintf(defs,"#define IPUT_SHORT 0x%X\n",opcode);
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Short = (s2) registers[addr.From].bits;
                        break;
                    case IPUT_CHAR:
                        fprintf(defs,"#define IPUT_CHAR 0x%X\n",opcode);

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Char = (u2) registers[addr.From].bits;
                        break;
                    case IPUT_BYTE:
                        fprintf(defs,"#define IPUT_BYTE 0x%X\n",opcode);

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Byte = (s1) registers[addr.From].bits;
                        break;
                    case IPUT_BOOL:
                        fprintf(defs,"#define IPUT_BOOL 0x%X\n",opcode);

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Byte = (s1) registers[addr.From].bits;
                        break;
                    case IPUT_OBJ:{
                        fprintf(defs,"#define IPUT_OBJ 0x%X\n",opcode);

                        if (((Instance *) registers[addr.To].bits)->fields[fieldIndex].isObjRef == 1)
                            ((Instance *) registers[addr.To].bits)->fields[fieldIndex].ObjectRef->references--;

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].ObjectRef = (struct Instance *) registers[addr.From].bits;

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].isObjRef = 1;
                        ((struct Instance *) registers[addr.From].bits)->references++;
                        break;
                    }
                    case IPUT:
                        fprintf(defs,"#define IPUT 0x%X\n",opcode);

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Integer = (s4) registers[addr.From].bits;
                        break;
                    case IPUT_WIDE: {
                        fprintf(defs,"#define IPUT_WIDE 0x%X\n",opcode);

                        u4 Lsb = registers[addr.From].bits;
                        u4 Msb = registers[addr.From + 1].bits;
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Long = recons64(Msb, Lsb);
                        break;
                    }

                }
                break;
            }

            case SGET_SHORT:
            case SGET_CHAR:
            case SGET_BOOL:
            case SGET_BYTE:
            case SGET_OBJ:
            case SGET_WIDE:
            case SGET: {//OK
                u1 opcode = insns[i];
                u2 destReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);

                switch(opcode) {
                    case SGET_OBJ:
                        fprintf(defs,"#define SGET_OBJ 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].ObjectRef;
                        break;
                    case SGET:
                        fprintf(defs,"#define SGET 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].Integer;
                        break;
                    case SGET_WIDE:{
                        fprintf(defs,"#define SGET_WIDE 0x%X\n",opcode);
                        u8 data = p->field_id_items[fieldIndex].Long;
                        u4 Lsb = (u4) data;
                        u4 Msb = (u4) (data >> 32);
                        registers[destReg].bits = Lsb;
                        registers[destReg + 1].bits = Msb;
                        break;
                    }
                    case SGET_BOOL:
                        fprintf(defs,"#define SGET_BOOL 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].Byte;
                        break;
                    case SGET_BYTE:
                        fprintf(defs,"#define SGET_BYTE 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].Byte;
                        break;

                    case SGET_CHAR:
                        fprintf(defs,"#define SGET_CHAR 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].Char;
                        break;
                    case SGET_SHORT:
                        fprintf(defs,"#define SGET_SHORT 0x%X\n",opcode);
                        registers[destReg].bits = p->field_id_items[fieldIndex].Short;
                        break;
                }
                break;
            }

            case SPUT_SHORT:
            case SPUT_CHAR:
            case SPUT_BYTE:
            case SPUT_BOOL:
            case SPUT_OBJ:
            case SPUT_WIDE:
            case SPUT: {//OK
                u1 opcode = insns[i];
                u2 fromReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 fieldIndex = recons16(msb, lsb);

                switch(opcode) {
                    case SPUT:
                        fprintf(defs,"#define SPUT 0x%X\n",opcode);
                        p->field_id_items[fieldIndex].Integer = (s4) registers[fromReg].bits;
                        break;
                    case SPUT_WIDE:{
                        fprintf(defs,"#define SPUT_WIDE 0x%X\n",opcode);
                        u4 Lsb = registers[fromReg].bits;
                        u4 Msb = registers[fromReg + 1].bits;

                        p->field_id_items[fieldIndex].Long = recons64(Msb, Lsb);
                        break;
                    }

                    case SPUT_OBJ:
                        fprintf(defs,"#define SPUT_OBJ 0x%X\n",opcode);
                        if (p->field_id_items[fieldIndex].isObjRef)
                            p->field_id_items[fieldIndex].ObjectRef->references--; //decrements refcount of current object pos4ed to by the field (if there is one)

                        p->field_id_items[fieldIndex].ObjectRef = (Instance*) registers[fromReg].bits;
                        p->field_id_items[fieldIndex].isObjRef = 1;
                        (p->field_id_items[fieldIndex].ObjectRef)->references++;
                        break;
                    case SPUT_BYTE:
                        fprintf(defs,"#define SPUT_BYTE 0x%X\n",opcode);
                        p->field_id_items[fieldIndex].Byte = (s1) registers[fromReg].bits;
                        break;
                    case SPUT_BOOL:
                        fprintf(defs,"#define SPUT_BOOL 0x%X\n",opcode);
                        p->field_id_items[fieldIndex].Byte = (s1) registers[fromReg].bits;
                        break;
                    case SPUT_CHAR:
                        fprintf(defs,"#define SPUT_CHAR 0x%X\n",opcode);

                        p->field_id_items[fieldIndex].Char = (u2) registers[fromReg].bits;
                        break;
                    case SPUT_SHORT:
                        fprintf(defs,"#define SPUT_SHORT 0x%X\n",opcode);

                        p->field_id_items[fieldIndex].Short = (s2) registers[fromReg].bits;
                        break;
                }
                    break;
            }

            case INVOKE_SUPER:
            case INVOKE_VIRTUAL: {
                if(insns[i]==INVOKE_SUPER)
                    fputs("#define INVOKE_SUPER 0x6f\n",defs);
                else
                    fputs("#define INVOKE_VIRTUAL 0x6e\n",defs);

                encoded_method *method;
                u1 paramsSize = insns[++i];
                paramsSize = MSB(paramsSize);
                u1 paramRegs[paramsSize];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];


                u2 methodIndex = recons16(msb, lsb);

                if (paramsSize > 0) {
                    paramsToPass = malloc(paramsSize * U4);
                    if (paramsSize % 2 == 0) {
                        for (u1 p = 0; p < paramsSize / 2; p++) {
                            u1 paramReg = insns[++i];
                            paramRegs[2 * p] = LSB(paramReg);
                            paramRegs[2 * p + 1] = MSB(paramReg);
                        }
                    } else {
                        u1 paramReg;
                        for (u1 p = 0; p < (paramsSize - 1) / 2; p++) {
                            paramReg = insns[++i];
                            paramRegs[p] = LSB(paramReg);
                            paramRegs[p + 1] = MSB(paramReg);
                        }
                        paramReg = insns[++i];
                        paramRegs[paramsSize - 1] = LSB(paramReg);
                    }

                    for (u1 p = 0; p < paramsSize; p++)
                        paramsToPass[p] = registers[paramRegs[p]].bits;

                }


                method_id_item methodIdItem = p->method_id_items[methodIndex]; //assume this is what is meant by "method table" , not sure. need to confirm.
                class_data_item *classDataItem;

                for (s4 j = 0; j < p->sizes.class_defs_size; j++) {
                    if (p->classDefs[j].class_idx[0] == methodIdItem.class_idx[0]) {
                        classDataItem = p->classDefs[j].classData;
                        break;
                    }

                }

                u4 base = classDataItem->virtual_methods[0].method_idx_diff;
                if (p->method_id_items[base].name_idx[0] == methodIdItem.name_idx[0])
                    method = classDataItem->virtual_methods;
                else {
                    for (s4 k = 1; k < classDataItem->virtual_methods_size; k++) {
                        if (p->method_id_items[classDataItem->virtual_methods[k].method_idx_diff + base].name_idx[0] ==
                            methodIdItem.name_idx[0]) {
                            method = (classDataItem->virtual_methods + k);
                            break;
                        } else
                            base += classDataItem->virtual_methods[k].method_idx_diff;
                    }
                }
                interpreter(paramsToPass, paramsSize, method);

                if (paramsToPass != NULL)
                    free(paramsToPass);

                ///////////////
                break;
            }
//
            case INVOKE_STATIC:
            case INVOKE_DIRECT: {
                if(insns[i]==INVOKE_STATIC)
                    fputs("#define INVOKE_STATIC 0x71\n",defs);
                else
                    fputs("#define INVOKE_DIRECT 0x70\n",defs);

                encoded_method *method;
                u1 paramsSize = insns[++i];
                paramsSize = MSB(paramsSize);
                u1 paramRegs[paramsSize];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];


                u2 methodIndex = recons16(msb, lsb);

                if (paramsSize > 0) {
                    paramsToPass = malloc(paramsSize * U4);
                    if (paramsSize % 2 == 0) {
                        for (u1 p = 0; p < paramsSize / 2; p++) {
                            u1 paramReg = insns[++i];
                            paramRegs[p] = LSB(paramReg);
                            paramRegs[p + 1] = MSB(paramReg);
                        }
                    } else {
                        u1 paramReg;
                        for (u1 p = 0; p < (paramsSize - 1) / 2; p++) {
                            paramReg = insns[++i];
                            paramRegs[2 * p] = LSB(paramReg);
                            paramRegs[2 * p + 1] = MSB(paramReg);
                        }
                        paramReg = insns[++i];
                        paramRegs[paramsSize - 1] = LSB(paramReg);
                    }

                    for (u1 p = 0; p < paramsSize; p++)
                        paramsToPass[p] = registers[paramRegs[p]].bits;

                }
                if (p->method_id_items[methodIndex].is_ObjInit) {
                    free(paramsToPass);
                    break; // if method is the Object class constructor
                }


                method_id_item methodIdItem = p->method_id_items[methodIndex]; //assume this is what is meant by "method table" , not sure. need to confirm.
                class_data_item *classDataItem;

                for (u1 j = 0; j < p->sizes.class_defs_size; j++) {
                    if (p->classDefs[j].class_idx[0] == methodIdItem.class_idx[0]) {
                        classDataItem = p->classDefs[j].classData;
                        break;
                    }

                }

                u4 base = classDataItem->direct_methods[0].method_idx_diff;
                if (p->method_id_items[base].name_idx[0] == methodIdItem.name_idx[0])
                    method = classDataItem->direct_methods;
                else {
                    for (s4 k = 1; k < classDataItem->direct_methods_size; k++) {
                        if (p->method_id_items[classDataItem->direct_methods[k].method_idx_diff + base].name_idx[0] ==
                            methodIdItem.name_idx[0]) {
                            method = (classDataItem->direct_methods + k);
                            break;
                        } else
                            base += classDataItem->direct_methods[k].method_idx_diff;
                    }
                }
                interpreter(paramsToPass, paramsSize, method);

                if (paramsToPass != NULL)
                    free(paramsToPass);

                ///////////////
                break;
            }

            case INVOKE_SUPER_RANGE:
            case INVOKE_VIRTUAL_RANGE: {
                if(insns[i]==INVOKE_SUPER_RANGE)
                    fprintf(defs,"#define INVOKE_SUPER_RANGE 0x%X\n",INVOKE_SUPER_RANGE);
                else
                    fprintf(defs,"#define INVOKE_VIRTUAL_RANGE 0x%X\n",INVOKE_VIRTUAL_RANGE);

                u1 paramsSize = insns[++i];
                u4 *params = (u4 *) malloc(paramsSize * U4);
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 methodIndex = recons16(msb, lsb);
                u1 RegNo = insns[++i];

                for (u1 k = 0; k < paramsSize; k++) {
                    params[k] = registers[RegNo++].bits;
                }

                method_id_item methodIdItem = p->method_id_items[methodIndex]; //assume this is what is meant by "method table" , not sure. need to confirm.
                class_data_item *classDataItem;

                for (s4 j = 0; j < p->sizes.class_defs_size; j++) {
                    if (p->classDefs[j].class_idx[0] == methodIdItem.class_idx[0]) {
                        classDataItem = p->classDefs[j].classData;
                        break;
                    }

                }

                encoded_method *method = NULL;
                u4 base = classDataItem->virtual_methods[0].method_idx_diff;

                if (p->method_id_items[base].name_idx[0] == methodIdItem.name_idx[0])
                    method = classDataItem->virtual_methods;
                else {
                    for (s4 k = 1; k < classDataItem->virtual_methods_size; k++) {
                        if (p->method_id_items[base + classDataItem->virtual_methods[k].method_idx_diff].name_idx[0] ==
                            methodIdItem.name_idx[0]) {
                            method = (classDataItem->virtual_methods + k);
                            break;
                        } else
                            base += classDataItem->virtual_methods[k].method_idx_diff;

                    }
                }
                interpreter(params, paramsSize, method);
                free(params);

                //interpreter(p->method_handle_items[methodRefIndex].insns,p->method_handle_items[methodRefIndex].insns_size);

                //require more information on how to pass values s4o method, continue later.
                break;
            }

            case INVOKE_STATIC_RANGE:
            case INVOKE_DIRECT_RANGE: {
                if(insns[i]==INVOKE_STATIC_RANGE)
                    fprintf(defs,"#define INVOKE_STATIC_RANGE 0x%X\n",INVOKE_STATIC_RANGE);
                else
                    fprintf(defs,"#define INVOKE_DIRECT_RANGE 0x%X\n",INVOKE_DIRECT_RANGE);

                u1 paramsSize = insns[++i];
                u4 *params = (u4 *) malloc(paramsSize * U4);
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 methodIndex = recons16(msb, lsb);
                u1 RegNo = insns[++i];

                for (u1 k = 0; k < paramsSize; k++) {
                    params[k] = registers[RegNo++].bits;
                }

                method_id_item methodIdItem = p->method_id_items[methodIndex]; //assume this is what is meant by "method table" , not sure. need to confirm.
                class_data_item *classDataItem;

                for (s4 j = 0; j < p->sizes.class_defs_size; j++) {
                    if (p->classDefs[j].class_idx[0] == methodIdItem.class_idx[0]) {
                        classDataItem = p->classDefs[j].classData;
                        break;
                    }

                }

                encoded_method *method = NULL;
                u4 base = classDataItem->direct_methods[0].method_idx_diff;

                if (p->method_id_items[base].name_idx[0] == methodIdItem.name_idx[0])
                    method = classDataItem->direct_methods;
                else {
                    for (s4 k = 1; k < classDataItem->direct_methods_size; k++) {
                        if (p->method_id_items[base + classDataItem->direct_methods[k].method_idx_diff].name_idx[0] ==
                            methodIdItem.name_idx[0]) {
                            method = (classDataItem->direct_methods + k);
                            break;
                        } else
                            base += classDataItem->direct_methods[k].method_idx_diff;

                    }
                }
                interpreter(params, paramsSize, method);
                free(params);

                //interpreter(p->method_handle_items[methodRefIndex].insns,p->method_handle_items[methodRefIndex].insns_size);

                //require more information on how to pass values s4o method, continue later.
                break;
            }
//
////            case INVOKE_INTERFACE_RANGE:
////
////                break;
//
            case NOT_INT:
            case NEG_INT: {
                if(insns[i]==NOT_INT)
                    fprintf(defs,"#define NOT_INT 0x%X\n",NOT_INT);
                else 
                    fprintf(defs,"#define NEG_INT 0x%X\n",NEG_INT);

                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits ^ x32BITMASK1s;

                if(opcode==NEG_INT)
                    registers[addr.To].bits++;
                break;
            }


            case NOT_LONG:
            case NEG_LONG:{

                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits ^ x32BITMASK1s;
                registers[addr.To + 1].bits = registers[addr.From + 1].bits ^ x32BITMASK1s;
                switch(opcode){
                    case NEG_LONG:{
                        fprintf(defs,"#define NEG_LONG 0x%X\n",opcode);

                        if (registers[addr.To].bits == x32BITMASK1s) //if overflow in 32 LSB, carry to 32 MSB
                            registers[addr.To + 1].bits++;
                        else
                            registers[addr.To].bits++; //An assumption is made that bytecode generated by compiler will not attempt to negate a 0.
                        break;
                    }

                    case NOT_LONG:
                        fprintf(defs,"#define NOT_LONG 0x%X\n",opcode);
                        break;

                   }
            }


            case NEG_FLOAT: {
                fprintf(defs,"#define NEG_FLOAT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float tmp = ieee754Float(registers[addr.From].bits);
                tmp *= -1;
                registers[addr.To].bits = floatieee754(tmp);
                break;
            }

            case NEG_DOUBLE: {
                fprintf(defs,"#define NEG_DOUBLE 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                double tmp = ieee754Double(recons64(registers[addr.From].bits, registers[addr.To].bits));
                tmp *= -1;
                u8 tmp1 = doubleieee754(tmp);
                registers[addr.To].bits = tmp1; //LSB
                registers[addr.To + 1].bits = (tmp1 >> 32); //MSB
                break;
            }

            case INT_TO_LONG: {
                fprintf(defs,"#define INT_TO_LONG 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                registers[addr.To + 1].bits = 0;

                break;
            }

            case INT_TO_FLOAT: {
                fprintf(defs,"#define INT_TO_FLOAT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float x = ((float) registers[addr.From].bits);
                registers[addr.To].bits = floatieee754(x);
                break;
            }

            case INT_TO_DOUBLE: //ok
            {
                fprintf(defs,"#define INT_TO_DOUBLE 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                double tmp = (double) registers[addr.From].bits;
                u8 ieee754bits = doubleieee754(tmp);
                registers[addr.To].bits = ieee754bits;
                registers[addr.To + 1].bits = (ieee754bits >> 32);
                break;
            }
//
            case LONG_TO_INT: //TRUNCATION //ok
            {
                fprintf(defs,"#define LONG_TO_INT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                registers[addr.To].bits = registers[addr.From].bits; //copies 32 LSB to new location
                break;
            }


            case LONG_TO_FLOAT: {
                fprintf(defs,"#define LONG_TO_FLOAT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.From = insns[i];
                u8 x = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                float y = (float) ieee754Double(x);
                registers[addr.To].bits = floatieee754(y);
                break;
            }

            case LONG_TO_DOUBLE: {
                fprintf(defs,"#define LONG_TO_DOUBLE 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.From = insns[i];
                long tmp = (long) recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                double tmp1 = (double) tmp;
                u8 ieee754bits = doubleieee754(tmp1);
                registers[addr.To].bits = ieee754bits; //copies 32 LSB to new location
                registers[addr.To + 1].bits = (ieee754bits >> 32); //copies 32 MSB to new location
                break;
            }


            case FLOAT_TO_INT: {
                fprintf(defs,"#define FLOAT_TO_INT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float x = ieee754Float(registers[addr.From].bits);
                s4 y = (s4) x;
                registers[addr.To].bits = y;
                break;
            }


            case FLOAT_TO_LONG: {
                fprintf(defs,"#define FLOAT_TO_LONG 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                float x = ieee754Float(registers[addr.From].bits);
                u8 y = (u8) x;
                registers[addr.To].bits = y;
                registers[addr.To + 1].bits = (y >> 32); //copies 32 LSB to new location
                break;
            }
            case FLOAT_TO_DOUBLE: {
                fprintf(defs,"#define FLOAT_TO_DOUBLE 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.From = insns[i];

                double x = (double) ieee754Float(registers[addr.From].bits); //copies 32 LSB to new location
                u8 y = doubleieee754(x);
                registers[addr.To].bits = y; //copies 32 LSB to new location
                registers[addr.To + 1].bits = (y >> 32); //copies 32 LSB to new location
                break;
            }


            case DOUBLE_TO_INT: //ok
            {
                fprintf(defs,"#define DOUBLE_TO_INT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                u8 tmp = recons64(registers[addr.From + 1].bits,
                                  registers[addr.From].bits); //copies 32 LSB to new location
                u4 tmp1 = (u4) ieee754Double(tmp);

                registers[addr.To].bits = tmp1;
                break;
            }


            case DOUBLE_TO_LONG: //ok
            {
                fprintf(defs,"#define DOUBLE_TO_LONG 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];

                u8 tmp = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                u8 tmp1 = (u8) ieee754Double(tmp);

                registers[addr.To].bits = tmp1; //copies 32 LSB to new location
                registers[addr.To + 1].bits = (tmp1 >> 32); //copies 32 LSB to new location
                break;

            }

            case DOUBLE_TO_FLOAT://ok
            {
                fprintf(defs,"#define DOUBLE_TO_FLOAT 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                u8 x = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                float y = (float) ieee754Double(x);
                registers[addr.To].bits = floatieee754(y);
                break;
            }


            case INT_TO_SHORT:
            case INT_TO_CHAR:
            case INT_TO_BYTE: {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
//                u1 tmp = (u1)registers[addr.From].bits;
                switch(opcode){
                    case INT_TO_BYTE:
                        fprintf(defs,"#define INT_TO_BYTE 0x%X\n",opcode);
                        registers[addr.To].bits = ((s1) registers[addr.From].bits);
                        break;
                    case INT_TO_CHAR:
                        fprintf(defs,"#define INT_TO_CHAR 0x%X\n",opcode);
                        registers[addr.To].bits = ((u2) registers[addr.From].bits);
                        break;
                    case INT_TO_SHORT:
                        fprintf(defs,"#define INT_TO_SHORT 0x%X\n",opcode);
                        registers[addr.To].bits = ((s2) registers[addr.From].bits);
                        break;
                }

                break;
            }


//                //BINARY OPERATIONS //OK
            case USHR_INT:
            case SHR_INT:
            case SHL_INT:
            case XOR_INT:
            case OR_INT:
            case AND_INT:
            case REM_INT:
            case DIV_INT:
            case MUL_INT:
            case SUB_INT:
            case ADD_INT: {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                s4 op1 = (s4) registers[binop.Op1Addr].bits;
                s4 op2 = (s4) registers[binop.Op2Addr].bits;
                s4 result = 0;

                switch (opcode) {
                    case ADD_INT:
                        fprintf(defs,"#define ADD_INT 0x%X\n",opcode);

                        result = op1 + op2;
                    case SUB_INT:
                        fprintf(defs,"#define SUB_INT 0x%X\n",opcode);

                        result = op1 - op2;
                        break;
                    case MUL_INT:
                        fprintf(defs,"#define MUL_INT 0x%X\n",opcode);

                        result = op1 * op2;
                        break;
                    case DIV_INT:
                        fprintf(defs,"#define DIV_INT 0x%X\n",opcode);

                        result = op1 / op2;
                        break;
                    case REM_INT:
                        fprintf(defs,"#define REM_INT 0x%X\n",opcode);

                        result = op1 - (op1 / op2) * op2;
                        break;
                    case AND_INT:
                        fprintf(defs,"#define AND_INT 0x%X\n",opcode);

                        result = op1 & op2;
                        break;
                    case OR_INT:
                        fprintf(defs,"#define OR_INT 0x%X\n",opcode);

                        result = op1 | op2;
                        break;
                    case XOR_INT:
                        fprintf(defs,"#define XOR_INT 0x%X\n",opcode);

                        result = op1 ^ op2;
                        break;
                    case SHL_INT:
                        fprintf(defs,"#define SHL_INT 0x%X\n",opcode);

                        result = op1 << (op2 & 0x1f);
                        break;
                    case SHR_INT: {
                        fprintf(defs,"#define SHR_INT 0x%X\n",opcode);

                        s4 tmp = (s4) registers[binop.Op1Addr].bits;
                        tmp = tmp >> (registers[binop.Op2Addr].bits & 0x1f);
                        result = tmp;
                        break;
                    }
                    case USHR_INT:
                        fprintf(defs,"#define USHR_INT 0x%X\n",opcode);

                        result = op1 >> (op2 & 0x1f);
                        break;
                }
                registers[binop.Dest].bits = result;
                break;
            }
            case XOR_LONG:
            case OR_LONG:
            case AND_LONG:
            case REM_LONG:
            case DIV_LONG:
            case MUL_LONG:
            case SUB_LONG:
            case ADD_LONG: {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                s8 op1 = (s8) recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits);
                s8 op2 = (s8) recons64(registers[binop.Op2Addr + 1].bits, registers[binop.Op2Addr].bits);

                switch (opcode) {
                    case ADD_LONG:
                        fprintf(defs,"#define ADD_LONG 0x%X\n",opcode);

                        op1 += op2;
                        break;
                    case SUB_LONG:
                        fprintf(defs,"#define SUB_LONG 0x%X\n",opcode);

                        op1 -= op2;
                        break;
                    case MUL_LONG:
                        fprintf(defs,"#define MUL_LONG 0x%X\n",opcode);

                        op1 *= op2;
                        break;
                    case DIV_LONG:
                        fprintf(defs,"#define DIV_LONG 0x%X\n",opcode);

                        op1 /= op2;
                        break;
                    case REM_LONG:
                        fprintf(defs,"#define REM_LONG 0x%X\n",opcode);

                        op1 = op1 - (op1 / op2) * op2;
                        break;
                    case AND_LONG:
                        fprintf(defs,"#define AND_LONG 0x%X\n",opcode);

                        op1 &= op2;
                        break;
                    case OR_LONG:
                        fprintf(defs,"#define OR_LONG 0x%X\n",opcode);

                        op1 |= op2;
                        break;
                    case XOR_LONG:
                        fprintf(defs,"#define XOR_LONG 0x%X\n",opcode);

                        op1 ^= op2;
                        break;

                }
                //result

                registers[binop.Dest].bits = op1;
                registers[binop.Dest + 1].bits = (op1 >> 32);

                break;
            }
            case USHR_LONG:
            case SHL_LONG: {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                u8 op1 = recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits);
                u4 op2 = registers[binop.Op2Addr].bits;
                u8 result = 0;

                switch (opcode) {
                    case SHL_LONG:
                        fprintf(defs,"#define SHL_LONG 0x%X\n",opcode);

                        result = op1 << (op2 & 0x3f);
                        break;
                    case USHR_LONG:
                        fprintf(defs,"#define USHR_LONG 0x%X\n",opcode);

                        result = op1 >> (op2 & 0x3f);
                        break;
                }


                registers[binop.Dest].bits = result;
                registers[binop.Dest + 1].bits = (result >> 32);

                break;
            }

            case SHR_LONG: {
                fprintf(defs,"#define SHR_LONG 0x%X\n",insns[i]);

                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                s8 op1 = recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits);
                s4 op2 = registers[binop.Op2Addr].bits;
                s8 result = op1 << (op2 & 0x3f);


                registers[binop.Dest].bits = result;
                u8 tmp = (u4) result;
                tmp = tmp >> 32;
                registers[binop.Dest + 1].bits = tmp;
                break;
            }

            case REM_FLOAT:
            case DIV_FLOAT:
            case MUL_FLOAT:
            case SUB_FLOAT:
            case ADD_FLOAT: {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                float op1 = ieee754Float(registers[binop.Op1Addr].bits);
                float op2 = ieee754Float(registers[binop.Op2Addr].bits);

                switch (opcode) {
                    case ADD_FLOAT:
                        fprintf(defs,"#define ADD_FLOAT 0x%X\n",opcode);

                        registers[binop.Dest].bits = floatieee754(op1 + op2);
                        break;
                    case SUB_FLOAT:
                        fprintf(defs,"#define SUB_FLOAT 0x%X\n",opcode);

                        registers[binop.Dest].bits = floatieee754(op1 - op2);
                        break;
                    case MUL_FLOAT:
                        fprintf(defs,"#define MUL_FLOAT 0x%X\n",opcode);

                        registers[binop.Dest].bits = floatieee754(op1 * op2);
                        break;
                    case DIV_FLOAT:
                        fprintf(defs,"#define DIV_FLOAT 0x%X\n",opcode);

                        registers[binop.Dest].bits = floatieee754(op1 / op2);
                        break;
                    case REM_FLOAT: {
                        fprintf(defs,"#define REM_FLOAT 0x%X\n",opcode);

                        s4 roundedQuotient = (s4) (op1 / op2); //rounded towards zero
                        float rounded = (float) roundedQuotient;
                        registers[binop.Dest].bits = floatieee754(op1 - rounded * op2);
                        break;
                    }
                }
                break;
            }

            case REM_DOUBLE:
            case DIV_DOUBLE:
            case MUL_DOUBLE:
            case SUB_DOUBLE:
            case ADD_DOUBLE: {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                double op1 = ieee754Double(recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits));
                double op2 = ieee754Double(recons64(registers[binop.Op2Addr + 1].bits, registers[binop.Op2Addr].bits));

                u8 tmp = 0; //result

                switch (opcode) {
                    case ADD_DOUBLE:
                        fprintf(defs,"#define ADD_DOUBLE 0x%X\n",opcode);

                        tmp = doubleieee754(op1 + op2);
                        break;
                    case SUB_DOUBLE:
                        fprintf(defs,"#define SUB_DOUBLE 0x%X\n",opcode);

                        tmp = doubleieee754(op1 - op2);
                        break;
                    case MUL_DOUBLE:
                        fprintf(defs,"#define MUL_DOUBLE 0x%X\n",opcode);

                        tmp = doubleieee754(op1 * op2);
                        break;
                    case DIV_DOUBLE:
                        fprintf(defs,"#define DIV_DOUBLE 0x%X\n",opcode);

                        tmp = doubleieee754(op1 / op2);
                        break;
                    case REM_DOUBLE: {
                        fprintf(defs,"#define REM_DOUBLE 0x%X\n",opcode);

                        s8 roundedQuotient = (s8) (op1 / op2); //rounded towards zero
                        double rounded = (double) roundedQuotient;
                        u8 tmp = doubleieee754(op1 * op2);
                        break;
                    }
                }

                registers[binop.Dest].bits = tmp;
                registers[binop.Dest + 1].bits = (tmp >> 32);
                break;
            }

                /*
                 * Identical to previous operations except only two address operands are given,
                 * result of calculation is stored in the first operand register rather than a 3rd destination register
                 * Also operands are given as 4 bits.*/
            case SHR_INT2:
            case SHL_INT2:
            case XOR_INT2:
            case OR_INT2:
            case AND_INT2:
            case REM_INT2:
            case DIV_INT2:
            case MUL_INT2:
            case SUB_INT2:
            case ADD_INT2://OK
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                s4 op1 = registers[addr.To].bits;
                s4 op2 = registers[addr.From].bits;

                switch (opcode) {
                    case ADD_INT2:
                        fprintf(defs,"#define ADD_INT2 0x%X\n",opcode);

                        op1 += op2;
                        break;
                    case SUB_INT2:
                        fprintf(defs,"#define SUB_INT2 0x%X\n",opcode);

                        op1 -= op2;
                        break;
                    case MUL_INT2:
                        fprintf(defs,"#define MUL_INT2 0x%X\n",opcode);

                        op1 *= op2;
                        break;
                    case DIV_INT2:
                        fprintf(defs,"#define DIV_INT2 0x%X\n",opcode);

                        op1 /= op2;
                        break;
                    case REM_INT2:
                        fprintf(defs,"#define REM_INT2 0x%X\n",opcode);

                        op1 = op1 - (op1 / op2) * op2;
                        break;
                    case AND_INT2:
                        fprintf(defs,"#define AND_INT2 0x%X\n",opcode);

                        op1 &= op2;
                        break;
                    case OR_INT2:
                        fprintf(defs,"#define OR_INT2 0x%X\n",opcode);

                        op1 |= op2;
                        break;
                    case XOR_INT2:
                        fprintf(defs,"#define XOR_INT2 0x%X\n",opcode);

                        op1 ^= op2;
                        break;
                    case SHL_INT2:
                        fprintf(defs,"#define SHL_INT2 0x%X\n",opcode);

                        op1 = op1 << op2;
                        break;
                    case SHR_INT2:
                        op1 = op1 >> op2;
                        break;
                }


                registers[addr.To].bits = op1;
                break;
            }

            case USHR_INT2://ok
            {
                fprintf(defs,"#define USHR_INT2 0x%X\n",insns[i]);

                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                s4 op2 = registers[addr.From].bits;
                registers[addr.To].bits >>= op2;
                break;
            }

            case SHR_LONG2:
            case SHL_LONG2:
            case XOR_LONG2:
            case OR_LONG2:
            case AND_LONG2:
            case REM_LONG2:
            case DIV_LONG2:
            case MUL_LONG2:
            case SUB_LONG2:
            case ADD_LONG2://ok
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                s8 op1 = (s8) recons64(registers[addr.To + 1].bits, registers[addr.To].bits);
                s8 op2 = (s8) recons64(registers[addr.From + 1].bits, registers[addr.From].bits);

                switch (opcode) {
                    case ADD_LONG2:
                        fprintf(defs,"#define ADD_LONG2 0x%X\n",opcode);

                        op1 += op2;
                        break;
                    case SUB_LONG2:
                        fprintf(defs,"#define SUB_LONG2 0x%X\n",opcode);

                        op1 -= op2;
                        break;
                    case MUL_LONG2:
                        fprintf(defs,"#define MUL_LONG2 0x%X\n",opcode);

                        op1 *= op2;
                        break;
                    case DIV_LONG2:
                        fprintf(defs,"#define DIV_LONG2 0x%X\n",opcode);

                        op1 /= op2;
                        break;
                    case REM_LONG2:
                        fprintf(defs,"#define REM_LONG2 0x%X\n",opcode);

                        op1 = op1 - (op1 / op2) * op2;
                        break;
                    case AND_LONG2:
                        fprintf(defs,"#define AND_LONG2 0x%X\n",opcode);

                        op1 &= op2;
                        break;
                    case OR_LONG2:
                        fprintf(defs,"#define OR_LONG2 0x%X\n",opcode);

                        op1 |= op2;
                        break;
                    case XOR_LONG2:
                        fprintf(defs,"#define XOR_LONG2 0x%X\n",opcode);

                        op1 ^= op2;
                        break;
                    case SHL_LONG2:
                        fprintf(defs,"#define SHL_LONG2 0x%X\n",opcode);

                        op1 <<= op2;
                        break;
                    case SHR_LONG2:
                        fprintf(defs,"#define SHR_LONG2 0x%X\n",opcode);

                        op1 >>= op2; //result
                        break;
                }
                        registers[addr.To].bits = op1;
                        registers[addr.To + 1].bits = (op1 >> 32);
                        break;
                }


                case USHR_LONG2: {
                    fprintf(defs,"#define USHR_LONG2 0x%X\n",insns[i]);

                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                    u8 op1 = (u8) recons64(registers[addr.To + 1].bits, registers[addr.To].bits);
                    u8 op2 = (u8) recons64(registers[addr.From + 1].bits, registers[addr.From].bits);

                    op1 = op1 >> op2; //result

                    registers[addr.To].bits = op1;
                    registers[addr.To + 1].bits = (op1 >> 32);

                    break;
                }

                case REM_FLOAT2:
                case DIV_FLOAT2:
                case MUL_FLOAT2:
                case SUB_FLOAT2:
                case ADD_FLOAT2: {
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                    float op1;
                    float op2;

                    op1 = ieee754Float(registers[addr.To].bits);
                    op2 = ieee754Float(registers[addr.From].bits);


                    switch (opcode) {
                        case ADD_FLOAT2:
                            fprintf(defs,"#define ADD_FLOAT2 0x%X\n",opcode);

                            registers[addr.To].bits = floatieee754(op1 + op2);
                            break;
                        case SUB_FLOAT2:
                            fprintf(defs,"#define SUB_FLOAT2 0x%X\n",opcode);
                            registers[addr.To].bits = floatieee754(op1 - op2);
                            break;
                        case MUL_FLOAT2:
                            fprintf(defs,"#define MUL_FLOAT2 0x%X\n",opcode);

                            registers[addr.To].bits = floatieee754(op1 * op2);
                            break;
                        case DIV_FLOAT2:
                            fprintf(defs,"#define DIV_FLOAT2 0x%X\n",opcode);

                            registers[addr.To].bits = floatieee754(op1 / op2);
                            break;
                        case REM_FLOAT2: {
                            fprintf(defs,"#define REM_FLOAT2 0x%X\n",opcode);

                            s4 roundedQuotient = (s4) (op1 / op2); //rounded towards zero
                            float rounded = (float) roundedQuotient;
                            registers[addr.To].bits = floatieee754(op1 - rounded * op2);
                            break;
                        }
                    }

                    registers[addr.To].bits = floatieee754(op1);

                    break;
                }

                case REM_DOUBLE2:
                case DIV_DOUBLE2:
                case MUL_DOUBLE2:
                case SUB_DOUBLE2:
                case ADD_DOUBLE2: {
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                    double op1 = ieee754Double(recons64(registers[addr.To + 1].bits, registers[addr.To].bits));
                    double op2 = ieee754Double(recons64(registers[addr.From + 1].bits, registers[addr.From].bits));

                    switch(opcode){
                        case ADD_DOUBLE2:
                            fprintf(defs,"#define ADD_DOUBLE2 0x%X\n",opcode);

                            op1 += op2;
                             break;
                        case SUB_DOUBLE2:
                            fprintf(defs,"#define SUB_DOUBLE2 0x%X\n",opcode);

                            op1 -= op2;
                            break;
                        case MUL_DOUBLE2:
                            fprintf(defs,"#define MUL_DOUBLE2 0x%X\n",opcode);

                            op1 *= op2;
                            break;
                        case DIV_DOUBLE2:
                            fprintf(defs,"#define DIV_DOUBLE2 0x%X\n",opcode);

                            op1 /= op2;
                            break;
                        case REM_DOUBLE2:{
                            fprintf(defs,"#define REM_DOUBLE2 0x%X\n",opcode);

                            s8 roundedQuotient = (s8) (op1 / op2); //rounded towards zero
                            double rounded = (double) roundedQuotient;
                            op1 = op1 - (rounded * op2);
                            break;
                        }

                    }


                    u8 tmp = doubleieee754(op1); //result

                    registers[addr.To].bits = tmp;
                    registers[addr.To + 1].bits = (tmp >> 32);
                    break;
                }

                case XOR_INT_LIT16:
                case OR_INT_LIT16:
                case AND_INT_LIT16:
                case REM_INT_LIT16:
                case DIV_INT_LIT16:
                case MUL_INT_LIT16:
                case RSUB_INT:
                case ADD_INT_LIT16: {//ok
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                    u1 lsb = insns[++i];
                    u1 msb = insns[++i];
                    u2 literal = recons16(msb, lsb);

                    s4 op1 = (s4) registers[addr.From].bits;
                    s4 result = 0;

                    switch (opcode) {
                        case ADD_INT_LIT16:
                            fprintf(defs,"#define ADD_INT_LIT16 0x%X\n",opcode);

                            result = op1 + literal;
                            break;
                        case RSUB_INT:
                            fprintf(defs,"#define RSUB_INT 0x%X\n",opcode);

                            result = op1 - literal;
                            break;
                        case MUL_INT_LIT16:
                            fprintf(defs,"#define MUL_INT_LIT16 0x%X\n",opcode);

                            result = op1 * literal;
                            break;
                        case DIV_INT_LIT16:
                            fprintf(defs,"#define DIV_INT_LIT16 0x%X\n",opcode);

                            result = op1 / literal;
                            break;
                        case REM_INT_LIT16:
                            fprintf(defs,"#define REM_INT_LIT16 0x%X\n",opcode);

                            result = op1 - (op1 / literal) * literal;
                            break;
                        case AND_INT_LIT16:
                            fprintf(defs,"#define AND_INT_LIT16 0x%X\n",opcode);

                            result = op1 & literal;
                            break;
                        case OR_INT_LIT16:
                            fprintf(defs,"#define OR_INT_LIT16 0x%X\n",opcode);

                            result = op1 | literal;
                            break;
                        case XOR_INT_LIT16:
                            fprintf(defs,"#define XOR_INT_LIT16 0x%X\n",opcode);

                            result = op1 ^ literal;
                            break;
                    }

                    registers[addr.To].bits = result;
                    break;
                }

                case RSUB_INT_LIT8:
                case ADD_INT_LIT8:
                case MUL_INT_LIT8:
                case DIV_INT_LIT8:
                case REM_INT_LIT8:
                case AND_INT_LIT8:
                case OR_INT_LIT8:
                case XOR_INT_LIT8:
                case SHR_INT_LIT8:
                case SHL_INT_LIT8: {
                    u1 opcode = insns[i];
                    Address8to8 addr;
                    addr.To = insns[++i];
                    addr.From = insns[++i];
                    s1 literal = insns[++i];

                    s4 op1 = (s4) registers[addr.From].bits;
                    s4 result = 0;

                    switch (opcode) {
                        case RSUB_INT_LIT8:
                            fprintf(defs,"#define RSUB_INT_LIT8 0x%X\n",opcode);

                            result = op1 - literal;
                            break;
                        case ADD_INT_LIT8:
                            fprintf(defs,"#define ADD_INT_LIT8 0x%X\n",opcode);

                            result = op1 + literal;
                            break;
                        case SHR_INT_LIT8:
                            fprintf(defs,"#define SHR_INT_LIT8 0x%X\n",opcode);

                            result = op1 >> literal;
                            break;
                        case SHL_INT_LIT8:
                            fprintf(defs,"#define SHL_INT_LIT8 0x%X\n",opcode);

                            result = op1 << literal;
                            break;
                        case XOR_INT_LIT8:
                            fprintf(defs,"#define XOR_INT_LIT8 0x%X\n",opcode);

                            result = op1 ^ literal;
                            break;
                        case OR_INT_LIT8:
                            fprintf(defs,"#define OR_INT_LIT8 0x%X\n",opcode);

                            result = op1 | literal;
                            break;
                        case AND_INT_LIT8:
                            fprintf(defs,"#define AND_INT_LIT8 0x%X\n",opcode);

                            result = op1 & literal;
                            break;
                        case REM_INT_LIT8:
                            fprintf(defs,"#define REM_INT_LIT8 0x%X\n",opcode);

                            result = op1 - (op1 / literal) * literal;
                            break;
                        case DIV_INT_LIT8:
                            fprintf(defs,"#define DIV_INT_LIT8 0x%X\n",opcode);

                            result = op1 / literal;
                            break;
                        case MUL_INT_LIT8:
                            fprintf(defs,"#define MUL_INT_LIT8 0x%X\n",opcode);

                            result = op1 * literal;
                            break;

                    }

                    registers[addr.To].bits = result;

                    break;
                }

                case USHR_INT_LIT8: {
                    fprintf(defs,"#define USHR_INT_LIT8 0x%X\n",insns[i]);

                    Address8to8 addr;
                    addr.To = insns[++i];
                    addr.From = insns[++i];
                    s1 literal = insns[++i];

                    u4 op1 = registers[addr.From].bits;
                    u4 result = op1 >> literal;
                    registers[addr.To].bits = result;

                    break;
                }


                default:
                    continue;

            }
        }
        RETURN:
        garbageCollector(instances, insts_count);
        free(registers);
    }
//
