#include "main.h"
#include "Main_FE.h"

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

/**
	This function forms the switch table by dynamically
*/
int fn_NetSim_FastEthernet_Switch_Table_Formation(NetSim_EVENTDETAILS *pstruEventDetails)
{

	/* NetSim Temp packet declaration*/
	NetSim_PACKET *pstruTempPacket;
	/* Switch table  declaration for type casting*/
	SWITCH_TABLE *pstruSwitchTable;
	NETSIM_ID nSourceId,interfaceId;
	/* Assign the packet*/

	pstruTempPacket=pstruEventDetails->pPacket;
	nSourceId = fn_NetSim_Stack_GetDeviceId_asMac(pstruTempPacket->pstruMacData->szSourceMac,&interfaceId);

	/* Type cast the switch table*/
	pstruSwitchTable=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->deviceVar))->ppstruSwitchTable[nSourceId-1];
	
	/*check the src node already has a entry in the switch table*/
	if(pstruSwitchTable->nNode_Id ==nSourceId )
	{
		/* Already Entry is there in the switch table don't add entry again*/
	}
	/*Entry is not there,then add the entry into the switch table*/
	else 
	{
		/*Add the source node*/
		pstruSwitchTable->nNode_Id =nSourceId;
		/*Add the source Mac address*/
		pstruSwitchTable->pszPort_MAC_Address=\
			_strdup(pstruTempPacket->pstruMacData->szSourceMac);
		/*Add the Port no to which the source is connected*/
		pstruSwitchTable->nOut_Port=pstruEventDetails->nInterfaceId;
	}
	/* Add the next event type*/
	pstruEventDetails->nSubEventType=SWITCH_FRAME_FORWARDING;
	fnpAddEvent(pstruEventDetails);

	return 1;
}



