/************************************************************************************
* Copyright (C) 2015                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"

int fn_NetSim_NAT_NetworkOut(NETSIM_ID ndev,NetSim_PACKET* packet)
{
	NETSIM_IPAddress dest = packet->pstruNetworkData->szDestIP;
	NETSIM_ID i;

	if(packet->nDestinationId == 0)
		return 1;

	for(i=0;i<DEVICE(ndev)->nNumOfInterface;i++)
	{
		if(DEVICE_INTERFACE(ndev,i+1) && DEVICE_INTERFACE(ndev,i+1)->szAddress)
		{
			NETSIM_IPAddress ip = DEVICE_INTERFACE(ndev,i+1)->szAddress;
			NETSIM_IPAddress mask = DEVICE_INTERFACE(ndev,i+1)->szSubnetMask;
			unsigned int prefix = DEVICE_INTERFACE(ndev,i+1)->prefix_len;
			NETSIM_IPAddress n1,n2;
			if(dest->type != ip->type)
				continue;
			n1=IP_NETWORK_ADDRESS(ip,mask,prefix);
			n2=IP_NETWORK_ADDRESS(dest,mask,prefix);
			if(!IP_COMPARE(n1,n2))
				return 2;
		}
	}
	for(i=0;i<DEVICE(ndev)->nNumOfInterface;i++)
	{
		if(DEVICE_INTERFACE(ndev,i+1) && DEVICE_INTERFACE(ndev,i+1)->szAddress && DEVICE_INTERFACE(ndev,i+1)->szDefaultGateWay)
		{
			packet->pstruNetworkData->szDestIP = DEVICE_INTERFACE(ndev,i+1)->szDefaultGateWay;
			break;
		}
	}
	return 0;
}

int fn_NetSim_NAT_NetworkIn(NETSIM_ID ndev,NetSim_PACKET* packet)
{
	NETSIM_ID dest = packet->nDestinationId;
	NETSIM_IPAddress ip = fn_NetSim_Stack_GetFirstIPAddressAsId(dest,0);
	if(ndev == dest || dest == 0)
		return 0;
	packet->pstruNetworkData->szDestIP = ip;
	return 1;
}
