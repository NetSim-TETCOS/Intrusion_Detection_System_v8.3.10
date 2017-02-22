/************************************************************************************
* Copyright (C) 2013     
*
* TETCOS, Bangalore. India                                                         *

* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *

* Author:   Basamma YB 
* Date  :    
************************************************************************************/
/*********************************************************************************
Function: Update Arp Table and Farward Packet
This function is called whenever the NETWORK_IN_EVENT triggred with the control 
packet as REPLY_PACKET. That is ARP reply is received. Then call this function to
update the MAC address and forward the buffred packet. Do the following, 
1.Update the ARP table by updating the destination IP and MAC address
2.Take the buffered packet, update the destination MAC address and add the 20 byte IP header.
3.Add the packet to access buffer to forward the packet to next layer.							
*********************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "ARP.h"
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Function: Update Arp Table and Farward Packet
This function is called whenever the NETWORK_IN_EVENT triggred with the control 
packet as REPLY_PACKET. That is ARP reply is received. Then call this function to
update the MAC address and forward the buffred packet. Do the following, 
1.Update the ARP table by updating the destination IP and MAC address
2.Take the buffered packet, update the destination MAC address and add the 20 byte IP header.
3.Add the packet to access buffer to forward the packet to next layer.							
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_Update_ARP_Table_ForwardPacket(NetSim_EVENTDETAILS *pstruEventDetails, struct stru_NetSim_Network *NETWORK)
{
	NETSIM_ID nDeviceId;
	NETSIM_ID nInterfaceId;
	NETSIM_ID nDestinationId;
	int nType;			
	NETSIM_MACAddress szTempMAC;
	NETSIM_IPAddress szSrcIPadd;
	NETSIM_IPAddress szDestIPadd;

	NetSim_PACKET *pstruTemp_Data, *pstruTempBufferData;	
	ARP_PACKET *pstruPacketArp;		
	ARP_TABLE  *pstruTableHead;
	ARP_VARIABLES *pstruArpVariables; 
	ARP_BUFFER *pstruPacketBuffer, *pstruPrevBuffer,*tempBuffer=NULL; 

	// Get the device and interface Id from the event
	nInterfaceId = pstruEventDetails->nInterfaceId;
	nDeviceId = pstruEventDetails->nDeviceId;
	//Update the ARP table with destination MAC and IP address
	pstruArpVariables = DEVICE_INTERFACE(nDeviceId,nInterfaceId)->ipVar;	
	pstruTableHead = pstruArpVariables->pstruArpTable;		
	//Get the packet from the event
	pstruTemp_Data =pstruEventDetails->pPacket;
	// Get the Arp Packet from the NetworkData of temp packet
	pstruPacketArp = pstruTemp_Data->pstruNetworkData->Packet_NetworkProtocol;		
	szSrcIPadd = IP_COPY(pstruPacketArp->sz_ar$spa);
	szTempMAC = _strdup(pstruPacketArp->sz_ar$sha);
	nType = DYNAMIC;
	// call the function to update the table entry by adding the destination MAC
	fn_NetSim_Add_IP_MAC_AddressTo_ARP_Table(&pstruTableHead,szSrcIPadd,szTempMAC,nType);	

	// Write to Log file	
	szDestIPadd =  pstruPacketArp->sz_ar$tpa;	
	printf("ARP---ARP_ReplyGot\tEventTime:%0.3lf\tSrcIP:%s\tDestIP:%s\tMAC_Add:%s\n",pstruEventDetails->dEventTime,szDestIPadd,szSrcIPadd,szTempMAC);

	// Get the device id from the event
	nDeviceId = pstruEventDetails->nDeviceId;	
	pstruPacketBuffer = pstruArpVariables->pstruPacketBuffer;
	pstruPrevBuffer = pstruArpVariables->pstruPacketBuffer;
	while(pstruPacketBuffer)		
	{					
		if((IP_COMPARE(pstruPacketBuffer->szDestAdd,szSrcIPadd) == 0))
		{			
			pstruTempBufferData = pstruPacketBuffer->pstruPacket;
			pstruArpVariables->pstruArpMetrics->nPacketsInBuffer -= 1;
			//Update the destination and Source MAC adress in the MacData of the packet
			pstruTempBufferData->pstruMacData->szDestMac =_strdup(szTempMAC);
			nDeviceId = pstruEventDetails->nDeviceId;
			pstruTempBufferData->pstruMacData->szSourceMac = _strdup(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruMACLayer->szMacAddress);
			
			
			//Shashi kant
			//Add overheads and size to NetworkData of the packet
			//if(pstruTempBufferData->pstruTransportData)
			//	pstruTempBufferData->pstruNetworkData->dPayload = pstruTempBufferData->pstruTransportData->dPacketSize;
			//unProtocolId = fn_NetSim_Stack_GetNWProtocol(nDeviceId);
			//if(unProtocolId == NW_PROTOCOL_IPV4)
			//	pstruTempBufferData->pstruNetworkData->dOverhead += IPV4_NETWORK_OVERHEADS;
			//else
			//	pstruTempBufferData->pstruNetworkData->dOverhead += IPV6_NETWORK_OVERHEADS;
			//pstruTempBufferData->pstruNetworkData->dPacketSize = pstruTempBufferData->pstruNetworkData->dPayload + pstruTempBufferData->pstruNetworkData->dOverhead;
			////Update NetworkData packet timings 
			//pstruTempBufferData->pstruNetworkData->dStartTime  =  pstruEventDetails->dEventTime;
			//pstruTempBufferData->pstruNetworkData->dEndTime  =  pstruEventDetails->dEventTime;
			////Get the Network protocol and assign it to nNetworkProtocol of network data
			//pstruTempBufferData->pstruNetworkData->nNetworkProtocol = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);		




			if(!fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruAccessInterface->pstruAccessBuffer))
			{			
				pstruEventDetails->nSubEventType = 0; 				
				pstruEventDetails->dPacketSize = pstruTempBufferData->pstruNetworkData->dPacketSize;				
				if(pstruTempBufferData->pstruAppData)
				{
					pstruEventDetails->nApplicationId = pstruTempBufferData->pstruAppData->nApplicationId;
					pstruEventDetails->nSegmentId = pstruTempBufferData->pstruAppData->nSegmentId;
				}
				else
				{
					pstruEventDetails->nApplicationId = 0;
					pstruEventDetails->nSegmentId = 0;
				}
				pstruEventDetails->nInterfaceId = nInterfaceId; //InterfaceId from the buffer				
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);				
				pstruEventDetails->nPacketId = pstruTempBufferData->nPacketId;
				pstruEventDetails->nEventType=MAC_OUT_EVENT;
				pstruEventDetails->pPacket = NULL;	
				fnpAddEvent( pstruEventDetails);				
			}
			fn_NetSim_Packet_AddPacketToList(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruAccessInterface->pstruAccessBuffer,pstruTempBufferData,0);//0);

			if(pstruArpVariables->pstruPacketBuffer == pstruPacketBuffer)
				pstruArpVariables->pstruPacketBuffer = pstruPacketBuffer->pstruNextBuffer;
			pstruPacketBuffer = pstruPacketBuffer->pstruNextBuffer;
			IP_FREE(pstruPrevBuffer->szDestAdd);
			fnpFreeMemory(pstruPrevBuffer);
			if(tempBuffer)
				tempBuffer->pstruNextBuffer = pstruPacketBuffer;
			pstruPrevBuffer = pstruPacketBuffer;
			continue;

		}
		tempBuffer = pstruPrevBuffer;
		pstruPacketBuffer=pstruPacketBuffer->pstruNextBuffer;
		pstruPrevBuffer = pstruPacketBuffer;
	}


	//Shashi kant
	//szDefaultGateWay = DEVICE_INTERFACE(nDeviceId,nInterfaceId)->szDefaultGateWay;
	//if(IP_COMPARE(szSrcIPadd,szDefaultGateWay) != 0)// Destination is in the same LAN	
	//	nDestinationId = pstruTemp_Data->nSourceId;//pstruEventDetails->pPacket->nSourceId;
	//else // Destination is in the different LAN
	//	nDestinationId = 0; // for the Default Gateway	
	nDestinationId = pstruTemp_Data->nSourceId;




	pstruArpVariables->pnArpReplyFlag[nDestinationId] = 1;	// Set Reply flag 
	pstruArpVariables->pnArpRequestFlag[nDestinationId] = 0; //Reset the Request flag
	// For Metrics
	//Increament the Reply Received count 
	pstruArpVariables->pstruArpMetrics->nArpReplyReceivedCount+= 1;
	IP_FREE(szSrcIPadd);
	fnpFreeMemory(szTempMAC);
	// Free the reply packet
	fn_NetSim_Packet_FreePacket(pstruTemp_Data);
	pstruTemp_Data = NULL;
	pstruEventDetails->pPacket = NULL;

	return 0;
}

