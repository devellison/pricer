/// \file  PricerStream.cpp
/// \brief Buffering I/O streams for PricerOrder objects.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#include <stdlib.h>
#include "PricerStream.h"
#include "PricerOrder.h"
#include "PricerOpt.h"

/// Some libraries get all nasty if you set the mode
/// on stdin more than once. This is just a guard for that.
bool PricerInputStream::sHasSetStdIn = false;

PricerInputStream::PricerInputStream(int fileNum, int maxBufferSize)
: fFileNum(fileNum), 
  fInvalidParse(false), 
  fStreamError(false), 
  fAtEndOfFile(false),
  fCanBuffer(false), 
  fBuffer(0), 
  fBufferPos(0),
  fEndBufferPos(0),
  fBufferSize(0),
  fStartPos(0), 
  fCurEndPos(0)
{
   if (maxBufferSize > 0)
   {
      fCanBuffer = SetupStreamForBinaryBuffer(fFileNum,
                                              fStartPos, 
                                              fCurEndPos);

      fBuffer       = new char[maxBufferSize];
      fBufferPos    = fBuffer;
      fEndBufferPos = fBufferPos;
      fBufferSize   = maxBufferSize;
   }
}

PricerInputStream::~PricerInputStream()
{
   delete fBuffer;
}

xplat_inline bool PricerInputStream::GetNextChar(char& val)
{
   // Try to pull from our buffer first.
   if (!fCanBuffer)
      return GetUnbufferedChar(val);

   if (fBufferPos == fEndBufferPos)
   {
      // Failed, hit the disk.
      if (!RefreshCache())
         return false;
   }

   val = *fBufferPos;
   ++fBufferPos;

   return true;
}
xplat_inline bool PricerInputStream::GetUnbufferedChar(char& val)
{
   xplat_ssize_t res;
   
   if (1 == (res = xplat_read(fFileNum,&val,1)))
      return true;

   fAtEndOfFile = (errno == EOF) || (res == 0);
   fStreamError = !fAtEndOfFile;
   return false;
}

PricerInputStream& PricerInputStream::operator >>(char& c)
{
   // read until we hit a non-delimiter, or end of file.
   do
   {
      GetNextChar(c);
   } while ((c < kMaxDelimiter) && (!fAtEndOfFile));
   
   return *this;
}


PricerInputStream& PricerInputStream::operator >>(PXUInt32& val)
{
   char c;
   do
   {
      if (!GetNextChar(c))
         return *this;

   } while ((c <= '.') && (!fAtEndOfFile));

   if ((c  < '0') || (c > '9'))
   {
      fInvalidParse = true;
      return *this;
   }
   
   // got first byte, now read direct. At most 9 more bytes may be read for a uint32
   val = (c - '0');
   int maxChars = 9;
   while (maxChars > 0)
   {
      if (!GetNextChar(c))
         return *this;

      // delimiter. return.
      if (c <= '.')
         return *this;

      // undelimited different type. bail.
      if ((c < '0') || (c > '9'))
      {
         fInvalidParse = true;
         return *this;
      }

      // shift val over a digi and add new value
      val = val*10 + (c - '0');
      --maxChars;
   }
   return *this;
}

PricerInputStream& PricerInputStream::operator >>(PXInt64& val)
{
   char c;
   do
   {
      if (!GetNextChar(c))
         return *this;

   } while ((c <= '.') && (!fAtEndOfFile));

   if ((c  < '0') || (c > '9'))
   {
      fInvalidParse = true;
      return *this;
   }
   
   // got first byte, now read direct.
   val = (c - '0');

   // 18446744073709551615
   int maxChars = 19;

   while (maxChars > 0)
   {
      if (!GetNextChar(c))
         return *this;

      // delimiter. return.
      if (c <= '.')
         return *this;

      // undelimited different type. bail.
      if ((c < '0') || (c > '9'))
      {
         fInvalidParse = true;
         return *this;
      }

      val *= 10;
      val += (c - '0');

      --maxChars;
   }
   return *this;
}

