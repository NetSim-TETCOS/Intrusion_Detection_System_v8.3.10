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
/** This function is used to call the inter and intra area routing protocols */
_declspec(dllexport) int fn_NetSim_Routing_Run()
{
	switch(pstruEventDetails->nProtocolId)
	{
	case APP_PROTOCOL_RIP:
		fn_NetSim_RIP_Run_F();
		break;
	case APP_PROTOCOL_OSPF:
		fn_NetSim_OSPF_Run_F();
		break;
	case APP_PROTOCOL_BGP:
		fn_NetSim_BGP_Run_F();
		break;
	default:
		break;
	}
	return 1;
}
/** This function is used to update the entries in routing table */
_declspec(dllexport)int fn_NetSim_UpdateEntryinRoutingTable(struct stru_NetSim_Network *NETWORK,NetSim_EVENTDETAILS *pstruEventDetails,NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId,unsigned int bgpRemoteAS,NETSIM_IPAddress szDestinationIP,NETSIM_IPAddress szNexthop,NETSIM_IPAddress szSubnetmask,unsigned int prefix_len,int nCost,int nFlag)
{
	DEVICE_ROUTER *pstruRouter;
	pstruRouter = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
	if(pstruRouter->AppIntRoutingProtocol == APP_PROTOCOL_RIP)
	{
		//
		fn_NetSim_RIP_UpdatingEntriesinRoutingDatabase(NETWORK,nDeviceId,szDestinationIP,szSubnetmask,szNexthop,nInterfaceId,pstruEventDetails->dEventTime,nCost);	
		if(nFlag)
		{
			fn_NetSim_RIP_TriggeredUpdate(NETWORK,pstruEventDetails);
		}
	}
	if(pstruRouter->AppIntRoutingProtocol == APP_PROTOCOL_OSPF)
	{
		//
		fn_NetSim_OSPF_UpdatingEntriesinRoutingDatabase(NETWORK,nDeviceId,szDestinationIP,szSubnetmask,prefix_len,szNexthop,nCost,0,intra_area);
		if(nFlag)
		{
			fn_NetSim_OSPF_LSUpacketformation(NETWORK,pstruEventDetails); 
		}
	}
	if(pstruRouter->AppExtRoutingProtocol == APP_PROTOCOL_BGP)
	{
		//
		fn_NetSim_BGP_UpdatingEntriesinRoutingDatabase(NETWORK,nDeviceId,nInterfaceId,szDestinationIP,bgpRemoteAS,szNexthop,szSubnetmask,1,nCost);
		if(nFlag)
		{
			NetSim_EVENTDETAILS pevent;
			memcpy(&pevent,pstruEventDetails,sizeof pevent);
			pstruEventDetails->nDeviceId = nDeviceId;
			fn_NetSim_BGP_UpdateMessageformation(NETWORK,pstruEventDetails);
			memcpy(pstruEventDetails,&pevent,sizeof* pstruEventDetails);
		}
	}
	return 0;
}
/** This function is used to update the protocol status in the interface */
_declspec(dllexport)int UpdateInterfaceList()
{
	int i,j;
	DEVICE_ROUTER *pstruRouter;
	NETSIM_ID nDeviceCount,nInterfaceCount,nConnectedDevId,nConnectedInterId;
	nDeviceCount = NETWORK->nDeviceCount;
	for(i=0;i<nDeviceCount;i++)
	{
		//if(NETWORK->ppstruDeviceList[i]->nDeviceType == ROUTER)
		if(fnCheckRoutingProtocol(i+1))
		{
			nInterfaceCount = NETWORK->ppstruDeviceList[i]->nNumOfInterface;
			if(NETWORK->ppstruDeviceList[i]->pstruApplicationLayer)
			{
				pstruRouter = NETWORK->ppstruDeviceList[i]->pstruApplicationLayer->routingVar;
				if(!pstruRouter->RoutingProtocol)
				{
					pstruRouter->RoutingProtocol = calloc(nInterfaceCount,sizeof* pstruRouter->RoutingProtocol);
				}
				for(j=0;j<nInterfaceCount;j++)
				{
					if(NETWORK->ppstruDeviceList[i]->ppstruInterfaceList[j]->nInterfaceType == INTERFACE_WAN_ROUTER)
					{
						fn_NetSim_Stack_GetConnectedDevice(i+1,j+1,&nConnectedDevId,&nConnectedInterId);
						if(NETWORK->ppstruDeviceList[i]->pstruApplicationLayer->nAppRoutingProtocol != APP_PROTOCOL_BGP)
						{
							pstruRouter->RoutingProtocol[j] = NETWORK->ppstruDeviceList[i]->pstruApplicationLayer->nAppRoutingProtocol;
						}
						else if(NETWORK->ppstruDeviceList[nConnectedDevId-1]->pstruApplicationLayer->nAppRoutingProtocol != APP_PROTOCOL_BGP)
						{
							pstruRouter->RoutingProtocol[j] = NETWORK->ppstruDeviceList[nConnectedDevId-1]->pstruApplicationLayer->nAppRoutingProtocol;
						}
						else
						{
							pstruRouter->RoutingProtocol[j] = APP_PROTOCOL_BGP;
						}
					}
				}
			}
		}
	}
	return 0;
}
int fnCheckRoutingPacket()
{
	if(pstruEventDetails->pPacket && pstruEventDetails->pPacket->nControlDataType/100 != APP_PROTOCOL_BGP)
		return 1;
	if(pstruEventDetails->pPacket && pstruEventDetails->pPacket->nControlDataType/100 != APP_PROTOCOL_OSPF)
		return 1;
	if(pstruEventDetails->pPacket && pstruEventDetails->pPacket->nControlDataType/100 != APP_PROTOCOL_RIP)
		return 1;
	return 0;
}

APPLICATION_LAYER_PROTOCOL fnCheckRoutingProtocol(NETSIM_ID deviceId)
{
	if(DEVICE_APPLAYER(deviceId) && (DEVICE_APPLAYER(deviceId)->nAppRoutingProtocol == APP_PROTOCOL_BGP ||
		DEVICE_APPLAYER(deviceId)->nAppRoutingProtocol == APP_PROTOCOL_OSPF ||
		DEVICE_APPLAYER(deviceId)->nAppRoutingProtocol == APP_PROTOCOL_RIP))
		return DEVICE_APPLAYER(deviceId)->nAppRoutingProtocol;
	return 0;
}