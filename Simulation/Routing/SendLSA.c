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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
			This function is used to forward the data to Network out and RIP packet to Transport in
			1. Check the packet type is data or not
			2.If it is data,Check the destination network address in the routing database
			If the destination is present in the database, update the output port and Next hop IPaddress
			3.if the destination ip address is not in the table then forward the packet to the default 
				gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_OSPF_SendLSAPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	LSA_PACKET *pstruLSApacket;
	NetSim_PACKET *pstruControlpacket,*pstruControlpacket_temp;
	OSPF_STATE *pstruOSPF;
	LSDB *pstruLSDB;
	int nLink_Id=0;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId;
	int nInterfaceCount;
	NETWORK=pstruNETWORK;
	pstruControlpacket_temp=pstruEventDetails->pPacket;
	nDeviceid=pstruEventDetails->nDeviceId;
	nInterfaceCount=NETWORK->ppstruDeviceList[nDeviceid-1]->nNumOfInterface;
	nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket->pstruNetworkData->szDestIP);
	pstruOSPF=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->interfaceVar;
	pstruLSApacket=calloc(1,sizeof(struct stru_OSPF_LinkStateAcknowledgement_Packet));
	pstruLSApacket->pstruHeader_OSPFHeader=calloc(1,sizeof(struct stru_OSPF_Packet_Header));
	pstruLSApacket->pstruHeader_LSA=calloc(1,sizeof(struct stru_OSPF_LSA_Header));
	pstruLSDB=((DEVICE_ROUTER*)NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar)->pstruRoutingTables->pstruOSPFTables->pstruLSDB;
	//Fill the header details in LSU packet
	pstruLSApacket->pstruHeader_OSPFHeader->nVersion=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nOSPFVersion;
	pstruLSApacket->pstruHeader_OSPFHeader->Type=LINKSTATEACKNOWLEDGEMENT;
	pstruLSApacket->pstruHeader_OSPFHeader->szRouterID=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
	pstruLSApacket->pstruHeader_OSPFHeader->szAreaID=STR_TO_IP4("0.0.0.0");
	//Free the Linkstate update packet
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);	
	nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
	pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
	pstruControlpacket->nSourceId=nDeviceid;
	pstruControlpacket->nDestinationId=nConnectedDevId;
	pstruControlpacket->nTransmitterId=nDeviceid;
	pstruControlpacket->nPacketType=PacketType_Control;
	pstruControlpacket->nReceiverId=nConnectedDevId;
	pstruControlpacket->nControlDataType=LINKSTATEACKNOWLEDGEMENT;
	pstruControlpacket->nPacketPriority=Priority_High;
	pstruControlpacket->nPacketId=0;
	pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dPayload=LSACK_PACKETSIZE_WITHHEADER;
	pstruControlpacket->pstruAppData->dOverhead=0;
	pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
	pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
	pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruLSApacket;
	pstruControlpacket->pstruNetworkData->nTTL = 1;
	pstruControlpacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
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
	return 0;
}
