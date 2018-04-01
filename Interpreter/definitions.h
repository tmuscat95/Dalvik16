/*
*  Macro Definitions and Supporting Data Structures
*/
#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#define INTERPRETER
#include "stdint.h"






#define HEADERSIZE 112


#define NOT_DEX -1
#define CORRUPT -2
#define VALID 0


#define TYPE_HEADER_ITEM 0x0000
#define TYPE_STRING_ID_ITEM 0x0001
#define TYPE_TYPE_ID_ITEM 0x0002
#define TYPE_PROTO_ID_ITEM 	0x0003
#define TYPE_FIELD_ID_ITEM 	0x0004
#define TYPE_METHOD_ID_ITEM 0x0005
#define TYPE_CLASS_DEF_ITEM 	0x0006
#define TYPE_CALL_SITE_ID_ITEM 	0x0007
#define TYPE_METHOD_HANDLE_ITEM 	0x0008
#define TYPE_MAP_LIST 	0x1000
#define TYPE_TYPE_LIST 	0x1001
#define TYPE_ANNOTATION_SET_REF_LIST 	0x1002
#define TYPE_ANNOTATION_SET_ITEM 	0x1003
#define TYPE_CLASS_DATA_ITEM 	0x2000
#define TYPE_CODE_ITEM 	0x2001
#define TYPE_STRING_DATA_ITEM 	0x2002
#define TYPE_DEBUG_INFO_ITEM 	0x2003
#define TYPE_ANNOTATION_ITEM 	0x2004
#define TYPE_ENCODED_ARRAY_ITEM 0x2005
#define TYPE_ANNOTATIONS_DIRECTORY_ITEM 0x2006


#define U1 1
#define U2 2
#define U4 4
#define U8 8

#define LITTLE 0x12345678
#define MAXLEN_STR 24
#define NO_INDEX 0xffffffff

typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;


typedef struct __attribute__((packed)) header {



    u4 file_size[1];


    u4 link_size[1];
    u4 link_off[1];
    u4 map_off[1];
    u4 string_ids_size[1];
    u4 string_ids_off[1];
    u4 type_ids_size[1];
    u4 type_ids_off[1];
    u4 proto_ids_size[1];
    u4 proto_ids_off[1];
    u4 field_ids_size[1];
    u4 field_ids_off[1];
    u4 method_ids_size[1];
    u4 method_ids_off[1];
    u4 class_defs_size[1];
    u4 class_defs_off[1];
    u4 data_size[1];
    u4 data_off[1];

    u4 class_data_count;
} header;

typedef struct __attribute__((packed)){
    u4 start_addr[1];
    u2 insn_count[1];
    u2 handler_off[1];
}tryItem;

typedef struct __attribute__((packed)){
    u4 type_idx;
    u4 addr;
}encoded_type_addr_pair;

typedef struct __attribute__((packed)){
    s4 size;
    encoded_type_addr_pair * handlers;
    u4 catch_all_addr;
}encoded_catch_handler;

typedef struct __attribute__((packed)){
    s4 size;
    encoded_catch_handler * list;
}encoded_catch_handler_list;

typedef struct __attribute__((packed)){
#ifdef LOADER
    u4 field_idx_diff;
    u4 access_flags;
#endif
#ifdef INTERPRETER
    u2 field_idx_diff;
#endif
}encoded_field;

typedef struct __attribute__((packed)) {
    u2 class_idx[1];

#ifdef LOADER
    u2 proto_idx[1];
    u4 name_idx[1];
#endif
#ifdef INTERPRETER
    u2 name_idx[1];
#endif
    u1 is_main;
    u1 is_ObjInit;

}method_id_item;

#define CODEITEMSIZE_LOADER 10
typedef struct __attribute__((packed)){
    u2 registers_size[1];
    u2 ins_size[1];
    u2 outs_size[1];
    u4 insns_size[1];
    u1 * insns;
#ifdef LOADER
    u2 tries_size[1];
    u4 debug_info_off[1];
    encoded_catch_handler_list handlers;
    tryItem * tries;
#endif

}code_item;

