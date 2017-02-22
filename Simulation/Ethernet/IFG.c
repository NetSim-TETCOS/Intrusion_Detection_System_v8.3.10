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
	This function is called when the MAC_OUT_EVENT is triggered with subevent type 
	WAIT_FOR_IFG.This function allows device to sense the medium for IFG time.
 */
int fn_NetSim_FastEthernet_IFG(NetSim_EVENTDETAILS* pstruEventDetails)
{
	NetSim_PACKET *pstruPacket=NULL;
	DEVICE_PHYSICALLAYER *pstruTemp_Phy=NULL;
	NetSIm_DEVICEINTERFACE *pstruInterface=NULL;
	pstruInterface=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1];
	pstruTemp_Phy=((DEVICE_PHYSICALLAYER *)(pstruInterface->pstruPhysicalLayer->phyVar));
	// Get the packet from MAC layer buffer
	pstruPacket = fn_NetSim_Packet_GetPacketFromBuffer(pstruInterface->pstruAccessInterface->pstruAccessBuffer,1);
	//pstruPacket->pstruNextPacket = NULL;

	if(pstruTemp_Phy->nLinkState == LINK_UP || !pstruPacket)
		fnNetSimError("IFG is called without packet or LINK_UP");
		
	switch(pstruEventDetails->nDeviceType)
	{
	default:
	{
		//Check the uplink state is free
		if(pstruTemp_Phy->nLinkState == LINK_DOWN)
		{

			pstruTemp_Phy->nLinkState = LINK_UP;	//Set the medium free flag as busy
			/* Check the payload is less than 46, than add padding to the payload */
			if(pstruPacket->pstruMacData->dPayload < 46)		/*check the payload size  */
			{
				pstruPacket->pstruMacData->dPayload = 46;		 /*add padding bytes*/
			}
		
			/* Add the Mac layer packet details and over heads */
			pstruPacket->pstruMacData->dArrivalTime=pstruPacket->pstruNetworkData->dEndTime;
			pstruPacket->pstruMacData->dStartTime=pstruEventDetails->dEventTime;
			pstruPacket->pstruMacData->dEndTime=pstruPacket->pstruMacData->dStartTime;
			pstruPacket->pstruMacData->dOverhead= dMac_Over_Heads;
			pstruPacket->pstruMacData->dPayload=pstruPacket->pstruNetworkData->dPacketSize;
			pstruPacket->pstruMacData->nMACProtocol=MAC_PROTOCOL_IEEE802_3;
			pstruPacket->pstruMacData->dPacketSize=pstruPacket->pstruMacData->dPayload + \
				pstruPacket->pstruMacData->dOverhead;
			if(pstruPacket->pstruMacData->Packet_MACProtocol == NULL)
			pstruPacket->pstruMacData->Packet_MACProtocol=fnpAllocateMemory(1,sizeof(FRAME_MAC));
			((FRAME_MAC *)(pstruPacket->pstruMacData->Packet_MACProtocol))->nPacket_Type=DATA_PACKET;
		
			/* Add the event details for PHYSICAL_OUT_EVENT*/
			pstruEventDetails->nEventType=PHYSICAL_OUT_EVENT;
			pstruEventDetails->nSubEventType=ZERO;
			pstruEventDetails->dPacketSize=pstruPacket->pstruMacData->dPacketSize;
			pstruEventDetails->pPacket=pstruPacket;	
			pstruEventDetails->nPacketId = pstruEventDetails->pPacket->nPacketId;
			
			if(pstruEventDetails->pPacket->pstruAppData)
				pstruEventDetails->nSegmentId = pstruEventDetails->pPacket->pstruAppData->nSegmentId;

			fnpAddEvent(pstruEventDetails);		
		}
		break;
	}

	case SWITCH:
	{		
		/*Check the uplink state is free*/
		if(pstruTemp_Phy->nLinkState == LINK_DOWN)
		{

			pstruTemp_Phy->nLinkState= LINK_UP;	/*Set the medium free flag as busy*/						

			/* Add the Mac layer packet details and over heads */
			pstruPacket->pstruMacData->dArrivalTime=pstruPacket->pstruMacData->dArrivalTime;
			pstruPacket->pstruMacData->dStartTime=pstruEventDetails->dEventTime;
			pstruPacket->pstruMacData->dOverhead=dMac_Over_Heads;
			pstruPacket->pstruMacData->dPayload=pstruPacket->pstruMacData->dPacketSize;
			pstruPacket->pstruMacData->dPacketSize=pstruPacket->pstruMacData->dPayload +\
				pstruPacket->pstruMacData->dOverhead;
			pstruPacket->pstruMacData->nMACProtocol=MAC_PROTOCOL_IEEE802_3;
			if(((FRAME_MAC *)(pstruPacket->pstruMacData->Packet_MACProtocol))== NULL)
			pstruPacket->pstruMacData->Packet_MACProtocol=fnpAllocateMemory(1,sizeof(FRAME_MAC));
			((FRAME_MAC *)(pstruPacket->pstruMacData->Packet_MACProtocol))->nPacket_Type=DATA_PACKET;
			/* Add the event details for PHYSICAL_OUT_EVENT*/
			pstruEventDetails->nEventType=PHYSICAL_OUT_EVENT;
			pstruEventDetails->nSubEventType=ZERO;
			pstruEventDetails->dPacketSize=pstruPacket->pstruMacData->dPacketSize;
			pstruEventDetails->pPacket=pstruPacket;
			/* Minus the buffer size to the packet size */
			pstruInterface->pstruAccessInterface->pstruAccessBuffer->dCurrentBufferSize -= pstruEventDetails->dPacketSize;
			fnpAddEvent(pstruEventDetails);		
			
		}
		break;
	}
	
	}
	return 1;
}
