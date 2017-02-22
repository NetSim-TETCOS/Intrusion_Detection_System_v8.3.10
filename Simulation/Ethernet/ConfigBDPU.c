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
 Call the function to generate configuration bpdu for spanning tree protocol		
*/
int fn_NetSim_FastEthernet_Config_bpdu_generation()
{
	NETSIM_ID nPort_No;
	int nNo_of_Ports;
	NETSIM_ID nConnected_DeviceId=0;
	NETSIM_ID nConnected_InterfaceId=0;
	NETSIM_ID nLink_Id=0;
	
	/*Interface variable declaration for type casting*/
	NetSIm_DEVICEINTERFACE *pstruInterface;
	
	/* Get the no.of ports used in the switch*/
	nNo_of_Ports=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->nNumOfInterface;
	
	for(nPort_No=1; nPort_No <= nNo_of_Ports; nPort_No++)
	{
		/*Type casting the interfcae variabel*/
		pstruInterface=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nPort_No-1];
		/*Check the port has Enable or Disable*/
		if(pstruInterface->pstruMACLayer != NULL) 
		{
			if(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->nDeviceType==SWITCH)
				/*Check the connected device type */
				nLink_Id=fn_NetSim_Stack_GetConnectedDevice((pstruEventDetails->nDeviceId),nPort_No,&nConnected_DeviceId,&nConnected_InterfaceId);
			if(nLink_Id)
				if(NETWORK->ppstruDeviceList[nConnected_DeviceId-1]->nDeviceType==SWITCH)
				{
					/* call the function to transmit the CBPDU packet to the port*/
					fn_NetSim_FastEthernet_TransmitConfig_BPDU(nPort_No,pstruEventDetails);

				}
		}
	}
	return 0;
}

