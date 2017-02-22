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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Creating LSA packets and forwarding it into the WAN interfaces of the router				  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec(dllexport)int fn_NetSim_OSPF_LSApacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	ROUTER_LSA *pstruRouterLSA,*pstruTempRLSA;
	NetSim_PACKET *pstruControlpacket;
	int nLoop1=0,nInterfaceType,nLink_Id=0;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId,nLoop=0;
	int nInterfaceCount;
	bool bWanPort=false;
	NETWORK=pstruNETWORK;
	nDeviceid=pstruEventDetails->nDeviceId;
	nInterfaceCount=NETWORK->ppstruDeviceList[nDeviceid-1]->nNumOfInterface;
	NETWORK=pstruNETWORK;
	for(nLoop=0;nLoop<nInterfaceCount;nLoop++)
	{
		pstruRouterLSA=calloc(1,sizeof(struct stru_OSPF_Router_LSA));
		pstruRouterLSA->pstrEntry=calloc(1,sizeof(struct stru_RouterLSA_Entry));
		pstruRouterLSA->pstruHeader=calloc(1,sizeof(struct stru_OSPF_LSA_Header));
		pstruTempRLSA=((OSPF_ROUTER*)NETWORK->ppstruDeviceList[nDeviceid-1]->pstruNetworkLayer->RoutingVar)->pstruRouterLSA;
		pstruRouterLSA->pstruHeader=pstruTempRLSA->pstruHeader;
		pstruRouterLSA->pstrEntry=pstruTempRLSA->pstrEntry;
		nInterfaceId=nLoop+1;
		nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
		pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);

		pstruControlpacket->nSourceId=nDeviceid;
		pstruControlpacket->nPacketType=PacketType_Control;
		pstruControlpacket->nDestinationId=nConnectedDevId;
		pstruControlpacket->nTransmitterId=nDeviceid;
		pstruControlpacket->nReceiverId=nConnectedDevId;
		pstruControlpacket->nControlDataType=LINKSTATEUPDATE;
		pstruControlpacket->nPacketPriority=Priority_High;
		pstruControlpacket->nPacketId=0;

		pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
		pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
		pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
		pstruControlpacket->pstruAppData->dPayload=LSA_PACKET_SIZE;
		pstruControlpacket->pstruAppData->dOverhead=0;
		pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
		pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
		pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruRouterLSA;

		pstruControlpacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;

		//Get the Interface type 
		nInterfaceType=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->nInterfaceType;
		if(nInterfaceType==INTERFACE_WAN_ROUTER)
		{
			pstruControlpacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceid,nInterfaceId));
			pstruControlpacket->pstruNetworkData->szNextHopIp=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterfaceId));
			pstruControlpacket->pstruNetworkData->szDestIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterfaceId));
		}
		else
		{
			pstruControlpacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceid,nInterfaceId));
			pstruControlpacket->pstruNetworkData->szNextHopIp=STR_TO_IP4("224.0.0.5");	
			pstruControlpacket->pstruNetworkData->szDestIP=STR_TO_IP4("224.0.0.5");	
		}
		//Add the packets to the socket buffer
		if(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
		{
			fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nDeviceid-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruControlpacket,3);
			pstruEventDetails->dEventTime=pstruEventDetails->dEventTime;
			pstruEventDetails->dPacketSize=pstruControlpacket->pstruAppData->dPacketSize;
			pstruEventDetails->nApplicationId=0;
			pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
			pstruEventDetails->nDeviceId=nDeviceid;
			pstruEventDetails->nInterfaceId=0;
			pstruEventDetails->nEventType=TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType=0;
			pstruEventDetails->pPacket=NULL;
			fnpAddEvent(pstruEventDetails);	
		}
		else
		{
			fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruControlpacket,3);
		}	
		pstruEventDetails->nPacketId=0;
		pstruEventDetails->nApplicationId= 0;
		pstruEventDetails->nEventType=TIMER_EVENT;
		pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
		pstruEventDetails->dEventTime=pstruEventDetails->dEventTime+((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nLSRefreshtime;
		fnpAddEvent(pstruEventDetails);
	}
	return 0;
}
