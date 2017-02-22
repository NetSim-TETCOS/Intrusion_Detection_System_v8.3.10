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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This function is used to forward the data to Network out and RIP packet to Transport in
1. Check the packet type is data or not
2.If it is data,Check the destination network address in the routing database
If the destination is present in the database, update the output port and Next hop IPaddress
3.if the destination ip address is not in the table then forward the packet to the default 
gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_OSPF_SendDDPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	DD_PACKET *pstruDDpacket;
	NetSim_PACKET *pstruControlpacket;
	LSDB *pstruLSDB;
	OSPF_STATE *pstruOSPF;
	int nLink_Id=0;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId;
	int nInterfaceCount;
	bool bInit;
	NETWORK=pstruNETWORK;
	nDeviceid=pstruEventDetails->nDeviceId;
	nInterfaceCount=NETWORK->ppstruDeviceList[nDeviceid-1]->nNumOfInterface;
	nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket->pstruNetworkData->szDestIP);
	pstruOSPF=NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->interfaceVar;
	//Free the hello packet
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
	bInit=((OSPF_VAR*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->bInit;
	pstruDDpacket=calloc(1,sizeof(struct stru_OSPF_DatabaseDescription_Packet));
	pstruDDpacket->pstruHeader_DD=calloc(1,sizeof(struct stru_OSPF_Packet_Header));
	pstruDDpacket->pstruHeader=calloc(1,sizeof(struct stru_OSPF_LSA_Header));
	pstruLSDB=((DEVICE_ROUTER*)NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar)->pstruRoutingTables->pstruOSPFTables->pstruLSDB;
	pstruDDpacket->pstruHeader_DD->nVersion=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nOSPFVersion;
	pstruDDpacket->pstruHeader_DD->Type=DATABASE_DESCRIPTION;
	pstruDDpacket->pstruHeader_DD->szRouterID=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
	pstruDDpacket->pstruHeader_DD->szAreaID=STR_TO_IP4("0.0.0.0");
	pstruDDpacket->nInterfaceMTU=MTU;
	pstruDDpacket->bIbit=0;
	pstruDDpacket->bMbit=1;
	pstruDDpacket->nDDsequencenumber=pstruDDpacket->nDDsequencenumber+1;
	pstruDDpacket->pstruHeader->LSType = pstruLSDB->pstruRLSA->pstruHeader->LSType;
	pstruDDpacket->pstruHeader->nLength = pstruLSDB->pstruRLSA->pstruHeader->nLength;
	pstruDDpacket->pstruHeader->nLS_Checksum = pstruLSDB->pstruRLSA->pstruHeader->nLS_Checksum;
	pstruDDpacket->pstruHeader->nLS_SequenceNumber = pstruLSDB->pstruRLSA->pstruHeader->nLS_SequenceNumber;
	pstruDDpacket->pstruHeader->nLSage = pstruLSDB->pstruRLSA->pstruHeader->nLSage;
	pstruDDpacket->pstruHeader->nOptions = pstruLSDB->pstruRLSA->pstruHeader->nOptions;
	pstruDDpacket->pstruHeader->szAdvertisingRouter = IP_COPY(pstruLSDB->pstruRLSA->pstruHeader->szAdvertisingRouter);
	pstruDDpacket->pstruHeader->szLinkStateID = IP_COPY(pstruLSDB->pstruRLSA->pstruHeader->szLinkStateID);
	nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
	pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
	pstruControlpacket->nSourceId=nDeviceid;
	pstruControlpacket->nDestinationId=nConnectedDevId;
	pstruControlpacket->nTransmitterId=nDeviceid;
	pstruControlpacket->nReceiverId=nConnectedDevId;
	pstruControlpacket->nControlDataType=DATABASE_DESCRIPTION;
	pstruControlpacket->nPacketPriority=Priority_High;
	pstruControlpacket->nPacketId=0;
	pstruControlpacket->nPacketType=PacketType_Control;
	pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
	pstruControlpacket->pstruAppData->dPayload=DD_PACKETSIZE_WITHHEADER;
	pstruControlpacket->pstruAppData->dOverhead=0;
	pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
	pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
	pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruDDpacket;
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
	((OSPF_VAR*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->bInit=true;
	return 0;
}
