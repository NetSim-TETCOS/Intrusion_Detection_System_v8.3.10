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

#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "ARP.h"
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Function: Read ARP Table
When packet want to transmit the MAC OUT EVENT, the job of Address Resolution Protocol
is to update the destination MAC address in the MAC data of the pacaket. To do that it 
follows the below steps.
1. Check the destination is in the same LAN. If yes, set the nDestinationId as well as
   DestinationIP from the packet you received. if not,then set the nDestinationId ZERO 
   for the DefaultGateway and the DestinationIP as the GatewayIP.
2. Then check the DestinationIP entry present in the ARP table. If present,
	set the nDestinationPresentFlag.
3  If nDestinationPresentFlag is set (ONE)then update the DestMAc of MACdata of packet
   and forward to MAC layer by keeping the packet in the AccessBuffer of AccessInterface.
4.  If nDestinationPresentFlag is not set and pnArpRequestFlag[nDestinationId](request flag 
	for the destination) is also not set, then buffer the packet and add event by adding 
	subevent type as GENERATE_ARP_REQUEST.
5. If nDestinationPresentFlag is not set pnArpRequestFlag[nDestinationId] is set, then just
	buffer the packet.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_Read_ARP_Table(NetSim_EVENTDETAILS *pstruEventDetails, struct stru_NetSim_Network *NETWORK)
{
	NETSIM_ID nDeviceId;
	NETSIM_ID nInterfaceId, nDestinationId=0;		
	NETSIM_IPAddress szDestIPadd;
	NETSIM_MACAddress szDestMAC=NULL;	
	bool bDestinationPresentFlag = false;
	int nType;
	static char broadcastMac[20]="ffffffffffff";

	NetSim_PACKET *pstruTemp_Data;// NetSim Temp packet to store the packet from the event details	
	ARP_TABLE  *pstruTableHead, *pstruCurrentTable; //Type casting ARP table 	
	ARP_VARIABLES *pstruArpVariables;//Type casting ARP variables	

	// Get packet from the event
	pstruTemp_Data =pstruEventDetails->pPacket;	

	// Get the interface and device id from the event
	nInterfaceId = pstruEventDetails->nInterfaceId;
	nDeviceId = pstruEventDetails->nDeviceId;		
	pstruArpVariables = DEVICE_INTERFACE(nDeviceId,nInterfaceId)->ipVar;
	pstruTableHead = pstruArpVariables->pstruArpTable;


	szDestIPadd = pstruTemp_Data->pstruNetworkData->szNextHopIp;

	if(IP_COMPARE(szDestIPadd,STR_TO_IP4("255.255.255.255"))!=0)  
	{		
		pstruCurrentTable = pstruTableHead; 		
		while(pstruCurrentTable != 0) // Check the destination MAC entry in the table
		{
			if(IP_COMPARE(pstruCurrentTable->szIPAddress,szDestIPadd)== 0)
			{
				bDestinationPresentFlag = true; //set 1 if destination MAC present in the ARP TAble					
				szDestMAC = pstruCurrentTable->szMACAddress;
				break;
			}
			else
				pstruCurrentTable = pstruCurrentTable->pstruNextEntry;
		}
		nDestinationId = pstruTemp_Data->nReceiverId;
	}//if end
	else // This is for Broadcast packet
	{
		pstruCurrentTable = pstruTableHead; 
		while(pstruCurrentTable != 0) // Check the destination MAC entry in the table
		{
			if(IP_COMPARE(pstruCurrentTable->szIPAddress,szDestIPadd)== 0)
			{
				bDestinationPresentFlag = true; //set 1 if destination MAC present in the ARP TAble				
				szDestMAC = pstruCurrentTable->szMACAddress;
				break;
			}
			else
				pstruCurrentTable = pstruCurrentTable->pstruNextEntry;
		}		
		if(!bDestinationPresentFlag)
		{
			nType = DYNAMIC;
			szDestMAC = broadcastMac;//_strdup("ffffffffffff");	// Broadcast MAC
			// call the function to update the table entry by adding the destination MAC
			fn_NetSim_Add_IP_MAC_AddressTo_ARP_Table(&pstruTableHead,szDestIPadd,szDestMAC,nType);
			bDestinationPresentFlag = true;  
		}
	}//else end
	if(bDestinationPresentFlag) //Destination MAC present,fordward the packet
	{
		free(pstruTemp_Data->pstruMacData->szDestMac);
		free(pstruTemp_Data->pstruMacData->szSourceMac);
		//Update the destination and Source MAC adress in the MacData of the packet
		pstruTemp_Data->pstruMacData->szDestMac = _strdup(szDestMAC);
		pstruTemp_Data->pstruMacData->szSourceMac = _strdup(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruMACLayer->szMacAddress);
		
		if(!fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruAccessInterface->pstruAccessBuffer))
		{
			pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);
			pstruEventDetails->dEventTime = pstruEventDetails->dEventTime;
			pstruEventDetails->nSubEventType = 0; 			
			pstruEventDetails->dPacketSize = pstruTemp_Data->pstruNetworkData->dPacketSize;				
			pstruEventDetails->nEventType=MAC_OUT_EVENT;
			pstruEventDetails->pPacket=NULL;	
			fnpAddEvent( pstruEventDetails);			
		}	
		fn_NetSim_Packet_AddPacketToList(DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruAccessInterface->pstruAccessBuffer,pstruTemp_Data,0);

	}	
	/*check the request flag, ZERO for first time, then generate the request,and set the flag. 
	When the flag is ONE, the packets are buffered */	
	else if(bDestinationPresentFlag == false && pstruArpVariables->pnArpRequestFlag[nDestinationId]== 0)
	{
		/* Buffer the Packet*/			
		
		
		//Shashi kant
		//pstruTemp_Data->pstruNetworkData->dArrivalTime = pstruEventDetails->dEventTime;		



		fn_NetSim_Add_PacketTo_Buffer(nDeviceId,pstruTemp_Data,szDestIPadd,nInterfaceId);		
		//Increament the buffered pkt count				
		pstruArpVariables->pstruArpMetrics->nPacketsInBuffer += 1;		
		/*Generate ARP Request */
		pstruEventDetails->nSubEventType = GENERATE_ARP_REQUEST;
		fnpAddEvent(pstruEventDetails);
		pstruArpVariables->pnArpRequestFlag[nDestinationId]= 1; 
	}
	else
	{	/* Buffer the Packet*/			
		
		
		//Shashi kant
		//pstruTemp_Data->pstruNetworkData->dArrivalTime = pstruEventDetails->dEventTime;		



		fn_NetSim_Add_PacketTo_Buffer(nDeviceId,pstruTemp_Data,szDestIPadd,nInterfaceId);
		//Increament the buffered pkt count		
		pstruArpVariables->pstruArpMetrics->nPacketsInBuffer += 1;		
	}	
	
	return 0;
}

