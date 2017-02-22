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
#include "main.h"
#include "Routing.h"
#include "List.h"
#include "../IP/IP.h"


int gettheindexofminweight(int, int[]);
int fn_NetSim_FreeRouterLSA(ROUTER_LSA*);
unsigned int fn_NetSim_GetMetrics(unsigned int);

/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Dijkstra's algorithm is used to find the shortest path from single source to all neighbors. 
This protocol runs within one autonomous system						  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

_declspec(dllexport)int fn_NetSim_OSPF_DijkstraAlgorithm(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	NetSim_PACKET *pstruControlPacket;
	LSU_PACKET *pstruLSUPacket;
	NETSIM_ID nDeviceId,i;
	OSPF_ROUTING_TABLE *pstruTable,*pstruTempTable;
	ROUTERLSA_ENTRY *pstruLSAEntry;
	ROUTER_LSA *pstruRouterLSA;
	nDeviceId = pstruEventDetails->nDeviceId;
	pstruControlPacket = pstruEventDetails->pPacket;
	pstruLSUPacket = pstruControlPacket->pstruAppData->Packet_AppProtocol;
	pstruRouterLSA = pstruLSUPacket->pstruRouterLSA;
	pstruLSAEntry = pstruRouterLSA->pstrEntry;
	pstruTable = ((DEVICE_ROUTER*)NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar)->pstruRoutingTables->pstruOSPFTables->pstruOSPF_RoutingTable;
	fn_NetSim_Stack_GetDeviceId_asIP(pstruControlPacket->pstruNetworkData->szNextHopIp,&i);
	while(pstruLSAEntry)
	{
		pstruTempTable = pstruTable;
		while(pstruTempTable)
		{
			if(!IP_COMPARE(pstruTempTable->szDestinationID,pstruLSAEntry->szLinkID))
			{
				if((pstruTempTable->nCost  > (unsigned int)pstruLSAEntry->nMetric))
				{
					pstruTempTable->szDestinationID=IP_COPY(pstruLSAEntry->szLinkID);
					pstruTempTable->szNexthop=IP_COPY(pstruControlPacket->pstruNetworkData->szGatewayIP);
					pstruTempTable->nCost=pstruLSAEntry->nMetric;
					{
						IP_ROUTINGTABLE** table = (IP_ROUTINGTABLE**)&(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipRoutingTables);
						IP_ROUTINGTABLE* newTable = IPROUTINGTABLE_ALLOC();
						newTable->gateway = IP_COPY(pstruControlPacket->pstruNetworkData->szGatewayIP);
						newTable->Metric = pstruLSAEntry->nMetric;
						newTable->Interface = IP_COPY(pstruControlPacket->pstruNetworkData->szNextHopIp);
						newTable->netMask = IP_COPY(pstruLSAEntry->szLinkData);
						newTable->networkDestination = IP_COPY(pstruLSAEntry->szLinkID);
						newTable->nInterfaceId = i;
						newTable->prefix_len = pstruLSAEntry->prefix_len;
						newTable->type = 0;
						IPROUTINGTABLE_ADD(table,newTable,iptable_add_check);
					}
				}
				break;
			}
			pstruTempTable = pstruTempTable->pstru_Router_NextEntry;
		}
		// If any new entry update that entry in the routing database
		if(!pstruTempTable)
		{
			fn_NetSim_UpdateEntryinRoutingTable(NETWORK,pstruEventDetails,nDeviceId,i,0,pstruLSAEntry->szLinkID,IP_COPY(pstruControlPacket->pstruNetworkData->szGatewayIP),pstruLSAEntry->szLinkData,pstruLSAEntry->prefix_len,pstruLSAEntry->nMetric,1);
				//Adding the entries in IP table
				{
					IP_ROUTINGTABLE** table =(IP_ROUTINGTABLE**) &(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipRoutingTables);
					IP_ROUTINGTABLE* newTable = IPROUTINGTABLE_ALLOC();
					newTable->gateway = IP_COPY(pstruControlPacket->pstruNetworkData->szGatewayIP);
					newTable->Metric = fn_NetSim_GetMetrics(pstruLSAEntry->nMetric);
					newTable->Interface = IP_COPY(pstruControlPacket->pstruNetworkData->szNextHopIp);
					newTable->netMask = IP_COPY(pstruLSAEntry->szLinkData);
					newTable->networkDestination = IP_COPY(pstruLSAEntry->szLinkID);
					newTable->nInterfaceId = i;
					newTable->type = 0;
					newTable->prefix_len = pstruLSAEntry->prefix_len;
					IPROUTINGTABLE_ADD(table,newTable,iptable_add_check);
				}
			
		}
		pstruLSAEntry=pstruLSAEntry->pstru_NextEntry;
	}
	

	return 0;
}
/**
 This function is used by OSPF to calculate IP Table metric
*/
unsigned int fn_NetSim_GetMetrics(unsigned int nCost)
{
	return (unsigned int)round(nCost*250.0/nMaxCost)+1;
}


