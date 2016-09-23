/**
 @file		md5.h
 */

#ifndef __MD5_H
#define __MD5_H

#include "psptypes.h"
/**
 @brief	MD5 context. 
 */
typedef struct {
        uint_32 state[4];    /**< state (ABCD)                            */
        uint_32 count[2];    /**< number of bits, modulo 2^64 (lsb first) */
        uint_8  buffer[64];  /**< input buffer                            */
      } md5_ctx;

extern void md5_init(md5_ctx *context);
extern void md5_update(md5_ctx *context, uint_8 *buffer, uint_32 length);
extern void md5_final(uint_8 result[16], md5_ctx *context);

#endif	// __md5_H
