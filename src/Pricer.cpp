/// \file  Pricer.cpp
/// \brief Implementation of primary interface for Pricer.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#include "Pricer.h"
#include "PricerParser.h"
#include "PricerStream.h"

int PRICER_CALL Pricer(   int targetShares,
                          int inFileNum,
                          int outAskNum,
                          int outBidNum,
                          int outErrNum)
{
   int result;
   PricerParser<PricerInputStream,PricerOutputStream> parser;
   PricerOutputStream errStream(outErrNum,PRICER_BUFFER_SIZE);

   // run debug test w/o args if defined.
   if ((0 >= targetShares)  || (targetShares == INT_MAX))
   {
      result = kPR_InvalidCmdLine;
      parser.PricerOutputError(result,errStream);
      return result;
   }

   // Wrap handles in (possibly) buffered streams.
   PricerInputStream inputStream(inFileNum,PRICER_BUFFER_SIZE);
   PricerOutputStream askStream( outAskNum,PRICER_BUFFER_SIZE);

   // Usually we'll have bid/ask going to the same stream.
   // If so, use the same object - otherwise we buffer separately.
   PricerOutputStream *bidStream = &askStream;
   if (outAskNum != outBidNum)
      bidStream = new PricerOutputStream(outBidNum,PRICER_BUFFER_SIZE);

   result = parser.ProcessStream(targetShares,
                                 inputStream,
                                 askStream,
                                 *bidStream,
                                 errStream);

   // clean up if bid/ask actually were going to different streams.
   if (outAskNum != outBidNum)
      delete bidStream;

   return result;
}

const char* PRICER_CALL PricerGetResultString(int result)
{
   const char* msg;
   switch(result)
   {
      case kPR_InvalidInStream:  msg="Input stream invalid.\n";           break;
      case kPR_ParserError:      msg="Parser error.\n";                   break;
      case kPR_ReduceOutOfRange: msg="Not enough shares for reduce.\n";   break;
      case kPR_OrderNotFound:    msg="No matching Add found.\n";          break;
      case kPR_InvalidCmdLine:   msg="Usage: pricer [targetNumShares]\n"; break;
      case kPR_OutOfMemory:      msg="Error allocating memory.\n";        break;
      case kPR_InvalidData:      msg="Invalid input data.\n";             break;
      case kPR_Success:          msg="Success.\n";                        break;
      case kPR_Exit:             msg="Exit.\n";                           break;

      default:
         if (PRICERERR(result))
            msg = "Unknown error.\n";
         else
            msg = "Unknown status.\n";
         break;
   }
   return msg;
}