/**
	This function creates and places the CONFIG_BPDU in the ports of the switch.
	Initial CONFIG_BPDU frame RootId is same as TxId, 
	then it will change  the root id when the port recives less root id from CONFIG_BPDU.
*/
int fn_NetSim_FastEthernet_TransmitConfig_BPDU(NETSIM_ID nPort_No,NetSim_EVENTDETAILS *pstruEventDetails)
{

	int nPath_Cost=0;
	NETSIM_ID nConnected_DeviceId=0;
	NETSIM_ID nConnected_InterfaceId=0;
	NETSIM_ID nLink_Id;
	
	NetSim_PACKET *pstruControl_Packet=NULL;
	/* CONFIG_BPDU frame declaration for type casting*/
	FRAME_MAC *pstruPacket_CBPDU=NULL;
	/*Device variable declaration for type casting*/
	BRIDGE_DATA *pstruBridgeVar=NULL;
	/*Interface variable declaration for type casting*/
	NetSIm_DEVICEINTERFACE *pstruInterface=NULL;
	pstruBridgeVar=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->deviceVar))->pstruBridgeVar;
	/* Type casting the interface list*/
	pstruInterface=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nPort_No-1];
	nLink_Id=fn_NetSim_Stack_GetConnectedDevice((pstruEventDetails->nDeviceId),nPort_No,&nConnected_DeviceId,&nConnected_InterfaceId);

					/*Create CONFIG_BPDU frame for spanning tree calculation*/
					 /*Allocate memory for CONFIG_BPDU frame*/
					pstruPacket_CBPDU = fnpAllocateMemory(1,sizeof(FRAME_MAC));
				
					/* Allocate memory for CONFIG_BPDU*/
					pstruPacket_CBPDU->pstruCBPDU=fnpAllocateMemory(1,sizeof * pstruCBPDU);

					//Get the root address  of the switch
					pstruPacket_CBPDU->pstruCBPDU->pszRoot_id =\
						_strdup(pstruBridgeVar->pszDesignated_root);

					//Calculate the path cost
					nPath_Cost=pstruBridgeVar->nRoot_Path_cost+\
						((PORT_DATA *)(pstruInterface->pstruMACLayer->MacVar))->nPath_Cost;
					//Set the path cost for the CONFIG_BPDU
					pstruPacket_CBPDU->pstruCBPDU->nRoot_Path_cost=nPath_Cost;

					//Set the transmiter switch id
					pstruPacket_CBPDU->pstruCBPDU->pszBridge_id= \
						_strdup(pstruBridgeVar->pszBridge_id);

					/*Set the transmitter switch no*/
					pstruPacket_CBPDU->pstruCBPDU->nTx_DeviceId=pstruEventDetails->nDeviceId;
					/*set the transmitter port id*/
					pstruPacket_CBPDU->pstruCBPDU->nPort_id=nPort_No;

					pstruPacket_CBPDU->pstruCBPDU->dHello_Time = 0.0;
					pstruPacket_CBPDU->pstruCBPDU->dForward_delay = 0.0;
					pstruPacket_CBPDU->pstruCBPDU->dMax_Age = 0.0;
					pstruPacket_CBPDU->pstruCBPDU->dMessage_Age =0.0;
					pstruPacket_CBPDU->pstruCBPDU->nTopology_Change =ZERO;
					pstruPacket_CBPDU->pstruCBPDU->nTopology_Change_Acknowledgment = ZERO;
				
					/* Allocate memory to netsim packet*/
					pstruControl_Packet = fn_NetSim_Packet_CreatePacket(MAC_LAYER);
					pstruControl_Packet->nDestinationId=ZERO;
					/*Set Data type for CONFIG_BPDU as control frames*/
					pstruControl_Packet->nPacketType=CBPDU_PACKET;
					pstruControl_Packet->nTransmitterId=pstruEventDetails->nDeviceId;
					pstruControl_Packet->nReceiverId=nConnected_DeviceId;
					pstruControl_Packet->nSourceId=pstruEventDetails->nDeviceId;
					
					pstruPacket_CBPDU->nPacket_Type=CBPDU_PACKET;			 
					pstruControl_Packet->pstruMacData->dPayload=ZERO;
					pstruControl_Packet->pstruMacData->nMACProtocol =MAC_PROTOCOL_IEEE802_3;
					/*Set frame size for CONFIG_BPDU frame as 35 bytes*/
					pstruControl_Packet->pstruMacData->dOverhead=CBPDU_PACKET_SIZE;
					pstruControl_Packet->pstruMacData->dPacketSize=pstruControl_Packet->pstruMacData->dPayload + pstruControl_Packet->pstruMacData->dOverhead;	 
					/*Set frame type broadcast frame*/
					pstruPacket_CBPDU->nTx_Flag = BROAD_CAST;
				
					pstruControl_Packet->pstruMacData->Packet_MACProtocol=pstruPacket_CBPDU;
					pstruControl_Packet->pstruNextPacket=NULL;
					//Set the packet type
					pstruControl_Packet->nPacketType = PacketType_Control;
					//Set the control packet type
					pstruControl_Packet->nControlDataType = CBPDU_PACKET;

					/*form the event details*/
					pstruEventDetails->nProtocolId=MAC_PROTOCOL_IEEE802_3;
					/*Add the sub event type*/
					pstruEventDetails->nSubEventType=CARRIER_SENSE;
					pstruEventDetails->nDeviceType=SWITCH;
					pstruEventDetails->dPacketSize=ZERO;
					pstruEventDetails->nApplicationId=ZERO;
					pstruEventDetails->nInterfaceId=nPort_No;
					 /*Write the MAC_OUT_event list*/
					pstruEventDetails->nEventType=MAC_OUT_EVENT;
					pstruEventDetails->pPacket=pstruControl_Packet;
#ifdef SPANNING_TREE_LOGFILE
					fprintf(stdout,"Sw-%d portNo-%d Generate CBPDU with the SwitchID of <%s>%d?%d\n",pstruEventDetails->nDeviceId,nPort_No,pstruPacket_CBPDU->pstruCBPDU->pszRoot_id,pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
#endif
					 /* Add the event to list*/
					fnpAddEvent(pstruEventDetails);
	
					return 0;
}
