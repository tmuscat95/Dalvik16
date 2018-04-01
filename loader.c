
#include "loader.h"
#include "definitions.h"


void diagnostic(int err){

    switch(err){
        case -1:
            puts("Not a valid dex file, exiting \n");
            break;

        case -2:
            puts("File corrupted,exiting \n");
            break;

        case 0:
            puts("Magic Byte check passed \n");
            break;
    }

}

int validDex(const u1 * magic){
    if(magic[0]!=0x64 || magic[1]!=0x65 || magic[2]!=0x78)
        return NOT_DEX;
    else if(magic[3]!=0x0a || magic[7]!=0x00)
        return CORRUPT;
    else return VALID;

}

long readLeb128(FILE *dexfile) {
    int bitpos = 0;
    long vln = 0L;
    do {
        int inp[1];
        fread(inp,1,1,dexfile);
        vln |= ((long)( inp[0] & 0x7F )) << bitpos;
        if( ( inp[0] & 0x80 ) == 0 )
            break;
        bitpos += 7;
    } while(1);
    return vln;
}

const char * readString(FILE * dexfile,u4 offset){
    fseek(dexfile,offset,SEEK_SET);
    long size = readLeb128(dexfile);
    long fpos = ftell(dexfile);
// size is the count of UTF-8 characters in the string while we need the number of bytes.
// We need to find the terminating zero character for that
    int stringByteCount = 0;
    do {
        u1 c[1];
        fread(c,1,1,dexfile);
        if( c[0] == 0 )
            break;
        ++stringByteCount;
    } while(1);

    u1 * stringBytes;
    if(stringByteCount>0)
        stringBytes = (u1*) malloc(stringByteCount*U1);

    fseek(dexfile,fpos,SEEK_SET);	// back to the beginning of the string's bytes
    for( int i = 0 ; i < stringByteCount ; ++i )
        fread((stringBytes+i),1,1,dexfile);
    fseek(dexfile,1,SEEK_CUR);		// Discard the terminating 0
    return (char*) stringBytes;
}

header * readHeader(FILE * dexfile){
    header * h = (header*) malloc(sizeof(header));

    u1 * magic = (u1*) malloc(U1*8);
    fread(magic,U1,8,dexfile);

    int validity = validDex(magic);

    diagnostic(validity);

    if(validity==NOT_DEX || validity==CORRUPT)
        return NULL; //Tells main that given file is not a valid dex file or is corrupt

    //memcpy(h->magic.dex,magic,3);

    free(magic);

    fseek(dexfile,sizeof(unsigned char)*20 + U4,SEEK_CUR); //skips checksum and signature bytes

    fread(h->file_size,U4,1,dexfile);
    // printf("%ud",h->file_size[0]);
    fseek(dexfile,U4,SEEK_CUR); //skips header size bytes, header size assumed to be 0x70 and hard coded
    ///
    u4 * endianbytes = (u4*) malloc(U4);
    fread(endianbytes,U4,1,dexfile);

    if(*endianbytes != LITTLE)
        endianTag = 1;
    else
        endianTag = 0;

    free(endianbytes);

    ///
    fread(h->link_size,U4,1,dexfile); //reads size of link list from header (may be 0)
    fread(h->link_off,U4,1,dexfile); //reads offset to link list in data section(0 if link_size is 0)
    fread(h->map_off,U4,1,dexfile); //reads offset to map list in data section
    fread(h->string_ids_size,U4,1,dexfile); //reads size of string list in data section
    fread(h->string_ids_off,U4,1,dexfile); //reads ffset to string ids list in data section
    fread(h->type_ids_size,U4,1,dexfile); //reads size of types list in data section
    fread(h->type_ids_off,U4,1,dexfile); //reads offset to type ids in data section
    fread(h->proto_ids_size,U4,1,dexfile);
    fread(h->proto_ids_off,U4,1,dexfile);
    fread(h->field_ids_size,U4,1,dexfile);
    fread(h->field_ids_off,U4,1,dexfile);
    fread(h->method_ids_size,U4,1,dexfile);
    fread(h->method_ids_off,U4,1,dexfile);
    fread(h->class_defs_size,U4,1,dexfile);
    fread(h->class_defs_off,U4,1,dexfile);
    fread(h->data_size,U4,1,dexfile);
    fread(h->data_off,U4,1,dexfile);




    return h;

}

