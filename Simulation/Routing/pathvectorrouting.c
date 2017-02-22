/************************************************************************************
* Copyright (C) 2013                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Thangarasu                                                       *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "Routing.h"
#include "List.h"
#include "../IP/IP.h"
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function is used to find the shortest path among the autonomus systems
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_BGP_PathVectorRouting(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	NETSIM_ID nInterfaceId,nDestinationId,nDestinationInterfaceId;
	NetSim_PACKET *pstruNetSimPacket;
	DEVICE_ROUTER *pstruBGPVariable;
	BGP_UPDATE *pstruUpdateMessage;
	BGP_ROUTING_TABLE *pstruBGPTable;
	NLRI *pstruNLRI;
	pstruNetSimPacket = pstruEventDetails->pPacket;
	pstruUpdateMessage = pstruNetSimPacket->pstruAppData->Packet_AppProtocol;
	pstruNLRI =pstruUpdateMessage->pstruNLRI;
	fn_NetSim_Stack_GetDeviceId_asIP(pstruNetSimPacket->pstruNetworkData->szNextHopIp,&nInterfaceId);
	pstruBGPVariable = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar;
	while(pstruNLRI)
	{
		pstruBGPTable = ((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar))->pstruRoutingTables->pstruBGPTables->pstruBGPTable;
		while(pstruBGPTable)
		{
			if(!IP_COMPARE(pstruNLRI->Prefix,pstruBGPTable->bgpPeerRemoteAddr))
			{
				if(pstruBGPTable->nASpathlength > pstruNLRI->nASpathlength+1)
				{
					IP_ROUTINGTABLE** table = (IP_ROUTINGTABLE**)&(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruNetworkLayer->ipRoutingTables);
					IP_ROUTINGTABLE* newTable = IPROUTINGTABLE_ALLOC();
					newTable->gateway = IP_COPY(pstruNetSimPacket->pstruNetworkData->szGatewayIP);
					newTable->Metric = pstruNLRI->nASpathlength+1;;
					newTable->Interface = IP_COPY(pstruNetSimPacket->pstruNetworkData->szNextHopIp);
					newTable->netMask = IP_COPY(pstruNLRI->bgpSubnetMask);
					newTable->networkDestination = IP_COPY(pstruNLRI->Prefix);
					newTable->nInterfaceId = nInterfaceId;
					newTable->type = 0;
					//Adding the entry in the IP table
					IPROUTINGTABLE_ADD(table,newTable,iptable_add_check);
				}	
				break;
			}
			pstruBGPTable = pstruBGPTable->pstruNextEntry;
		}
		if(!pstruBGPTable)
		{
			unsigned int bgpPeerRemoteAs;
			NETSIM_IPAddress bgpPeerIdentifier,bgpPeerLocalAddr,bgpPeerRemoteAddr,bgpNextHop;
			IP_ROUTINGTABLE** table = (IP_ROUTINGTABLE**)&(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruNetworkLayer->ipRoutingTables);
			IP_ROUTINGTABLE* newTable = IPROUTINGTABLE_ALLOC();
			//Adding the Entry in the routing table
			bgpPeerIdentifier = IP_COPY(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
			bgpPeerLocalAddr = IP_COPY(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
			bgpPeerRemoteAddr = IP_COPY(pstruNLRI->Prefix);
			bgpNextHop = IP_COPY(pstruNetSimPacket->pstruNetworkData->szGatewayIP);
			nDestinationId = fn_NetSim_Stack_GetDeviceId_asIP(pstruNetSimPacket->pstruNetworkData->szSourceIP,&nDestinationInterfaceId);
			bgpPeerRemoteAs = fn_NetSim_FindAS(nDestinationId);
			pstruNLRI->nASpathlength++;
			fn_NetSim_UpdateEntryinRoutingTable(NETWORK,pstruEventDetails,pstruEventDetails->nDeviceId,nInterfaceId,bgpPeerRemoteAs,bgpPeerRemoteAddr,bgpNextHop,pstruNLRI->bgpSubnetMask,pstruNLRI->prefix_len,pstruNLRI->nASpathlength,1);
			//fn_NetSim_BGP_UpdatingEntriesinRoutingDatabase(NETWORK,pstruEventDetails->nDeviceId,nInterfaceId-1,bgpPeerRemoteAddr,bgpPeerRemoteAs,bgpNextHop,pstruNLRI->bgpSubnetMask,1,pstruNLRI->nASpathlength);
			fn_NetSim_BGP_UpdatingEntriesinRoutingDatabase(NETWORK,pstruEventDetails->nDeviceId,nInterfaceId,bgpPeerRemoteAddr,bgpPeerRemoteAs,bgpNextHop,pstruNLRI->bgpSubnetMask,4,pstruNLRI->nASpathlength);
			newTable->gateway = IP_COPY(pstruNetSimPacket->pstruNetworkData->szGatewayIP);
			newTable->Metric = pstruNLRI->nASpathlength;
			newTable->Interface = IP_COPY(pstruNetSimPacket->pstruNetworkData->szNextHopIp);
			newTable->netMask = IP_COPY(pstruNLRI->bgpSubnetMask);
			newTable->networkDestination = IP_COPY(pstruNLRI->Prefix);
			newTable->nInterfaceId = nInterfaceId;
			newTable->type = 0;
			//Adding the entry in the IP table
			IPROUTINGTABLE_ADD(table,newTable,iptable_add_check);
		}
		
		pstruNLRI = pstruNLRI->pstruNextNLRI;
	}

	if(pstruNetSimPacket)
		fn_NetSim_Packet_FreePacket(pstruNetSimPacket);
	return 0;
}
