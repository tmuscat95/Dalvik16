/*Dalvik16. (Timothy Muscat Bsc.Computing Science, University of Malta Class of 2018 (Undergraduate Dissertation))
 *
 * Dalvik Bytecode interpreter for 16-bit architectures/resource constrained devices.
 * Support for most features of Dalvik 38 (Dalvik 35 will also work, provided none of the opcodes removed in Dalvik 38 are present)
 * Language features NOT supported at the moment (for any Dalvik version):
 * Multithreading
 * Exception Handling
 * Method Handles
 *
 * Timothy Muscat Bsc.Computing Science, University of Malta Class of 2018 (Undergraduate Dissertation)
 *
 * Takes as input file of format .rdex, outputted by the loader program.
 * */
#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "ieee754.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "definitions.h"
//#include <math.h>
#define NOP 0x00
#define MOVE 0x01
#define MOVE16to8 0x02
#define MOVE16to16 0x03
#define MOVEWIDE 0x04
#define MOVEWIDE16to8 0x05
#define MOVEWIDE16to16 0x06
#define MOVEOBJECT 0x07
#define MOVEOBJECT16to8 0x08
#define MOVEOBJECT16to16 0x09
#define MOVERESULT 0x0a
#define MOVERESULTWIDE 0x0b
#define MOVERESULTOBJECT 0x0c
#define MOVEEXCEPTION 0x0d
#define RET_VOID 0x0e
#define RET 0x0f
#define RETWIDE 0x10
#define RETOBJ 0x11
#define CONSTvAB 0x12
#define CONSTvAABBBB 0x13
#define CONSTvAABBBBBBBB 0x14
#define CONSTvAA_4B_40 0x15
#define CONSTWIDEvAABBBB 0x16
#define CONSTWIDEvAA_8B 0x17
#define CONSTWIDEvAA_16B 0x18
#define CONSTWIDEvAA_4B_120 0x19
#define CONSTSTRINGvAA_4B 0x1a
#define CONSTSTRINGvAA_8B 0x1b
#define CONSTCLASSvAA_4B 0x1c
#define MONITOR_ENTER 0x1d
#define MONITOR_EXIT 0x1e
#define CHECKCAST 0x1f
#define INSTANCE_OF 0x20
#define ARRAY_LENGTH 0x21
#define NEW_INSTANCE 0x22
#define NEW_ARRAY 0x23
#define FILLED_NEW_ARRAY 0x24
#define FILLED_NEW_ARRAY_RANGE 0x25
#define FILL_ARRAY_DATA 0x26
#define THROW 0x27
#define GOTO 0x28
#define GOTO16 0x29
#define GOTO32 0x2a
#define PACKED_SWITCH 0x2b
#define SPARSE_SWITCH 0x2c
#define CMPL_FLOAT 0x2d
#define CMPG_FLOAT 0x2e
#define CMPL_DOUBLE 0x2f
#define CMPG_DOUBLE 0x30
#define CMP_LONG 0x31
#define IFEQ 0x32
#define IFNE 0x33
#define IFLT 0x34
#define IFGE 0x35
#define IFGT 0x36
#define IFLE 0x37
#define IFEQZ 0x38
#define IFNEZ 0x39
#define IFLTZ 0x3a
#define IFGEZ 0x3b
#define IFGTZ 0x3c
#define IFLEZ 0x3d
#define AGET 0x44
#define AGET_WIDE 0x45
#define AGET_OBJ 0x46
#define AGET_BOOL 0x47
#define AGET_BYTE 0x48
#define AGET_CHAR 0x49
#define AGET_SHORT 0x4a
#define APUT 0x4b
#define APUT_WIDE 0x4c
#define APUT_OBJ 0x4d
#define APUT_BOOL 0x4e
#define APUT_BYTE 0x4f
#define APUT_CHAR 0x50
#define APUT_SHORT 0x51
#define IGET 0x52
#define IGET_WIDE 0x53
#define IGET_OBJ 0x54
#define IGET_BOOL 0x55
#define IGET_BYTE 0x56
#define IGET_CHAR 0x57
#define IGET_SHORT 0x58
#define IPUT 0x59
#define IPUT_WIDE 0x5a
#define IPUT_OBJ 0x5b
#define IPUT_BOOL 0x5c
#define IPUT_BYTE 0x5d
#define IPUT_CHAR 0x5e
#define IPUT_SHORT 0x5f
#define SGET 0x60
#define SGET_WIDE 0x61
#define SGET_OBJ 0x62
#define SGET_BOOL 0x63
#define SGET_BYTE 0x64
#define SGET_CHAR 0x65
#define SGET_SHORT 0x66
#define SPUT 0x67
#define SPUT_WIDE 0x68
#define SPUT_OBJ 0x69
#define SPUT_BOOL 0x6a
#define SPUT_BYTE 0x6b
#define SPUT_CHAR 0x6c
#define SPUT_SHORT 0x6d
#define INVOKE_VIRTUAL 0x6e
#define INVOKE_SUPER 0x6f
#define INVOKE_DIRECT 0x70
#define INVOKE_STATIC 0x71
#define INVOKE_INTERFACE 0x72
#define INVOKE_VIRTUAL_RANGE 0x74
#define INVOKE_SUPER_RANGE 0x75
#define INVOKE_DIRECT_RANGE 0x76
#define INVOKE_STATIC_RANGE 0x77
#define INVOKE_INTERFACE_RANGE 0x78
#define NEG_INT 0x7b
#define NOT_INT 0x7c
#define NEG_LONG 0x7d
#define NOT_LONG 0x7e
#define NEG_FLOAT 0x7f
#define NEG_DOUBLE 0x80
#define INT_TO_LONG 0x81
#define INT_TO_FLOAT 0x82
#define INT_TO_DOUBLE 0x83
#define LONG_TO_INT 0x84
#define LONG_TO_FLOAT 0x85
#define LONG_TO_DOUBLE 0x86
#define FLOAT_TO_INT 0x87
#define FLOAT_TO_LONG 0x88
#define FLOAT_TO_DOUBLE 0x89
#define DOUBLE_TO_INT 0x8a
#define DOUBLE_TO_LONG 0x8b
#define DOUBLE_TO_FLOAT 0x8c
#define INT_TO_BYTE 0x8d
#define INT_TO_CHAR 0x8e
#define INT_TO_SHORT 0x8f
#define ADD_INT 0x90
#define SUB_INT 0x91
#define MUL_INT 0x92
#define DIV_INT 0x93
#define REM_INT 0x94
#define AND_INT 0x95
#define OR_INT 0x96
#define XOR_INT 0x97
#define SHL_INT 0x98
#define SHR_INT 0x99
#define USHR_INT 0x9a
#define ADD_LONG 0x9b
#define SUB_LONG 0x9c
#define MUL_LONG 0x9d
#define DIV_LONG 0x9e
#define REM_LONG 0x9f
#define AND_LONG 0xa0
#define OR_LONG 0xa1
#define XOR_LONG 0xa2
#define SHL_LONG 0xa3
#define SHR_LONG 0xa4
#define USHR_LONG 0xa5
#define ADD_FLOAT 0xa6
#define SUB_FLOAT 0xa7
#define MUL_FLOAT 0xa8
#define DIV_FLOAT 0xa9
#define REM_FLOAT 0xaa
#define ADD_DOUBLE 0xab
#define SUB_DOUBLE 0xac
#define MUL_DOUBLE 0xad
#define DIV_DOUBLE 0xae
#define REM_DOUBLE 0xaf
#define ADD_INT2 0xb0
#define SUB_INT2 0xb1
#define MUL_INT2 0xb2
#define DIV_INT2 0xb3
#define REM_INT2 0xb4
#define AND_INT2 0xb5
#define OR_INT2 0xb6
#define XOR_INT2 0xb7
#define SHL_INT2 0xb8
#define SHR_INT2 0xb9
#define USHR_INT2 0xba
#define ADD_LONG2 0xbb
#define SUB_LONG2 0xbc
#define MUL_LONG2 0xbd
#define DIV_LONG2 0xbe
#define REM_LONG2 0xbf
#define AND_LONG2 0xc0
#define OR_LONG2 0xc1
#define XOR_LONG2 0xc2
#define SHL_LONG2 0xc3
#define SHR_LONG2 0xc4
#define USHR_LONG2 0xc5
#define ADD_FLOAT2 0xc6
#define SUB_FLOAT2 0xc7
#define MUL_FLOAT2 0xc8
#define DIV_FLOAT2 0xc9
#define REM_FLOAT2 0xca
#define ADD_DOUBLE2 0xcb
#define SUB_DOUBLE2 0xcc
#define MUL_DOUBLE2 0xcd
#define DIV_DOUBLE2 0xce
#define REM_DOUBLE2 0xcf