class_def_item * readClassDefs(FILE * dexfile,map_list * mapList,DexStruct * d){
    int i;
    for(i=0;i<mapList->size[0];i++){
        if(mapList->map_items[i].type[0]==TYPE_CLASS_DEF_ITEM)
            break;
    }

    fseek(dexfile,mapList->map_items[i].offset[0],SEEK_SET);

    class_def_item * class_def_items = (class_def_item*) malloc(sizeof(class_def_item)*mapList->map_items[i].size[0]);

    for(int j=0;j<d->head->class_defs_size[0];j++){

        fread((class_def_items+j)->class_idx,U4,1,dexfile);
        fread((class_def_items+j)->access_flags,U4,1,dexfile);
        fread((class_def_items+j)->superclass_idx,U4,1,dexfile);
        fread((class_def_items+j)->interfaces_off,U4,1,dexfile);
        fread((class_def_items+j)->source_file_idx,U4,1,dexfile);
        fread((class_def_items+j)->annotations_off,U4,1,dexfile);
        fread((class_def_items+j)->class_data_off,U4,1,dexfile);
        fread((class_def_items+j)->static_values_off,U4,1,dexfile);
        if(class_def_items[j].class_data_off[0]==0)
            class_def_items[j].classData = NULL;

    }

    for(int j=0;j<d->head->class_defs_size[0];j++){
        for(u2 k=0;k<d->head->class_defs_size[0];k++){
            if(class_def_items[j].superclass_idx[0]!=NO_INDEX)
                if(d->type_ids[class_def_items[j].superclass_idx[0]]==d->type_ids[class_def_items[k].class_idx[0]]){
                        class_def_items[j].superclass_index = k;
                }
        }
    }

    return class_def_items;



}

class_data_item * readClassData(FILE * dexfile,class_def_item * classDefs,header * h){
    int classCount = h->class_defs_size[0];// not correct, some classes do not have data ex. interfaces
    h->class_data_count = 0;
    class_data_item * classDataItems = (class_data_item*) malloc(sizeof(class_data_item)*classCount);

    for(int i=0;i<classCount;i++){
        if((classDefs+i)->class_data_off[0]==0)
            continue;

        h->class_data_count++;
        rewind(dexfile);
        fseek(dexfile,classDefs[i].class_data_off[0],SEEK_SET);//offset appears to place file pointer too far, causing uleb128 values to be read incorrectly, decrementing the offset by 1 byte seems to fix this, not entirely sure why.

        (classDataItems+i)->static_fields_size = (u4) readLeb128(dexfile);
        (classDataItems+i)->instance_fields_size = readLeb128(dexfile);
        (classDataItems+i)->direct_methods_size = readLeb128(dexfile);
        (classDataItems+i)->virtual_methods_size = readLeb128(dexfile);

        (classDataItems+i)->static_fields = (encoded_field*) malloc(sizeof(encoded_field)*(classDataItems+i)->static_fields_size);

        for(int j=0;j<(classDataItems+i)->static_fields_size;j++){

            ((classDataItems+i)->static_fields+j)->field_idx_diff = readLeb128(dexfile);
            ((classDataItems+i)->static_fields+j)->access_flags = readLeb128(dexfile);

        }

        (classDataItems+i)->instance_fields = (encoded_field*) malloc(sizeof(encoded_field)*(classDataItems+i)->instance_fields_size);

        for(int j=0;j<(classDataItems+i)->instance_fields_size;j++){

            ((classDataItems+i)->instance_fields+j)->field_idx_diff = readLeb128(dexfile);
            ((classDataItems+i)->instance_fields+j)->access_flags = readLeb128(dexfile);

        }

        (classDataItems+i)->direct_methods = (encoded_method*) malloc(sizeof(encoded_method)*(classDataItems+i)->direct_methods_size);

        for(int j=0;j<(classDataItems+i)->direct_methods_size;j++){

            ((classDataItems+i)->direct_methods+j)->method_idx_diff = readLeb128(dexfile);
            ((classDataItems+i)->direct_methods+j)->access_flags = readLeb128(dexfile);
            ((classDataItems+i)->direct_methods+j)->code_off = readLeb128(dexfile);


        }

        (classDataItems+i)->virtual_methods = (encoded_method*) malloc(sizeof(encoded_method)*(classDataItems+i)->virtual_methods_size);

        for(int j=0;j<(classDataItems+i)->virtual_methods_size;j++){

            ((classDataItems+i)->virtual_methods+j)->method_idx_diff = readLeb128(dexfile);
            ((classDataItems+i)->virtual_methods+j)->access_flags = readLeb128(dexfile);
            ((classDataItems+i)->virtual_methods+j)->code_off = readLeb128(dexfile);


        }

        (classDefs+i)->classData = (classDataItems+i);
    }

    return classDataItems;
}

