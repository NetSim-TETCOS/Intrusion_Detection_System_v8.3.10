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

# include <stdio.h>

# include <stdlib.h>

# include <string.h>

# include <windows.h>

# include <math.h>

# include <malloc.h>

 

#ifndef TRADITIONALETHERNET_H_

#define TRADITIONALETHERNET_H_

/*

 * This structure contains variables related to Ethernet kernel.

 */

typedef struct TEPDATASTRUCTURE {

 

      /*CARRIER SENSING*/

      int nTEPacketNumber;          // To get the current packet number

      int nTESourceID;              // To get the source id

      int nTENumberofAttempt; // To assign the number of attempts

      int nTEJamSequenceFlag;       // To assign the jam sequence flag

      double dTEPacketSize;         // To get the packet size

      double dTEGlobalTimer; // To get the global timer

      double dTESlotTime;           // To assign the slot time (51.2 us)

      double dTECollisionStartTime;// To get the collision start time

      double dTEMediumBusyTime;     // To get the medium busy time

      int nTEMediumState;           // To get the medium state

      double dTEMinColTime;         // To get the minimum collision time (time taken to know the collision effect by nearest node)

      double dTEInterFrameGap;      // To get the inter frame gap

      double dTEPersistance; // To get the persistent

      int nTEMoveFlag;              // To decide the data packet transmission flag value

      double dTEDeferTime;          // To get the defer time to count the defer count

      double dTEMediumEndTime;      // To get the medium end time

 

      /*BACK-OFF PROCESS*/

      int nTERetryCount;            // To get the retry count

      int nTERetryLimit;            // To get the retry limit

      int nTECurrentCW;             // To get the current contention window

      int nTETransmittingNodeStatus;// To get the node status ('0' for receiver and '1' for transmitter)

      double dTEBackOffTime;        // To get the back off time

 

      /*COLLISION DETECTION*/

      int nTEDeviceName;            // To get the device name

      int nTETransmittingNodes;     // To get the number of transmitting nodes

      double nTEJamSequenceByte;  // To get the number of jam sequence byte

      double dTEJamSequenceEndTime; // To get the jam sequence end time

      int nTECollisionDetectTable[200]; // To store the collision count for individual nodes packet

      int nTECollisionProcess;   // To decide the collision action whether collision check or collision detection

 

      /*RANDOM NUMBER GENERATION*/

      unsigned long ulSeed3; // First seed for random number generation function

      unsigned long ulSeed4; // Second seed for random number generation function

 

      int nBER;                     // To get the bit error rate value

      unsigned long ulErrorSeed1; // First seed for random number generation function

      unsigned long ulErrorSeed2; // Second seed for random number generation function

 

 

 

} TEPDataStructure;

/*

 * Preprocessor directives  for ease of use.

 */

// To get the current packet number

# define nETH_PacketNumber          (*Ethernet)->nTEPacketNumber

 

// To get the source id

# define nETH_SourceNodeId          (*Ethernet)->nTESourceID

 

// To assign the number of attempts

# define nETH_AttemptNumber         (*Ethernet)->nTENumberofAttempt

 

// To assign the jam sequence flag

# define nETH_JAMSequenceFlag       (*Ethernet)->nTEJamSequenceFlag

 

// To get the packet size

# define dETH_PacketSize            (*Ethernet)->dTEPacketSize

 

// To get the global timer

# define dETH_GlobalTimer           (*Ethernet)->dTEGlobalTimer

 

// To assign the slot time (51.2 us)

# define dETH_SlotTimer       (*Ethernet)->dTESlotTime

 

// To get the collision start time

# define dETH_CollisionStartTimer   (*Ethernet)->dTECollisionStartTime

 

// To get the medium busy time

# define dETH_MediumBusyTimer       (*Ethernet)->dTEMediumBusyTime

 

// To get the medium state

# define nETH_MediumState           (*Ethernet)->nTEMediumState

 

// To get the minimum collision time (time taken to know the collision effect by nearest node)

# define dETH_MinColTimer           (*Ethernet)->dTEMinColTime

 

// To get the inter frame gap

# define dETH_InterFrameGap         (*Ethernet)->dTEInterFrameGap

 

// To get the persistent

# define dETH_Persistance           (*Ethernet)->dTEPersistance

 

// To decide the data packet transmission flag value

# define nETH_MoveFlag              (*Ethernet)->nTEMoveFlag

 

// To get the defer time to count the defer count

# define dETH_DeferTimer            (*Ethernet)->dTEDeferTime

 

// To get the medium end time

# define dETH_MediumEndTimer        (*Ethernet)->dTEMediumEndTime

 

// To get the retry count

# define nETH_RetryCount            (*Ethernet)->nTERetryCount

 

// to get the retry limit

# define nETH_RetryLimit            (*Ethernet)->nTERetryLimit

 

// to get the current contention window

# define nETH_CurrentCW       (*Ethernet)->nTECurrentCW

 

// To get the node status ('0' for receiver and '1' for transmitter)

# define nETH_TransNodeStatus (*Ethernet)->nTETransmittingNodeStatus

 

// To get the back off time

# define dETH_BackOffTimer          (*Ethernet)->dTEBackOffTime

 

// To get the device name

# define nETH_DeviceName            (*Ethernet)->nTEDeviceName

 

// To get the number of transmitting nodes

# define nETH_TransmittingNodes     (*Ethernet)->nTETransmittingNodes

 

// To get the number of jam sequence byte

# define nETH_JamSequenceByte (*Ethernet)->nTEJamSequenceByte

 

// To get the jam sequence end time

# define dETH_JamSequenceEndTimer   (*Ethernet)->dTEJamSequenceEndTime

 

// To store the collision count for individual nodes packet

# define nETH_CollisionDetectTable  (*Ethernet)->nTECollisionDetectTable

 

// To decide the collision action whether collision check or collision detection

# define nETH_CollisionProcess      (*Ethernet)->nTECollisionProcess

 

// First seed for random number generation function

# define ulETH_Seed3                      (*Ethernet)->ulSeed3

 

// Second seed for random number generation function

# define ulETH_Seed4                      (*Ethernet)->ulSeed4

 

// To get the bit error rate value

# define nETH_BER                         (*Ethernet)->nBER

 

//First seed for random number generation function

# define ulETH_ErrorSeed1                 (*Ethernet)->ulErrorSeed1

 

// Second seed for random number generation function

# define ulETH_ErrorSeed2                 (*Ethernet)->ulErrorSeed2

 

 

/***********************************************************************

 *

 * function declaration

 *

 ***********************************************************************/

 

//Declare the persistence calculation

int fnPersistentCalculation(double , double* , double* , unsigned long* ,unsigned long*  );

 

//Declare the random number generation function

double fn_NP_RandomNoBOA(long ,double *,unsigned long *,unsigned long *);
double fn_NP_RandomNoCS(long ,double *,unsigned long *,unsigned long *);

 

//To declare the collision check function

int fnDataTransmission(int* ,int ,int *);

 

//To declare the collision detection function

int fnCollision(int* ,int ,int *);

 

//Declare the expand contention window function

int fnExpandContentionWindow(int*,int*,int*,int);

 

 

//Declare the error random function

double fnErrorRandomNo(double ,unsigned long *,unsigned long *);

 

 

 

 

#endif /* TRADITIONALETHERNET_H_ */

 
