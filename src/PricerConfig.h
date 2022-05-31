/*! \file  PricerConfig.h
 *  \brief Compile time configuration for Pricer.
 *
 *  Copyright (c) 2009 Michael Ellison. All Rights Reserved.
 */

#ifndef _PricerConfig_H_
#define _PricerConfig_H_

/*!
 *  If set to 1, will log the runtime to stderr.
 */
#ifndef PRICER_LOG_TIME
   #define PRICER_LOG_TIME 1
#endif

/*
 *! Numeric ids make it faster, but the spec for the ids is
 *  somewhat unknown. If they might blow a 64-bit integer,
 *  then go back to strings, maybe hash 'em into a multimap.
 */
#ifndef PRICER_USE_64BIT_IDS
   #define PRICER_USE_64BIT_IDS 1
#endif

/*
 *! Turn on to use assembler optimization for 64-bit int to ASCII.
 *  x86-64 calling conventions for Mac OSX / Linux 64-bit.
 *  (probably best to define in your platform project/makefile)
*/ 
#ifndef PRICER_64BIT_LINUXOSX_ASSEMBLER_OPT
   #define PRICER_64BIT_LINUXOSX_ASSEMBLER_OPT 0
#endif

/*
 *! Turn on to use assembler optimization for 32-bit int to ASCII.
 *  x86 is the only one supported currently (e.g. Mac/Win 32-bit)
 *  (probably best to define in your platform project/makefile)
*/ 
#ifndef PRICER_32BIT_ASSEMBLER_OPT
   #define PRICER_32BIT_ASSEMBLER_OPT 0
#endif

/*
 *! Buffering size to use when buffering is possible on a stream.
 *  128k seems to get pretty nice performance for me. May need to
 *  be tuned to type of stream / drive format / average load on stream.
 *  Generally >= 16k, but will start hurting performance if it gets too big.
*/ 
#ifndef PRICER_BUFFER_SIZE
   #define PRICER_BUFFER_SIZE        1024*128
#endif

/* 
 *! Call type for PricerProcess() and PricerGetResultString() functions.
 *  Useful if you want to call from another language
 *  or compile as a DLL or somesuch.
*/
#ifndef PRICER_CALL
   #define PRICER_CALL
#endif

#endif