#define ADD_INT_LIT16 0xd0
#define RSUB_INT 0xd1
#define MUL_INT_LIT16 0xd2
#define DIV_INT_LIT16 0xd3
#define REM_INT_LIT16 0xd4
#define AND_INT_LIT16 0xd5
#define OR_INT_LIT16 0xd6
#define XOR_INT_LIT16 0xd7
#define ADD_INT_LIT8 0xd8
#define RSUB_INT_LIT8 0xd9
#define MUL_INT_LIT8 0xda
#define DIV_INT_LIT8 0xdb
#define REM_INT_LIT8 0xdc
#define AND_INT_LIT8 0xdd
#define OR_INT_LIT8 0xde
#define XOR_INT_LIT8 0xdf
#define SHL_INT_LIT8 0xe0
#define SHR_INT_LIT8 0xe1
#define USHR_INT_LIT8 0xe2
#define INVOKE_POLYMORPHIC 0xfa
#define INVOKE_POLYMORPHIC_RANGE 0xfb
#define INVOKE_CUSTOM 0xfc
#define INVOKE_CUSTOM_RANGE 0xfd
#define LSB(X) X & 0b00001111
#define MSB(X) X >> 4
#define SIGNEX4to32(X) 0b11111111111111111111111111110000 ^ X;
#define SIGNEX8to32(X) 0b11111111111111111111111100000000 ^ X;
#define SIGNEX16to32(X) 0b11111111111111110000000000000000 ^ X;
#define x32BITMASK1s 0b11111111111111111111111111111111
#define x64BITMASK_32LSB 0b0000000000000000000000000000000011111111111111111111111111111111

