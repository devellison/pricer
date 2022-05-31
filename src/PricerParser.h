/// \file  PricerParser.h
/// \brief Parser implementation for incoming market data
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#ifndef _PricerParser_H_
#define _PricerParser_H_

#include <map>
#include "Pricer.h"
#include "PricerBook.h"

/// \class PricerParser
/// \brief Parser object to read a market log and process it.
///
/// Reads data from an input stream into PricerOrder objects,
/// then dispatches them to PricerBook handlers.
///
/// ProcessStream() is the main external interface.
///
template<class InStream, class OutStream>
class PricerParser
{
   public:
      PricerParser()
      : fTimeStamp(0),
        fBuyToAskHandler(kPOT_Buy),
        fSellToBidHandler(kPOT_Sell),
        fIdOrderMap()
      {
      }
      
      ~PricerParser()
      {
         Reset();
      }

      /// Resets the books and clears out all orders.
      void Reset()
      {
         fSellToBidHandler.Reset();
         fBuyToAskHandler.Reset();
         
         PricerIdOrderIter iter = fIdOrderMap.begin();
         while (iter != fIdOrderMap.end())
         {
            delete iter->second;
            ++iter;
         }   
         fIdOrderMap.clear();
      }

      /// Processes the incoming stream and sends
      /// parsed messages to Dispatch(), which
      /// calls the Bid/Ask handlers.
      ///
      /// \param targetShares Number of shares to track the bid/ask price for.
      /// \param inStream     Input stream object
      /// \param outBidStream Output stream to receive Bids
      /// \param outAskStream Output stream to receive Asks
      /// \param errStream    Output stream to receive errors and diagnostics
      ///
      /// \return ePricerResult 0 on success, < 0 on error.
      ePricerResult ProcessStream(  PXInt64     targetShares,
                                    InStream&   inStream,
                                    OutStream&  outBidStream,
                                    OutStream&  outAskStream,
                                    OutStream&  errStream)
      {
         fSellToBidHandler.Init( targetShares, 
                                 outBidStream, 
                                 errStream);

         fBuyToAskHandler.Init ( targetShares, 
                                 outAskStream, 
                                 errStream);

         ePricerResult result = kPR_Success;

         // This is our read buffer. It is re-used on reduces/failures.
         // If we add it to our map, the map becomes the owner and 
         // we create a new one.
         PricerOrder *readOrder = new PricerOrder();

         for (;;)
         {
            inStream >> fTimeStamp;
            if (!inStream.fail())
               inStream >> (*readOrder);
            
            if (inStream.bad()  || 
                inStream.fail() || 
                (kPOT_None == (readOrder->fType)))
            {
               if (inStream.eof())
                  break;

               inStream.clear();
               // skip to next valid line.
               inStream.ignore(512,'\n');

               // only spew one error until we get out of an error condition.
               if (result != kPR_ParserError)
               {
                  result = kPR_ParserError;
                  PricerOutputError(kPR_ParserError, errStream);
               }
               continue;
            }
            
            switch ((readOrder->fType))
            {
               case kPOT_AddBuy:
               case kPOT_AddSell:
                  {
                     // Saving it to the map - allocate a new read buffer.
                     fIdOrderMap.insert(PricerIdOrderPair(&readOrder->fId,
                                                           readOrder));
                     result = Dispatch(readOrder);

                     readOrder = new PricerOrder();
                  }
                  break;
               case kPOT_Reduce:
                  {
                     // Find the order by ID, determine if it's fully reduced,
                     // then notify PricerBook and remove if needed.
                     PricerIdOrderIter iter = fIdOrderMap.find(&readOrder->fId);
                     if (iter == fIdOrderMap.end())
                        return kPR_InvalidData;

                     PricerOrder* reduceOrder = iter->second;

                     if (reduceOrder->fNumShares <= readOrder->fReduceCount)
                     {
                        if (reduceOrder->fNumShares < readOrder->fReduceCount)
                           PricerOutputError(kPR_ReduceOutOfRange, errStream);

                        reduceOrder->SetReduceInfo(kPOT_Remove,
                                                   readOrder->fReduceCount);

                        result = Dispatch(reduceOrder);

                        fIdOrderMap.erase(iter);

                        delete reduceOrder;
                     }
                     else
                     {
                        reduceOrder->SetReduceInfo(kPOT_Reduce,
                                                   readOrder->fReduceCount);
                        
                        result = Dispatch(reduceOrder);
                     }
                  }
                  break;
               default:
               case kPOT_Exit:
                  result = kPR_InvalidData;
                  break;
            }
            if (PRICERERR(result))
               PricerOutputError(result,errStream);
         }
         delete readOrder;
         return result;
      }

      /// Dispatches a parsed order to the appropriate handler.
      ePricerResult Dispatch(PricerOrder* order)
      {
         ePricerResult result;
         switch (order->fType)
         {
            case kPOT_AddBuy:
               result = fBuyToAskHandler.AddOrder(order,fTimeStamp);
               break;
            case kPOT_AddSell:
               result = fSellToBidHandler.AddOrder(order,fTimeStamp);
               break;
            case kPOT_ReduceBuy:
               result = fBuyToAskHandler.ReduceOrder(order,fTimeStamp);
               break;
            case kPOT_ReduceSell:
               result = fSellToBidHandler.ReduceOrder(order,fTimeStamp);
               break;
            case kPOT_RemoveBuy:
               result = fBuyToAskHandler.RemoveOrder(order,fTimeStamp);
               break;
            case kPOT_RemoveSell:
               result = fSellToBidHandler.RemoveOrder(order,fTimeStamp);
               break;
            case kPOT_Exit:
               result = kPR_Exit;
               break;
            default:
               result = kPR_InvalidData;
               break;
         }   
         return result;
      }

      /// Dump an error or status string to errStream
      static void PricerOutputError(int result, OutStream& errStream)
      {
         errStream << PricerGetResultString(result);
      }

   protected:
      PXUInt32                                    fTimeStamp;

      /// Processes Buy entries and outputs Asks
      PricerBook< OutStream >    fBuyToAskHandler;

      /// Processes Sell entries and outputs Bids
      PricerBook< OutStream >    fSellToBidHandler;

   private:
      typedef std::map< PricerOrderId*, 
                        PricerOrder*,
                        PricerOrderId_cmp>        PricerIdOrderMap;
      
      typedef std::pair<PricerOrderId*,
                        PricerOrder*>             PricerIdOrderPair;

      typedef PricerIdOrderMap::iterator          PricerIdOrderIter;
      
      /// Internal map of all PricerOrders (buy and sell) referenced
      /// by the id.  This is the "master list" of all PricerOrder
      /// objects.  When deleting, objects must be removed from
      /// PricerBooks first, then the map, then deleted.
      PricerIdOrderMap fIdOrderMap;
};


#endif // _PricerParser_H_
