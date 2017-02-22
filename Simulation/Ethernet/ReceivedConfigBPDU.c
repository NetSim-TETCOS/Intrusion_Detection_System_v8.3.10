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


/**
	This function will update rootid status from the received CONFIG_BPDU frame.
	if Received CONFIG_BPDU RootId is less than its present rootid,				
	 it will replace the rootid with received CONFIG_BPDU root id,					
	 else it will discard the recived CONFIG_BPDU frame								
*/

int fn_NetSim_FastEthernet_ReceiveConfig_BPDU(int nPort_No,FRAME_MAC *pstruConfig, NetSim_EVENTDETAILS * pstruEventDetails)
{
	
	NETSIM_ID nDeviceId=0;
	NETSIM_ID nPortId=0;
	NETSIM_ID nConnected_DeviceId=0; 
	NETSIM_ID nConnected_PortId=0;
	NETSIM_ID nLink_Id=0;
	int nPath_Cost=0;
	NETSIM_ID nTx_DeviceId=0;			/* To store the Transmitted device Id*/
	NETSIM_ID nPort_id=0;				/* To store the Transmitted device portId*/
	char *pszBridge_id=NULL;	/* To store the Transmitted device switchId*/
	char *pszTx_SwitchRootId=NULL;/*To store the switchRootId*/
	/* Device variable declaration for Type casting*/
	BRIDGE_DATA *pstruBridgeVar=NULL;
	
	PORT_DATA *pstruPort=NULL;
	NetSim_PACKET *pstruPacket;
	/*Type casting device variable*/
	pstruBridgeVar=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->deviceVar))->pstruBridgeVar;
	
	pstruPacket= pstruEventDetails->pPacket;
	pstruPort=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->pstruMACLayer->MacVar;
	/*Get the received CONFIG_BPDU path cost*/
	nPath_Cost=pstruConfig->pstruCBPDU->nRoot_Path_cost;
	/* Get the Tx Switch ID*/
	pszBridge_id= _strdup(pstruConfig->pstruCBPDU->pszBridge_id);
	/* Get the Tx Device ID*/
	nTx_DeviceId=pstruConfig->pstruCBPDU->nTx_DeviceId;
	/* Get the Tx Port ID*/
	nPort_id =pstruConfig->pstruCBPDU->nPort_id;
	/*Copy the root switch id from CONFIG_BPDU frame*/
	pszTx_SwitchRootId = _strdup(pstruConfig->pstruCBPDU->pszRoot_id);

	/*Compare the root switch id of received CONFIG_BPDU and the received switch rootid*/
	/*If its greater,then update the CONFIG_BPDU of the switch with least root switch id*/

#ifdef SPANNING_TREE_LOGFILE
	fprintf(stdout,"Sw-%d portNo-%d Receives the CBPDU as Root Switch ID of\t<%s>\tWith the cost of %d\n",pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,pszTx_SwitchRootId,nPath_Cost);