extern rDex * p;;
FILE * defs;
//little endian
typedef struct dalvikRegister{
    u4 bits: 32; //MSP430 ints are 16 bits same as short int, long ints are 32 long long int is 64
}Reg;


typedef struct nibbleAddress{
    unsigned int To: 4;
    unsigned int From: 4; //Used for 4 bit addressing (first 16 registers)
}NibbleAddress;

typedef struct instance{
    unsigned int fieldIndex: 4;
    unsigned int InstanceRef: 4; //Used for 4 bit addressing (first 16 registers)
}InstanceFieldAddress;

typedef struct _16to8{
    unsigned int To: 8;
    unsigned int From: 16;
}Address16to8;

typedef struct _8to8{
    unsigned int To: 8;
    unsigned int From: 8;
}Address8to8;

typedef struct _16to16{
    unsigned int To: 16;
    unsigned int From: 16;
}Address16to16;

typedef struct{
    unsigned int Dest:8;
    unsigned int Op1Addr:8;
    unsigned int Op2Addr:8;
}BinOP;

void RunMain();
inline float ieee754Float(u4 x);
inline double ieee754Double(u8 x);
inline u4 floatieee754(float x);
inline u8 doubleieee754(double x);
void garbageCollector(Instance ** instances,u2 insts_count);
//inline void read(u1 *ptr, size_t size, size_t nmemb);
//rDex * readBytes();
//rDex * readBytes_1();

///Takes the 8 MSB and LSB as two 8 bit words and reconstructs them into the original 16 bit word. (short int)
u2 recons16(u1 msb,u1 lsb);
///Takes the 16 MSB and LSB as two 16 bit words and reconstructs them into the original 32 bit word. (int)
u4 recons32(u2 msb,u2 lsb);

u8 recons64(u4 msb,u4 lsb);

void interpreter(const u4 *params, u1 paramsNo, encoded_method *methodItem);






#endif // INTERPRETER_H_INCLUDED