void readMethodCode(FILE * dexfile,class_data_item * classDataItem,header * h){

    for(int j=0;j<classDataItem->direct_methods_size;j++){
        classDataItem->direct_methods[j].methodCode = (code_item*) malloc(sizeof(code_item));
        code_item * codeItem = classDataItem->direct_methods[j].methodCode;

        fseek(dexfile,classDataItem->direct_methods[j].code_off,SEEK_SET);

        fread(codeItem->registers_size,U2,1,dexfile);
        fread(codeItem->ins_size,U2,1,dexfile);
        //insns size must be in single bytes for interpreter whereas Dalvik calculates size in 16-bit words
        fread(codeItem->outs_size,U2,1,dexfile);
        fread(codeItem->tries_size,U2,1,dexfile);
        fread(codeItem->debug_info_off,U4,1,dexfile);
        fread(codeItem->insns_size,U4,1,dexfile);
        codeItem->insns_size[0] *= 2; //insns size in 16-bit words, whereas interpreter works in 8-bit words
        u4 insns_size = codeItem->insns_size[0];
        u2 tries_size = codeItem->tries_size[0];

        // (classDataItem->directMethodData+j)->insns = (u2*) malloc(U2*insns_size);
        codeItem->insns = (u1*) malloc(U1*insns_size);
        //fread((classDataItem->directMethodData+j)->insns,U2,insns_size,dexfile);
        fread(codeItem->insns,U1,insns_size,dexfile);
        //loading actual method instructions

        if(tries_size > 0 && insns_size % 2 != 0)
            fseek(dexfile,U2,SEEK_CUR);

        if(tries_size>0){
            codeItem->tries = (tryItem*) malloc(sizeof(tryItem)*tries_size);

            for(int i=0;i<tries_size;i++){
                fread(codeItem->tries->start_addr,U4,1,dexfile);
                fread(codeItem->tries->insn_count,U2,1,dexfile);
                fread(codeItem->tries->handler_off,U2,1,dexfile);


            }

            int handlersListSize = readLeb128(dexfile);
            codeItem->handlers.size = handlersListSize;
            codeItem->handlers.list = (encoded_catch_handler*) malloc(sizeof(encoded_catch_handler)*handlersListSize);

            for(int i=0;i<handlersListSize;i++){
                int addrPairsSize = readLeb128(dexfile); //encoded_type_addr_pair
                codeItem->handlers.list[i].size = addrPairsSize;

                if(addrPairsSize<0)
                    codeItem->handlers.list[i].handlers = (encoded_type_addr_pair*) malloc(sizeof(encoded_type_addr_pair)*(-1*addrPairsSize));
                else
                    codeItem->handlers.list[i].handlers = (encoded_type_addr_pair*) malloc(sizeof(encoded_type_addr_pair)*(addrPairsSize));


                for(int k=0;k<addrPairsSize;k++){
                    codeItem->handlers.list[i].handlers[k].type_idx = readLeb128(dexfile);
                    codeItem->handlers.list[i].handlers[k].addr = readLeb128(dexfile);
                }

                if(addrPairsSize<0)
                    codeItem->handlers.list[i].catch_all_addr = readLeb128(dexfile);


            }
        }
        else{
            codeItem->tries = NULL;
        }
    }

    for(int j=0;j<classDataItem->virtual_methods_size;j++){
        if(classDataItem->virtual_methods[j].code_off!=0){
            classDataItem->virtual_methods[j].methodCode = (code_item*) malloc(sizeof(code_item));
            code_item * codeItem = classDataItem->virtual_methods[j].methodCode;
            fseek(dexfile,classDataItem->virtual_methods[j].code_off,SEEK_SET);

            fread(codeItem->registers_size,U2,1,dexfile);
            fread(codeItem->ins_size,U2,1,dexfile); //do not confuse with insns_size!!
            fread(codeItem->outs_size,U2,1,dexfile);
            fread(codeItem->tries_size,U2,1,dexfile);
            fread(codeItem->debug_info_off,U4,1,dexfile);
            fread(codeItem->insns_size,U4,1,dexfile);
            codeItem->insns_size[0] *= 2; //insns size must be in single bytes for interpreter whereas Dalvik calculates size in 16-bit words


            u4 insns_size = codeItem->insns_size[0];
            u2 tries_size = codeItem->tries_size[0];

            // (classDataItem->directMethodData+j)->insns = (u2*) malloc(U2*insns_size);
            codeItem->insns = (u1*) malloc(U1*insns_size);
            //fread((classDataItem->directMethodData+j)->insns,U2,insns_size,dexfile);
            fread(codeItem->insns,U1,insns_size,dexfile);
            //loading actual method instructions

            if(tries_size > 0 && insns_size % 2 != 0)
                fseek(dexfile,U2,SEEK_CUR);

            if(tries_size>0){
                codeItem->tries = (tryItem*) malloc(sizeof(tryItem)*tries_size);

                for(int i=0;i<tries_size;i++){
                    fread(codeItem->tries->start_addr,U4,1,dexfile);
                    fread(codeItem->tries->insn_count,U2,1,dexfile);
                    fread(codeItem->tries->handler_off,U2,1,dexfile);


                }

                int handlersListSize = readLeb128(dexfile);
                codeItem->handlers.size = handlersListSize;
                codeItem->handlers.list = (encoded_catch_handler*) malloc(sizeof(encoded_catch_handler)*handlersListSize);

                for(int i=0;i<handlersListSize;i++){
                    int addrPairsSize = readLeb128(dexfile); //encoded_type_addr_pair
                    codeItem->handlers.list[i].size = addrPairsSize;

                    if(addrPairsSize<0)
                        codeItem->handlers.list[i].handlers = (encoded_type_addr_pair*) malloc(sizeof(encoded_type_addr_pair)*(-1*addrPairsSize));
                    else
                        codeItem->handlers.list[i].handlers = (encoded_type_addr_pair*) malloc(sizeof(encoded_type_addr_pair)*(addrPairsSize));


                    for(int k=0;k<addrPairsSize;k++){
                        codeItem->handlers.list[i].handlers[k].type_idx = readLeb128(dexfile);
                        codeItem->handlers.list[i].handlers[k].addr = readLeb128(dexfile);
                    }

                    if(addrPairsSize<0)
                        codeItem->handlers.list[i].catch_all_addr = readLeb128(dexfile);


                }
            }
            else{
                codeItem->tries = NULL;
            }
        }
        else{
            classDataItem->virtual_methods[j].methodCode = NULL;
        }

    }
    // }

}

