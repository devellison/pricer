/// \file  PricerBook.h
/// \brief PricerBook tracks the state of the order book
///
/// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
///
#ifndef _PricerBook_H_
#define _PricerBook_H_

#include "PricerConfig.h"
#include "PricerOrder.h"

/// \class PricerBook
/// \brief PricerBook tracks the state of the current order book.
///
/// PricerOrder objects are not copied and are owned by the caller.
/// PricerBook just keeps pointers.
///
/// PricerOrder objects are sorted from low to high price (Sell/Bid) or
/// from high to low price (Buy/Ask), depending on the type.
///
/// PricerBook may be instantiated using ostreams or any other relatively
/// compatible object type. \see PricerOutputStream
///
template<class OutStream>
class PricerBook
{
   public:
      PricerBook(ePricerOrderType buyOrSell)
      : fOrderType(buyOrSell),
        fTargetShares(0),
        fOrders(),
        fBookValid(false),
        fTotalPrice(0),
        fNumShares(0),
        fLastUsedOrder(fOrders.end()),
        fOutStream(0),
        fErrStream(0)
      {
      }

      ~PricerBook()
      {
      }

      /// Initialize should be called before other use.
      void Init( PXInt64            targetShares, 
                 OutStream&         outStream,
                 OutStream&         errStream)
      {
         Reset();
         fOutStream        = &outStream;
         fErrStream        = &errStream;
         fTargetShares     = targetShares;
      }

      /// Resets the book values, but not the
      /// streams.
      void Reset()
      {
         fOrders.clear();
         fTargetShares  = 0;
         fBookValid     = false;
         fTotalPrice    = 0;
         fNumShares     = 0;
         fLastUsedOrder = fOrders.end();
      }

      /// Adds an order to the book and updates the
      /// current state.
      ePricerResult AddOrder(PricerOrder* order, PXUInt32 timeStamp)
      {  
         ePricerResult result = kPR_Success;

         PricerOrderSetIter newOrderIter = fOrders.insert(order);

         // Save a copy of the iterator for reduce/removal
         order->fOrderBookIter = newOrderIter;

         // temp variables tracking state for change detection
         PXInt64  curPrice      = fTotalPrice;
         bool     curValid      = fBookValid;

         PricerOrderSetIter nextIter(newOrderIter);
         ++nextIter;   

         PXInt64 needed      = fTargetShares - fNumShares;
         PXInt64 newShares   = order->fNumShares;

         if (nextIter == fOrders.end())
         {
            // At the end of the list. Only take some if 
            // we need 'em.
            if (needed > 0)
            {         
               if (newShares >= needed)
               {
                  // Got enough to fill the order now.
                  order->fNumOwned += needed;
                  curPrice         += needed * order->fLimitPrice;
                  fNumShares       += needed;
                  curValid          = true;
               }
               else
               {
                  // Not enough... take 'em all.
                  order->fNumOwned += newShares;
                  curPrice         += newShares * order->fLimitPrice;
                  fNumShares       += newShares;
               }
               fLastUsedOrder = newOrderIter;
            }
         }
         else if ((*nextIter)->fNumOwned != 0)
         {
            // If this one doesn't yet fill the order, 
            // or fills it exactly, no traversal needed.
            // Just take 'em.
            if (newShares <= needed)
            {
               order->fNumOwned += newShares;
               curPrice         += newShares * order->fLimitPrice;
               fNumShares       += newShares;
               curValid          = (fNumShares == fTargetShares);
            }
            else
            {
               // Overfill the order then reduce.
               order->fNumOwned += newShares;
               fNumShares       += newShares;   
               curPrice         += newShares * order->fLimitPrice;

               PXInt64 overFlow = fNumShares - fTargetShares;
               
               while (fNumShares != fTargetShares)
               {            
                  PricerOrder* curLast = *fLastUsedOrder;
                  if (curLast->fNumOwned < overFlow )
                  {
                     PXInt64 numOwned   = curLast->fNumOwned;
                     overFlow          -= numOwned;
                     fNumShares        -= numOwned;
                     curPrice          -= numOwned * 
                                          curLast->fLimitPrice;
                     curLast->fNumOwned = 0;
                  }
                  else
                  {
                     fNumShares         -= overFlow;
                     curPrice           -= overFlow * curLast->fLimitPrice;
                     curLast->fNumOwned -= overFlow;
                     curValid = true;

                     if (curLast->fNumOwned != 0)
                        break;

                     overFlow = 0;
                  }
                  --fLastUsedOrder;
               }
            }
         }

         bool changed = 
            ( (curPrice != fTotalPrice) && (curValid));
         
         // Store state
         fTotalPrice   = curPrice;
         fBookValid    = curValid;

         // Output new if changed and valid
         if (changed)
         {
            OutputNewState( timeStamp);
         }

         return result;
      }

