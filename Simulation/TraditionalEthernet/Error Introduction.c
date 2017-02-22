/*Error Introduction*/

 

/**************************************************************************************

 *                      Traditional Ethernet                                              *

 **************************************************************************************

 * Copyright (C) 2010                                                                     *

 * TETCOS, Bangalore. India                                                               *

 *                                                                                        *

 * Tetcos owns the intellectual property rights in the Product and its content.           *

 * The copying, redistribution, reselling or publication of any or all of the             *

 * Product or its content without express prior written consent of Tetcos is        *

 * prohibited. Ownership and / or any other right relating to the software and all *

 * intellectual property rights therein shall remain at all times with Tetcos.            *

 *                                                                                        *

 * Author:    Karthikeyan.D                                                               *

 *                                                                                        *

 * -----------------------------------------------------------------------------------    *

 * This file contains information of Traditional Ethernet Project Work Header             *

 * data flow.                                                                             *

 *                                                                                        *

 **************************************************************************************/

# include "TraditionalEthernet.h"

 

 

/******************************************************************************************

 * This function is used to invoke the decide error functionalities of the shared medium

 ******************************************************************************************/

 

_declspec(dllexport) int fnDecideErrorPrimitive(TEPDataStructure ** Ethernet)

{

      double fEpsilon = 0.0;        // Stores the epsilon value

      double nPacketBits = 0;       // Stores the packet size in bit format

      double dPacketnoerror=0.0 ; // Stores the packet no error

      double dPEP=0.0;              // Stores the packet error probability

      double ldRandomNumber=0.0;    // Stores the random number

      double ldBER=0.0;             // Stores the bit error rate

      double fRand=0.0;             // Stores the error decision

 

/*

 * This condition checks the bit error rate

 */

 

 

      switch (nETH_BER)

      {

      case 1:

            ldBER = 10; // Mean bit error rate is 10^-1

            break;

      case 2:

            ldBER = 100; // Mean bit error rate is 10^-2

            break;

      case 3:

            ldBER = 1000; // Mean bit error rate is 10^-3

            break;

      case 4:

            ldBER = 10000; // Mean bit error rate is 10^-4

            break;

      case 5:

            ldBER = 100000; // Mean bit error rate is 10^-5

            break;

      case 6:

            ldBER = 1000000; // Mean bit error rate is 10^-6

            break;

      case 7:

            ldBER = 10000000; // Mean bit error rate is 10^-7

            break;

      case 8:

            ldBER = 100000000; // Mean bit error rate is 10^-8

            break;

      case 9:

            ldBER = 1000000000; // Mean bit error rate is 10^-9

            break;

      default:

            return 0;

 

      }

 

      // To calculate the epsilon value

 

      /* For example, nBER IS 5, fEpsilon = (1/ldBER)

       * Actual BIT ERROR RATE = 1/100000 IS ASSIGNED TO fEpsilon.

       */

      fEpsilon = (double)(1/(1.0*ldBER));

 

      // To convert the packet size in bit format

      nPacketBits = dETH_PacketSize * 8;

 

      // To calculate the packet with no error

      dPacketnoerror = (double)(pow((1-fEpsilon),nPacketBits));

 

      // To calculate packet error probability

      dPEP = (double)(1-dPacketnoerror);

 

      // Call function to generate the random number

      ldRandomNumber = fnErrorRandomNo(ldBER,&(ulETH_ErrorSeed1),&(ulETH_ErrorSeed2));

 

      // To assign the random number

      fRand = (double) ldRandomNumber;

 

      // To decide the error packet introduction

      if (fRand <= dPEP)

      {

            //This packet treated as a error packet

            return 1;

      }

      else //if(fRand>dPEP)

      {

            // This packet treated as a successful packet

            return 0;

      }

 

 

 

 

}

/*************************************************************************************************/

 

 

/**************************************************************************************************

 * This function used to calculate the random number for the decide error function

 **************************************************************************************************/

double fnErrorRandomNo(double ldBER,unsigned long *g_ulnErrorSeed1,unsigned long *g_ulnErrorSeed2)

{

      long lx = 0;

      double ldy = 0;

      double ltemp;

 

      *g_ulnErrorSeed1 = (unsigned long)((40014 * (*g_ulnErrorSeed1))  % (unsigned long)(2147483563));

      *g_ulnErrorSeed2 =  (unsigned long)((40692 * (*g_ulnErrorSeed2)) % (unsigned long)(2147483399));

 

      lx = (long)(((*g_ulnErrorSeed1 )- (*g_ulnErrorSeed2)) % (long)(2147483562));

 

      if (lx != 0)

      {

            ldy = (double)((double)(lx) / (double)(2147483563));

      }

      else

      {

            ldy = (double)((double)(2147483562) / (double)(2147483563));

      }

      ltemp = ldy;

 

      return ltemp;

}

/***************************************************************************************************/

 
