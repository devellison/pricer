/// \file  PricerMain.cpp
/// \brief main() for Pricer.
//
// Copyright (c) 2009 Michael Ellison. All Rights Reserved.
#include <time.h>
#include "PricerXplat.h"
#include "Pricer.h"

/// main() for Pricer.
/// One integer argument is required - targetShares.
int main(int argc, char** argv)
{
   int targetShares = 0;
   
   if (argc >= 2)
      targetShares = atoi(argv[1]);

#if (PRICER_LOG_TIME > 0)
   clock_t start = clock();
#endif

   int res = Pricer(targetShares,
                 xplat_fileno(stdin),
                 xplat_fileno(stdout),
                 xplat_fileno(stdout),
                 xplat_fileno(stderr));

#if (PRICER_LOG_TIME > 0)
   float seconds = (float)(clock() - start)/(float)CLOCKS_PER_SEC;
   char buf[64];
   sprintf(buf,"Runtime (seconds): %f\n",seconds);
   xplat_write(xplat_fileno(stderr),buf,(unsigned int)strlen(buf));
#endif

   return res;
}