#endif

	if(strcmp(pstruBridgeVar->pszDesignated_root, pszTx_SwitchRootId) > 0)
	{
#ifdef SPANNING_TREE_LOGFILE
		fprintf(stdout,"Received CPDU Switch ID <%s> is Less than Switch-%d ID  <%s>\t\tReceived Switch Cost is %d\n",pszTx_SwitchRootId,pstruEventDetails->nDeviceId,pstruBridgeVar->pszDesignated_root,pstruBridgeVar->nRoot_Path_cost);
#endif
		 /*Forward and block the port state*/
		nDeviceId=pstruBridgeVar->nTx_DeviceId;
		nPortId=pstruBridgeVar->nPort_id;
		/*Block the port whose information is present in the CONFIG_BPDU of the switch*/
		if(nPortId!= ZERO)
		{
#ifdef SPANNING_TREE_LOGFILE
			fprintf(stdout,"Block the port whose information is present in the CPDU of the switch\n");
#endif
			/* Set the port state as blocked*/
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nPortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;
			/* Get the connected device id of this blocked port*/
			nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nPortId,&nConnected_DeviceId,&nConnected_PortId);
			/*Set the port state of connected port as blocked*/
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;
		}
		/* Update the CONFIG_BPDU of the switch with the info from the CONFIG_BPDU received*/
		/*Update the root switch id*/
		strcpy(pstruBridgeVar->pszDesignated_root,pszTx_SwitchRootId);
		/*Update the path cost*/
		pstruBridgeVar->nRoot_Path_cost=nPath_Cost;
		//Update the transmiiter switch no
		pstruBridgeVar->nTx_DeviceId=nTx_DeviceId;
		//Update the transmitter port id
		pstruBridgeVar->nPort_id=nPort_id;
		
		strcpy(pstruBridgeVar->pszTx_Bridge_id,pszBridge_id);
		strcpy(pstruPort->pszDesignated_bridge,pszBridge_id);
		((PORT_DATA *)(NETWORK->ppstruDeviceList[nTx_DeviceId-1]->\
		ppstruInterfaceList[nPort_id-1]->pstruMACLayer->MacVar))->nPort_state=FORWARDING;
		nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nTx_DeviceId,nPort_id,\
		&nConnected_DeviceId,&nConnected_PortId);

		/*Set the port state as forward*/
		((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
		ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=FORWARDING;

#ifdef SPANNING_TREE_LOGFILE
		fprintf(stdout,"Switch-%d  broadcast this updated CPDU, Through the port which are all connected with this switches with cost of %d\n",pstruEventDetails->nDeviceId,pstruBridgeVar->nRoot_Path_cost);
#endif

		/*Call this function to broadcast this updated CONFIG_BPDU to other switches*/
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
		pstruEventDetails->pPacket=NULL;
		pstruPacket = NULL;
		fn_NetSim_FastEthernet_Config_bpdu_generation();
		
	}

	/*If both are equal,then compare the path cost value*/
	else if(strcmp(pstruBridgeVar->pszDesignated_root,pszTx_SwitchRootId)==0)
	{
#ifdef SPANNING_TREE_LOGFILE
		fprintf(stdout,"Received CBPDU Switch ID <%s> and Switch ID-%d  <%s> are same\n",pszTx_SwitchRootId,pstruEventDetails->nDeviceId,pstruBridgeVar->pszDesignated_root);
#endif
		/*If the path cost is grater ,then the existing path to the root switch is longer,then update with the new path(shortest)*/
		if(pstruBridgeVar->nRoot_Path_cost > nPath_Cost)
		{
#ifdef SPANNING_TREE_LOGFILE
			fprintf(stdout,"Received CBPDU Switch path cost <%d> is less than Switch path cast <%d>\n",nPath_Cost,pstruBridgeVar->nRoot_Path_cost);
#endif
			/*Block the port whose information is present in the CONFIG_BPDU of the switch*/
			nDeviceId=pstruBridgeVar->nTx_DeviceId;
			nPortId=pstruBridgeVar->nPort_id;
			/* Set the port state as blocked*/
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nDeviceId-1]->\
			ppstruInterfaceList[nPortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;
			/* Get the connected device id of this blocked port*/
			nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nPortId,\
				&nConnected_DeviceId,&nConnected_PortId);

			/* Set the port state of connected port as blocked*/
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
			ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;
			 /*Update the CONFIG_BPDU of the switch with the info from the CONFIG_BPDU received*/
		     /*Update the root switch id*/
			strcpy(pstruBridgeVar->pszDesignated_root,pszTx_SwitchRootId);
			/*Update the path cost*/
			pstruBridgeVar->nRoot_Path_cost=nPath_Cost;		
			/*Update the transmiiter switch no*/
			pstruBridgeVar->nTx_DeviceId=nTx_DeviceId;
			/*Update the transmitter port id*/
			pstruBridgeVar->nPort_id=nPort_id;
				
			strcpy(pstruBridgeVar->pszTx_Bridge_id,pszBridge_id);
			strcpy(pstruPort->pszDesignated_bridge,pszBridge_id);
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nTx_DeviceId-1]->\
			ppstruInterfaceList[nPort_id-1]->pstruMACLayer->MacVar))->nPort_state = FORWARDING;
			nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nTx_DeviceId,nPort_id,\
			&nConnected_DeviceId,&nConnected_PortId);
			/*Set the port state as forward*/
			((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
			ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state = FORWARDING;

			/*Call this function to broadcast this updated CONFIG_BPDU to other switches*/
				fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
				pstruEventDetails->pPacket=NULL;
				pstruPacket = NULL;
				fn_NetSim_FastEthernet_Config_bpdu_generation();
		}
		else if(pstruBridgeVar->nRoot_Path_cost == nPath_Cost)
		{
#ifdef SPANNING_TREE_LOGFILE
				fprintf(stdout,"Received CBPDU Switch path cost <%d> and Switch path cast <%d> are same\n",nPath_Cost,pstruBridgeVar->nRoot_Path_cost);
#endif
			/*compare the txr switch id*/
			if(strcmp(pstruBridgeVar->pszTx_Bridge_id, pszBridge_id)>0)
			{
#ifdef SPANNING_TREE_LOGFILE
					fprintf(stdout,"Received CBPDU Tx-Switch Id <%s> is less than Switch Id <%s>\n",pszBridge_id,pstruBridgeVar->pszTx_Bridge_id);
#endif
			/* Block the port whose information is present in the CONFIG_BPDU of the switch*/
				nDeviceId=pstruBridgeVar->nTx_DeviceId;
				nPortId=pstruBridgeVar->nPort_id;
				 /*Set the port state as blocked*/
				((PORT_DATA*)(NETWORK->ppstruDeviceList[nDeviceId-1]->\
				ppstruInterfaceList[nPortId-1]->pstruMACLayer->MacVar))->nPort_state= BLOCKING;
				nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nPortId,\
					&nConnected_DeviceId,&nConnected_PortId);

				/*Set the port state of connected port as blocked*/
				((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
				ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;

				/* Update the CONFIG_BPDU of the switch with the info from the CONFIG_BPDU received*/
				/*Update the root switch id*/
				strcpy(pstruBridgeVar->pszDesignated_root,pszTx_SwitchRootId);
				/*Update the path cost*/
				pstruBridgeVar->nRoot_Path_cost=nPath_Cost;				
				/*Update the transmiiter switch no*/
				pstruBridgeVar->nTx_DeviceId=nTx_DeviceId;
				/*Update the transmitter port id	*/
				pstruBridgeVar->nPort_id=nPort_id;
				
				strcpy(pstruBridgeVar->pszTx_Bridge_id,pszBridge_id);
				strcpy(pstruPort->pszDesignated_bridge,pszBridge_id);
				/*Set the port state as forward*/
				((PORT_DATA *)(NETWORK->ppstruDeviceList[nTx_DeviceId-1]->\
				ppstruInterfaceList[nPort_id-1]->pstruMACLayer->MacVar))->nPort_state=FORWARDING;
				nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nTx_DeviceId,nPort_id,\
					&nConnected_DeviceId,&nConnected_PortId);
 				/*Set the port state as forward*/
				((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
				ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=FORWARDING;
			
				/*Call this function to broadcast this updated CONFIG_BPDU to other switches*/
				fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
				pstruEventDetails->pPacket=NULL;
				pstruPacket = NULL;
				fn_NetSim_FastEthernet_Config_bpdu_generation();
			}

			else if(strcmp(pstruBridgeVar->pszTx_Bridge_id,pszBridge_id)==0)
	
			{
#ifdef SPANNING_TREE_LOGFILE
					fprintf(stdout,"Received CBPDU Tx-Switch Id <%s> and Switch Id <%s> are same\n",pszBridge_id,pstruBridgeVar->pszTx_Bridge_id);
#endif
				/*compare the txr port id*/
				if(pstruBridgeVar->nPort_id > nPort_id)
				{
#ifdef SPANNING_TREE_LOGFILE
						fprintf(stdout,"Received CBPDU Tx-Switch port Id <%d> is less than Switch port Id <%d>\n",nPort_id,pstruBridgeVar->nPort_id);
#endif
					/*Block the port whose information is present in the CONFIG_BPDU of the switch*/
					nDeviceId=pstruBridgeVar->nTx_DeviceId;
					nPortId=pstruBridgeVar->nPort_id;
					/*Set the port state as blocked*/
					((PORT_DATA *)(NETWORK->ppstruDeviceList[nDeviceId-1]->\
					ppstruInterfaceList[nPortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;
					nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nPortId,\
					&nConnected_DeviceId,&nConnected_PortId);

					/*Set the port state of connected port as blocked*/
					((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
					ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state=BLOCKING;

					 /*Update the CONFIG_BPDU of the switch with the info from the CONFIG_BPDU received*/
					/*Update the root switch id*/
					strcpy(pstruBridgeVar->pszDesignated_root,pszBridge_id);
					/*Update the path cost*/
					pstruBridgeVar->nRoot_Path_cost=nPath_Cost;
					/*Update the transmiiter switch no*/
					pstruBridgeVar->nTx_DeviceId=nTx_DeviceId;
					/*Update the transmitter port id*/
					pstruBridgeVar->nPort_id=nPort_id;
						
					strcpy(pstruBridgeVar->pszTx_Bridge_id,pszBridge_id);
					strcpy(pstruPort->pszDesignated_bridge,pszBridge_id);
					/*Set the port state as forward of the port thru which the CONFIG_BPDU is received*/
					/*Set the port state as forward*/
					((PORT_DATA *)(NETWORK->ppstruDeviceList[nTx_DeviceId-1]->\
					ppstruInterfaceList[nPort_id-1]->pstruMACLayer->MacVar))->nPort_state =FORWARDING;
					nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nTx_DeviceId,nPort_id,\
					&nConnected_DeviceId,&nConnected_PortId);
					/* Set the port state of connected port as blocked*/
					((PORT_DATA *)(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->\
					ppstruInterfaceList[nConnected_PortId-1]->pstruMACLayer->MacVar))->nPort_state =FORWARDING;
					
					/*Call this function to broadcast this updated CONFIG_BPDU to other switches*/
					fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
					pstruEventDetails->pPacket=NULL;
					pstruPacket = NULL;
					fn_NetSim_FastEthernet_Config_bpdu_generation();
				}
				else if(pstruBridgeVar->nPort_id==nPort_id);
				{
#ifdef SPANNING_TREE_LOGFILE
				fprintf(stdout,"Received CBPDU Tx-Switch port Id <%d> and Switch port Id <%d> are same\n",nPort_id,pstruBridgeVar->nPort_id);
#endif
		

					/*Nothing*/

				}

			}
			else
			{

			}

		}
		else
		{

		}

	}
	else
	{

	}

	if(pstruPacket !=NULL)
	{
		fn_NetSim_Packet_FreePacket(pstruPacket);
		pstruPacket=NULL;
	}

	fnpFreeMemory(pszTx_SwitchRootId);
	fnpFreeMemory(pszBridge_id);
	pszTx_SwitchRootId=NULL;
	return 1;
}