      /// Removes an existing order from the book
      /// and updates the current state.
      ePricerResult RemoveOrder(PricerOrder* order, PXUInt32 timeStamp) 
      {
         PXInt64  curPrice     = fTotalPrice;
         bool     curValid     = fBookValid;

         ePricerResult      result = kPR_Success;

         // Retrieve stored iterator
         PricerOrderSetIter iter(order->fOrderBookIter);

         PXInt64 numRemoved  = order->fNumOwned;
         if (numRemoved > 0)
         {
            // We own more than is left... find replacements if we can.
            fNumShares   -= numRemoved;
            curPrice     -= numRemoved * order->fLimitPrice;
            curValid      = false;

            // Update it
            if (fLastUsedOrder == iter)
            {
               if (fLastUsedOrder != fOrders.begin())
               {
                  --fLastUsedOrder;
                  fOrders.erase(iter);
               }
               else
               {
                  fOrders.erase(iter);
                  fLastUsedOrder = fOrders.begin();
               }
            }
            else
            {
               fOrders.erase(iter);
            }

            // Find replacements if we can.
            PricerOrderSetIter scanIter(fLastUsedOrder);
            curValid = FillOrder(scanIter,curPrice);
         }
         else
         {
            fOrders.erase(iter);
         }

         bool changed   = ( (curValid != fBookValid) ||
                            (curValid && (curPrice != fTotalPrice)) );

         fBookValid     = curValid;
         fTotalPrice    = curPrice;

         if (changed)
         {
            OutputNewState( timeStamp);
         }

         return result;
      }

      ePricerResult ReduceOrder(PricerOrder* order, PXUInt32 timeStamp) 
      {
         PXInt64  curPrice     = fTotalPrice;
         bool     curValid     = fBookValid;

         ePricerResult      result = kPR_Success;
         
         // Retrieve the stored iterator (to skip re-finding it)
         PricerOrderSetIter iter(order->fOrderBookIter);

         order->fNumShares   -= order->fReduceCount;
         PXInt64 numRemoved   = order->fNumOwned - order->fNumShares;

         if (order->fNumOwned > order->fNumShares)
         {
            // We own more than is left... find replacements if we can.
            fNumShares -= numRemoved;
            curPrice   -= numRemoved * order->fLimitPrice;
            curValid    = false;

            order->fNumOwned -= numRemoved;

            // Find replacements if we can.
            PricerOrderSetIter scanIter(fLastUsedOrder);
            curValid = FillOrder(scanIter,curPrice);
         }

         bool changed = ( (curValid != fBookValid) ||
                          (curValid && (curPrice != fTotalPrice)) );

         fBookValid    = curValid;
         fTotalPrice   = curPrice;

         if (changed)
         {
            OutputNewState( timeStamp);
         }

         return result;
      }


      /// Scans for orders to try to fill the target shares.
      /// Returns number of shares held and total price.
      bool FillOrder(PricerOrderSetIter& scanIter,
                     PXInt64&            curPrice)
      {
         bool curValid = false;
         PXInt64 sharesLeft = fTargetShares - fNumShares;

         while (scanIter != fOrders.end())
         {
            PricerOrder* curOrder = *scanIter;

            PXInt64 scanPrice  = curOrder->fLimitPrice;
          
            PXInt64 scanShares = (curOrder->fNumShares - 
                                  curOrder->fNumOwned);

            // skip to the next one if this one's allocated.
            if (scanShares == 0)
            {
               ++scanIter;
               continue;
            }

            if (sharesLeft <= scanShares)
            {
               curPrice            += scanPrice*sharesLeft;
               fNumShares          += sharesLeft;
               curOrder->fNumOwned += sharesLeft;
               
               sharesLeft      = 0;
               curValid        = true;
               fLastUsedOrder  = scanIter;
               break;
            }
            else
            {
               sharesLeft          -= scanShares;
               curPrice            += scanPrice * scanShares;
               curOrder->fNumOwned += scanShares;
               fNumShares          += scanShares;
               fLastUsedOrder       = scanIter;
            }

            ++scanIter;
         }
         
         return curValid;
      }

      /// Outputs new Bid/Ask state to the output stream.
      void OutputNewState( PXUInt32         timeStamp)
      {
         // inverted from input (e.g. they buy, we're selling)
         char orderChar = (fOrderType & kPOT_Buy)?'S':'B';

         if (fBookValid)
         {
            PXUInt32 cents = (PXUInt32)(fTotalPrice%100);
            (*fOutStream) << timeStamp      << ' '
                          << orderChar      << ' '
                          << (PXUInt64)fTotalPrice/100
                          << ((cents<10)?".0":".")
                          << cents
                          << '\n';
         }
         else
         {
            (*fOutStream) << timeStamp << ' '
                          << orderChar << ' '
                          << "NA\n";
         }
      }
   protected:
      ePricerOrderType             fOrderType;     ///< kPOT_Buy | kPOT_Sell
      PXInt64                      fTargetShares;
      PricerOrderSet               fOrders;

      bool                         fBookValid;
      PXInt64                      fTotalPrice;
      PXInt64                      fNumShares;
      PricerOrderSetIter           fLastUsedOrder;

      OutStream*                   fOutStream;
      OutStream*                   fErrStream;

   private:
      /// Copy not implemented.
      PricerBook(const PricerBook&)
      : fOrderType(kPOT_None),fTargetShares(0),fOrders(),fBookValid(false),
        fTotalPrice(0),fNumShares(0),fLastUsedOrder(fOrders.end()),fOutStream(0),fErrStream(0)
      {throw;}
      
      /// Assignment not implemented.
      PricerBook& operator=(const PricerBook&)
      {throw; return *this;}

};

#endif
