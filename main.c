#include "loader.h"
#include "interpreter.h"

DexStruct d;
rDex q;
extern rDex * p;
extern FILE * defs;

int main(int argc,char *argv[]){

    FILE * dexfile = fopen(argv[1],"rb");
    defs = fopen("Interpreter/opcodes.h","w");

    d.head = readHeader(dexfile); //ok
    d.MapList = mapList(dexfile,d.head);
    d.strings = strings(dexfile,d.head);
    d.string_ids = readStringIds(dexfile,d.head);
    d.field_id_items = fieldsIds(dexfile,d.head);
    d.type_ids = types(dexfile, d.head);
    d.classDefs = readClassDefs(dexfile,d.MapList,&d);
    d.classDataItems = readClassData(dexfile,d.classDefs,d.head);
    for(int i=0;i<d.head->class_defs_size[0];i++){
        if((d.classDefs+i)->classData!=NULL)
            readMethodCode(dexfile,(d.classDefs+i)->classData,d.head);
    }
    d.proto_id_items = protoIds(dexfile,d.head);
    d.call_site_ids = callSiteIds(&d,dexfile,d.MapList);
    d.method_handle_items = methodHandleItems(&d,dexfile,d.MapList);
    d.data = dataSection(dexfile,d.head);
    d.link_data = linkData(dexfile,d.head);
    d.method_id_items = methodIdItems(dexfile,d.head);
    findMain(dexfile,&d);
    fclose(dexfile);
    q = makePayload(&d);
    p = &q;
    writePayload(p);
    writePayloadByteArray();

    RunMain();

    puts("\nDone loading.\n");
    fclose(defs);
//    getchar();

    return 0;

}
