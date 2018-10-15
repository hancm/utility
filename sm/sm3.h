/************************************************************************
* FileName:
* SM3.h
* Version:
* SM3_V1.1
* Date:
* Sep 18,2016
* Description:
* This headfile provide macro defination, parameter definition
* and function declaration needed in SM3 algorithm implement
* Function List:
* 1.SM3_256 //calls SM3_init, SM3_process and SM3_done to calculate hash value
* 2.SM3_init //init the SM3 state
* 3.SM3_process //compress the the first len/64 blocks of the message
* 4.SM3_done //compress the rest message and output the hash value
* 5.SM3_compress //called by SM3_process and SM3_done, compress a single block of message
* 6.BiToW //called by SM3_compress,to calculate W from Bi
* 7.WToW1 //called by SM3_compress, calculate W' from W
* 8.CF //called by SM3_compress, to calculate CF function.
* 9.BigEndian //called by SM3_compress and SM3_done.GM/T 0004-2012 requires to use big-endian.
* //if CPU uses little-endian, BigEndian function is a necessary call to change the
* //little-endian format into big-endian format.
* 10.SM3_SelfTest //test whether the SM3 calculation is correct by comparing the hash result with the standard data
* History:
* 1. Date: Sep 18,2016
* Author: Mao Yingying, Huo Lili
* Modification: 1)add notes to all the functions
* 2)add SM3_SelfTest function
************************************************************************/
#ifndef __SM3_H__
#define __SM3_H__

typedef struct {
    unsigned int    state[8];
    unsigned int    length;
    unsigned int    curlen;
    unsigned char   buf[64];
} SM3_STATE;

void SM3_init( SM3_STATE *md );
void SM3_process( SM3_STATE * md, unsigned char buf[], int len );
void SM3_done( SM3_STATE *md, unsigned char hash[32]);
void SM3_256( unsigned char buf[], int len, unsigned char hash[32] );

#endif /* __SM3_H__ */