/*Collision Detection*/

 

 

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

 * -----------------------------------------------------------------------------------*

 * This file contains information of Traditional Ethernet Project Work Header             *

 * data flow.                                                                             *

 *                                                                                        *

 **************************************************************************************/

 

# include "TraditionalEthernet.h"

 

 

 

/**********************************************************************

* This function is used to decide the collision status

*

**********************************************************************/

_declspec(dllexport) int fnCollision_DetectionPrimitive(TEPDataStructure ** Ethernet)

{

      int nTEDeviceName;            // To get the device id

      int nTECollisionCount=0;      // To get the collision count

 

      // To assign the device name

      nTEDeviceName = nETH_DeviceName;

 

      // To assign the collision count

      nTECollisionCount = nETH_CollisionDetectTable[nTEDeviceName];

 

      // To check the collision process

      if(nETH_CollisionProcess == 1)

 

      {

            //Call the collision check function

            fnDataTransmission(&nETH_TransmittingNodes,nTECollisionCount,&nETH_MoveFlag);

 

      }

      else if(nETH_CollisionProcess ==2)

      {

            //Call the collision detection function

            fnCollision(&nETH_TransmittingNodes,nTECollisionCount,&nETH_MoveFlag);

      }

 

 

return 0;

}

/*********************************************************************/

/**********************************************************************

 * This function invokes the collision check process,

 * If single data transmission arrives, then it must sense the shared medium for next one slot time (51.2 us)*

 **********************************************************************/

int fnDataTransmission(int* nTETransmittingNodes,int nTECollisionCount,int *dTEMoveFlag)

{

      /*

       * To check the number of transmitting nodes

       * And collision detection table state

       * If ntecollisioncount == 0, it represents no collision

       * Attempts before this attempt.

       *

       */

      if((*nTETransmittingNodes == 1) &&(nTECollisionCount == 0 ))

      {

            //To initiate the data transmission process

            *dTEMoveFlag = 1;

 

      }

      else if (nTECollisionCount > 0)

      {

            // To start the jam sequence process

            *dTEMoveFlag = 0;

 

      }

      else if((*nTETransmittingNodes > 1) &&(nTECollisionCount == 0 ))

      {

            //To start the jam sequence process

            *dTEMoveFlag = 2;

 

      }

      else

      {     //Sanity check

            //MessageBox(NULL,"nCollision CHECK else part ","NullSimulator",0);

      }

 

      return 0;

}

/**********************************************************************

 This function invokes the process after the collision to generate the jam sequence

 **********************************************************************/

int fnCollision(int* nTETransmittingNodes,int nTECollisionCount,int *dTEMoveFlag)

{

 

      if((*nTETransmittingNodes > 1) &&(nTECollisionCount >= 0 ))

            {

 

            // To start the jam sequence process

                  *dTEMoveFlag = 2;

 

            }

            else if (nTECollisionCount > 0)

            {

                  //To start the jam sequence process

                  *dTEMoveFlag = 0;

 

            }

            else

            {

                  //Sanity check

                  // MessageBox(NULL,"nCollision Detection else part ","NullSimulator",0);

            }

 

      return 0;

}

/*********************************************************************/

 

 

 
