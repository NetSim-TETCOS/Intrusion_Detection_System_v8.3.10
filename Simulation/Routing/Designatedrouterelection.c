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
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This functions runs the algorithm used for calculating a network's Designated Router and 
Backup Designated Router.  This algorithm is invoked by the Interface state machine							  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

_declspec(dllexport)int fn_NetSim_OSPF_DesignatedRouterElection(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	int nRouterPriority,nPriority;
	NETSIM_ID nRouterID;
	NETSIM_IPAddress szDesignatedIP;
	HELLO_PACKET *pstruPacket;
	int nDesignatedRouterId;
	OSPF_STATE *pstruOSPF;


	nRouterPriority=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nRtrPri;
	pstruOSPF=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->interfaceVar;

	nRouterID=pstruEventDetails->nDeviceId;

	pstruPacket=pstruEventDetails->pPacket->pstruAppData->Packet_AppProtocol;
	nPriority=pstruPacket->nRtrPri;

	if(nPriority>nRouterPriority)
	{
		nDesignatedRouterId=nPriority;
		szDesignatedIP=pstruPacket->pstruNeighbor->szNeighbor;
		fn_NetSim_OSPF_ChangeState(pstruOSPF,DR,1);

	}
	else if(nRouterPriority==0)
		fn_NetSim_OSPF_ChangeState(pstruOSPF,DRother,1);

	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nApplicationId= 0;
	pstruEventDetails->nEventType=APPLICATION_IN_EVENT;
	pstruEventDetails->nSubEventType=ONEWAY;
	pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
	fnpAddEvent(pstruEventDetails);

	return 0;
}
