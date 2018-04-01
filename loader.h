#ifndef LOADER_H_INCLUDED
#define LOADER_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "definitions.h"


extern u4 * header_size;

long readLeb128(FILE *dexfile);
const char * readString(FILE * dexfile,u4 offset);

void diagnostic(int err);

int validDex(const u1 * magic);

header * readHeader(FILE * dexfile);

//class_def_item * readClassDefs(FILE * dexfile,map_list * mapList);
class_def_item * readClassDefs(FILE * dexfile,map_list * mapList,DexStruct * d);

class_data_item * readClassData(FILE * dexfile,class_def_item * classDefs,header * h);
void readMethodCode(FILE * dexfile,class_data_item * classDataItem,header * h);
String* strings(FILE * dexfile,header* h);
u4 *types(FILE *dexfile, header *h);
proto_id_item * protoIds(FILE * dexfile,header *h);
field_id_item * fieldsIds(FILE * dexfile,header * h);
map_list * mapList(FILE* dexfile,header* h);
u4 * callSiteIds(DexStruct * dexStruct,FILE* dexfile,map_list * mapList);
method_handle_item * methodHandleItems(DexStruct * dexStruct,FILE* dexfile,map_list * mapList);
u1 * dataSection(FILE *dexfile,header*h);
u1 * linkData(FILE* dexfile,header *h);
method_id_item * methodIdItems(FILE * dexfile,header *h);
void findMain(FILE * dexfile,DexStruct * p);
rDex makePayload(DexStruct * dexStruct);
void writePayload(rDex * p);
void writePayloadByteArray();
u4 * readStringIds(FILE * dexfile,header* h);
rDex readRdex(const char * f);
//void readInterfaces(FILE * dexfile,DexStruct *p);





#endif // LOADER_H_INCLUDED