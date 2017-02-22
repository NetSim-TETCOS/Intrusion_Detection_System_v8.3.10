/************************************************************************************
* Copyright (C) 2012     
*
* TETCOS, Bangalore. India                                                         *

* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *

* Author:  Thangarasu.K                                                       *
* ---------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "Routing.h"

NETSIM_IPAddress fn_NetSim_OSPF_HighestOfTwoIP(NETSIM_IPAddress szIP,NETSIM_IPAddress szIPaddress);
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function is used to forward the data to Network OUT and RIP packet to Transport IN
1. Check the packet type is data or not
2.If it is data,Check the destination network address in the routing database
If the destination is present in the database, update the output port and Next hop IPaddress
3.if the destination ip address is not in the table then forward the packet to the default 
gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_OSPF_ReceiveDDPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	NETSIM_ID nDId,nInterfaceId=0;
	NETSIM_IPAddress szIPaddress,szCurrentIP,szResultIP;	
	// NetSim packet To store the OSPF control packet
	NetSim_PACKET *pstruControlPacket = NULL;
	// Database PACKET to store DBD packet data 	
	DD_PACKET *pstruDD;
	NETWORK=pstruNETWORK;
	pstruControlPacket=pstruEventDetails->pPacket;
	pstruDD=pstruControlPacket->pstruAppData->Packet_AppProtocol;
	szIPaddress=pstruDD->pstruHeader_DD->szRouterID;
	nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,pstruControlPacket->pstruNetworkData->szDestIP);
	szCurrentIP=fn_NetSim_Stack_GetIPAddressAsId(pstruEventDetails->nDeviceId,nInterfaceId);
	if(pstruDD->bIbit==true)
	{
		szResultIP=fn_NetSim_OSPF_HighestOfTwoIP(szCurrentIP,szIPaddress);
		if(szResultIP==szCurrentIP && pstruDD->pstruHeader!=NULL)
		{
			pstruEventDetails->nPacketId=0;
			pstruEventDetails->nApplicationId= 0;
			pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
			pstruEventDetails->nSubEventType=EXCHANGEDONE;
			pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
			fnpAddEvent(pstruEventDetails);
		}
		else
		{	
		nDId=fn_NetSim_Stack_GetDeviceId_asIP(szResultIP,&nInterfaceId);
		pstruEventDetails->nDeviceId=nDId;
		pstruEventDetails->nInterfaceId=nInterfaceId;
		pstruEventDetails->nPacketId=0;
		pstruEventDetails->nApplicationId= 0;
		pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
		pstruEventDetails->nSubEventType=EXCHANGEDONE;
		pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
		fnpAddEvent(pstruEventDetails);
		}
	}
	else
	{
		pstruEventDetails->nPacketId=0;
		pstruEventDetails->nApplicationId= 0;
		pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
		pstruEventDetails->nSubEventType=EXCHANGEDONE;
		pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
		fnpAddEvent(pstruEventDetails);
	}
	return 0;
}
/** This function is to find the highest IPAddress between two IPAddresses. */
NETSIM_IPAddress fn_NetSim_OSPF_HighestOfTwoIP(NETSIM_IPAddress szIP,NETSIM_IPAddress szIPaddress)
{
	if(IP_COMPARE(szIP,szIPaddress)>=0)
		return szIP;
	else 
		return szIPaddress;
}
