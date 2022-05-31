/// \file  PricerOpt.h
/// \brief Header file for x86/x64 assembler optimized routines.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#ifndef _PricerOpt_H_
#define _PricerOpt_H_

#include "PricerXplat.h"

// Here we define two main functions via macros that will be available
// on all platforms:
// void PricerUInt32ToA(PXUInt32 val, char* buffer)
// void PricerUInt64ToA(PXUInt64 val, char* buffer)
//
// If assembler optimizations are available, they will use those.
// Otherwise they fall back to sprintf(), as itoa is nonstandard.

extern "C"
{
#if (PRICER_32BIT_ASSEMBLER_OPT > 0)
   /// Convert a 32-bit unsigned int to ASCIIZ.
   /// max number is 10 bytes: 4294967295, so buffer should be 11+
   void PricerUI32ToA(PXUInt32 val, char* buffer);
#endif 

#if (PRICER_64BIT_LINUXOSX_ASSEMBLER_OPT > 0)
   /// Converts a 64-bit unsigned int to ASCIIZ
   /// max number is 20 bytes, so buffer should be 21+.
   void PricerUI64ToA(PXUInt64 val, char* buffer);
#endif
}


// Define PricerUInt32ToA() depending on platform 
// and available optimizations.
#if (PRICER_64BIT_LINUXOSX_ASSEMBLER_OPT > 0)
   // 64-bit Mac/Linux optimization
   #define PricerUInt32ToA(val,tmpBuf) PricerUI64ToA((val),(tmpBuf))
#elif (PRICER_32BIT_ASSEMBLER_OPT > 0)
   // 32-bit processor optimization
   #define PricerUInt32ToA(val,tmpBuf) PricerUI32ToA(val,tmpBuf)
#else
   // No optimization
   #define PricerUInt32ToA(val,tmpBuf) sprintf((tmpBuf),"%u",(val))
#endif

// Define PricerPXUInt64ToA() as approatiate function depending
// on platform and available optimizations.
//
// If it's really a 32-bit value, fall back to the previously
// defined PricerUInt32ToA() in case it's optimized.
#if (PRICER_64BIT_LINUXOSX_ASSEMBLER_OPT > 0)
   // 64-bit Mac/Linux optimization
   #define PricerUInt64ToA(val,tmpBuf) PricerUI64ToA((val),(tmpBuf))
#else
   // No 64-bit optimization. Try 32-bit if appropriate.
   #define PricerUInt64ToA(val,tmpBuf)              \
      if (((val) & 0xFFFFFFFF) == (val))            \
      {                                             \
         PricerUInt32ToA((PXUInt32)(val),(tmpBuf)); \
      }                                             \
      else                                          \
      {                                             \
         sprintf((tmpBuf),"%lld",(val));            \
      }
#endif




#endif // _PricerOpt_H_
