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
	After receiving the LinkStateUpdate, add the send LSA event to send the Link state Acknowledgement 
*/
int fn_NetSim_OSPF_ReceiveLSUPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nApplicationId= 0;
	pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
	pstruEventDetails->nSubEventType=SEND_LSA;
	pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
	fnpAddEvent(pstruEventDetails);
	return 0;
}
