#include "interpreter.h"

rDex * p;
Reg * reservedRegs; //used to store method return results

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

    if(insts_count>0) {
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


const u1 * bytePntr;

rDex * readBytes(){
    bytePntr = bytes;
    rDex * rdex = malloc(sizeof(rDex));
    read((u1*)&rdex->sizes,SIZESSTRUCT_LOADER_SIZE,1);

    rdex->field_id_items = malloc(rdex->sizes.field_ids_size*FIELDIDITEM_LOADER_SIZE);
    for(u2 i=0;i<rdex->sizes.field_ids_size;i++){
        read((u1*)rdex->field_id_items+i,FIELDIDITEM_LOADER_SIZE,1);
    }

    rdex->type_ids = malloc(rdex->sizes.type_ids_size*U2);

    for(u1 i=0;i<rdex->sizes.type_ids_size;i++){
        read((u1*)&rdex->type_ids[i],U2,1);
    }
    rdex->method_id_items = malloc(rdex->sizes.method_ids_size*sizeof(method_id_item));
    for(u1 i=0;i<rdex->sizes.method_ids_size;i++){
        read((u1*)&rdex->method_id_items[i].class_idx[0],U2,1);
        read((u1*)&rdex->method_id_items[i].name_idx[0],U2,1);
        read(&rdex->method_id_items[i].is_main,U1,1);
        read(&rdex->method_id_items[i].is_ObjInit,U1,1);
    }

    rdex->classDefs = malloc(sizeof(class_def_item)*rdex->sizes.class_defs_size);
    for(u1 i=0;i<rdex->sizes.class_defs_size;i++){
        read((u1*)&rdex->classDefs[i].class_idx[0],U2,1);
        read(& rdex->classDefs[i].class_data_off,U1,1);
        if(rdex->classDefs[i].class_data_off!=0) {
            rdex->classDefs[i].classData = malloc(sizeof(class_data_item));
            read(&rdex->classDefs[i].classData->static_fields_size, U1, 1);
            read(&rdex->classDefs[i].classData->instance_fields_size, U1, 1);
            read(&rdex->classDefs[i].classData->direct_methods_size, U1, 1);
            read(&rdex->classDefs[i].classData->virtual_methods_size, U1, 1);

            rdex->classDefs[i].classData->instance_fields = malloc(rdex->classDefs[i].classData->instance_fields_size*sizeof(encoded_field));
            for(u1 j=0;j<rdex->classDefs[i].classData->instance_fields_size;j++) {
                read((u1*)&rdex->classDefs[i].classData->instance_fields[j].field_idx_diff,U2, 1);
            }

            rdex->classDefs[i].classData->static_fields = malloc(rdex->classDefs[i].classData->static_fields_size*sizeof(encoded_field));
            for(u1 j=0;j<rdex->classDefs[i].classData->static_fields_size;j++) {
                read((u1*)&rdex->classDefs[i].classData->static_fields[j].field_idx_diff,U2, 1);
            }

            if(rdex->classDefs[i].classData->direct_methods_size>0){
                rdex->classDefs[i].classData->direct_methods = malloc(rdex->classDefs[i].classData->direct_methods_size*sizeof(encoded_method));
                for(int j=0;j<rdex->classDefs[i].classData->direct_methods_size;j++){
                    read((u1*)&rdex->classDefs[i].classData->direct_methods[j].method_idx_diff,U2,1);

                    (rdex->classDefs[i].classData->direct_methods+j)->methodCode = malloc(sizeof(code_item));
                    read((u1*)(rdex->classDefs[i].classData->direct_methods+j)->methodCode,CODEITEMSIZE_LOADER,1);

                    (rdex->classDefs[i].classData->direct_methods+j)->methodCode->insns = malloc((rdex->classDefs[i].classData->direct_methods+j)->methodCode->insns_size[0]);
                    read((u1*)(rdex->classDefs[i].classData->direct_methods+j)->methodCode->insns,U1,(rdex->classDefs[i].classData->direct_methods+j)->methodCode->insns_size[0]);
                }
            }

            if(rdex->classDefs[i].classData->virtual_methods_size>0){
                rdex->classDefs[i].classData->virtual_methods = malloc(rdex->classDefs[i].classData->virtual_methods_size*sizeof(encoded_method));
                for(int j=0;j<rdex->classDefs[i].classData->virtual_methods_size;j++){
                    read((u1*)&rdex->classDefs[i].classData->virtual_methods[j].method_idx_diff,U1,1);

                    (rdex->classDefs[i].classData->virtual_methods+j)->methodCode = malloc(sizeof(code_item));
                    read((u1*)(rdex->classDefs[i].classData->virtual_methods+j)->methodCode,CODEITEMSIZE_LOADER,1);

                    (rdex->classDefs[i].classData->virtual_methods+j)->methodCode->insns = malloc((rdex->classDefs[i].classData->virtual_methods+j)->methodCode->insns_size[0]);
                    read((u1*)(rdex->classDefs[i].classData->virtual_methods+j)->methodCode->insns,U1,(rdex->classDefs[i].classData->virtual_methods+j)->methodCode->insns_size[0]);
                }
        }
        }
    }

    rdex->strings = malloc(sizeof(String)*rdex->sizes.string_ids_size);
    for(int i=0;i<rdex->sizes.string_ids_size;i++){
        read((u1*)rdex->strings+i,STRINGSTRUCTSIZE_LOADER,1);
        /*string data in constant pool larger than 2 chars (ie: types) is not written, string comparisons to strings in the constant pool may be carried
         * out anyway since string_id is still stored.
         * String literals may be used but must be prefixed with a leading $ in the Java code, otherwise they will be optimized out of
         * the constant pool.*/
        if(rdex->strings[i].len>0){
            rdex->strings[i].string_data = malloc(rdex->strings[i].len);
            read((u1*)rdex->strings[i].string_data, U1, rdex->strings[i].len);
        }
    }
    //read(p.method_handle_items,sizeof(method_handle_item),p.sizes.method_handle_items_size);
    //read(p.call_site_ids,U4,p.sizes.call_site_ids_size);


//    fclose(pfile);
    return rdex;
}

void read(u1 *ptr, size_t size, size_t nmemb){
    size_t x = size*nmemb;
    u4 i = 0;
    while(x--){
        ptr[i++] = *(bytePntr++);

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
            u2 base = classDataItem->direct_methods[0].method_idx_diff;
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

    u4 i;
    for (i = 0; i < methodItem->methodCode->insns_size[0]; i++) {
        switch (insns[i]) {
#ifdef MOVE
            case MOVE: {
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
#endif
#ifdef MOVE16to8
            case MOVE16to8: {
                Address16to8 addr816;
                addr816.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);
                registers[addr816.To].bits = registers[addr816.From].bits;
                registers[addr816.From].bits = 0;
                break;
            }
#endif
#ifdef MOVE16to16
            case MOVE16to16: {
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
#endif
                //default wide moved is 4 bit addressed (first 16 registers)
#ifdef MOVEWIDE
            case MOVEWIDE: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "From" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                registers[addr.To + 1].bits = registers[addr.From + 1].bits;
                break;
            }
#endif
#ifdef MOVEWIDE16to8
            case MOVEWIDE16to8: {
                Address16to8 addr816;
                addr816.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);

                registers[addr816.To].bits = registers[addr816.From].bits; //16 LSB
                registers[addr816.To + 1].bits = registers[addr816.From + 1].bits; //16 MSB
                break;
            }
#endif
#ifdef MOVEWIDE16to16
            case MOVEWIDE16to16: {
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
#endif

#ifdef MOVEOBJECT
            case MOVEOBJECT: //Why is this a different instruciton from MOVE ? Bytecode spec doesn't say.
            {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                ((Instance *) registers[addr.To].bits)->references++;
//                registers[addr.From].bits = 0;
                break;
            }
#endif
#ifdef MOVEOBJECT16to8
            case MOVEOBJECT16to8: {
                Address16to8 addr816;
                addr816.To = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                addr816.From = recons16(msb, lsb);

                registers[addr816.To].bits = registers[addr816.From].bits;
                ((Instance *) registers[addr816.To].bits)->references++;
                break;
            }
#endif
#ifdef MOVEOBJECT16to16
            case MOVEOBJECT16to16: {
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
#endif

#ifdef MOVERESULTOBJECT
                case MOVERESULTOBJECT:
#endif
#ifdef MOVERESULTWIDE
                case MOVERESULTWIDE:
#endif
#ifdef MOVERESULT
                case MOVERESULT:
#endif
#ifdef MOVERESULTOBJECT
#elif MOVERESULTWIDE
#elif MOVERESULT
            {
            u1 opcode = insns[i];
            u1 dest = insns[++i];
            registers[dest].bits = reservedRegs[0].bits;

            switch(opcode){
#ifdef MOVERESULTWIDE
                case MOVERESULTWIDE:
                    registers[dest + 1].bits = reservedRegs[1].bits;
                    break;
#endif
#ifdef MOVERESULTOBJECT
                case MOVERESULTOBJECT:
                    ((Instance *) registers[dest].bits)->references--;
                    break;
#endif
                default:
                    break;
            }
            free(reservedRegs);

            break;
        }
#endif

//            case MOVEEXCEPTION:
//
//                break;
#ifdef RET_VOID
            case RET_VOID:
                goto RETURN;
#endif
#ifdef RET_OBJ
                case RETOBJ:
#endif
#ifdef RETWIDE
                case RETWIDE:
#endif
#ifdef RET
                case RET:
#endif
#ifdef RET_OBJ
#elif RETWIDE
#elif RET
            {
            u1 opcode = insns[i];
            u2 retValueReg = insns[++i];
            reservedRegs = (Reg *) malloc(sizeof(Reg));
            reservedRegs[0].bits = registers[retValueReg].bits;

            switch(opcode){
#ifdef RETWIDE
                case RETWIDE:
                    reservedRegs[1].bits = registers[retValueReg + 1].bits;
                    break;
#endif
#ifdef RETOBJ
                case RETOBJ:
                    ((Instance *) registers[retValueReg].bits)->references++; //prevent object referredto by returned object ref from being Garbage Collected when function ends.
                    break;
#endif
                default:
                    break;
            }
            goto RETURN;
        }
#endif
#ifdef CONSTvAB
            case CONSTvAB: //OK
            {
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
#endif
#ifdef CONSTvAABBBB
            case CONSTvAABBBB://OK
            {
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                s2 Const = recons16(msb, lsb);
                registers[addr168.To].bits = Const;
                break;
            }
#endif
#ifdef CONSTvAABBBBBBBB
            case CONSTvAABBBBBBBB: //OK
            {
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
#endif
#ifdef CONSTvAA_4B_40

            case CONSTvAA_4B_40: {
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 msb = recons16(lsb2, lsb1);
                registers[addr168.To].bits = recons32(msb, 0);
                break;
            }
#endif
#ifdef CONSTWIDEvAABBBB
            case CONSTWIDEvAABBBB: {
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
#endif

#ifdef CONSTWIDEvAA_8B
            case CONSTWIDEvAA_8B: {
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
#endif

#ifdef CONSTWIDEvAA_16B
            case CONSTWIDEvAA_16B: {
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
#endif

#ifdef CONSTWIDEvAA_4B_120
            case CONSTWIDEvAA_4B_120: {
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb1 = insns[++i];
                u1 lsb2 = insns[++i];
                u2 msb = recons16(lsb2, lsb1);
                registers[addr168.To].bits = 0;
                registers[addr168.To + 1].bits = recons32(msb, 0);
                break;
            }
#endif
#ifdef CONSTSTRINGvAA_4B
            //NOTE: Following 3 instructions deal with references to strings/classes not class data/string literals themselves
        case CONSTSTRINGvAA_4B: {
            Address16to8 addr168;
            addr168.To = insns[++i];

            u1 lsb = insns[++i];
            u1 msb = insns[++i];
            registers[addr168.To].bits = recons16(msb, lsb);
            break;
        }
#endif

#ifdef CONSTSTRINGvAA_8B
            case CONSTSTRINGvAA_8B: //ok
            {
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
#endif

#ifdef CONSTCLASSvAA_4B
            case CONSTCLASSvAA_4B://ok
            {
                Address16to8 addr168;
                addr168.To = insns[++i];

                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                registers[addr168.To].bits = recons16(msb, lsb);
                break;
            }
#endif
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
#ifdef INSTANCE_OF
            case INSTANCE_OF: {
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
#endif
#ifdef ARRAY_LENGTH
            case ARRAY_LENGTH: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                Array *array = (Array *) registers[addr.From].bits;
                registers[addr.To].bits = array->size;
                break;
            }
#endif
#ifdef NEW_INSTANCE
            case NEW_INSTANCE: {
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
#endif
#ifdef NEW_ARRAY
            case NEW_ARRAY: { //ok
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

                registers[addr.To].bits = (u4) array;
                break;
            }
#endif

#ifdef FILLED_NEW_ARRAY
            case FILLED_NEW_ARRAY: {
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
#endif
#ifdef FILLED_NEW_ARRAY_RANGE

            case FILLED_NEW_ARRAY_RANGE: { //I can't seem to find the Java code to generate this opcode, thus not tested yet
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
#endif
#ifdef FILL_ARRAY_DATA
            case FILL_ARRAY_DATA: { //ok
                long L = i;
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

                for (s4 e = 0; e < numberOfElements; e++) {
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
#endif
//            case THROW:
//
//                break;
#ifdef GOTO
            case GOTO: {
                s8 L = i;
                s1 offset = (s1) insns[++i];
                i = L + (2 * offset) - 1; //offset is in words, Dalvik words are 16-bit
                continue;
            }
#endif
#ifdef GOTO16
            case GOTO16: {
                s8 L = i;
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                s2 offset = (s2) recons16(msb, lsb);
                i = L + (2 * offset) - 1;
                continue;
            }
#endif
#ifdef GOTO32
            case GOTO32: {
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
#endif
#ifdef PACKED_SWITCH
            case PACKED_SWITCH: { //ok
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
#endif
#ifdef SPARSE_SWITCH

            case SPARSE_SWITCH: { //ok
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
#endif
#ifdef CMPG_FLOAT
                case CMPG_FLOAT:
#endif
#ifdef CMPL_FLOAT
                case CMPL_FLOAT:
#endif
#if defined(CMPG_FLOAT) || defined(CMPL_FLOAT)
            { //ok

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
#endif
#ifdef CMPG_DOUBLE
                case CMPG_DOUBLE:
#endif
#ifdef CMPL_DOUBLE
                case CMPL_DOUBLE:
#endif
#if defined(CMPG_DOUBLE) || defined(CMPL_DOUBLE)

            {
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
#endif
#ifdef CMP_LONG
            case CMP_LONG: {
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
#endif
#ifdef IFLE
            case IFLE:
#endif
#ifdef IFGT
                case IFGT:
#endif
#ifdef IFGE
            case IFGE:
#endif
#ifdef IFLT
                case IFLT:
#endif
#ifdef IFNE
                case IFNE:
#endif
#ifdef IFEQ
                case IFEQ:
#endif

#if  defined(IFLE) || defined(IFGE) || defined (IFGT) || defined (IFLT) || defined(IFNE) || defined(IFEQ)

            {
                u1 opCode = insns[i];
                u1 cond = 0;
                u4 I = i;
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                s2 offset = recons16(msb, lsb);

                switch (opCode) {
#ifdef IFEQ
                    case IFEQ: {
                        if (registers[addr.From].bits == registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
#ifdef IFNE
                    case IFNE: {
                        if (registers[addr.From].bits != registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
#ifdef IFLT
                    case IFLT: {
                        if (registers[addr.From].bits > registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
#ifdef IFGE
                    case IFGE: {
                        if (registers[addr.From].bits <= registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
#ifdef IFGT
                    case IFGT: {
                        if (registers[addr.From].bits < registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
#ifdef IFLE
                    case IFLE: {
                        if (registers[addr.From].bits >= registers[addr.To].bits)
                            cond = 1;
                        break;
                    }
#endif
                }

                if (cond)
                    i = I + (2 * offset) - 1;

                break;
            }
#endif
#ifdef IFLEZ
                case IFLEZ:
#endif
#ifdef IFGTZ
                case IFGTZ:
#endif
#ifdef IFGEZ
                case IFGEZ:
#endif
#ifdef IFLTZ
                case IFLTZ:
#endif
#ifdef IFNEZ
                case IFNEZ:
#endif
#ifdef IFEQZ
                case IFEQZ:
#endif
#if defined(IFLEZ) || defined(IFGTZ) || defined(IFGEZ) || defined(IFLTZ) || defined(IFNEZ) || defined(IFEQZ)
            {//ok
                u1 opcode = insns[i];
                u1 cond = 0;
                long I = i;
                u1 reg = insns[++i];
                s4 x = (s4) registers[reg].bits;

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                s2 offset = recons16(msb, lsb);

                switch (opcode) {
#ifdef IFEQZ
                    case IFEQZ:
                        if (x == 0)
                            cond = 1;
                        break;
#endif
#ifdef IFNEZ
                    case IFNEZ:
                        if (x != 0)
                            cond = 1;
                            break;
#endif
#ifdef IFLTZ
                    case IFLTZ:
                        if (x < 0)
                            cond = 1;
                            break;
#endif
#ifdef IFGEZ
                    case IFGEZ:
                        if (x >= 0)
                            cond = 1;
                            break;
#endif
#ifdef IFGTZ
                    case IFGTZ:
                        if (x > 0)
                            cond = 1;
                            break;
#endif
#ifdef IFLEZ
                    case IFLEZ:
                        if (x <= 0)
                            cond = 1;
                            break;
#endif
                }


                if (cond)
                    i = I + (2 * offset) - 1;
                break;
            }
#endif

#ifdef AGET
            case AGET:
#endif
#ifdef AGET_SHORT
                case AGET_SHORT:
#endif
#ifdef AGET_CHAR
                case AGET_CHAR:
#endif
#ifdef AGET_BOOL
                case AGET_BOOL:
#endif
#ifdef AGET_BYTE
                case AGET_BYTE:
#endif
#ifdef AGET_OBJ
                case AGET_OBJ:
#endif
#ifdef AGET_WIDE
                case AGET_WIDE:
#endif
#if defined(AGET) || defined(AGET_SHORT) || defined(AGET_CHAR) || defined(AGET_BOOL) || defined(AGET_BYTE) || defined(AGET_OBJ) || defined(AGET_WIDE)
            {//32 bit s4eger/float value
                u1 opcode = insns[i];
                u1 destReg = insns[++i];
                u1 arrayReg = insns[++i];
                u1 indexReg = insns[++i];

                Array *array = (Array *) registers[arrayReg].bits;


                u2 index = (u2) registers[indexReg].bits;
                switch (opcode) {
#ifdef AGET
                    case AGET:
                        registers[destReg].bits = ((s4 *) array->array)[index];
                        break;
#endif
#ifdef AGET_BOOL
                    case AGET_BOOL:
                       registers[destReg].bits = ((s1 *) array->array)[index];
                        break;
#endif
#ifdef AGET_BYTE
                    case AGET_BYTE:
                        registers[destReg].bits = ((s1 *) array->array)[index];
                        break;
#endif
#ifdef AGET_OBJ
                    case AGET_OBJ:
                        registers[destReg].bits = ((s4 *) array->array)[index];
                        break;
#endif
#ifdef AGET_CHAR
                    case AGET_CHAR:
                        registers[destReg].bits = ((u2 *) array->array)[index];
                        break;
#endif
#ifdef AGET_SHORT
                    case AGET_SHORT:
                        registers[destReg].bits = ((s2 *) array->array)[index];
                        break;
#endif
#ifdef AGET_WIDE
                    case AGET_WIDE:{
                        u4 lsb = (u4) ((u8 *) array->array)[index];
                        u8 _msb = ((u8 *) array->array)[index];
                        u4 msb = (_msb >> 32);

                        registers[destReg].bits = lsb; //32 lsb
                        registers[destReg + 1].bits = msb; //32 msb
                        break;
                    }
#endif
                }


                break;
            }
#endif

#ifdef APUT_SHORT
                case APUT_SHORT:
#endif
#ifdef APUT_CHAR
                case APUT_CHAR:
#endif
#ifdef APUT_BOOL
                case APUT_BOOL:
#endif
#ifdef APUT_BYTE
                case APUT_BYTE:
#endif
#ifdef APUT_OBJ
                case APUT_OBJ:
#endif
#ifdef APUT_WIDE
                case APUT_WIDE:
#endif
#ifdef APUT
            case APUT:
#endif

#if defined(APUT) || defined(APUT_WIDE) || defined(APUT_SHORT) || defined(APUT_CHAR) || defined(APUT_BOOL) || defined(APUT_BYTE) || defined(APUT_OBJ)
            {//32 bit s4eger/float value //ok
                u1 opcode = insns[i];
                u1 fromReg = insns[++i];
                u1 arrayReg = insns[++i];
                u1 indexReg = insns[++i];

                Array *array = (Array *) registers[arrayReg].bits;
                u2 index = (u2) registers[indexReg].bits;

                switch (opcode) {
#ifdef APUT
                    case APUT:
                        ((s4 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
#ifdef APUT_WIDE
                    case APUT_WIDE:{
                        u2 index = (u2) registers[indexReg].bits;
                        u4 lsb = registers[fromReg].bits;
                        u4 msb = registers[fromReg + 1].bits;
                        u8 put = recons64(msb, lsb);

                        ((u8 *) array->array)[index] = put;

                        break;
                    }
#endif
#ifdef APUT_OBJ
                    case APUT_OBJ:
                        ((u2 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
#ifdef APUT_BOOL
                    case APUT_BOOL:
                    ((s1 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
#ifdef APUT_BYTE
                    case APUT_BYTE:
                        ((s1 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
#ifdef APUT_CHAR
                    case APUT_CHAR:
                        ((u2 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
#ifdef APUT_SHORT
                    case APUT_SHORT:
                        ((s2 *) array->array)[index] = registers[fromReg].bits;
                        break;
#endif
                }


                break;
            }
#endif
#ifdef IGET_SHORT
                case IGET_SHORT:
#endif
#ifdef IGET_CHAR
                case IGET_CHAR:
#endif
#ifdef IGET_BYTE
                case IGET_BYTE:
#endif
#ifdef IGET_BOOL
                case IGET_BOOL:
#endif
#ifdef IGET_OBJ
                case IGET_OBJ:
#endif
#ifdef IGET_WIDE
                case IGET_WIDE:
#endif
#ifdef IGET
                case IGET:
#endif

#if defined(IGET) || defined(IGET_WIDE) || defined(IGET_OBJ) || defined(IGET_BOOL) || defined(IGET_SHORT) || defined(IGET_CHAR) || defined(IGET_BYTE)
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //Instance Ref
                addr.To = insns[i]; //Register from

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);

                switch(opcode){
#ifdef IGET
                    case IGET:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Integer;
                        break;
#endif
#ifdef IGET_WIDE
                    case IGET_WIDE:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Long;
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Long >> 32;
                        break;
#endif
#ifdef IGET_OBJ
                    case IGET_OBJ:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].ObjectRef;
                        break;
#endif
#ifdef IGET_BYTE
                    case IGET_BYTE:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Byte;
                            break;
#endif
#ifdef IGET_BOOL
                    case IGET_BOOL:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Byte;
                        break;
#endif
#ifdef IGET_CHAR
                    case IGET_CHAR:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Char;
                        break;
#endif
#ifdef IGET_SHORT
                    case IGET_SHORT:
                        registers[addr.To].bits = ((Instance *) registers[addr.From].bits)->fields[fieldIndex].Short;
                        break;
#endif
                }

                break;
            }
#endif
#ifdef IPUT_SHORT
                case IPUT_SHORT:
#endif
#ifdef IPUT_CHAR
                case IPUT_CHAR:
#endif
#ifdef IPUT_BYTE
                case IPUT_BYTE:
#endif
#ifdef IPUT_BOOL
                case IPUT_BOOL:
#endif
#ifdef IPUT_OBJ
                case IPUT_OBJ:
#endif
#ifdef IPUT_WIDE
                case IPUT_WIDE:
#endif
#ifdef IPUT
                case IPUT:
#endif

#if defined(IPUT) || defined(IPUT_WIDE) || defined(IPUT_OBJ) || defined(IPUT_BOOL) || defined(IPUT_SHORT) || defined(IPUT_CHAR) || defined(IPUT_BYTE)
            { //value in reg s4o Instance Ref
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //Instance Ref
                addr.From = insns[i]; //Register from

                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);
                switch(opcode){
#ifdef IPUT_SHORT
                    case IPUT_SHORT:
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Short = (s2) registers[addr.From].bits;
                        break;
#endif
#ifdef IPUT_CHAR
                    case IPUT_CHAR:
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Char = (u2) registers[addr.From].bits;
                        break;
#endif
#ifdef IPUT_BYTE
                    case IPUT_BYTE:
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Byte = (s1) registers[addr.From].bits;
                        break;
#endif
#ifdef IPUT_BOOL
                    case IPUT_BOOL:
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Byte = (s1) registers[addr.From].bits;
                        break;
#endif
#ifdef IPUT_OBJ
                    case IPUT_OBJ:{
                        if (((Instance *) registers[addr.To].bits)->fields[fieldIndex].isObjRef == 1)
                            ((Instance *) registers[addr.To].bits)->fields[fieldIndex].ObjectRef->references--;

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].ObjectRef = (struct Instance *) registers[addr.From].bits;

                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].isObjRef = 1;
                        ((struct Instance *) registers[addr.From].bits)->references++;
                        break;
                    }
#endif
#ifdef IPUT
                    case IPUT:
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Integer = (s4) registers[addr.From].bits;
                        break;
#endif
#ifdef IPUT_WIDE
                    case IPUT_WIDE: {
                        u4 Lsb = registers[addr.From].bits;
                        u4 Msb = registers[addr.From + 1].bits;
                        ((Instance *) registers[addr.To].bits)->fields[fieldIndex].Long = recons64(Msb, Lsb);
                        break;
                    }
#endif
                }
                break;
            }
#endif
#ifdef SGET_SHORT
                case SGET_SHORT:
#endif
#ifdef SGET_CHAR
                case SGET_CHAR:
#endif
#ifdef SGET_BOOL
                case SGET_BOOL:
#endif
#ifdef SGET_BYTE
                case SGET_BYTE:
#endif
#ifdef SGET_OBJ
            case SGET_OBJ:
#endif
#ifdef SGET_WIDE
                case SGET_WIDE:
#endif
#ifdef SGET
                case SGET:
#endif

#if defined(SGET_OBJ) || defined(SGET) || defined(SGET_WIDE) || defined(SGET_SHORT) || defined(SGET_BYTE) || defined(SGET_CHAR) || defined(SGET_BOOL)
            {//OK
                u1 opcode = insns[i];
                u2 destReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];

                u2 fieldIndex = recons16(msb, lsb);

                switch (opcode) {
#ifdef SGET_OBJ
                    case SGET_OBJ:
                        registers[destReg].bits = (u4) p->field_id_items[fieldIndex].ObjectRef;
                        break;
#endif
#ifdef SGET
                    case SGET:
                        registers[destReg].bits = p->field_id_items[fieldIndex].Integer;
                        break;
#endif
#ifdef SGET_WIDE
                    case SGET_WIDE:{
                        u8 data = p->field_id_items[fieldIndex].Long;
                        u4 Lsb = (u4) data;
                        u4 Msb = (u4) (data >> 32);
                        registers[destReg].bits = Lsb;
                        registers[destReg + 1].bits = Msb;
                        break;
                    }
#endif
#ifdef SGET_BOOL
                    case SGET_BOOL:
                        registers[destReg].bits = p->field_id_items[fieldIndex].Byte;
                    break;
#endif
#ifdef SGET_BYTE
                    case SGET_BYTE:
                        registers[destReg].bits = p->field_id_items[fieldIndex].Byte;
                    break;
#endif
#ifdef SGET_CHAR
                    case SGET_CHAR:
                        registers[destReg].bits = p->field_id_items[fieldIndex].Char;
                        break;
#endif
#ifdef SGET_SHORT
                    case SGET_SHORT:
                        registers[destReg].bits = p->field_id_items[fieldIndex].Short;
                        break;
#endif
                }
                break;
            }
#endif

#ifdef SPUT_SHORT
                case SPUT_SHORT:
#endif
#ifdef SPUT_CHAR
                case SPUT_CHAR:
#endif
#ifdef SPUT_BYTE
                case SPUT_BYTE:
#endif
#ifdef SPUT_BOOL
                case SPUT_BOOL:
#endif
#ifdef SPUT_OBJ
            case SPUT_OBJ:
#endif
#ifdef SPUT_WIDE
                case SPUT_WIDE:
#endif
#ifdef SPUT
                case SPUT:
#endif
#if defined(SPUT_OBJ) || defined(SPUT) || defined(SPUT_WIDE) || defined(SPUT_BOOL) || defined(SPUT_BYTE) || defined(SPUT_SHORT) || defined(SPUT_CHAR)
            {//OK
                u1 opcode = insns[i];
                u2 fromReg = insns[++i];
                u1 lsb = insns[++i];
                u1 msb = insns[++i];
                u2 fieldIndex = recons16(msb, lsb);

                switch (opcode) {
#ifdef SPUT
                    case SPUT:
                        p->field_id_items[fieldIndex].Integer = (s4) registers[fromReg].bits;
                        break;
#endif
#ifdef SPUT_WIDE
                    case SPUT_WIDE:{
                        u4 Lsb = registers[fromReg].bits;
                        u4 Msb = registers[fromReg + 1].bits;

                        p->field_id_items[fieldIndex].Long = recons64(Msb, Lsb);
                        break;
                    }
#endif
#ifdef SPUT_OBJ
                    case SPUT_OBJ:
                        if (p->field_id_items[fieldIndex].isObjRef)
                            p->field_id_items[fieldIndex].ObjectRef->references--; //decrements refcount of current object pos4ed to by the field (if there is one)

                        p->field_id_items[fieldIndex].ObjectRef = (Instance *) registers[fromReg].bits;
                        p->field_id_items[fieldIndex].isObjRef = 1;
                        (p->field_id_items[fieldIndex].ObjectRef)->references++;
                        break;
#endif
#ifdef SPUT_BYTE
                    case SPUT_BYTE:
                        p->field_id_items[fieldIndex].Byte = (s1) registers[fromReg].bits;
                        break;
#endif
#ifdef SPUT_BOOL
                    case SPUT_BOOL:
                        p->field_id_items[fieldIndex].Byte = (s1) registers[fromReg].bits;
                        break;
#endif
#ifdef SPUT_CHAR
                    case SPUT_CHAR:
                        p->field_id_items[fieldIndex].Char = (u2) registers[fromReg].bits;
                        break;
#endif
#ifdef SPUT_SHORT
                    case SPUT_SHORT:
                        p->field_id_items[fieldIndex].Short = (s2) registers[fromReg].bits;
                        break;
#endif
                }
                break;
            }
#endif
#ifdef INVOKE_SUPER
                case INVOKE_SUPER:
#endif
#ifdef INVOKE_VIRTUAL
                case INVOKE_VIRTUAL:
#endif
#if defined(INVOKE_VIRTUAL) || defined(INVOKE_SUPER)
            {
                encoded_method *method;
                u1 paramsSize = insns[++i];
                paramsSize = MSB(paramsSize);
                u1 * paramRegs = malloc(paramsSize);

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
#endif
#ifdef INVOKE_STATIC
            case INVOKE_STATIC:
#endif
#ifdef INVOKE_DIRECT
                case INVOKE_DIRECT:
#endif
#if defined(INVOKE_STATIC) || defined(INVOKE_DIRECT)
            {
                encoded_method *method;
                u1 paramsSize = insns[++i];
                paramsSize = MSB(paramsSize);
                u1 * paramRegs = malloc(sizeof(paramsSize));

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

                for (s4 j = 0; j < p->sizes.class_defs_size; j++) {
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
#endif
#ifdef INVOKE_SUPER_RANGE
                case INVOKE_SUPER_RANGE:
#endif
#ifdef INVOKE_VIRTUAL_RANGE
                case INVOKE_VIRTUAL_RANGE:
#endif
#if defined(INVOKE_VIRTUAL_RANGE) || defined(INVOKE_SUPER_RANGE)
            {
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
#endif
#ifdef INVOKE_STATIC_RANGE
                case INVOKE_STATIC_RANGE:
#endif
#ifdef INVOKE_DIRECT_RANGE
                case INVOKE_DIRECT_RANGE:
#endif
#if defined(INVOKE_STATIC_RANGE) || defined(INVOKE_DIRECT_RANGE)
            {
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
#endif

////            case INVOKE_INTERFACE_RANGE:
////
////                break;

#ifdef NOT_INT
                case NOT_INT:
#endif
#ifdef NEG_INT
                case NEG_INT:
#endif
#if defined(NOT_INT) || defined(NEG_INT)
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits ^ x32BITMASK1s;
#ifdef NEG_INT
                if(opcode==NEG_INT)
                    registers[addr.To].bits++;
#endif
                break;
            }
#endif

#ifdef NOT_LONG
                case NOT_LONG:
#endif
#ifdef NEG_LONG
                case NEG_LONG:
#endif
#if defined(NOT_LONG) || defined(NEG_LONG)
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits ^ x32BITMASK1s;
                registers[addr.To + 1].bits = registers[addr.From + 1].bits ^ x32BITMASK1s;
                switch(opcode){
#ifdef NEG_LONG
                    case NEG_LONG:{
                        if (registers[addr.To].bits == x32BITMASK1s) //if overflow in 32 LSB, carry to 32 MSB
                            registers[addr.To + 1].bits++;
                        else
                            registers[addr.To].bits++; //An assumption is made that bytecode generated by compiler will not attempt to negate a 0.
                        break;
                    }
#endif
#ifdef NOT_LONG
                    case NOT_LONG:
                        break;
#endif
                   }
            }
#endif
#ifdef NEG_FLOAT
            case NEG_FLOAT: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float tmp = ieee754Float(registers[addr.From].bits);
                tmp *= -1;
                registers[addr.To].bits = floatieee754(tmp);
                break;
            }
#endif
#ifdef NEG_DOUBLE
            case NEG_DOUBLE: {
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
#endif
#ifdef INT_TO_LONG
            case INT_TO_LONG: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                registers[addr.To].bits = registers[addr.From].bits;
                registers[addr.To + 1].bits = 0;

                break;
            }
#endif
#ifdef INT_TO_FLOAT
            case INT_TO_FLOAT: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float x = ((float) registers[addr.From].bits);
                registers[addr.To].bits = floatieee754(x);
                break;
            }
#endif
#ifdef INT_TO_DOUBLE
            case INT_TO_DOUBLE: //ok
            {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                double tmp = (double) registers[addr.From].bits;
                u8 ieee754bits = doubleieee754(tmp);
                registers[addr.To].bits = ieee754bits;
                registers[addr.To + 1].bits = (ieee754bits >> 32);
                break;
            }
#endif

#ifdef LONG_TO_INT
            case LONG_TO_INT: //TRUNCATION //ok
            {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                registers[addr.To].bits = registers[addr.From].bits; //copies 32 LSB to new location
                break;
            }
#endif

#ifdef LONG_TO_FLOAT
            case LONG_TO_FLOAT: {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.From = insns[i];
                u8 x = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                float y = (float) ieee754Double(x);
                registers[addr.To].bits = floatieee754(y);
                break;
            }
#endif
#ifdef LONG_TO_DOUBLE
            case LONG_TO_DOUBLE: {
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
#endif

#ifdef FLOAT_TO_INT
            case FLOAT_TO_INT: {
                NibbleAddress addr;
                addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                float x = ieee754Float(registers[addr.From].bits);
                s4 y = (s4) x;
                registers[addr.To].bits = y;
                break;
            }
#endif

#ifdef FLOAT_TO_LONG
            case FLOAT_TO_LONG: {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                float x = ieee754Float(registers[addr.From].bits);
                u8 y = (u8) x;
                registers[addr.To].bits = y;
                registers[addr.To + 1].bits = (y >> 32); //copies 32 LSB to new location
                break;
            }
#endif
#ifdef FLOAT_TO_DOUBLE
            case FLOAT_TO_DOUBLE: {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                addr.From = insns[i];

                double x = (double) ieee754Float(registers[addr.From].bits); //copies 32 LSB to new location
                u8 y = doubleieee754(x);
                registers[addr.To].bits = y; //copies 32 LSB to new location
                registers[addr.To + 1].bits = (y >> 32); //copies 32 LSB to new location
                break;
            }
#endif
#ifdef DOUBLE_TO_INT
            case DOUBLE_TO_INT: //ok
            {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                u8 tmp = recons64(registers[addr.From + 1].bits,
                                  registers[addr.From].bits); //copies 32 LSB to new location
                u4 tmp1 = (u4) ieee754Double(tmp);

                registers[addr.To].bits = tmp1;
                break;
            }
#endif
#ifdef DOUBLE_TO_LONG
            case DOUBLE_TO_LONG: //ok
            {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];

                u8 tmp = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                u8 tmp1 = (u8) ieee754Double(tmp);

                registers[addr.To].bits = tmp1; //copies 32 LSB to new location
                registers[addr.To + 1].bits = (tmp1 >> 32); //copies 32 LSB to new location
                break;

            }
#endif
#ifdef DOUBLE_TO_FLOAT
            case DOUBLE_TO_FLOAT://ok
            {
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
                u8 x = recons64(registers[addr.From + 1].bits, registers[addr.From].bits);
                float y = (float) ieee754Double(x);
                registers[addr.To].bits = floatieee754(y);
                break;
            }
#endif
//
#ifdef INT_TO_SHORT
                case INT_TO_SHORT:
#endif
#ifdef INT_TO_CHAR
                case INT_TO_CHAR:
#endif
#ifdef INT_TO_BYTE
                case INT_TO_BYTE:
#endif

#if defined(INT_TO_SHORT) || defined(INT_TO_BYTE) || defined(INT_TO_CHAR)
            {
                u1 opcode = insns[i];
                NibbleAddress addr;
                addr.To = insns[++i] >> 4; //extracts 4 MSB, "To" register number

                addr.From = insns[i];
//                u1 tmp = (u1)registers[addr.From].bits;
                switch(opcode){
#ifdef INT_TO_BYTE
                    case INT_TO_BYTE:
                        registers[addr.To].bits = ((s1) registers[addr.From].bits);
                        break;
#endif
#ifdef INT_TO_CHAR
                    case INT_TO_CHAR:
                        registers[addr.To].bits = ((u2) registers[addr.From].bits);
                        break;
#endif
#ifdef INT_TO_SHORT
                    case INT_TO_SHORT:
                        registers[addr.To].bits = ((s2) registers[addr.From].bits);
                        break;
#endif
                    }
                break;
            }
#endif

//                //BINARY OPERATIONS //OK
#ifdef USHR_INT
                case USHR_INT:
#endif
#ifdef SHR_INT
                case SHR_INT:
#endif
#ifdef SHL_INT
                case SHL_INT:
#endif
#ifdef XOR_INT
                case XOR_INT:
#endif
#ifdef OR_INT
                case OR_INT:
#endif
#ifdef AND_INT
                case AND_INT:
#endif
#ifdef REM_INT
                case REM_INT:
#endif
#ifdef DIV_INT
                case DIV_INT:
#endif
#ifdef MUL_INT
                case MUL_INT:
#endif
#ifdef SUB_INT
            case SUB_INT:
#endif
#ifdef ADD_INT
                case ADD_INT:
#endif
#if defined(ADD_INT) || defined(SUB_INT) || defined(MUL_INT) || defined(DIV_INT) || defined(REM_INT) || defined(AND_INT) || defined(OR_INT) || defined(XOR_INT) || defined(SHL_INT) || defined(SHR_INT) || defined(INT_TO_SHORT)
            {
                u1 opcode = insns[i];
                BinOP binop;
                binop.Dest = insns[++i];
                binop.Op1Addr = insns[++i];
                binop.Op2Addr = insns[++i];

                s4 op1 = (s4) registers[binop.Op1Addr].bits;
                s4 op2 = (s4) registers[binop.Op2Addr].bits;
                s4 result = 0;

                switch (opcode) {
#ifdef ADD_INT
                    case ADD_INT:
                        result = op1 + op2;
#endif
#ifdef SUB_INT
                    case SUB_INT:
                        result = op1 - op2;
                        break;
#endif
#ifdef MUL_INT
                    case MUL_INT:
                        result = op1 * op2;
                        break;
#endif
#ifdef DIV_INT
                    case DIV_INT:
                        result = op1 / op2;
                        break;
#endif
#ifdef REM_INT
                    case REM_INT:
                        result = op1 - (op1 / op2) * op2;
                        break;
#endif
#ifdef AND_INT
                    case AND_INT:
                        result = op1 & op2;
                        break;
#endif
#ifdef OR_INT
                    case OR_INT:
                        result = op1 | op2;
                        break;
#endif
#ifdef XOR_INT
                    case XOR_INT:
                        result = op1 ^ op2;
                        break;
#endif
#ifdef SHL_INT
                    case SHL_INT:
                        result = op1 << (op2 & 0x1f);
                        break;
#endif
#ifdef SHR_INT
                    case SHR_INT: {
                        s4 tmp = (s4) registers[binop.Op1Addr].bits;
                        tmp = tmp >> (registers[binop.Op2Addr].bits & 0x1f);
                        result = tmp;
                        break;
                    }
#endif
#ifdef USHR_INT
                    case USHR_INT:
                        result = op1 >> (op2 & 0x1f);
                        break;

#endif
                }
                        registers[binop.Dest].bits = result;
                        break;
                }
#endif
#ifdef XOR_LONG
                case XOR_LONG:
#endif
#ifdef OR_LONG
                case OR_LONG:
#endif
#ifdef AND_LONG
                case AND_LONG:
#endif
#ifdef REM_LONG
                case REM_LONG:
#endif
#ifdef DIV_LONG
                case DIV_LONG:
#endif
#ifdef MUL_LONG
                case MUL_LONG:
#endif
#ifdef SUB_LONG
                case SUB_LONG:
#endif
#ifdef ADD_LONG
                case ADD_LONG:
#endif
#if defined(ADD_LONG) || defined(SUB_LONG) || defined(MUL_LONG) || defined(DIV_LONG) || defined(REM_LONG) || defined(AND_LONG) || defined(OR_LONG) || defined(XOR_LONG)
                {
                   u1 opcode = insns[i];
                   BinOP binop;
                   binop.Dest = insns[++i];
                   binop.Op1Addr = insns[++i];
                   binop.Op2Addr = insns[++i];

                   s8 op1 = (s8) recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits);
                   s8 op2 = (s8) recons64(registers[binop.Op2Addr + 1].bits, registers[binop.Op2Addr].bits);

                   switch (opcode) {
#ifdef ADD_LONG
                       case ADD_LONG:
                           op1 += op2;
                           break;
#endif
#ifdef SUB_LONG
                       case SUB_LONG:
                           op1 -= op2;
                           break;
#endif
#ifdef MUL_LONG
                       case MUL_LONG:
                           op1 *= op2;
                           break;
#endif
#ifdef DIV_LONG
                       case DIV_LONG:
                           op1 /= op2;
                           break;
#endif
#ifdef REM_LONG
                       case REM_LONG:
                           op1 = op1 - (op1 / op2) * op2;
                           break;
#endif
#ifdef AND_LONG
                       case AND_LONG:
                           op1 &= op2;
                           break;
#endif
#ifdef OR_LONG
                       case OR_LONG:
                           op1 |= op2;
                           break;
#endif
#ifdef XOR_LONG
                       case XOR_LONG:
                           op1 ^= op2;
                           break;
#endif
                   }
                   //result

                   registers[binop.Dest].bits = op1;
                   registers[binop.Dest + 1].bits = (op1 >> 32);

                   break;
               }
#endif
#ifdef USHR_LONG
                case USHR_LONG:
#endif
#ifdef SHL_LONG
                case SHL_LONG:
#endif
#if defined(SHL_LONG) || defined(USHR_LONG)
                {
                    u1 opcode = insns[i];
                    BinOP binop;
                    binop.Dest = insns[++i];
                    binop.Op1Addr = insns[++i];
                    binop.Op2Addr = insns[++i];

                    u8 op1 = recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits);
                    u4 op2 = registers[binop.Op2Addr].bits;
                    u8 result = 0;

                    switch (opcode) {
#ifdef SHL_LONG
                        case SHL_LONG:
                            result = op1 << (op2 & 0x3f);
                            break;
#endif
#ifdef USHR_LONG
                        case USHR_LONG:
                            result = op1 >> (op2 & 0x3f);
                            break;
#endif
                    }


                    registers[binop.Dest].bits = result;
                    registers[binop.Dest + 1].bits = (result >> 32);

                    break;
                }
#endif

#ifdef SHR_LONG
                case SHR_LONG: {
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
#endif
#ifdef REM_FLOAT
                case REM_FLOAT:
#endif
#ifdef DIV_FLOAT
                case DIV_FLOAT:
#endif
#ifdef MUL_FLOAT
                case MUL_FLOAT:
#endif
#ifdef SUB_FLOAT
                case SUB_FLOAT:
#endif
#ifdef ADD_FLOAT
                case ADD_FLOAT:
#endif
#if defined(ADD_FLOAT) || defined(SUB_FLOAT) || defined(MUL_FLOAT) || defined(REM_FLOAT) || defined(DIV_FLOAT)
                {
                    u1 opcode = insns[i];
                    BinOP binop;
                    binop.Dest = insns[++i];
                    binop.Op1Addr = insns[++i];
                    binop.Op2Addr = insns[++i];

                    float op1 = ieee754Float(registers[binop.Op1Addr].bits);
                    float op2 = ieee754Float(registers[binop.Op2Addr].bits);

                    switch (opcode) {
#ifdef ADD_FLOAT
                        case ADD_FLOAT:
                            registers[binop.Dest].bits = floatieee754(op1 + op2);
                            break;
#endif
#ifdef SUB_FLOAT
                        case SUB_FLOAT:
                            registers[binop.Dest].bits = floatieee754(op1 - op2);
                            break;
#endif
#ifdef MUL_FLOAT
                        case MUL_FLOAT:
                            registers[binop.Dest].bits = floatieee754(op1 * op2);
                            break;
#endif
#ifdef DIV_FLOAT
                        case DIV_FLOAT:
                            registers[binop.Dest].bits = floatieee754(op1 / op2);
                            break;
#endif
#ifdef REM_FLOAT
                        case REM_FLOAT: {
                            s4 roundedQuotient = (s4) (op1 / op2); //rounded towards zero
                            float rounded = (float) roundedQuotient;
                            registers[binop.Dest].bits = floatieee754(op1 - rounded * op2);
                            break;
                        }
#endif
                    }
                    break;
                }
#endif
#ifdef REM_DOUBLE
                case REM_DOUBLE:
#endif
#ifdef DIV_DOUBLE
                case DIV_DOUBLE:
#endif
#ifdef MUL_DOUBLE
                case MUL_DOUBLE:
#endif
#ifdef SUB_DOUBLE
                case SUB_DOUBLE:
#endif
#ifdef ADD_DOUBLE
                case ADD_DOUBLE:
#endif
#if defined(ADD_DOUBLE) || defined(SUB_DOUBLE) || defined(REM_DOUBLE) || defined(DIV_DOUBLE) || defined(MUL_DOUBLE)
                {
                    u1 opcode = insns[i];
                    BinOP binop;
                    binop.Dest = insns[++i];
                    binop.Op1Addr = insns[++i];
                    binop.Op2Addr = insns[++i];

                    double op1 = ieee754Double(recons64(registers[binop.Op1Addr + 1].bits, registers[binop.Op1Addr].bits));
                    double op2 = ieee754Double(recons64(registers[binop.Op2Addr + 1].bits, registers[binop.Op2Addr].bits));

                    u8 tmp = 0; //result

                    switch (opcode) {
#ifdef ADD_DOUBLE
                        case ADD_DOUBLE:
                            tmp = doubleieee754(op1 + op2);
                            break;
#endif
#ifdef SUB_DOUBLE
                        case SUB_DOUBLE:
                            tmp = doubleieee754(op1 - op2);
                            break;
#endif
#ifdef MUL_DOUBLE
                        case MUL_DOUBLE:
                            tmp = doubleieee754(op1 * op2);
                            break;
#endif
#ifdef DIV_DOUBLE
                        case DIV_DOUBLE:
                            tmp = doubleieee754(op1 / op2);
                            break;
#endif
#ifdef REM_DOUBLE
                        case REM_DOUBLE: {
                            s8 roundedQuotient = (s8) (op1 / op2); //rounded towards zero
                            double rounded = (double) roundedQuotient;
                            u8 tmp = doubleieee754(op1 * op2);
                            break;
                        }
#endif
                    }

                    registers[binop.Dest].bits = tmp;
                    registers[binop.Dest + 1].bits = (tmp >> 32);
                    break;
                }
#endif

                /*
                 * Identical to previous operations except only two address operands are given,
                 * result of calculation is stored in the first operand register rather than a 3rd destination register
                 * Also operands are given as 4 bits.*/
#ifdef SHR_INT2
                case SHR_INT2:
#endif
#ifdef SHL_INT2
                case SHL_INT2:
#endif
#ifdef XOR_INT2
                case XOR_INT2:
#endif
#ifdef OR_INT2
                case OR_INT2:
#endif
#ifdef AND_INT2
                case AND_INT2:
#endif
#ifdef REM_INT2
                case REM_INT2:
#endif
#ifdef DIV_INT2
                case DIV_INT2:
#endif
#ifdef MUL_INT2
                case MUL_INT2:
#endif
#ifdef SUB_INT2
                case SUB_INT2:
#endif
#ifdef ADD_INT2
                case ADD_INT2://OK
#endif
#if defined(ADD_INT2) || defined(SUB_INT2) || defined(MUL_INT2) || defined(DIV_INT2) || defined(REM_INT2) || defined(AND_INT2) || defined(OR_INT2) || defined(XOR_INT2) || defined(SHL_INT2) || defined(SHR_INT2)
                {
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                    s4 op1 = registers[addr.To].bits;
                    s4 op2 = registers[addr.From].bits;

                    switch (opcode) {
#ifdef ADD_INT2
                        case ADD_INT2:
                            op1 += op2;
                            break;
#endif
#ifdef SUB_INT2
                        case SUB_INT2:
                            op1 -= op2;
                            break;
#endif
#ifdef MUL_INT2
                        case MUL_INT2:
                            op1 *= op2;
                            break;
#endif
#ifdef DIV_INT2
                        case DIV_INT2:
                            op1 /= op2;
                            break;
#endif
#ifdef REM_INT2
                        case REM_INT2:
                            op1 = op1 - (op1 / op2) * op2;
                            break;
#endif
#ifdef AND_INT2
                        case AND_INT2:
                            op1 &= op2;
                            break;
#endif
#ifdef OR_INT2
                        case OR_INT2:
                            op1 |= op2;
                            break;
#endif
#ifdef XOR_INT2
                        case XOR_INT2:
                            op1 ^= op2;
                            break;
#endif
#ifdef SHL_INT2
                        case SHL_INT2:
                            op1 = op1 << op2;
                            break;
#endif
#ifdef SHR_INT2
                        case SHR_INT2:
                            op1 = op1 >> op2;
                            break;
#endif
                    }


                    registers[addr.To].bits = op1;
                    break;
                }
#endif
#ifdef USHR_INT2
                case USHR_INT2://ok
                {
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number
                    s4 op2 = registers[addr.From].bits;
                    registers[addr.To].bits >>= op2;
                    break;
                }
#endif
////////////////
#ifdef SHR_LONG2
                case SHR_LONG2:
#endif
#ifdef SHL_LONG2
                case SHL_LONG2:
#endif
#ifdef XOR_LONG2
                case XOR_LONG2:
#endif
#ifdef OR_LONG2
                case OR_LONG2:
#endif
#ifdef AND_LONG2
                case AND_LONG2:
#endif
#ifdef REM_LONG2
                case REM_LONG2:
#endif
#ifdef DIV_LONG2
                case DIV_LONG2:
#endif
#ifdef MUL_LONG2
                case MUL_LONG2:
#endif
#ifdef SUB_LONG2
                case SUB_LONG2:
#endif
#ifdef ADD_LONG2
                case ADD_LONG2://ok
#endif
#if defined(ADD_LONG2) || defined(SUB_LONG2) || defined(MUL_LONG2) || defined(DIV_LONG2) || defined(REM_LONG2) || defined(AND_LONG2) || defined(XOR_LONG2) || defined(SHL_LONG2) || defined(SHR_LONG2) || defined(OR_LONG2)
                {
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                    s8 op1 = (s8) recons64(registers[addr.To + 1].bits, registers[addr.To].bits);
                    s8 op2 = (s8) recons64(registers[addr.From + 1].bits, registers[addr.From].bits);

                    switch (opcode) {
#ifdef ADD_LONG2
                        case ADD_LONG2:
                            op1 += op2;
                            break;
#endif
#ifdef SUB_LONG2
                        case SUB_LONG2:
                            op1 -= op2;
                            break;
#endif
#ifdef MUL_LONG2
                        case MUL_LONG2:
                            op1 *= op2;
                            break;
#endif
#ifdef DIV_LONG2
                        case DIV_LONG2:
                            op1 /= op2;
                            break;
#endif
#ifdef REM_LONG2
                        case REM_LONG2:
                            op1 = op1 - (op1 / op2) * op2;
                            break;
#endif
#ifdef AND_LONG2
                        case AND_LONG2:
                            op1 &= op2;
                            break;
#endif
#ifdef OR_LONG2
                        case OR_LONG2:
                            op1 |= op2;
                            break;
#endif
#ifdef XOR_LONG2
                        case XOR_LONG2:
                            op1 ^= op2;
                            break;
#endif
#ifdef SHL_LONG2
                        case SHL_LONG2:
                            op1 <<= op2;
                            break;
#endif
#ifdef SHR_LONG2
                        case SHR_LONG2:
                            op1 >>= op2; //result
                            break;
#endif
                    }
                            registers[addr.To].bits = op1;
                            registers[addr.To + 1].bits = (op1 >> 32);
                            break;
                    }

#endif
#ifdef USHR_LONG2
                case USHR_LONG2: {
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
#endif
#ifdef REM_FLOAT2
                case REM_FLOAT2:
#endif
#ifdef DIV_FLOAT2
                case DIV_FLOAT2:
#endif
#ifdef MUL_FLOAT2
                case MUL_FLOAT2:
#endif
#ifdef SUB_FLOAT2
                case SUB_FLOAT2:
#endif
#ifdef ADD_FLOAT2
                case ADD_FLOAT2:
#endif
#if defined(ADD_FLOAT2) || defined(SUB_FLOAT2) || defined(MUL_FLOAT2) || defined(REM_FLOAT2) || defined(DIV_FLOAT2)
                {
                        u1 opcode = insns[i];
                        NibbleAddress addr;
                        addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                        addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                        float op1;
                        float op2;

                        op1 = ieee754Float(registers[addr.To].bits);
                        op2 = ieee754Float(registers[addr.From].bits);


                        switch (opcode) {
#ifdef ADD_FLOAT2
                            case ADD_FLOAT2:
                                registers[addr.To].bits = floatieee754(op1 + op2);
                                break;
#endif
#ifdef SUB_FLOAT2
                            case SUB_FLOAT2:
                                registers[addr.To].bits = floatieee754(op1 - op2);
                                break;
#endif
#ifdef MUL_FLOAT2
                            case MUL_FLOAT2:
                                registers[addr.To].bits = floatieee754(op1 * op2);
                                break;
#endif
#ifdef DIV_FLOAT2
                            case DIV_FLOAT2:
                                registers[addr.To].bits = floatieee754(op1 / op2);
                                break;
#endif
#ifdef REM_FLOAT2
                            case REM_FLOAT2: {
                                s4 roundedQuotient = (s4) (op1 / op2); //rounded towards zero
                                float rounded = (float) roundedQuotient;
                                registers[addr.To].bits = floatieee754(op1 - rounded * op2);
                                break;
                            }
#endif
                        }

                        registers[addr.To].bits = floatieee754(op1);

                        break;
                    }
#endif
#ifdef REM_DOUBLE2
                case REM_DOUBLE2:
#endif
#ifdef DIV_DOUBLE2
                case DIV_DOUBLE2:
#endif
#ifdef MUL_DOUBLE2
                case MUL_DOUBLE2:
#endif
#ifdef SUB_DOUBLE2
                case SUB_DOUBLE2:
#endif
#ifdef ADD_DOUBLE2
                case ADD_DOUBLE2:
#endif
#if defined(ADD_DOUBLE2) || defined(SUB_DOUBLE2) || defined(MUL_DOUBLE2) || defined(REM_DOUBLE2) || defined(DIV_DOUBLE2)
                {
                    u1 opcode = insns[i];
                    NibbleAddress addr;
                    addr.From = insns[++i] >> 4; //extracts 4 MSB, "To" register number
                    addr.To = insns[i]; //downcast from 8 to 4 bit word extracts LSB , "From" register number

                    double op1 = ieee754Double(recons64(registers[addr.To + 1].bits, registers[addr.To].bits));
                    double op2 = ieee754Double(recons64(registers[addr.From + 1].bits, registers[addr.From].bits));

                    switch(opcode){
#ifdef ADD_DOUBLE2
                        case ADD_DOUBLE2:
                            op1 += op2;
                             break;
#endif
#ifdef SUB_DOUBLE2
                        case SUB_DOUBLE2:
                            op1 -= op2;
                            break;
#endif
#ifdef MUL_DOUBLE2
                        case MUL_DOUBLE2:
                            op1 *= op2;
                            break;
#endif
#ifdef DIV_DOUBLE2
                        case DIV_DOUBLE2:
                            op1 /= op2;
                            break;
#endif
#ifdef REM_DOUBLE2
                        case REM_DOUBLE2:{
                            s8 roundedQuotient = (s8) (op1 / op2); //rounded towards zero
                            double rounded = (double) roundedQuotient;
                            op1 = op1 - (rounded * op2);
                            break;
                        }
#endif
                    }


                    u8 tmp = doubleieee754(op1); //result

                    registers[addr.To].bits = tmp;
                    registers[addr.To + 1].bits = (tmp >> 32);
                    break;
                }
#endif
#ifdef XOR_INT_LIT16
                case XOR_INT_LIT16:
#endif
#ifdef OR_INT_LIT16
                case OR_INT_LIT16:
#endif
#ifdef AND_INT_LIT16
                case AND_INT_LIT16:
#endif
#ifdef REM_INT_LIT16
                case REM_INT_LIT16:
#endif
#ifdef DIV_INT_LIT16
                case DIV_INT_LIT16:
#endif
#ifdef MUL_INT_LIT16
                case MUL_INT_LIT16:
#endif
#ifdef RSUB_INT
                case RSUB_INT:
#endif
#ifdef ADD_INT_LIT16
                case ADD_INT_LIT16:
#endif
#if defined(ADD_INT_LIT16) || defined(RSUB_INT) || defined(MUL_INT_LIT16) || defined(DIV_INT_LIT16) || defined(REM_INT_LIT16) || defined(AND_INT_LIT16) || defined(XOR_INT_LIT16) || defined(OR_INT_LIT16)
                {//ok
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
#ifdef ADD_INT_LIT16
                        case ADD_INT_LIT16:
                            result = op1 + literal;
                            break;
#endif
#ifdef RSUB_INT
                        case RSUB_INT:
                            result = op1 - literal;
                            break;
#endif
#ifdef MUL_INT_LIT16
                        case MUL_INT_LIT16:
                            result = op1 * literal;
                            break;
#endif
#ifdef DIV_INT_LIT16
                        case DIV_INT_LIT16:
                            result = op1 / literal;
                            break;
#endif
#ifdef REM_INT_LIT16
                        case REM_INT_LIT16:
                            result = op1 - (op1 / literal) * literal;
                            break;
#endif
#ifdef AND_INT_LIT16
                        case AND_INT_LIT16:
                            result = op1 & literal;
                            break;
#endif
#ifdef OR_INT_LIT16
                        case OR_INT_LIT16:
                            result = op1 | literal;
                            break;
#endif
#ifdef XOR_INT_LIT16
                        case XOR_INT_LIT16:
                            result = op1 ^ literal;
                            break;
#endif
                    }

                    registers[addr.To].bits = result;
                    break;
                }
#endif

#ifdef RSUB_INT_LIT8
                case RSUB_INT_LIT8:
#endif
#ifdef ADD_INT_LIT8
                case ADD_INT_LIT8:
#endif
#ifdef MUL_INT_LIT8
                    case MUL_INT_LIT8:
#endif
#ifdef DIV_INT_LIT8
                    case DIV_INT_LIT8:
#endif
#ifdef REM_INT_LIT8
                    case REM_INT_LIT8:
#endif
#ifdef AND_INT_LIT8
                    case AND_INT_LIT8:
#endif
#ifdef OR_INT_LIT8
                    case OR_INT_LIT8:
#endif
#ifdef XOR_INT_LIT8
                    case XOR_INT_LIT8:
#endif
#ifdef SHR_INT_LIT8
                    case SHR_INT_LIT8:
#endif
#ifdef SHL_INT_LIT8
                    case SHL_INT_LIT8:
#endif
#if defined(SHL_INT_LIT8) || defined(SHR_INT_LIT8) || defined(XOR_INT_LIT8) || defined(OR_INT_LIT8) || defined(AND_INT_LIT8) || defined(REM_INT_LIT8) || defined(DIV_INT_LIT8) || defined(MUL_INT_LIT8) || defined(ADD_INT_LIT8) || defined(RSUB_INT_LIT8)
                {
                    u1 opcode = insns[i];
                    Address8to8 addr;
                    addr.To = insns[++i];
                    addr.From = insns[++i];
                    s1 literal = insns[++i];

                    s4 op1 = (s4) registers[addr.From].bits;
                    s4 result = 0;

                    switch (opcode) {
#ifdef RSUB_INT_LIT8
                        case RSUB_INT_LIT8:
                            result = op1 - literal;
                            break;
#endif
#ifdef ADD_INT_LIT8
                        case ADD_INT_LIT8:
                            result = op1 + literal;
                            break;
#endif
#ifdef SHR_INT_LIT8
                        case SHR_INT_LIT8:
                            result = op1 >> literal;
                            break;
#endif
#ifdef SHL_INT_LIT8
                        case SHL_INT_LIT8:
                            result = op1 << literal;
                            break;
#endif
#ifdef XOR_INT_LIT8
                        case XOR_INT_LIT8:
                            result = op1 ^ literal;
                            break;
#endif
#ifdef OR_INT_LIT8
                        case OR_INT_LIT8:
                            result = op1 | literal;
                            break;
#endif
#ifdef AND_INT_LIT8
                        case AND_INT_LIT8:
                            result = op1 & literal;
                            break;
#endif
#ifdef REM_INT_LIT8
                        case REM_INT_LIT8:
                            result = op1 - (op1 / literal) * literal;
                            break;
#endif
#ifdef DIV_INT_LIT8
                        case DIV_INT_LIT8:
                            result = op1 / literal;
                            break;
#endif
#ifdef MUL_INT_LIT8
                        case MUL_INT_LIT8:
                            result = op1 * literal;
                            break;
#endif

                    }

                    registers[addr.To].bits = result;

                    break;
                }
#endif
#ifdef USHR_INT_LIT8
                case USHR_INT_LIT8: {
                    Address8to8 addr;
                    addr.To = insns[++i];
                    addr.From = insns[++i];
                    s1 literal = insns[++i];

                    u4 op1 = registers[addr.From].bits;
                    u4 result = op1 >> literal;
                    registers[addr.To].bits = result;

                    break;
                }
#endif

                default:
                    continue;

            }
        }

        RETURN:
        garbageCollector(instances, insts_count);
        free(registers);

    }







