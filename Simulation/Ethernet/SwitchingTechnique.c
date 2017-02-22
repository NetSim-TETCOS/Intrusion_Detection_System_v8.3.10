/************************************************************************************
 * Copyright (C) 2012     
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    Thamothara Kannan                                                      *
 * ---------------------------------------------------------------------------------*/


#include "main.h"
#include "Main_FE.h"
/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	This function performs the switching techniques,
	1.Store and Forward 
	2.Cut Through 
	3.Fragment Free
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_FastEthernet_Switching_Techniques(NetSim_EVENTDETAILS *pstruEventDetails)
{

	NetSim_PACKET *pstruTempData=NULL;
	DEVICE_VARIABLES *pstruTempDevice;
	pstruTempDevice=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->deviceVar));
	pstruTempData = pstruEventDetails->pPacket;
	
	switch(pstruTempDevice->nSwitchingTechnique)
	{
	case STORE_FORWARD:
		/*Check the switching technique is store and forward*/
	{
		/*check the data is error*/
		if(pstruTempData->pstruPhyData->nPacketErrorFlag == 1)  
		{
			/*If there is an Error drop the frame*/
			fn_NetSim_Packet_FreePacket(pstruTempData);
		}
		else
		{
			/*add the next event type*/
			pstruEventDetails->nSubEventType=SWITCH_TABLE_FORMATION;
			fnpAddEvent(pstruEventDetails);
		}
		break;
	}
	case CUT_THROUGH: /*Cut through*/
	{
		/* Once the destination MAC address receives, it will forward the frame*/
		if(pstruTempData->pstruMacData->dPacketSize < 6)		
		{				
			/*If there is an Error drop the frame*/
			fn_NetSim_Packet_FreePacket(pstruTempData);
		}
		else
		{
			/*add the next event type*/
			pstruEventDetails->nSubEventType=SWITCH_TABLE_FORMATION;
			fnpAddEvent(pstruEventDetails);
		}
		break;
	}

	case FRAGMENT_FREE: /*Fragment free*/
	{
		/* Wait upto 64 bytes, then forward the frame*/
		if(pstruTempData->pstruMacData->dPacketSize < 64)
		{
			/*If there is an Error drop the frame*/
			fn_NetSim_Packet_FreePacket(pstruTempData);
		}
		else
		{
			/*add the next event type*/
			pstruEventDetails->nSubEventType=SWITCH_TABLE_FORMATION;
			fnpAddEvent(pstruEventDetails);
		}
		break;
	}
	default : /*No other switching technique*/
		break;
	}

	return 1;
}


