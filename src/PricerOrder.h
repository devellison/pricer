/// \file  PricerOrder.h
/// \brief Object for holding an order within the PricerBook.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#ifndef _PricerOrder_H_
#define _PricerOrder_H_

#include <set>
#include <string>

#include "PricerDefs.h"

/// Holds an Order in the PricerParser / PricerBook.
struct PricerOrder
{
   /// Sort buys from high to low, since we're looking to sell.
   /// Sort sells from low to high, since we're looking to buy.
   struct PricerOrderCompare_Cmp
   {
      xplat_inline bool operator()(  const PricerOrder* cmp1, 
                                     const PricerOrder* cmp2) const
      {
         if (cmp1->fType & kPOT_Buy)
            return cmp1->fLimitPrice > cmp2->fLimitPrice;
         else
            return cmp1->fLimitPrice < cmp2->fLimitPrice;
      }
   };

   PricerOrderType    fType;
   PXInt64            fLimitPrice;
   
   // mutable so we can stay const, since these do not
   // affect sorting order.
   mutable PXInt64    fNumOwned;
   mutable PXInt64    fNumShares;
   mutable PXInt64    fReduceCount;
   
   
   // Iterator into order book.
   mutable std::multiset<PricerOrder*, PricerOrder::PricerOrderCompare_Cmp>::iterator fOrderBookIter;

   PricerOrderId      fId;
   

   PricerOrder(  ePricerOrderType orderType    = kPOT_None,
                 PXInt64          limitPrice     = 0,
                 PXInt64          numShares      = 0,
                 PXInt64          numOwned       = 0)
      : fType(orderType),
        fLimitPrice(limitPrice),
        fNumOwned(numOwned),
        fNumShares(numShares),
        fReduceCount(0),
        fOrderBookIter(),
        fId()
   {
   }

   /// Sets Reduce/Remove flag and count to reduce by.
   xplat_inline void SetReduceInfo(PricerOrderType newAction,
                                   PXInt64         reduceCount)
   {
      fReduceCount = reduceCount;
      fType = ((fType & kPOT_BuySellMask) | newAction);;
   }

};


/// Ordered set of PricerOrder objects 
/// Sorted by limitPrice depending on order type.
typedef std::multiset<PricerOrder*,PricerOrder::PricerOrderCompare_Cmp>  PricerOrderSet;
typedef PricerOrderSet::iterator PricerOrderSetIter;

/// Stream parsing operator for PricerOrders.
///
/// This is used for new stream types, or if you
/// use iostreams directly.
///
/// PricerStream's classes override for speed.
template <class InStream>
InStream& operator>>(InStream&     inStream, 
                     PricerOrder&  target)
{
   target.fType = kPOT_None;

   // tmps for inputting
   char    c;
   double  f;

   inStream >> c;
   switch(c)
   {
      case 'A':
         {
            target.fType |= kPOT_Add;

            inStream >> target.fId;
            inStream >> c;
            inStream >> f;
            inStream >> target.fNumShares;

            target.fLimitPrice = (PXInt64)(f*100.0+.5);

            switch (c)
            {
               case 'S':
                  target.fType |= kPOT_Sell;
                  break;
               case 'B':
                  target.fType |= kPOT_Buy;
                  break;
               default:
                  break;
            }
         }
         break;
      case 'R':
         {
            target.fType |= kPOT_Reduce;
            inStream >> target.fId;
            inStream >> target.fReduceCount;
         }
         break;
      default:
         break;
   }
   return inStream;
}

#endif // _PricerOrder_H_