u4 * readStringIds(FILE * dexfile,header* h){
    fseek(dexfile,h->string_ids_off[0],SEEK_SET);
    u4 * string_Ids = malloc(U4*h->string_ids_size[0]);
    fread(string_Ids,U4,h->string_ids_size[0],dexfile);
    return string_Ids;
}

String * strings(FILE * dexfile,header* h){

    fseek(dexfile,h->string_ids_off[0],SEEK_SET);
    String * strings = (String*) malloc(sizeof(String)*h->string_ids_size[0]);
    u4 string_Ids[h->string_ids_size[0]];
    fread(string_Ids,U4,h->string_ids_size[0],dexfile);
    for(int i=0;i<h->string_ids_size[0];i++){
        strings[i].string_id = (u1) string_Ids[i];
        const char * str = readString(dexfile,string_Ids[i]);
        strings[i].len = strlen(str);
        strings[i].string_data = malloc(strings[i].len);
        strcpy(strings[i].string_data,str);

    }
    return strings;


}

u4 * types(FILE *dexfile, header *h) {
    fseek(dexfile,h->type_ids_off[0],SEEK_SET);
    u4 * type_Ids= (u4*) malloc(U4*h->type_ids_size[0]);;
    fread(type_Ids,U4,h->type_ids_size[0],dexfile);
    return type_Ids;

}

proto_id_item * protoIds(FILE * dexfile,header *h){
    fseek(dexfile,h->proto_ids_off[0],SEEK_SET);
    proto_id_item * proto_Ids = (proto_id_item*) malloc(sizeof(proto_id_item)*h->proto_ids_size[0]);

    for(int i=0;i<h->proto_ids_size[0];i++){
        fread(proto_Ids[i].shorty_idx,U4,1,dexfile);
        fread(proto_Ids[i].return_type_idx,U4,1,dexfile);
        fread(proto_Ids[i].parameters_off,U4,1,dexfile);

    }
    return proto_Ids;
}

field_id_item * fieldsIds(FILE * dexfile,header * h){
    fseek(dexfile,h->field_ids_off[0],SEEK_SET);
    field_id_item * field_Ids = (field_id_item*) malloc(sizeof(field_id_item)*h->field_ids_size[0]);

    for(int i=0;i<h->field_ids_size[0];i++){
        fread(field_Ids[i].class_idx,U2,1,dexfile);
        fread(field_Ids[i].type_idx,U2,1,dexfile);
        fread(field_Ids[i].name_idx,U4,1,dexfile);

    }
    return field_Ids;
}

