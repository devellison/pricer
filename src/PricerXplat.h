/// \file  PricerXplat.h
/// \brief Cross-Platform definitions for Pricer.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#ifndef _PricerXplat_H_
#define _PricerXplat_H_

#include "PricerConfig.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined (_WIN32)   
   #include <io.h>
   #include <fcntl.h>
   #include <memory.h>
   #include <string.h>

   // Make sure the type definitions are correct.
   // If these fail, make a new section for the platform.
   #if (UCHAR_MAX  != 0xFF)
      #error UCHAR size wrong.
   #elif (USHRT_MAX != 0xFFFF)
      #error USHORT size wrong.
   #elif (UINT_MAX != 0xFFFFFFFF)
      #error UINT size wrong.
   #elif (ULLONG_MAX != 0xFFFFFFFFFFFFFFFF)
      #error ULONGLONG size wrong
   #endif

   // Portable type sizes.
   typedef char               PXInt8;
   typedef unsigned char      PXUInt8;
   typedef short              PXInt16;
   typedef unsigned short     PXUInt16;
   typedef int                PXInt32;
   typedef unsigned int       PXUInt32;
   typedef long long          PXInt64;
   typedef unsigned long long PXUInt64;

   // standardized standard i/o...
   #define xplat_isatty(x)        _isatty(x)
   #define xplat_fileno(x)        _fileno(x)
   #define xplat_fstat(x,y)       _fstati64(x,y)
   #define xplat_stat             _stat64
   #define xplat_write(x,y,z)     _write(x,y,z)
   #define xplat_fileno(x)        _fileno(x)
   #define xplat_readfunc(x,y,z)  _read(x,y,z)
   #define xplat_setmode(x,y)     _setmode(x,y)

   #define xplat_itoa(x,y,z)      _i64toa(x,y,z)

   #define xplat_lseek(x,y,z)     _lseeki64(x,y,z)
   #define xplat_open(x,y,z)      _open(x,y,z)
   #define xplat_close(x)         _close(x)
   #define xplat_commit(x)        _commit(x)

   #define xplat_IsRegularStream  _S_IFREG
   #define xplat_BinaryMode       _O_BINARY
   #define xplat_ReadOnly         _O_RDONLY
   #define xplat_ReadRights       _S_IREAD

   #define xplat_inline           _inline

   #define xplat_ssize_t          PXInt64

#else // nix, osx
   #include <unistd.h>
   #include <errno.h>
   #include <memory.h>
   #include <string.h>
   // Make sure the type definitions are correct.
   // If these fail, make a new section for the platform.
   #if (UCHAR_MAX  != 0xFF)
      #error UCHAR size wrong.
   #elif (USHRT_MAX != 0xFFFF)
      #error USHORT size wrong.
   #elif (UINT_MAX != 0xFFFFFFFF)
      #error UINT size wrong.
   #elif (ULLONG_MAX != 0xFFFFFFFFFFFFFFFF)
      #error ULONGLONG size wrong
   #endif

   // Portable type sizes.
   typedef char               PXInt8;
   typedef unsigned char      PXUInt8;
   typedef short              PXInt16;
   typedef unsigned short     PXUInt16;
   typedef int                PXInt32;
   typedef unsigned int       PXUInt32;
   typedef long long          PXInt64;
   typedef unsigned long long PXUInt64;
   // standardized standard i/o...
   #define xplat_isatty(x)        isatty(x)
   #define xplat_fileno(x)        fileno(x)
   #define xplat_fstat(x,y)       fstat(x,y)
   #define xplat_stat             stat
   #define xplat_write(x,y,z)     write(x,y,z)
   #define xplat_fileno(x)        fileno(x)
   #define xplat_readfunc(x,y,z)  read(x,y,z)
   #define xplat_setmode(x,y)     (0)
   #define xplat_lseek(x,y,z)     lseek(x,y,z)
   #define xplat_close(x)         close(x)

   #define xplat_itoa(x,y,z)      itoa(x,y,z)

   #define xplat_IsRegularStream  S_IFREG
   #define xplat_BinaryMode       O_BINARY
   #define xplat_ReadOnly         O_RDONLY
   #define xplat_ReadRights       S_IREAD

   #define xplat_inline           inline

   #define xplat_ssize_t          ssize_t
#endif // end nix, osx



// if EINTR is defined, assume low level i/o can be interrupted and must be retried.
#if defined(EINTR)
static xplat_inline ssize_t xplat_read(int fd,void* buf, unsigned int count)
{
   ssize_t rc;
   for (;;)
   {
      rc = xplat_readfunc(fd,buf,count);
      if (rc >= 0)
         return rc;
      if (EINTR != errno)
         return rc;
   }
}
// otherwise assume non-interrupted.
#else
   #define xplat_read(x,y,z) xplat_readfunc(x,y,z)
#endif

/// 64-bit value packed with ASCII characters.
/// May be used for ids.
typedef PXUInt64           PXPacked64;

#endif //_PricerXplat_H_
