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
#include <stdio.h>
#include "definitions.h"
#include <stdlib.h>
#include "Bytes.h"
#include "opcodes.h"
#define LSB(X) X & 0b00001111
#define MSB(X) X >> 4
#define SIGNEX4to32(X) 0b11111111111111111111111111110000 ^ X;
#define SIGNEX8to32(X) 0b11111111111111111111111100000000 ^ X;
#define SIGNEX16to32(X) 0b11111111111111110000000000000000 ^ X;
#define x32BITMASK1s 0b11111111111111111111111111111111
#define x64BITMASK_32LSB 0b0000000000000000000000000000000011111111111111111111111111111111

extern rDex * p;;

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
inline void read(u1 *ptr, size_t size, size_t nmemb);
rDex * readBytes();

//rDex * readBytes_1();

///Takes the 8 MSB and LSB as two 8 bit words and reconstructs them into the original 16 bit word. (short int)
u2 recons16(u1 msb,u1 lsb);
///Takes the 16 MSB and LSB as two 16 bit words and reconstructs them into the original 32 bit word. (int)
u4 recons32(u2 msb,u2 lsb);

u8 recons64(u4 msb,u4 lsb);

void interpreter(const u4 *params, u1 paramsNo, encoded_method *methodItem);






#endif // INTERPRETER_H_INCLUDED