method_id_item * methodIdItems(FILE * dexfile,header *h) {
    fseek(dexfile, h->method_ids_off[0], SEEK_SET);
    method_id_item *method_id_items = (method_id_item *) malloc(sizeof(method_id_item) * h->method_ids_size[0]);

    for (int i = 0; i < h->method_ids_size[0]; i++) {
        fread(method_id_items[i].class_idx,U2,1,dexfile);
        fread(method_id_items[i].proto_idx,U2,1,dexfile);
        fread(method_id_items[i].name_idx,U4,1,dexfile);
    }
    return method_id_items;
}

map_list * mapList(FILE* dexfile,header* h){
    fseek(dexfile,h->map_off[0],SEEK_SET);
    u4 mapSize[1];
    fread(mapSize,U4,1,dexfile);
    map_list * mapList = (map_list*) malloc(sizeof(map_list));
    mapList->size[0] = mapSize[0];
    mapList->map_items = (map_item*) malloc(sizeof(map_item)*mapSize[0]);

    for(int i=0;i<mapSize[0];i++){
        fread(mapList->map_items[i].type,U2,1,dexfile);
        fseek(dexfile,U2,SEEK_CUR); //skip unused 2 bytes
        fread(mapList->map_items[i].size,U4,1,dexfile);
        fread(mapList->map_items[i].offset,U4,1,dexfile);

    }

    return mapList;
}

u4 * callSiteIds(DexStruct * dexStruct,FILE* dexfile,map_list * mapList){
    u4 * call_site_ids = NULL;
    int k=-1;
    for(int i=0;i<mapList->size[0];i++) {
        if (mapList->map_items[i].type[0] == TYPE_CALL_SITE_ID_ITEM) {
            k = i;
            break;
        }
    }

    if(k!=-1) {
        fseek(dexfile, mapList->map_items[k].offset[0], SEEK_SET);
        call_site_ids = (u4 *) malloc(sizeof(u4) * mapList->map_items[k].size[0]);
        dexStruct ->call_site_ids_size = mapList->map_items[k].size[0];
        fread(call_site_ids, U4, mapList->map_items[k].size[0], dexfile);
    }

    return call_site_ids;
}

method_handle_item * methodHandleItems(DexStruct * dexStruct,FILE* dexfile,map_list * mapList){
    method_handle_item * method_handle_items = NULL;
    int k = -1;
    for(int i=0;i<mapList->size[0];i++){
        if(mapList->map_items[i].type[0]==TYPE_METHOD_HANDLE_ITEM){
            k=i;
            break;
        }
    }

    if(k!=-1) {
        fseek(dexfile, mapList->map_items[k].offset[0], SEEK_SET);
        method_handle_items = (method_handle_item *) malloc(sizeof(method_handle_item) * mapList->map_items[k].size[0]);
        dexStruct->method_handle_items_size = mapList->map_items[k].size[0];

        for (int j = 0; j < mapList->map_items[k].size[0]; j++) {
            fread(method_handle_items[j].method_handle_type, U2, 1, dexfile);
            fseek(dexfile, U2, SEEK_CUR);
            fread(method_handle_items[j].field_or_method_id, U2, 1, dexfile);
            fseek(dexfile, U2, SEEK_CUR);
        }
    }
    return method_handle_items;
}

u1 * dataSection(FILE *dexfile,header*h){
    fseek(dexfile,h->data_off[0],SEEK_SET);
    u1 * data = (u1*) malloc(sizeof(u1)*h->data_size[0]);
    fread(data,U1,h->data_size[0],dexfile);

    return data;
}

u1 * linkData(FILE* dexfile,header *h){
    fseek(dexfile,h->link_off[0],SEEK_SET);
    u1 * link_data = (u1*) malloc(sizeof(u1)*h->link_size[0]);
    fread(link_data,U1,h->link_size[0],dexfile);


    return link_data;
}

