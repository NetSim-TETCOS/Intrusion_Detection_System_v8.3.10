#include "main.h"
#include "UDP.h"

/*---------------------------------------------------------------------------------
* Copyright (C) 2013															  *
*																				  *
* TETCOS, Bangalore. India                                                        *
*
* Tetcos owns the intellectual property rights in the Product and its content.    *
* The copying, redistribution, reselling or publication of any or all of the      *
* Product or its content without express prior written consent of Tetcos is       *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.     *

* Author:   P.Sathishkumar 
* Date  :   05-Mar-2013
* --------------------------------------------------------------------------------*/

/** This function is used to send the user datagram */

int fn_NetSim_UDP_Send_User_Datagram(NetSim_EVENTDETAILS *pstruEventDetails,struct stru_NetSim_Network *NETWORK)
{
	NetSim_PACKET *pstru_User_Datagram;
	NetSim_SOCKETINTERFACE *pstru_Socket_Interface;
	DATAGRAM_HEADER_UDP *pstruUDP_Header;
	NETSIM_ID nAppId,nSocketId;
	UDP_METRICS *pstruUDP_Metrics=NULL;
	NETSIM_ID nSrcID;
	NETSIM_ID usnSourcePort, usnDest_Port;
	NETSIM_IPAddress szSourceIP, szDestIP;

	/* Get the socket interface address and assign to local variable */
	pstru_Socket_Interface = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface;
	nAppId = pstruEventDetails->nApplicationId;

#pragma warning(disable:4305)
	nSocketId = (NETSIM_ID)pstruEventDetails->szOtherDetails;
#pragma warning(default:4305)

	pstru_User_Datagram = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;
	nSrcID = pstruEventDetails->nDeviceId;
	usnSourcePort = pstru_User_Datagram->pstruTransportData->nSourcePort;  
	usnDest_Port = pstru_User_Datagram->pstruTransportData->nDestinationPort;
	szSourceIP = pstru_User_Datagram->pstruNetworkData->szSourceIP;
	szDestIP = pstru_User_Datagram->pstruNetworkData->szDestIP;

	if(fn_NetSim_UDP_Check_ApplicationMetrics(nSrcID, usnSourcePort, usnDest_Port, szSourceIP, szDestIP, &pstruUDP_Metrics))
		fn_NetSim_UDP_Create_ApplicationMetrics(nSrcID, usnSourcePort, usnDest_Port, szSourceIP, szDestIP, &pstruUDP_Metrics);

	while(pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist)
	{
		/* Get the user datagram from the socket buffer and assign to the local variable to send */
		pstru_User_Datagram = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;
		/* Move the socket buffer segment head to next for getting next user datagram */
		pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist = pstru_User_Datagram->pstruNextPacket;
		pstru_User_Datagram->pstruNextPacket = NULL;

		/* allocate memory and update UDP header */
		pstruUDP_Header = (DATAGRAM_HEADER_UDP*)fnpAllocateMemory(1,sizeof(DATAGRAM_HEADER_UDP));
		pstruUDP_Header->usnSourcePort = pstru_User_Datagram->pstruTransportData->nSourcePort;
		pstruUDP_Header->usnDestinationPort = pstru_User_Datagram->pstruTransportData->nDestinationPort;
		pstruUDP_Header->usnLength = (unsigned short)pstru_User_Datagram->pstruAppData->dPacketSize+TRANSPORT_UDP_OVERHEADS;
		pstru_User_Datagram->pstruTransportData->Packet_TransportProtocol = pstruUDP_Header;

		/* Add payload and overheads */
		pstru_User_Datagram->pstruTransportData->dPayload = pstru_User_Datagram->pstruAppData->dPacketSize;
		pstru_User_Datagram->pstruTransportData->dOverhead = TRANSPORT_UDP_OVERHEADS;

		/* Assign the packet size */
		pstru_User_Datagram->pstruTransportData->dPacketSize = pstru_User_Datagram->pstruTransportData->dPayload + pstru_User_Datagram->pstruTransportData->dOverhead;

		/* Update TransportLayer user datagram time */
		pstru_User_Datagram->pstruTransportData->dArrivalTime = pstruEventDetails->dEventTime;
		pstru_User_Datagram->pstruTransportData->dStartTime = pstruEventDetails->dEventTime;
		pstru_User_Datagram->pstruTransportData->dEndTime = pstruEventDetails->dEventTime;
		pstru_User_Datagram->pstruTransportData->nTransportProtocol = TX_PROTOCOL_UDP;		

		//Update Metrics
		pstruUDP_Metrics->nDataGramsSent++;

		/* Add the event details for NETWORK_OUT_EVENT*/
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
		pstruEventDetails->nPacketId = pstru_User_Datagram->nPacketId;
		pstruEventDetails->nSegmentId = pstru_User_Datagram->pstruAppData->nSegmentId;
		pstruEventDetails->dPacketSize=pstru_User_Datagram->pstruTransportData->dPacketSize;
		pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
		pstruEventDetails->nSubEventType=0;
		pstruEventDetails->pPacket=pstru_User_Datagram;
		fnpAddEvent(pstruEventDetails);	
	}
	return 1;
}
