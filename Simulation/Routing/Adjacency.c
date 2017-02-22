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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			This function is used to forward the data to Network OUT and RIP packet to Transport IN
			1. Check if the packet type is data or not
			2.If it is data, check the destination network address in the routing database
				If the destination is present in the database, update the output port and Next hop IPaddress
			3.if the destination ip address is not in the table then forward the packet to the default 
				gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_OSPF_Adjacency(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	// NetSim packet To store the RIP packet
	NetSim_PACKET *pstruControlPacket = NULL;

	// HELLO PACKET to store Hello packet data 	
	HELLO_PACKET *pstruHello;

	pstruControlPacket=pstruEventDetails->pPacket;
	pstruHello=pstruControlPacket->pstruAppData->Packet_AppProtocol;
	if((HELLO_INTERVAL==pstruHello->nHelloInterval)||(ROUTER_DEAD_INTERVAL==pstruHello->nRouterDeadInterval))
	{
		pstruEventDetails->nPacketId=0;
		pstruEventDetails->nApplicationId= 0;
		pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
		pstruEventDetails->nSubEventType=NEGOTIATIONDONE;
		pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
		fnpAddEvent(pstruEventDetails);
	}
	else
	{
		printf("Adjaceny is not established\n");
	}
	return 0;
}
