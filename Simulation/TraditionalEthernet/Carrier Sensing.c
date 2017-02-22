/*Carrier Sensing*/

 

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

 * Author:    Karthikeyan.D                                                                *

 *                                                                                        *

 * -----------------------------------------------------------------------------------    *

 * This file contains information of Traditional Ethernet Project Work Header             *

 * data flow.                                                                             *

 *                                                                                        *

 **************************************************************************************/

 

# include "TraditionalEthernet.h"

 

# define NANOSEC (0.1*1000) //To sense the medium

 

/**************************************************************************************************

 * Variable declaration

 **************************************************************************************************/

 

      double d_NP_ProbFailureAttempt=0;         // Stores the failure attempt

      double d_NP_ProbSuccessfullAttempt=0;     // Stores the successful attempt

      double ldRandomNumber;                          // Stores the random number

/*************************************************************************************************************

 *

 * This function used to invoke the carrier sense operation,

 * If medium is idle, then process the inter frame gap period;

 * Otherwise wait till medium gets idle

 *

*************************************************************************************************************/

_declspec(dllexport) int fnCarrier_SensingPrimitive(TEPDataStructure **Ethernet)

{

            //To check the shared medium state (whether idle or not)

            if(nETH_MediumState == 0)

            {

                  //This function invokes the persistence process

                  fnPersistentCalculation(dETH_Persistance,&d_NP_ProbFailureAttempt,&d_NP_ProbSuccessfullAttempt,&ulETH_Seed3,&ulETH_Seed4 );

 

                  //To decide the data packet transmission

                  if (d_NP_ProbFailureAttempt <= d_NP_ProbSuccessfullAttempt)

                  {

                        //Increment the number of attempt of each data packet to differentiate transmission

                        nETH_AttemptNumber = nETH_AttemptNumber + 1;

 

                        //To set the shared medium state as busy

                        nETH_MediumState = 1;

 

                        //To set the shared medium busy time

                        dETH_MediumBusyTimer = dETH_GlobalTimer + dETH_SlotTimer;

 

 

                        //To set the minimum collision time to decide the jam sequence process

                        dETH_MinColTimer = dETH_GlobalTimer -(dETH_MediumBusyTimer - dETH_SlotTimer);

 

                        // Sanity check

                        if(dETH_MinColTimer <= 0)

                        {

                              //To assign the minimum collision time as zero

                              dETH_MinColTimer = 0.0l;

                        }

                        else

                        {

                              //To assign the minimum collision time

                              dETH_MinColTimer = dETH_MinColTimer;

 

                        }

 

                        //To refresh the global timer details to start the interframe gap process

                        dETH_GlobalTimer = dETH_GlobalTimer;

 

                        /*

                         * To decide the data packet process (whether carrier sensing process = 0, ifg process = 1,

                         * And defer process = 2, back off process =3)

                         */

                        nETH_MoveFlag = 1;

 

                        //To forward the moveflag information to netsim simulation engine

                        return nETH_MoveFlag;

 

                  }

                  else

                  {//To wait up to next one successfull slot time = 51.2 us

 

                        dETH_DeferTimer = dETH_GlobalTimer;

                        //to set the global timer to sense the medium

                        dETH_GlobalTimer = dETH_GlobalTimer;

 

                        /*

                         * To decide the data packet process (whether carrier sensing process = 0, ifg process = 1,

                         * And defer process = 2, back off process =3)

                         */

                        nETH_MoveFlag = 2;

 

                        //To forward the moveflag information to netsim simulation engine

                        return nETH_MoveFlag;

                  }

 

            }

            /*

             *This chek is used to allow the data transmission within one slot time and there is no jam sequence process

             */

            else if(((dETH_MediumBusyTimer) > (dETH_GlobalTimer) ) && (nETH_JAMSequenceFlag == 0))

            {

 

                  /*

                   * To check the collision start time to identify the previous data frame

                   * Collision

                   */

                  if(dETH_CollisionStartTimer == 0)

                  {

                        //To assign the the current data packet transmission as collsion start time

                        dETH_CollisionStartTimer = dETH_GlobalTimer;

 

                  }

 

                  /*

                   * This checking is used to allow the collision process with same time period

                   * 1. Within one slot time,

                   * 2. After collision start time

                   */

 

                  if((dETH_CollisionStartTimer ) == dETH_GlobalTimer)

                  {

                        //This call function invokes persistence process

                        fnPersistentCalculation(dETH_Persistance,&d_NP_ProbFailureAttempt,&d_NP_ProbSuccessfullAttempt,&ulETH_Seed3,&ulETH_Seed4 );

 

                        // To decide the data packet transmission

                        if (d_NP_ProbFailureAttempt <= d_NP_ProbSuccessfullAttempt)

                        {

 

                              //Increment the number of attempt of each data packet to differentiate transmission

                              nETH_AttemptNumber = nETH_AttemptNumber + 1;

 

                              //To set the minimum collision time to decide the jam sequence process

                              dETH_MinColTimer = dETH_GlobalTimer -(dETH_MediumBusyTimer - dETH_SlotTimer);

 

                              //Sanity check

                              if(dETH_MinColTimer <= 0)

                              {

                                    //To assign the minimum collision time as zero

                                    dETH_MinColTimer = 0.0l;

                              }

                              else

                              {

                                    //To assign the minimum collision time

                                    dETH_MinColTimer = dETH_MinColTimer;

 

                              }

 

                              //To refresh the global timer details to start the interframe gap process

                              dETH_GlobalTimer = dETH_GlobalTimer;

 

                              /*

                               * To decide the data packet process (whether carrier sensing process = 0, ifg process = 1,

                               * And defer process = 2, back off process =3)

                               */

                              nETH_MoveFlag = 1;

 

                              //To forward the moveflag information to netsim simulation engine

                              return      nETH_MoveFlag ;

 

                        }

                        else

                        {//To wait up to next one successful slot time = 51.2 us

 

                              //To set the global timer to sense the medium

                              dETH_GlobalTimer = dETH_GlobalTimer;

 

                              /*

                               * To decide the data packet process (whether carrier sensing process = 0, IFG process = 1,

                               * And defer process = 2, back off process =3)

                               */

                              nETH_MoveFlag = 2;

 

                              //To forward the moveflag information to NETSIM simulation engine

                              return nETH_MoveFlag;

                        }

 

                  }

                  else

                  {

                        /*

                         * This section is used to control the carrier sensing process up to previous packet data transmission

                         * 1. Beyond the one slot time

                         */

                        //To check the shared medium data trasmission end time to decide the carrier sensing process

                        if((dETH_MediumEndTimer) < (dETH_GlobalTimer))

                        {

 

                                    //To increment the global timer by one

                              dETH_GlobalTimer =  dETH_GlobalTimer +NANOSEC;

 

 

                              /*

                               * To decide the data packet process (whether carrier sensing process = 0, IFG process = 1,

                               * And defer process = 2, back off process =3)

                               */

                               nETH_MoveFlag = 0;

 

                              //To forward the moveflag information to netsim simulation engine

                              return nETH_MoveFlag;

                        }

                        else

                        {

 

 

                              //To assign the global timer as medium end time

                              dETH_GlobalTimer =  dETH_MediumEndTimer;

 

                              /*

                               * To decide the data packet process (whether carrier sensing process = 0, ifg process = 1,

                               * And defer process = 2, back off process =3)

                               */

                              nETH_MoveFlag = 0;

 

                              //To forward the move flag information to netsim simulation engine

                              return nETH_MoveFlag;

                        }

                  }

 

            }

            else

            {

                  /*

                   *  This section is used to control the carrier sensing process up to previous packet

                   *  Data transmission

                   */

                  //To check the shared medium data transmission end time to decide the carrier sensing process

                  if((dETH_MediumEndTimer) < (dETH_GlobalTimer))

                  {

 

                        //To check the jam sequence process (if any)

                         if( ((dETH_MediumBusyTimer) > (dETH_GlobalTimer) )&&  (dETH_JamSequenceEndTimer) >= (dETH_GlobalTimer))

                        {

 

                                           //This call function invokes persistence process

                                          fnPersistentCalculation(dETH_Persistance,&d_NP_ProbFailureAttempt,&d_NP_ProbSuccessfullAttempt,&ulETH_Seed3,&ulETH_Seed4 );

 

                                          // To decide the data packet transmission

                                          if (d_NP_ProbFailureAttempt <= d_NP_ProbSuccessfullAttempt)

                                          {

                                                //Increment the number of attempt of each data packet to differentiate transmission

                                                nETH_AttemptNumber = nETH_AttemptNumber + 1;

 

 

                                            //To assign the jam sequence end time

                                          dETH_GlobalTimer =  dETH_JamSequenceEndTimer;

 

                                          /*

                                           * To decide the data packet process (whether ifg process = 1 or carrier sensing

                                           * Process = 0)

                                           */

                                           nETH_MoveFlag = 3;

 

 

                                          }

 

                                          else

                                          {

                                                 //To assign the jam sequence end time

                                                dETH_GlobalTimer = dETH_GlobalTimer;

 

                                                /*

                                                 * To decide the data packet process (whether carrier sensing process = 0, IFG process = 1,

                                                * And defer process = 2, back off process =3)

                                                 */

                                                // To starts the defer process

                                                nETH_MoveFlag = 2;

 

                                          }

 

 

                        }else

                        {

 

                        //To increment the global timer by one

                        dETH_GlobalTimer =  dETH_GlobalTimer+NANOSEC ;

 

 

 

                         /*

                          * To decide the data packet process (whether carrier sensing process = 0, ifg process = 1,

                          * And defer process = 2, back off process =3)

                          */

                          nETH_MoveFlag = 0;

 

 

                        }

 

                        //To forward the moveflag information to netsim simulation engine

                        return nETH_MoveFlag;

                  }

                  else

                  {

                        //To assign the global timer as medium end time

                        dETH_GlobalTimer =  dETH_MediumEndTimer;

 

 

                         /*

                          * To decide the data packet process (whether carrier sensing process = 0, IFG process = 1,

                          * And defer process = 2, back off process =3)

                          */

                        nETH_MoveFlag = 0;

 

                        //To forward the moveflag information to netsim simulation engine

                        return nETH_MoveFlag;

                  }

            }

 

      return 0;

}

 

 

      /*********************************************************************************************************

       *

       * This function invoke the persistence concept to decide the data transmission.

       * Persistent denominator is need input for this function,

       * Successful attempt and failure attempt are the outputs of this function to decide data transmission

       *

       ********************************************************************************************************/

 

      int fnPersistentCalculation(double dPersistance, double *dProbFailureAttempt, double *dProbSuccessfullAttempt, unsigned long *ulSeed3,unsigned long *ulSeed4 )

      {

 

            //To assign the persistence as the denominator

            dPersistance = (1/dPersistance);

 

            /*

             * To generate the random number (between the 0 to 10000

             * (user can vary this value))to decide the data transmission

             */

			fn_NP_RandomNoCS(10000,&ldRandomNumber,ulSeed3,ulSeed4);

 

            //To asssign the random number as failure probability

            d_NP_ProbFailureAttempt = ldRandomNumber;

 

            //To calculate the successful probability

            d_NP_ProbSuccessfullAttempt = (long)(10000 * dPersistance);

 

            return 0;

      }

/*************************************************************************************************************

*

* This function is used to generate random number using linear congruential random number generator

* for persistence calculation and backoff calculation

*

**************************************************************************************************************/

 

double fn_NP_RandomNoCS(long lm,double *ldRandomNumber,unsigned long *ulSeed3,unsigned long *ulSeed4)

{

      long ldTemp=0;

      long double ldy = 0;

      *ulSeed3 = ( long)(( 50014  * (*ulSeed3))  % ( long)(2147483563));

      *ulSeed4 =  ( long)(( 50692 * (*ulSeed4)) % ( long)(2147483399));

      ldTemp = (long)((*ulSeed3 - *ulSeed4) % (long)(2147483562));

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

      return 1;

}

/************************************************************************************************************/

 

 
