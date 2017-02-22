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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creating Hello packets and forwarding it into the WAN interfaces of the router					  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

int fn_NetSim_OSPF_Hellopacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	HELLO_PACKET *pstruHellopacket;
	NetSim_PACKET *pstruControlpacket;
	DEVICE_ROUTER *pstruRouter;
	OSPF_STATE *pstruOSPF;
	int nLink_Id=0;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId,nLoop=0;
	int nInterfaceCount;
	bool bWanPort=false;
	NETWORK=pstruNETWORK;
	nDeviceid=pstruEventDetails->nDeviceId;
	nInterfaceCount=NETWORK->ppstruDeviceList[nDeviceid-1]->nNumOfInterface;
	for(nLoop=0;nLoop<nInterfaceCount;nLoop++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->szAddress && 
			NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->nInterfaceType==INTERFACE_WAN_ROUTER)
		{
			nLink_Id = fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nLoop+1,&nConnectedDevId,&nConnectedInterfaceId);
			if(!nLink_Id)
				continue;
			pstruRouter = NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar;
			if(pstruRouter && pstruRouter->RoutingProtocol[nLoop] == APP_PROTOCOL_OSPF)
			{
				bWanPort=true;
				pstruOSPF=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->interfaceVar;
				fn_NetSim_OSPF_ChangeState(pstruOSPF,Waiting,1);
				fn_NetSim_OSPF_ChangeState(pstruOSPF,Start,2);

				pstruHellopacket=calloc(1,sizeof(struct stru_OSPF_Hello_Packet));
				pstruHellopacket->pstruHeader=calloc(1,sizeof(struct stru_OSPF_Packet_Header));
				pstruHellopacket->pstruNeighbor=calloc(1,sizeof(struct stru_NeighborIP));
				//Fill the header details in Hello packet
				pstruHellopacket->pstruHeader->nVersion=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nOSPFVersion;
				pstruHellopacket->pstruHeader->Type=HELLO;
				pstruHellopacket->pstruHeader->szRouterID=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->szAddress);

				pstruHellopacket->szNetworkMask=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nLoop]->szSubnetMask);
				pstruHellopacket->nHelloInterval=HELLO_INTERVAL;
				pstruHellopacket->nRouterDeadInterval=ROUTER_DEAD_INTERVAL;
				pstruHellopacket->nRtrPri=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nRtrPri;
				pstruHellopacket->szDesignatedRouter=STR_TO_IP4("0.0.0.0");
				pstruHellopacket->szBackupDesignatedRouter=STR_TO_IP4("0.0.0.0");
				nInterfaceId=nLoop+1;
				nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
				pstruHellopacket->pstruNeighbor->szNeighbor=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterfaceId));

				pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
				pstruControlpacket->nSourceId=nDeviceid;
				pstruControlpacket->nDestinationId=nConnectedDevId;
				pstruControlpacket->nPacketType=PacketType_Control;
				pstruControlpacket->nTransmitterId=nDeviceid;
				pstruControlpacket->nReceiverId=nConnectedDevId;
				pstruControlpacket->nControlDataType=HELLO;
				pstruControlpacket->nPacketPriority=Priority_High;
				pstruControlpacket->nPacketId=0;

				pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dPayload=HELLO_PACKETSIZE_WITHHEADER;
				pstruControlpacket->pstruAppData->dOverhead=0;
				pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
				pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
				pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruHellopacket;

				pstruControlpacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
				pstruControlpacket->pstruNetworkData->nTTL = 1;

				pstruControlpacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceid,nInterfaceId));
				pstruControlpacket->pstruNetworkData->szDestIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterfaceId));

				//Add the packets to the socket buffer

				if(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nDeviceid-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruControlpacket,3);
					pstruEventDetails->dEventTime=pstruEventDetails->dEventTime;
					pstruEventDetails->dPacketSize=pstruControlpacket->pstruAppData->dPacketSize;
					pstruEventDetails->nApplicationId=0;
					pstruEventDetails->nProtocolId=0;
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
			}
			else
			{
				//
			}
		}
	}
	if(bWanPort)
	{
		pstruEventDetails->nPacketId=0;
		pstruEventDetails->nApplicationId= 0;
		pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
		pstruEventDetails->nSubEventType=INTERFACEUP;
		pstruEventDetails->nProtocolId=APP_PROTOCOL_OSPF;
		pstruEventDetails->dEventTime=pstruEventDetails->dEventTime+HELLO_INTERVAL;
		fnpAddEvent(pstruEventDetails);
	}
	return 0;
}
