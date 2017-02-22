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
/**
This function is used to check the adjacency between the neighbors when receiving hello packets.
*/

int fn_NetSim_OSPF_ReceiveHelloPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	int nFlag;
	OSPF_STATE *pstruOSPF;
	NETSIM_IPAddress szIPAddress;
	NETSIM_ID nInterfaceId;
	NEIGHBOR *pstruNeighbor,*pstruTraverse;
	IPADDRESS *pstruIP;
	// NetSim packet To store the OSPF packet
	NetSim_PACKET *pstruControlPacket = NULL;
	// HELLO PACKET to store Hello packet data 	
	HELLO_PACKET *pstruHello;
	NETWORK=pstruNETWORK;
	pstruControlPacket=pstruEventDetails->pPacket;
	pstruHello=pstruControlPacket->pstruAppData->Packet_AppProtocol;
	nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,pstruControlPacket->pstruNetworkData->szDestIP);
	pstruOSPF=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->interfaceVar;
	fn_NetSim_OSPF_ChangeState(pstruOSPF,Init,2);
	pstruNeighbor=pstruHello->pstruNeighbor;
	pstruTraverse=pstruHello->pstruNeighbor;
	szIPAddress=fn_NetSim_Stack_GetIPAddressAsId(pstruEventDetails->nDeviceId,nInterfaceId);
	pstruIP=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.pstruNeighborIP;
	while(pstruTraverse)
	{
		if(IP_COMPARE(pstruTraverse->szNeighbor,szIPAddress)!=0)
		{
			nFlag=1;
			pstruTraverse=pstruTraverse->pstruNextNeighborIP;
		}
		else
		{
			fn_NetSim_OSPF_ChangeState(pstruOSPF,DRother,1);
			fn_NetSim_OSPF_ChangeState(pstruOSPF,TwoWay,2);
			break;
		}
		if(nFlag==1)
		{
			fn_NetSim_OSPF_ChangeState(pstruOSPF,DRother,1);
			fn_NetSim_OSPF_ChangeState(pstruOSPF,Init,2);
		}
	}
	if(pstruOSPF->NeighborCurrentState==TwoWay)
	{
		fn_NetSim_OSPF_Adjacency(NETWORK,pstruEventDetails);
	}
	return 0;
}