void findMain(FILE * dexfile,DexStruct * p){
    const char * methodName = NULL;
    for(int i=0;i<p->head->method_ids_size[0];i++){
        if(strcmp(p->strings[p->method_id_items[i].name_idx[0]].string_data,"main")==0)
            p->method_id_items[i].is_main = 1;
        else if(strcmp(p->strings[p->method_id_items[i].name_idx[0]].string_data,"<init>")==0 && strcmp(p->strings[p->type_ids[p->method_id_items[i].class_idx[0]]].string_data,"Ljava/lang/Object;")==0){
            p->method_id_items[i].is_ObjInit = 1;
            p->method_id_items[i].is_main = 0;
        }
        else if(strcmp(p->strings[p->method_id_items[i].name_idx[0]].string_data,"<clinit>")==0){
            p->method_id_items[i].is_main = 2; //only present if there are static values with initial values in some class, each occurrence of <clinit> in all classes must run before main.
        }
        else
            p->method_id_items[i].is_main = 0;
    }
    free(methodName);
}

rDex makePayload(DexStruct * dexStruct){
    rDex payload;
    payload.strings = dexStruct->strings;
    payload.sizes.method_ids_size = dexStruct->head->method_ids_size[0];
    payload.classDefs = dexStruct->classDefs;
    payload.sizes.class_defs_size = dexStruct->head->class_defs_size[0];
    payload.method_id_items = dexStruct->method_id_items;
    payload.sizes.string_ids_size = dexStruct->head->string_ids_size[0];
    payload.sizes.type_ids_size = dexStruct->head->type_ids_size[0];
    payload.type_ids = dexStruct->type_ids;
    payload.sizes.field_ids_size = dexStruct->head->field_ids_size[0];
    payload.field_id_items = dexStruct->field_id_items;
    return payload;
}

void writePayload(rDex * p){
    FILE * pfile = fopen("classes.rdex","w");
    fwrite(&p->sizes,SIZESSTRUCT_LOADER_SIZE,1,pfile);

    for(u2 i=0;i<p->sizes.field_ids_size;i++){
        fwrite(p->field_id_items+i,FIELDIDITEM_LOADER_SIZE,1,pfile);
    }
    //fflush(pfile);
    for(u1 i=0;i<p->sizes.type_ids_size;i++){
        u2 type_id = (u2) p->type_ids[i];
        fwrite(&type_id,U2,1,pfile);
    }

    for(u1 i=0;i<p->sizes.method_ids_size;i++){
        fwrite(&p->method_id_items[i].class_idx[0],U2,1,pfile);
        u2 name_idx = (u2) p->method_id_items[i].name_idx[0];
        fwrite(&name_idx,U2,1,pfile);
        fwrite(&p->method_id_items[i].is_main,U1,1,pfile);
        fwrite(&p->method_id_items[i].is_ObjInit,U1,1,pfile);
    }

    for(u1 i=0;i<p->sizes.class_defs_size;i++){
        u2 class_def = (u2) p->classDefs[i].class_idx[0];
        fwrite(&class_def,U2,1,pfile);
        u1 class_data_off = (u1) p->classDefs[i].class_data_off;
        fwrite(&class_data_off,U1,1,pfile);
        if(p->classDefs[i].class_data_off!=0) {
            u1 static_fields_size = (u1) p->classDefs[i].classData->static_fields_size;
            u1 instance_fields_size = (u1) p->classDefs[i].classData->instance_fields_size;
            u1 direct_methods_size = (u1) p->classDefs[i].classData->direct_methods_size;
            u1 virtual_methods_size = (u1) p->classDefs[i].classData->virtual_methods_size;
            fwrite(&static_fields_size, U1, 1, pfile);
            fwrite(&instance_fields_size, U1, 1, pfile);
            fwrite(&direct_methods_size, U1, 1, pfile);
            fwrite(&virtual_methods_size, U1, 1, pfile);

            for(u1 j=0;j<p->classDefs[i].classData->instance_fields_size;j++) {
                u2 field_idx_diff = (u2) p->classDefs[i].classData->instance_fields[j].field_idx_diff;
                fwrite(&field_idx_diff,U2, 1, pfile);
            }
            for(u1 j=0;j<p->classDefs[i].classData->static_fields_size;j++) {
                u2 field_idx_diff = (u2) p->classDefs[i].classData->static_fields[j].field_idx_diff;
                fwrite(&field_idx_diff,U2, 1, pfile);
            }

            for(int j=0;j<p->classDefs[i].classData->direct_methods_size;j++){
                u2 method_idx_diff = (u2) p->classDefs[i].classData->direct_methods[j].method_idx_diff;
                fwrite(&method_idx_diff,U2,1, pfile);
                //fflush(pfile);
                fwrite((p->classDefs[i].classData->direct_methods+j)->methodCode,CODEITEMSIZE_LOADER,1, pfile);
                //fflush(pfile);
                fwrite((p->classDefs[i].classData->direct_methods+j)->methodCode->insns,U1,(p->classDefs[i].classData->direct_methods+j)->methodCode->insns_size[0], pfile);
                //fflush(pfile);
            }
            for(int j=0;j<p->classDefs[i].classData->virtual_methods_size;j++){
                u2 method_idx_diff = (u2) p->classDefs[i].classData->virtual_methods[j].method_idx_diff;
                fwrite(&method_idx_diff,U1,1, pfile);
                //fflush(pfile);
                if(p->classDefs[i].classData->virtual_methods[j].code_off!=0) {
                    fwrite((p->classDefs[i].classData->virtual_methods + j)->methodCode, CODEITEMSIZE_LOADER, 1, pfile);
                    //fflush(pfile);
                    fwrite((p->classDefs[i].classData->virtual_methods + j)->methodCode->insns, U1,
                           (p->classDefs[i].classData->virtual_methods + j)->methodCode->insns_size[0], pfile);
                }
            }
        }
    }


    for(int i=0;i<p->sizes.string_ids_size;i++){
        fwrite(p->strings+i,STRINGSTRUCTSIZE_LOADER,1,pfile);
        /*string data in constant pool larger than 2 chars (ie: types) is not written, string comparisons to strings in the constant pool may be carried
         * out anyway since string_id is still stored.
         * String literals may be used but must be prefixed with a leading $ in the Java code, otherwise they will be optimized out of
         * the constant pool.*/
        if(p->strings[i].len>0){
            fwrite(p->strings[i].string_data, sizeof(char), p->strings[i].len, pfile);
        }
    }
    //fwrite(p->method_handle_items,sizeof(method_handle_item),p->sizes.method_handle_items_size,pfile);
    //fwrite(p->call_site_ids,U4,p->sizes.call_site_ids_size,pfile);

    fflush(pfile);
    fclose(pfile);

}

