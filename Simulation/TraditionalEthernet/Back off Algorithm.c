/*Back off Algorithm*/

 

 

 

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

 * ------------------------------------------------------------------------------------*

 * This file contains information of Traditional Ethernet Project Work Header             *

 * data flow.                                                                             *

 *                                                                                        *

 **************************************************************************************/

 

# include "TraditionalEthernet.h"

 

 

/*************************************************************************************************************

 *

 * This function invokes the back-off process primitive

 *

 * Increment retry count for every collision attempts

 * Check whether retry count exceeds the retry limit

 * If yes, check retry count value is equal to 10 [truncated binary exponential back-off algorithm]

 *          1. If yes, assign the back-time attempt as retry count

 *          2. Otherwise, assign the back-time attempt as 10 for truncated binary exponential back-off algorithm

 *          3. Calculate the current window size      [current cw = 2^retry count - 1]

 *          4. Generate the random number (between 0 to current contention window) for back off time calculation

 *          5. Calculate the back-off time

 *

*************************************************************************************************************/

_declspec(dllexport) int fnBackoff_AlgorithmPrimitive(TEPDataStructure **Ethernet)

{

      int ntempCount = 0;     // To calculate the current window size

 

      double ldRandomNumber;  // Stores the random number

 

 

      //To reset the shared medium busy time

      dETH_MediumBusyTimer = 0.0f;

 

      //To reset the collision start time

      dETH_CollisionStartTimer =0.0f;

 

      //To reset the shared medium state

      nETH_MediumState = 0;

 

      ntempCount = nETH_MoveFlag;

 

      

      //To reset the jam sequence flag [jam process end]

      nETH_JAMSequenceFlag = 0;

 

      //To reset the nodes status (transmitting node as 1)

      nETH_TransNodeStatus = 0;

      

      // Call the expand contention window

      fnExpandContentionWindow(&nETH_RetryCount,&nETH_RetryLimit,&nETH_CurrentCW,ntempCount);

 

      //To check the dropped packet attempts

      if(nETH_CurrentCW != -1 && ntempCount == 0)

      {

 

            //To generate the random number for back off time calculation

		  fn_NP_RandomNoBOA(nETH_CurrentCW,&ldRandomNumber,&ulETH_Seed3,&ulETH_Seed4);

 

            //To calculate the back-off time [SLOTTIME = 51.2 us]

            dETH_BackOffTimer  = (ldRandomNumber) * dETH_SlotTimer;

      }

      else

      {

            //To discard the data frame

            dETH_BackOffTimer  = -1;

      }

 

return 0;

 

}

/************************************************************************************************************/

 

int fnExpandContentionWindow(int *nTERetryCount,int *nTERetryLimit,int *nTECurrentCW,int ntempCount)

{

 

      //To assign the back off attempt to calculate the current contention window

            int nBackoffAttempt=0;

 

    // after collision and jam sequence process,

            if(ntempCount == 0)

            {

                  //To increment the retry count

                  *nTERetryCount = *nTERetryCount + 1;

 

                  //To check if the retransmission count =< 16 as per standard

 

                  if(*nTERetryCount <= *nTERetryLimit)

                  {

                        //To set the max contention window size as 1023

                        if(*nTERetryCount <= 10)

                        {

                              //To assign the back-time attempt as retry count

                              nBackoffAttempt = *nTERetryCount;

                        }

                        else

                        {

                              //To assign the back-time attempt as 10 for truncated binary backoff algorithm

                              nBackoffAttempt = 10;

                        }

 

                        //To calculate the current contention window size     [currentcw = 2^retry count - 1]

                        *nTECurrentCW = (int)(pow(2,(nBackoffAttempt))-1);

                  }

 

                  else

                  {

                        //To identify the dropped packet process

                        *nTECurrentCW  = -1;

                  }

            }

            else

            {

                  //To reset the retry count

                  *nTERetryCount = 0;

 

                  // To reset the current cw

                  *nTECurrentCW = 0;

 

 

            }

 

      return 0;

 

}

 

/*************************************************************************************************************

*

* This function is used to generate random number using linear congruential random number generator

* For persistence calculation and backoff calculation

*

**************************************************************************************************************/

 

double fn_NP_RandomNoBOA(long lm,double *ldRandomNumber,unsigned long *pulSeed3,unsigned long *pulSeed4)

{

      long ldTemp=0;

      long double ldy = 0;

      *pulSeed3 = ( long)(( 50014  * (*pulSeed3))  % ( long)(2147483563));

      *pulSeed4 =  ( long)(( 50692 * (*pulSeed4)) % ( long)(2147483399));

      ldTemp = (long)((*pulSeed3 - *pulSeed4) % (long)(2147483562));

      if (ldTemp != 0)

      {

            ldy = (long double)((long double)(ldTemp) / (long double)(2147483563));

      }

      else

      {

            ldy = (long double)((long double)(2147483562) / (long double)(2147483563));

      }

        ldy = ldy * 100000000;

      ldTemp = (long)ldy;

 

      *ldRandomNumber = (long )(ldTemp % (long)(lm + 1));

 

      return 0;

}

/************************************************************************************************************/

 
