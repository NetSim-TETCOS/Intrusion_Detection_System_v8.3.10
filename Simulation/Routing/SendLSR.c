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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This function is used to forward the data to Network out and RIP packet to Transport in
1. Check the packet type is data or not
2.If it is data,Check the destination network address in the routing database
If the destination is present in the database, update the output port and Next hop IPaddress
3.if the destination ip address is not in the table then forward the packet to the default 
gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_OSPF_LSRpacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	LSR_PACKET *pstruLSRpacket;
	NetSim_PACKET *pstruControlpacket,*pstruControlpacket_temp;
	NETSIM_IPAddress szLinkIP,szAdvertisingIP;
	OSPF_STATE *pstruOSPF;
	DD_PACKET *pstruDD;
	int nInterfaceType,nLink_Id=0;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId;
	int nInterfaceCount;
	NETWORK=pstruNETWORK;
	pstruControlpacket_temp=pstruEventDetails->pPacket;
	pstruDD=pstruControlpacket_temp->pstruAppData->Packet_AppProtocol;
	szLinkIP = IP_COPY(pstruDD->pstruHeader->szLinkStateID);
	szAdvertisingIP = IP_COPY(pstruDD->pstruHeader->szAdvertisingRouter);
	nDeviceid=pstruEventDetails->nDeviceId;
	nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket->pstruNetworkData->szDestIP);
	//Free the Database description packet
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	nInterfaceCount=NETWORK->ppstruDeviceList[nDeviceid-1]->nNumOfInterface;
	pstruOSPF=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->interfaceVar;
	pstruLSRpacket=calloc(1,sizeof(struct stru_OSPF_LinkStateRequest_Packet));
	pstruLSRpacket->pstruHeader_LSR=calloc(1,sizeof(struct stru_OSPF_Packet_Header));
	pstruLSRpacket->pstruLSR=calloc(1,sizeof(struct stru_LSR));
	//Fill the header details in LSR packet
	pstruLSRpacket->pstruHeader_LSR->nVersion=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nOSPFVersion;
	pstruLSRpacket->pstruHeader_LSR->Type=LINKSTATEREQUEST;
	pstruLSRpacket->pstruHeader_LSR->szRouterID=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
	pstruLSRpacket->pstruHeader_LSR->szAreaID=STR_TO_IP4("0.0.0.0");
	pstruLSRpacket->pstruLSR->LSType=ROUTER_LSAs;
	pstruLSRpacket->pstruLSR->szLinkStateID=IP_COPY(szLinkIP);
	pstruLSRpacket->pstruLSR->szAdvertisingRouter=IP_COPY(szAdvertisingIP);
	nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
	pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
	pstruControlpacket->nSourceId=nDeviceid;
	pstruControlpacket->nDestinationId=nConnectedDevId;
	pstruControlpacket->nTransmitterId=nDeviceid;
	pstruControlpacket->nReceiverId=nConnectedDevId;
	pstruControlpacket->nPacketType=PacketType_Control;
	pstruControlpacket->nControlDataType=LINKSTATEREQUEST;
	pstruControlpacket->nPacketPriority=Priority_High;
	pstruControlpacket->nPacketId=0;
	pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dPayload=LSR_PACKETSIZE_WITHHEADER;
	pstruControlpacket->pstruAppData->dOverhead=0;
	pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
	pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
	pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruLSRpacket;
	pstruControlpacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
	nInterfaceType=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->nInterfaceType;
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
	IP_FREE(szLinkIP);
	IP_FREE(szAdvertisingIP);
	return 0;
}