typedef struct __attribute__((packed)){
#ifdef LOADER
    u4 method_idx_diff;
    u4 access_flags;
    u4 code_off;
#endif
#ifdef INTERPRETER
    u2 method_idx_diff;
#endif
    code_item * methodCode;
}encoded_method;

typedef struct __attribute__((packed)){
#ifdef LOADER
    u4 static_fields_size;
    u4 instance_fields_size;
    u4 direct_methods_size;
    u4 virtual_methods_size;
#endif
#ifdef INTERPRETER
    u1 static_fields_size;
    u1 instance_fields_size;
    u1 direct_methods_size;
    u1 virtual_methods_size;
#endif
    encoded_field * static_fields;
    encoded_field *instance_fields;
    encoded_method *direct_methods;
    encoded_method *virtual_methods;

}class_data_item;

typedef struct __attribute__((packed)) class_def_item{
#ifdef LOADER
    u4 class_idx[1];
    u4 class_data_off[1];
    u4 access_flags[1];
    u4 superclass_idx[1];
    u4 interfaces_off[1];
    u4 source_file_idx[1];
    u4 annotations_off[1];
    u4 static_values_off[1];

    u2 superclass_index;
    u1 interfaces_len;
    u2 * interfaces_indices;
 #endif
#ifdef INTERPRETER
    u2 class_idx[1];
    u1 class_data_off;
#endif
    class_data_item * classData;
} class_def_item;

typedef struct __attribute__((packed)){
    u2 utf16_size;
    u1 * data;
}string_data_item;

typedef struct __attribute__((packed)){
    u4 shorty_idx[1];
    u4 return_type_idx[1];
    u4 parameters_off[1];
}proto_id_item;

typedef struct __attribute__((packed)){
    u2 type[1];
    u4 size[1];
    u4 offset[1];
}map_item;

typedef struct __attribute__((packed)){
    u4 size[1];
    map_item * map_items;
}map_list;

typedef struct __attribute__((packed)){
    u2 method_handle_type[1];
    u2 field_or_method_id[1];
}method_handle_item;

#define STRINGSTRUCTSIZE_LOADER 3
typedef struct __attribute__((packed)){
    u2 string_id;
    u1 len;
    char * string_data;
}String;

struct __attribute__((packed)) Instance;
struct __attribute__((packed)) field_id_item;

#define FIELDIDITEM_LOADER_SIZE 6
typedef struct __attribute__((packed)) field_id_item{
    u2 class_idx[1];
    u2 type_idx[1];
    u2 name_idx[1];

    u1 isObjRef;

    union{
        u2 Char;
        s1 Byte;
        s2 Short;
        struct Instance * ObjectRef;
        s4 Integer;
        s8 Long;
        double Double;
        float Float;
    };
}field_id_item;

typedef struct __attribute__((packed)) Instance{
    class_def_item * classDef;
    field_id_item * fields;
    u2 _pad;
    u1 __pad;
    u2 references;
}Instance;

typedef struct __attribute__((packed)){
    void* array;
    field_id_item * pad; //not used, used to make Array and Instance structures same size
    u2 size; //CHANGED
    u1 type;
    u2 references; //CHANGED
}Array;

typedef struct __attribute__((packed)){
    header * head;
    class_def_item * classDefs;
    class_data_item * classDataItems;
    u4 * string_ids;
    String * strings;
    u4 * type_ids;

    proto_id_item * proto_id_items;
    field_id_item * field_id_items;
    map_list * MapList;
    u4 * call_site_ids;
    u4 call_site_ids_size;
    method_id_item * method_id_items;
    method_handle_item * method_handle_items;
    u4 method_handle_items_size;
    u1 * data;
    u1 * link_data;
}DexStruct;

#define SIZESSTRUCT_LOADER_SIZE 5
struct __attribute__((packed)) sizes {
    u1 string_ids_size;
    u1 type_ids_size;
    u1 field_ids_size;
    u1 method_ids_size;
    u1 class_defs_size;
};

typedef struct __attribute__((packed)) {
    struct sizes sizes;
    String * strings;
    class_def_item * classDefs;
#ifdef LOADER
    u4 * type_ids;
#endif
#ifdef INTERPRETER
    u2 * type_ids;
#endif
    method_id_item * method_id_items;
    field_id_item * field_id_items;

}rDex;
#endif
