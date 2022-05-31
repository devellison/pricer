/// \file  PricerDefs.h
/// \brief Internal Defitions for Pricer
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.

#ifndef _PricerDefs_H_
#define _PricerDefs_H_

#include "PricerConfig.h"
#include "PricerXplat.h"

/// Typedef for ePricerOrderType values where we'll be
/// performing bit operations.
typedef PXInt32 PricerOrderType;

/// Type of message - mostly used
/// in PricerOrder objects to indicate
/// If they are a buy, a sell, or a reduce request.
enum ePricerOrderType
{
   kPOT_None        = 0x00,
   kPOT_Buy         = 0x01,
   kPOT_Sell        = 0x02,
   kPOT_Add         = 0x10,
   kPOT_Reduce      = 0x20,
   kPOT_Remove      = 0x40,
   kPOT_AddBuy      = 0x11, // kPOT_Add | kPOT_Buy
   kPOT_AddSell     = 0x12, // kPOT_Add | kPOT_Sell,
   kPOT_ReduceBuy   = 0x21, // kPOT_Reduce | kPOT_Buy,
   kPOT_ReduceSell  = 0x22, // kPOT_Reduce | kPOT_Sell,
   kPOT_RemoveBuy   = 0x41, // kPOT_Remove | kPOT_Buy,
   kPOT_RemoveSell  = 0x42, // kPOT_Remove | kPOT_Sell,
   kPOT_BuySellMask = 0x03,
   kPOT_ActionMask  = 0x70,
   kPOT_Exit        = 0x80
};

/// Retrieve the char used for messages of specified type
xplat_inline char PricerGetMsgChar(ePricerOrderType msgType)
{
   if (msgType & kPOT_Buy)
      return 'B';
   return 'S';
}

/// Returns true on non-error codes.
/// \see ePricerResult
#define PRICEROK(x)      ((x)>=0)

/// Returns true on error codes. 
/// \see ePricerResult
#define PRICERERR(x)     ((x)<0)

// If we're using numeric ids, define them.
// Otherwise define string comparison usage.
#if (PRICER_USE_64BIT_IDS > 0)
   
   typedef PXPacked64 PricerOrderId;
   
   /// PricerOrderId* comparator for std::map searching by id.
   struct PricerOrderId_cmp
   {
      bool operator()(const PricerOrderId* cmp1, 
                      const PricerOrderId* cmp2) const
      {
         return ( (*cmp1) < (*cmp2) );
      }
   };

#else

   typedef std::string PricerOrderId;
   
   /// String ptr comparator for std:map searching by id.
   struct PricerOrderId_cmp
   {
      bool operator()(const PricerOrderId* cmp1, 
                      const PricerOrderId* cmp2) const
      {
         return ( cmp1->compare(*cmp2) < 0);
      }
   };
#endif


#endif // _PricerDefs_H_