PricerInputStream& PricerInputStream::operator >>(std::string& val)
{
   bool gotChar;
   char c;

   val.clear();
   // get the first character, skipping whitespace.
   do
   {
      gotChar = GetNextChar(c);
   } while ((gotChar) && (c < '.'));
   
   if (!gotChar)
      return *this;
   
   // now retrieve the rest.
   val.push_back(c);
   for (;;)
   {
      if (false == (gotChar = GetNextChar(c)))
         return *this;
      
      if (c <= kMaxDelimiter)
         return *this;
      
      val.push_back(c);
   }
}

PricerInputStream& PricerInputStream::operator >>(PXPacked64& val)
{
   bool gotChar;
   char c;

   val = 0;

   // get the first character, skipping whitespace.
   do
   {
      gotChar = GetNextChar(c);
   } while ((gotChar) && (c < '.'));
   
   if (!gotChar)
      return *this;
   
   // For the packed integers (e.g. Order ID) we just
   // pack the ASCII bytes directly into the 64-bit
   // integer as individual bytes.
   val += c;
   
   for (;;)
   {
      if (false == (gotChar = GetNextChar(c)))
         return *this;
      if (c <= kMaxDelimiter)
         return *this;
      val = val<<8;
      val += c;
   }
}

PricerInputStream& PricerInputStream::operator >>(PricerOrder& order)
{
   order.fType = kPOT_None;

   char    c = 0;
   PXInt64 dollars = 0;
   PXInt64 cents = 0;

   (*this) >> c;
   switch(c)
   {
      case 'A':
         {
            order.fType |= kPOT_Add;

            (*this) >> order.fId;
            (*this) >> c;
            (*this) >> dollars;
            (*this) >> cents;
            (*this) >> order.fNumShares;

            // We store the price as a fixed-point to simplify/speed
            // the code and avoid rounding weirdness.
            order.fLimitPrice = dollars*100+cents;

            switch (c)
            {
               case 'S':
                  order.fType |= kPOT_Sell;
                  break;
               case 'B':
                  order.fType |= kPOT_Buy;
                  break;
               default:
                  break;
            }
         }
         break;
      case 'R':
         {
            order.fType |= kPOT_Reduce;
            (*this) >> order.fId;
            (*this) >> order.fReduceCount;
         }
         break;
      default:
         break;
   }
   return (*this);
}


void PricerInputStream::ignore(int maxBytes, char endChar)
{
   // Read until we fail to get a character or we get
   // to the requested end character or EOF.
   char val;
   while (GetNextChar(val) && 
          (val != endChar) && 
          (0 != maxBytes--));
}

void PricerInputStream::clear()
{
   fStreamError = 0;
   fInvalidParse = 0;
}


bool PricerInputStream::SetupStreamForBinaryBuffer(  int      fileNum,
                                                     PXInt64& startPos,
                                                     PXInt64& endPos)
{
   struct xplat_stat fileInfo;
   startPos = 0;
   endPos   = -1;

   // Try to set binary mode on stream.
   // If it fails, bail - avoid CR/LF calc for now on win32.
   if (fileNum == xplat_fileno(stdin))
   {
      if (!sHasSetStdIn)
      {
         if (xplat_setmode(fileNum,xplat_BinaryMode) < 0)
         {
            return false;
         }
         sHasSetStdIn = true;
      }
   }
   else if (xplat_setmode(fileNum,xplat_BinaryMode) < 0)
   {
      return false;
   }

   // Get the file information and check if it's seekable/bufferable.
   if (xplat_fstat(fileNum,&fileInfo) != 0)
      return false;
   
   if (0 == (fileInfo.st_mode & xplat_IsRegularStream))
      return false;

   if ( -1 == (startPos = xplat_lseek(fileNum,0,SEEK_CUR)))
      return false;

   if (-1 == (endPos = xplat_lseek(fileNum,0,SEEK_END)))
      return false;

   if ( -1 == (xplat_lseek(fileNum,-(endPos - startPos),SEEK_CUR)))
      return false;

   return true;
}

