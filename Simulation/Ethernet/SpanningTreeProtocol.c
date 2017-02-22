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


#define _CRT_SECURE_NO_DEPRECATE

#include "main.h"
#include "Main_FE.h"



char* fn_NetSim_STP__GetLowestMacAddress(int nDeviceId);
/**  This Function will perform the spanning tree calculation by calling				
	Transmit Config BPDU when the Event type is MAC_OUT_EVENT and					
  Receive Config BPDU when the event type is MAC_IN_EVENT.						
*/
int fn_NetSim_FastEthernet_Spanning_Tree_Protocol(NetSim_EVENTDETAILS *pstruEventDetails)
{
	/*Check the event type*/
	switch(pstruEventDetails->nEventType)
	{
	/*MacOut-21*/
	case MAC_OUT_EVENT:

		/*Call the function to initialise the switch*/
			fn_NetSim_FastEthernet_Initialisation();
		break;

		/*MacIn-22*/
	case MAC_IN_EVENT:

		/*This function recieves the CONFIG_BPDU and performs the root switch updation */
		fn_NetSim_FastEthernet_ReceiveConfig_BPDU(pstruEventDetails->nInterfaceId,pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol,\
			pstruEventDetails);
		break;
	}
	return 1;

}

/**
 This function is used to initialize the bridge Config BPDU
 */
int fn_NetSim_FastEthernet_Initialisation()
{
	NETSIM_ID nPort_No=0;
	NETSIM_ID nDevice_id=0;
	int nLength=0;
	char *pszPriority;
	char *pszMacAdd;

	nDevice_id=pstruEventDetails->nDeviceId;
	
	/* Initialise the switch variables */
		if(NETWORK->ppstruDeviceList[nDevice_id-1]->nDeviceType == SWITCH)
			{
				pstruBridge_Var=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[nDevice_id-1]->deviceVar))->pstruBridgeVar;

				pstruBridge_Var->nRoot_Path_cost=ZERO;
				pstruBridge_Var->nPort_id= ZERO;
				pstruBridge_Var->nTx_DeviceId=nDevice_id;
				pszPriority= _strdup(((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[nDevice_id-1]->deviceVar))->pszSwitch_Priority);
				nLength=(int)strlen(pszPriority);
				pszPriority=fn_NetSim_FastEthernet_Addstring(pszPriority,"0",(5-nLength),1);

				if(!pstruBridge_Var->pszBridge_id)
				{
					pszMacAdd= fn_NetSim_STP__GetLowestMacAddress(nDevice_id); //_strdup(NETWORK->ppstruDeviceList[nDevice_id-1]->ppstruInterfaceList[0]->pstruMACLayer->szMacAddress);
					pstruBridge_Var->pszBridge_id =fnpAllocateMemory(1,(strlen(pszPriority)+strlen(pszMacAdd)+4));			
					nLength=(int)strlen(pszMacAdd);	
				
					fn_NetSim_Utilities_ConcatString(3,pstruBridge_Var->pszBridge_id,pszPriority,pszMacAdd);
					fnpFreeMemory(pszMacAdd);
				}
				pstruBridge_Var->pszDesignated_root= _strdup(pstruBridge_Var->pszBridge_id);
				pstruBridge_Var->pszTx_Bridge_id = _strdup(pstruBridge_Var->pszBridge_id);

				for(nPort_No=1;nPort_No <=((int) NETWORK->ppstruDeviceList[nDevice_id-1]->nNumOfInterface); nPort_No++)
				{
					if(NETWORK->ppstruDeviceList[nDevice_id-1]->ppstruInterfaceList[nPort_No-1]->pstruMACLayer != NULL)
					fn_NetSim_FastEthernet_Initialize_port(nPort_No);
				}
				/*Call the function to generate configuration bpdu for spanning tree protocol*/
				fn_NetSim_FastEthernet_Config_bpdu_generation();
				free(pszPriority);
		}

	return 0;			
}
/**
 This function is to initialise the switch ports.
*/
int fn_NetSim_FastEthernet_Initialize_port(NETSIM_ID nPort_No)
{
	NETSIM_ID nDevice_id=0;
	NETSIM_ID nConn_Deviceid=0;
	NETSIM_ID nConn_Portid=0;
	NETSIM_ID nLink_id=0;
	PORT_DATA *pstruPortVar;
	nDevice_id=pstruEventDetails->nDeviceId;

	pstruPortVar=((PORT_DATA *)(NETWORK->ppstruDeviceList[nDevice_id-1]->ppstruInterfaceList[nPort_No-1]->pstruMACLayer->MacVar));
			
		nLink_id=fn_NetSim_Stack_GetConnectedDevice(nDevice_id,nPort_No,&nConn_Deviceid,&nConn_Portid);
		if(!nLink_id)
		{
			//Port is not connected.
			pstruPortVar->nPort_state = BLOCKING;
			return 0;
		}
		if(NETWORK->ppstruDeviceList[nConn_Deviceid-1]->nDeviceType == SWITCH)
			pstruPortVar->nPort_state = BLOCKING;
		
		else if(NETWORK->ppstruDeviceList[nConn_Deviceid-1]->nDeviceType != SWITCH)
			pstruPortVar->nPort_state = FORWARDING;
	
		return 0;
}
/** This function is to get lowest MAC address */
char* fn_NetSim_STP__GetLowestMacAddress(int nDeviceId)
{
	char* pszMacAddress;
	NETSIM_ID nLoop;
	pszMacAddress = _strdup(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[0]->pstruMACLayer->szMacAddress);
	for(nLoop=0;nLoop<NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;nLoop++)
	{
		if(strcmp(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->pstruMACLayer->szMacAddress,pszMacAddress)<0)
		{
			fnpFreeMemory(pszMacAddress);
			pszMacAddress = _strdup(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->pstruMACLayer->szMacAddress);
		}
	}
	return pszMacAddress;
}