void writePayloadByteArray(){
    FILE * rDexFile = fopen("classes.rdex","rb");
    FILE * byteFile = fopen("Interpreter/Bytes.c","w+");
    u1 byte;
    fread(&byte,U1,1,rDexFile);
    fputs("#include \"Bytes.h\"\n\n",byteFile);
    fprintf(byteFile,"const u1 bytes[]={0x%X",byte);
    long i = 0;

    while(fread(&byte,U1,1,rDexFile)) {
        fprintf(byteFile, ",0x%X", byte);

        if(i++%12==0) {
            fprintf(byteFile, "\n");
            fflush(byteFile);
        }
    }
    fputs("};\n",byteFile);
    fprintf(byteFile,"\nconst uint32_t len = %ld;\n",i);
    fclose(rDexFile);
    fflush(byteFile);
}

rDex readRdex(const char * f){
    FILE * pfile = fopen(f,"r");
    rDex p;
    fread(&p.sizes,SIZESSTRUCT_LOADER_SIZE,1,pfile);

    p.field_id_items = malloc(p.sizes.field_ids_size*FIELDIDITEM_LOADER_SIZE);
    for(u2 i=0;i<p.sizes.field_ids_size;i++){
        fread(p.field_id_items+i,FIELDIDITEM_LOADER_SIZE,1,pfile);
    }
    #ifdef LOADER //for testing
        p.type_ids = malloc(p.sizes.type_ids_size*U4);
    #endif
    #ifdef INTERPRETER
        p.type_ids = malloc(p.sizes.type_ids_size*U2);
    #endif
    for(u1 i=0;i<p.sizes.type_ids_size;i++){
        fread(&p.type_ids[i],U2,1,pfile);
    }
    p.method_id_items = malloc(sizeof(method_id_item));
    for(u1 i=0;i<p.sizes.method_ids_size;i++){
        fread(&p.method_id_items[i].class_idx[0],U2,1,pfile);
        fread(&p.method_id_items[i].name_idx[0],U2,1,pfile);
        fread(&p.method_id_items[i].is_main,U1,1,pfile);
        fread(&p.method_id_items[i].is_ObjInit,U1,1,pfile);
    }

    p.classDefs = malloc(sizeof(class_def_item));
    for(u1 i=0;i<p.sizes.class_defs_size;i++){
        fread(&p.classDefs[i].class_idx[0],U2,1,pfile);
        fread(& p.classDefs[i].class_data_off,U1,1,pfile);
        if(p.classDefs[i].class_data_off!=0) {
            p.classDefs[i].classData = malloc(sizeof(class_data_item));
            fread(&p.classDefs[i].classData->static_fields_size, U1, 1, pfile);
            fread(&p.classDefs[i].classData->instance_fields_size, U1, 1, pfile);
            fread(&p.classDefs[i].classData->direct_methods_size, U1, 1, pfile);
            fread(&p.classDefs[i].classData->virtual_methods_size, U1, 1, pfile);

            p.classDefs[i].classData->instance_fields = malloc(p.classDefs[i].classData->instance_fields_size*sizeof(encoded_field));
            for(u1 j=0;j<p.classDefs[i].classData->instance_fields_size;j++) {
                fread(&p.classDefs[i].classData->instance_fields[j].field_idx_diff,U2, 1, pfile);
            }

            p.classDefs[i].classData->static_fields = malloc(p.classDefs[i].classData->static_fields_size*sizeof(encoded_field));
            for(u1 j=0;j<p.classDefs[i].classData->static_fields_size;j++) {
                fread(&p.classDefs[i].classData->static_fields[j].field_idx_diff,U2, 1, pfile);
            }

            p.classDefs[i].classData->direct_methods = malloc(p.classDefs[i].classData->direct_methods_size*sizeof(encoded_method));
            for(int j=0;j<p.classDefs[i].classData->direct_methods_size;j++){
                fread(&p.classDefs[i].classData->direct_methods[j].method_idx_diff,U1,1, pfile);
                
                (p.classDefs[i].classData->direct_methods+j)->methodCode = malloc(sizeof(code_item));
                fread((p.classDefs[i].classData->direct_methods+j)->methodCode,CODEITEMSIZE_LOADER,1, pfile);

                (p.classDefs[i].classData->direct_methods+j)->methodCode->insns = malloc((p.classDefs[i].classData->direct_methods+j)->methodCode->insns_size[0]);
                fread((p.classDefs[i].classData->direct_methods+j)->methodCode->insns,U1,(p.classDefs[i].classData->direct_methods+j)->methodCode->insns_size[0], pfile);
                //fflush(pfile);
            }
            p.classDefs[i].classData->virtual_methods = malloc(p.classDefs[i].classData->virtual_methods_size*sizeof(encoded_method));
            for(int j=0;j<p.classDefs[i].classData->virtual_methods_size;j++){
                fread(&p.classDefs[i].classData->virtual_methods[j].method_idx_diff,U1,1, pfile);

                (p.classDefs[i].classData->virtual_methods+j)->methodCode = malloc(sizeof(code_item));
                fread((p.classDefs[i].classData->virtual_methods+j)->methodCode,CODEITEMSIZE_LOADER,1, pfile);

                (p.classDefs[i].classData->virtual_methods+j)->methodCode->insns = malloc((p.classDefs[i].classData->virtual_methods+j)->methodCode->insns_size[0]);
                fread((p.classDefs[i].classData->virtual_methods+j)->methodCode->insns,U1,(p.classDefs[i].classData->virtual_methods+j)->methodCode->insns_size[0], pfile);
                //fflush(pfile);
            }
        }
    }

    p.strings = malloc(sizeof(String));
    for(int i=0;i<p.sizes.string_ids_size;i++){
        fread(p.strings+i,STRINGSTRUCTSIZE_LOADER,1,pfile);
        /*string data in constant pool larger than 2 chars (ie: types) is not written, string comparisons to strings in the constant pool may be carried
         * out anyway since string_id is still stored.
         * String literals may be used but must be prefixed with a leading $ in the Java code, otherwise they will be optimized out of
         * the constant pool.*/
        if(p.strings[i].len>0){
            p.strings[i].string_data = malloc(p.strings[i].len);
            fread(p.strings[i].string_data, U1, p.strings[i].len, pfile);
        }
    }
    //fread(p.method_handle_items,sizeof(method_handle_item),p.sizes.method_handle_items_size,pfile);
    //fread(p.call_site_ids,U4,p.sizes.call_site_ids_size,pfile);


    fclose(pfile);

    return p;
}

//void readInterfaces(FILE * dexfile,DexStruct *p){
//    for(u2 i =0;i<p->head->class_defs_size[0];i++){
//        if(p->classDefs[i].interfaces_off[0]>0) {
//            fseek(dexfile, p->classDefs[i].interfaces_off[0], SEEK_SET);
//            u4 intrf_size[1];
//            fread(intrf_size, U4, 1, dexfile);
//            p->classDefs[i].interfaces_len = intrf_size[0];
//            p->classDefs[i].interfaces_indices = malloc(U2 * intrf_size[0]);
//            fread(p->classDefs[i].interfaces_indices, U2, intrf_size[0], dexfile);
//        }
//
//    }
//}