bool PricerInputStream::RefreshCache()
{
   if (!fCanBuffer)
      return false;

   PXInt64 amountLeft = fCurEndPos - fStartPos;
   int readSize;

   if (amountLeft > fBufferSize)
   {
      readSize = fBufferSize;
   }
   else
   {
      readSize = (int)(fCurEndPos - fStartPos);
   }
   
   xplat_ssize_t res = xplat_read(fFileNum, fBuffer, readSize);
   if (res <= 0)
   {
      fAtEndOfFile = (errno == EOF) || (res == 0);
      fStreamError = !fAtEndOfFile;
      return false;
   }

   fStartPos+=res;

   fBufferPos = fBuffer;
   fEndBufferPos = fBufferPos + res;
   return true;
}


//------------------------------------------------------------------
// Output stream
PricerOutputStream::PricerOutputStream(int fileNum, int maxBufSize)
: fCanBuffer(false), 
  fWriteError(false),
  fFileNum(fileNum), 
  fBuffer(0),
  fBufPtr(0),
  fBufEndPtr(0),
  fBufferSize(maxBufSize)
{
   if (fBufferSize <= 1)
      fBufferSize = 1;

   if (xplat_setmode(fileNum,xplat_BinaryMode) >= 0)
   {
      if (fBufferSize > 1)
      {
         struct xplat_stat fileInfo;
         if (0 == xplat_fstat(fileNum,&fileInfo))
         {     
            if ((fileInfo.st_mode & xplat_IsRegularStream) != 0)
               fCanBuffer = true;
         }
      }
   }

   if (!fCanBuffer)
      maxBufSize = 256; // just line buffer.

   fBuffer    = new char[maxBufSize];
   fBuffer[0] = 0;
   fBufPtr    = fBuffer;
   fBufEndPtr = fBufPtr+maxBufSize;
}
PricerOutputStream::~PricerOutputStream()
{
   Flush();
}

PricerOutputStream& PricerOutputStream::operator <<(char c)
{
   *fBufPtr = c;
   fBufPtr++;
   
   if (fBufPtr == fBufEndPtr)
   {
      Flush();
   }
   else if (!fCanBuffer)
   {
      if (c == '\n')
         Flush();
   }
   
   return *this;
}

void PricerOutputStream::Flush()
{
   int   bufSize  = (int)(fBufPtr - fBuffer);
   char* startBuf = fBuffer;

   if (!bufSize)
      return;
   
   for(;;)
   {
      xplat_ssize_t res = xplat_write(fFileNum,startBuf,bufSize);
      if (res == bufSize)
      {
         fBufPtr = fBuffer;
         break;
      }

      if (res <= 0)
      {
         fWriteError = true;
         break;
      }

      // Hmmm... partial write.. accept what's been written and retry.
      startBuf += res;
      bufSize -= (int)res;
   }
}

PricerOutputStream& PricerOutputStream::operator <<(PXUInt32 val)
{
   char tmpBuf[32];
   PricerUInt32ToA(val,tmpBuf);
   return (*this)<<tmpBuf;
}

PricerOutputStream& PricerOutputStream::operator <<(PXUInt64  val)
{
   char tmpBuf[32]; // 21 needed.
   PricerUInt64ToA(val,tmpBuf);
   return (*this)<<tmpBuf;
}

PricerOutputStream& PricerOutputStream::operator <<(const char* str)
{
   int length = (int)strlen(str);
   
   // If we've got the buffer space for it,
   // copy it directly.
   if (length + fBufPtr < fBufEndPtr)
   {
      memcpy(fBufPtr,str,length);
      fBufPtr += length;
      return *this;
   }

   while (*str != 0)
   {
      (*this) << *str;
      str++;
   }

   return *this;
}

