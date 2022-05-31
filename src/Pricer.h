/*! \file  Pricer.h
 *  \brief Primary include for using Pricer externally.
 *
 *  Copyright (c) 2009 Michael Ellison. All Rights Reserved.
 */

#ifndef _PricerProcess_H_
#define _PricerProcess_H_

#include "PricerConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! 
 * Result code enumeration for Pricer
 */
enum ePricerResult
{
   kPR_InvalidInStream  = -8, /*!< Error opening input stream */
   kPR_ParserError      = -7, /*!< Input data was not parsed correctly */
   kPR_ReduceOutOfRange = -6, /*!< A reduce req. had more shares than exist. */
   kPR_OrderNotFound    = -5, /*!< A reduce request specified an unknown order */
   kPR_InvalidCmdLine   = -4, /*!< Invalid command line arguments. */
   kPR_OutOfMemory      = -3, /*!< A memory allocation failed. */
   kPR_InvalidData      = -1, /*!< The input data parsed, but didn't make sense. */
   kPR_Success          =  0, /*!< Success */
   kPR_Exit             =  1  /*!< Exit code ( internal ) */
};

/*---------------------------------------------------------------------------
 *! Pricer() processes market data from inHandle.
 * 
 *  Quotes for bids and asks of targetShares are output
 *  to outAskHandle and outBidHandle when changed.
 * 
 *  \param targetShares  Target number of shares for bid/ask calculations.
 *  \param inFileNum     Input file number                          (stdin)
 *  \param outAskNum     Output handle for Ask quotes.              (stdout)
 *  \param outBidNum     Output handle for Bid quotes.              (stdout)
 *  \param outErrNum     Output handle for errors and diagnostics.  (stderr)
 *  
 *  \return int 0 on success, negative on error. 
 *  \see ePricerResult
 */
int PRICER_CALL Pricer(int targetShares,
                       int inFileNum,
                       int outAskNum,
                       int outBidNum,
                       int outErrNum);

/*---------------------------------------------------------------------------
 *! PricerGetResultString() retrieves a result code string.
 * 
 *  \param  result        Result code to translate.
 *  \return const char*   Ptr to string for result.
 * 
 *  \see ePricerResult
 */
const char* PRICER_CALL PricerGetResultString(int result);



#ifdef __cplusplus
} /* extern C */
#endif


/* _PricerProcess_H_ */
#endif 

