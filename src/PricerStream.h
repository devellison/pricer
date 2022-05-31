/// \file  PricerStream.h
/// \brief Buffering I/O streams for PricerOrder objects.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#ifndef _PricerStream_H_
#define _PricerStream_H_

#include "PricerXplat.h"
#include <string>

struct PricerOrder;

/// \class PricerInputStream
/// \brief Input stream implementation for PricerOrder objects.
///
/// NOTE: THIS IS NOT AN ISTREAM REPLACEMENT.
/// It has a few similar methods to make the template work.
/// But it's pretty specifically for reading PricerOrders from stdin.
class PricerInputStream
{
   public:
      /// This class treats everything <= '.' as a delimiter.
      static const char kMaxDelimiter = '.';

      /// fileNum is the std C fileno (e.g. fileno(stdin))
      PricerInputStream(int fileNum, int maxBufferSize);
      ~PricerInputStream();
      
      PricerInputStream& operator >>(char& c);
      PricerInputStream& operator >>(std::string& val);
      PricerInputStream& operator >>(PXUInt32&      val);
      PricerInputStream& operator >>(PXInt64&       val);
      PricerInputStream& operator >>(PXPacked64&    val);
      PricerInputStream& operator >>(PricerOrder&  order);

      // iostream-like status functions
      bool eof()     { return fAtEndOfFile;}
      bool good()    { return (!(fInvalidParse | fStreamError));}
      bool fail()    { return fInvalidParse; }
      bool bad()     { return fStreamError; }
      void ignore(int maxBytes, char endChar);
      void clear();
      

      /// Sets a stream into binary mode, checks if it
      /// is seekable, and gets current start/end positions.
      ///
      /// \param fileNum   std C file number
      /// \param startPos  On success, start position of the stream.
      /// \param endPos    On success, current end position of the stream.
      ///
      /// \return bool true if all succeeds and the stream may be buffered.
      static bool SetupStreamForBinaryBuffer(int      fileNum,
                                             PXInt64& startPos,
                                             PXInt64& endPos);

      /// Loads the maximum amount of data it can into our buffer.
      /// Returns true on success.
      bool RefreshCache();


   protected:
      /// Get the next character. set error flags.
      /// xplat_read takes care of EINTR if present on the system.
      xplat_inline bool GetNextChar(char& val);

      /// Unbuffered version
      xplat_inline bool GetUnbufferedChar(char& val);

      /// Mainly for Win32 - if you set a stdin handle
      /// to binary twice, it throws an exception.
      static bool sHasSetStdIn;

      int      fFileNum;      ///< stdio fileno for stream.
      
      // error states
      bool     fInvalidParse;
      bool     fStreamError;
      bool     fAtEndOfFile;

      bool     fCanBuffer;    ///< True if it's a regular file and 
                              ///< we can fully buffer it.

      char*    fBuffer;       ///< Input buffer
      char*    fBufferPos;    ///< Position within input buffer
      char*    fEndBufferPos; ///< End of loaded data
      int      fBufferSize;   ///< Size of buffer (not loaded data)


      PXInt64  fStartPos;     ///< Current starting file position in buffer.
      PXInt64  fCurEndPos;    ///< Current known ending file position
   private:
      /// Not implemented.
      PricerInputStream(const PricerInputStream&)
      : fFileNum(0),fInvalidParse(false),fStreamError(false),fAtEndOfFile(false),fCanBuffer(false),
        fBuffer(0),fBufferPos(0),fEndBufferPos(0),fBufferSize(0),fStartPos(0),fCurEndPos(0)
      {throw;}
      
      /// Not implemented.
      PricerInputStream& operator=(const PricerInputStream&)
         {throw;return *this;}
};


/// \class PricerOutputStream
/// \brief Output stream implementation for PricerOrder objects.
///
/// NOTE: THIS IS NOT AN OSTREAM REPLACEMENT.
/// Specialized for writing PricerOrders to stdout.
class PricerOutputStream
{
   public:
      PricerOutputStream(int fileNum, int maxBufSize);
      ~PricerOutputStream();

      PricerOutputStream& operator <<(char     c);
      PricerOutputStream& operator <<(PXUInt32 val);
      PricerOutputStream& operator <<(PXUInt64  val);
      PricerOutputStream& operator <<(const char* str);

      void Flush();
   protected:
      bool                 fCanBuffer;
      bool                 fWriteError;
      int                  fFileNum;
      
      char*                fBuffer;
      char*                fBufPtr;
      char*                fBufEndPtr;

      int                  fBufferSize;
   private:
      /// Not implemented.
      PricerOutputStream(const PricerOutputStream&)
      : fCanBuffer(false),fWriteError(false),fFileNum(0),fBuffer(0),fBufPtr(0),fBufEndPtr(0),fBufferSize(0)
      {throw;}
      
      /// Not implemented.
      PricerOutputStream& operator=(const PricerOutputStream&)
      {throw; return *this;}
};

#endif // _PricerStream_H_

