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
	After receiving the LinkStateRequest, add the send LSU event to send the Link state update
*/
int fn_NetSim_OSPF_ReceiveLSRPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nApplicationId= 0;
	pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
	pstruEventDetails->nSubEventType=SEND_LSU;
	pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
	fnpAddEvent(pstruEventDetails);

	return 0;
}
