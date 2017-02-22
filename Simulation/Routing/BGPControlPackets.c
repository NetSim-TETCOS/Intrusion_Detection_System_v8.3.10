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
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Parameters: Network structure, event details
  return type: int
  LOGIC: Creating Open message and forwarding it into the WAN interfaces of the router	
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	                                                                                                    
*/

int fn_NetSim_BGP_OpenMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	BGP_OPEN *pstruOpenMessage;
	DEVICE_ROUTER *pstruBGP,*pstruRouter;
	NetSim_PACKET *pstruNetSimPacket;
	NETSIM_ID nDeviceId,nInterfaceCount,nLoop,nBGPPortFlag=0;
	NETSIM_ID nLinkId,nConnectedDevId,nConnectedInterId;
	nDeviceId = pstruEventDetails->nDeviceId;
	nInterfaceCount = NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;	
	for(nLoop = 0;nLoop<nInterfaceCount;nLoop++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->szAddress && 
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->nInterfaceType==INTERFACE_WAN_ROUTER)
		{
			nLinkId = fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nLoop+1,&nConnectedDevId,&nConnectedInterId);
			pstruRouter = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
			if(pstruRouter && pstruRouter->RoutingProtocol[nLoop] == APP_PROTOCOL_BGP)
			{
				pstruOpenMessage=calloc(1,sizeof* pstruOpenMessage);
				pstruOpenMessage->pstruHeader = calloc(1,sizeof* pstruOpenMessage->pstruHeader);
				pstruOpenMessage->pstruHeader->Length = BGP_OPEN_MESSAGE_SIZE;
				pstruOpenMessage->pstruHeader->Type = 1;
				pstruOpenMessage->Version = BGP_VERSION;
				pstruOpenMessage->BGP_Identifier = IP_COPY(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->szAddress);
				pstruBGP = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
				pstruBGP->struBGP.bgpPeerOutTotalMessages++;
				pstruOpenMessage->Hold_Time = pstruBGP->struBGP.dHoldTimerConfigured;
				pstruOpenMessage->My_Autonomous_System = pstruBGP->struBGP.nAutonomous_system_number;
				pstruOpenMessage->Optional_Parameters_Length = 0;
				pstruNetSimPacket = fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
				pstruNetSimPacket->nSourceId=nDeviceId;
				pstruNetSimPacket->nDestinationId=nConnectedDevId;
				pstruNetSimPacket->nPacketType=PacketType_Control;
				pstruNetSimPacket->nTransmitterId=nDeviceId;
				pstruNetSimPacket->nReceiverId=nConnectedDevId;
				pstruNetSimPacket->nControlDataType=ctrlPacket_OPEN;
				pstruNetSimPacket->nPacketPriority=Priority_High;
				pstruNetSimPacket->nPacketId=0;
				//Assign the Application layer details of the packet
				pstruNetSimPacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dPayload=BGP_OPEN_MESSAGE_SIZE;
				pstruNetSimPacket->pstruAppData->dOverhead=0;
				pstruNetSimPacket->pstruAppData->dPacketSize=pstruNetSimPacket->pstruAppData->dPayload+pstruNetSimPacket->pstruAppData->dOverhead;
				pstruNetSimPacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_BGP;
				pstruNetSimPacket->pstruAppData->Packet_AppProtocol=pstruOpenMessage;
				pstruNetSimPacket->pstruAppData->nAppEndFlag =1;
				//Assign the Network layer details of the packet
				pstruNetSimPacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
				pstruNetSimPacket->pstruNetworkData->nTTL = 1;
				pstruNetSimPacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceId,nLoop+1));
				pstruNetSimPacket->pstruNetworkData->szDestIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterId));
				//Assign the Transport layer details of the packet
				pstruNetSimPacket->pstruTransportData->nSourcePort = rand()*65535/RAND_MAX;
				pstruNetSimPacket->pstruTransportData->nDestinationPort=BGP_DESTINATION_PORT;
				pstruNetSimPacket->pstruTransportData->nTransportProtocol=0;
				//Add the packets to the socket buffer

				if(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
					pstruEventDetails->dEventTime=pstruEventDetails->dEventTime;
					pstruEventDetails->dPacketSize=pstruNetSimPacket->pstruAppData->dPacketSize;
					pstruEventDetails->nApplicationId=0;
					pstruEventDetails->nProtocolId=pstruNetSimPacket->pstruTransportData->nTransportProtocol;
					pstruEventDetails->nDeviceId=nDeviceId;
					pstruEventDetails->nInterfaceId=0;
					pstruEventDetails->nEventType=TRANSPORT_OUT_EVENT;
					pstruEventDetails->nSubEventType=0;
					pstruEventDetails->pPacket=NULL;
					fnpAddEvent(pstruEventDetails);	
				}
				else
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
				}	
			}
		}

	}
	return 0;
}
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 KEEPALIVE messages are exchanged between peers often enough not to cause the Hold Timer to expire. 
 A reasonable maximum time between KEEPALIVE messages would be one third of the Hold Time interval.  
 KEEPALIVE messages MUST NOT be sent more frequently than one per second.
 
 If the negotiated Hold Time interval is zero, then periodic KEEPALIVE messages MUST NOT be sent.

 A KEEPALIVE message consists of only the message header and has a length of 19 octets.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	                                                                                                    
*/
int fn_NetSim_BGP_KeepAliveMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	BGP_KEEPALIVE *pstruKeepaliveMessage;
	DEVICE_ROUTER *pstruBGP;
	NetSim_PACKET *pstruNetSimPacket;
	NETSIM_ID nDeviceId,nInterfaceCount,nLoop;
	NETSIM_ID nLinkId,nConnectedDevId,nConnectedInterId;
	nDeviceId = pstruEventDetails->nDeviceId;
	nInterfaceCount = NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;
	for(nLoop = 0;nLoop<nInterfaceCount;nLoop++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->szAddress&& 
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->nInterfaceType==INTERFACE_WAN_ROUTER)
		{
			nLinkId = fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nLoop+1,&nConnectedDevId,&nConnectedInterId);
			if(NETWORK->ppstruDeviceList[nConnectedDevId-1]->pstruApplicationLayer->nAppRoutingProtocol == APP_PROTOCOL_BGP)
			{
				pstruKeepaliveMessage=calloc(1,sizeof* pstruKeepaliveMessage);
				pstruKeepaliveMessage->pstruHeader = calloc(1,sizeof* pstruKeepaliveMessage->pstruHeader);
				pstruKeepaliveMessage->pstruHeader->Length = BGP_KEEPALIVE_MESSAGE_SIZE;
				pstruKeepaliveMessage->pstruHeader->Type = 4;
				pstruBGP = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
				pstruBGP->struBGP.nBGPState = OpenConfirm;
				pstruBGP->struBGP.bgpPeerOutTotalMessages++;
				pstruNetSimPacket = fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
				pstruNetSimPacket->nSourceId=nDeviceId;
				pstruNetSimPacket->nDestinationId=nConnectedDevId;
				pstruNetSimPacket->nPacketType=PacketType_Control;
				pstruNetSimPacket->nTransmitterId=nDeviceId;
				pstruNetSimPacket->nReceiverId=nConnectedDevId;
				pstruNetSimPacket->nControlDataType=ctrlPacket_KEEPALIVE;
				pstruNetSimPacket->nPacketPriority=Priority_High;
				pstruNetSimPacket->nPacketId=0;
				//Assign the Application layer details of the packet
				pstruNetSimPacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dPayload=BGP_KEEPALIVE_MESSAGE_SIZE;
				pstruNetSimPacket->pstruAppData->dOverhead=0;
				pstruNetSimPacket->pstruAppData->dPacketSize=pstruNetSimPacket->pstruAppData->dPayload+pstruNetSimPacket->pstruAppData->dOverhead;
				pstruNetSimPacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_BGP;
				pstruNetSimPacket->pstruAppData->Packet_AppProtocol=pstruKeepaliveMessage;
				pstruNetSimPacket->pstruAppData->nAppEndFlag = 1;
				//Assign the Network layer details of the packet
				pstruNetSimPacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
				pstruNetSimPacket->pstruNetworkData->nTTL = 1;
				pstruNetSimPacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceId,nLoop+1));
				pstruNetSimPacket->pstruNetworkData->szDestIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterId));
				//Assign the Transport layer details of the packet
				pstruNetSimPacket->pstruTransportData->nSourcePort = rand()*65535/RAND_MAX;
				pstruNetSimPacket->pstruTransportData->nDestinationPort=BGP_DESTINATION_PORT;
				pstruNetSimPacket->pstruTransportData->nTransportProtocol=0;
				//Add the packets to the socket buffer
				if(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
					pstruEventDetails->dEventTime=pstruEventDetails->dEventTime;
					pstruEventDetails->dPacketSize=pstruNetSimPacket->pstruAppData->dPacketSize;
					pstruEventDetails->nApplicationId=0;
					pstruEventDetails->nProtocolId=pstruNetSimPacket->pstruTransportData->nTransportProtocol;
					pstruEventDetails->nDeviceId=nDeviceId;
					pstruEventDetails->nInterfaceId=0;
					pstruEventDetails->nEventType=TRANSPORT_OUT_EVENT;
					pstruEventDetails->nSubEventType=0;
					pstruEventDetails->pPacket=NULL;
					fnpAddEvent(pstruEventDetails);	
				}
				else
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
				}	
			}
		}
	}
	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nApplicationId= 0;
	pstruEventDetails->nEventType=TIMER_EVENT;
	pstruEventDetails->nSubEventType=BGP_KeepaliveTimer_Expires;
	pstruEventDetails->nProtocolId=APP_PROTOCOL_BGP;
	pstruEventDetails->dEventTime=pstruEventDetails->dEventTime+pstruBGP->struBGP.dKeepaliveTimerConfigured;
	fnpAddEvent(pstruEventDetails);

	return 0;
}
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 UPDATE messages are used to transfer routing information between BGP peers.  The information in the UPDATE message can be used to
 construct a graph that describes the relationships of the various Autonomous Systems.  By applying rules to be discussed, routing
 information loops and some other anomalies may be detected and removed from inter-AS routing.

 An UPDATE message is used to advertise feasible routes that share common path attributes to a peer, or to withdraw multiple unfeasible
 routes from service.  An UPDATE message MAY simultaneously advertise a feasible route and withdraw multiple unfeasible routes
 from service.  The UPDATE message always includes the fixed-size BGP header, and also includes the other fields, as shown below (note,
 some of the shown fields may not be present in every UPDATE message):

						   +-----------------------------------------------------+
						  |   Withdrawn Routes Length (2 octets)                |
						  +-----------------------------------------------------+
						  |   Withdrawn Routes (variable)                       |
						  +-----------------------------------------------------+
						  |   Total Path Attribute Length (2 octets)            |
						  +-----------------------------------------------------+
						  |   Path Attributes (variable)                        |
						  +-----------------------------------------------------+
						  |   Network Layer Reachability Information (variable) |
						  +-----------------------------------------------------+

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_BGP_UpdateMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	BGP_UPDATE *pstruUpdateMessage;
	DEVICE_ROUTER *pstruRouter;
	NLRI *pstruTempNLRI,*pstruTempNLRI1;
	BGP_ROUTING_TABLE *pstruBGPTable;
	DEVICE_ROUTER *pstruBGP;
	NetSim_PACKET *pstruNetSimPacket;
	NETSIM_ID nDeviceId,nInterfaceCount,nLoop,nBGPPortFlag=0;
	NETSIM_ID nLinkId,nConnectedDevId,nConnectedInterId;
	nDeviceId = pstruEventDetails->nDeviceId;
	nInterfaceCount = NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;
	for(nLoop = 0;nLoop<nInterfaceCount;nLoop++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->szAddress&& 
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nLoop]->nInterfaceType==INTERFACE_WAN_ROUTER
			)
		{
			nLinkId = fn_NetSim_Stack_GetConnectedDevice(nDeviceId,nLoop+1,&nConnectedDevId,&nConnectedInterId);
			pstruRouter = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
			if(pstruRouter && pstruRouter->RoutingProtocol[nLoop] == APP_PROTOCOL_BGP)
			{
				pstruBGPTable = ((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar))->pstruRoutingTables->pstruBGPTables->pstruBGPTable;
				pstruUpdateMessage=calloc(1,sizeof* pstruUpdateMessage);
				pstruUpdateMessage->pstruHeader = calloc(1,sizeof* pstruUpdateMessage->pstruHeader);
				pstruUpdateMessage->pstruPathAttribute = calloc(1,sizeof* pstruUpdateMessage->pstruPathAttribute);
				pstruUpdateMessage->pstruWithdrawn_routes = calloc(1,sizeof* pstruUpdateMessage->pstruPathAttribute);			
				pstruUpdateMessage->pstruHeader->Length = BGP_UPDATE_MESSAGE_SIZE;
				pstruUpdateMessage->pstruHeader->Type = 3;
				pstruUpdateMessage->Withdrawn_Routes_Length = 0;
				pstruUpdateMessage->Total_PathAttribute_Length = 0;
				pstruUpdateMessage->pstruPathAttribute->AttrFlags = 0;
				pstruTempNLRI = calloc(1,sizeof* pstruTempNLRI);
				pstruTempNLRI1 = pstruTempNLRI;
				while(pstruBGPTable)
				{
					pstruTempNLRI->Prefix = pstruBGPTable->bgpPeerRemoteAddr;
					pstruTempNLRI->bgpSubnetMask = pstruBGPTable->bgpSubnetMask;
					pstruTempNLRI->nASpathlength = pstruBGPTable->nASpathlength;
					pstruBGPTable = pstruBGPTable->pstruNextEntry;
					if(pstruBGPTable)
					{
						pstruTempNLRI->pstruNextNLRI = calloc(1,sizeof* pstruUpdateMessage->pstruNLRI);
						pstruTempNLRI = pstruTempNLRI->pstruNextNLRI;		
					}
				}
				pstruUpdateMessage->pstruNLRI = pstruTempNLRI1;
				pstruBGP = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
				pstruBGP->struBGP.bgpPeerOutTotalMessages++;
				pstruBGP->struBGP.bgpPeerOutUpdates++;
				pstruNetSimPacket = fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
				pstruNetSimPacket->nSourceId=nDeviceId;
				pstruNetSimPacket->nDestinationId=nConnectedDevId;
				pstruNetSimPacket->nPacketType=PacketType_Control;
				pstruNetSimPacket->nTransmitterId=nDeviceId;
				pstruNetSimPacket->nReceiverId=nConnectedDevId;
				pstruNetSimPacket->nControlDataType=ctrlPacket_UPDATE;
				pstruNetSimPacket->nPacketPriority=Priority_High;
				pstruNetSimPacket->nPacketId=0;
				//Assign the Application layer details of the packet
				pstruNetSimPacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
				pstruNetSimPacket->pstruAppData->dPayload=BGP_UPDATE_MESSAGE_SIZE;
				pstruNetSimPacket->pstruAppData->dOverhead=0;
				pstruNetSimPacket->pstruAppData->dPacketSize=pstruNetSimPacket->pstruAppData->dPayload+pstruNetSimPacket->pstruAppData->dOverhead;
				pstruNetSimPacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_BGP;
				pstruNetSimPacket->pstruAppData->Packet_AppProtocol=pstruUpdateMessage;
				pstruNetSimPacket->pstruAppData->nAppEndFlag =1;
				//Assign the Network layer details of the packet

				pstruNetSimPacket->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
				pstruNetSimPacket->pstruNetworkData->nTTL = 1;
				pstruNetSimPacket->pstruNetworkData->szSourceIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nDeviceId,nLoop+1));
				pstruNetSimPacket->pstruNetworkData->szDestIP=IP_COPY(fn_NetSim_Stack_GetIPAddressAsId(nConnectedDevId,nConnectedInterId));

				//Assign the Transport layer details of the packet
				pstruNetSimPacket->pstruTransportData->nSourcePort = rand()*65535/RAND_MAX;
				pstruNetSimPacket->pstruTransportData->nDestinationPort=BGP_DESTINATION_PORT;
				pstruNetSimPacket->pstruTransportData->nTransportProtocol=0;
				//Add the packets to the socket buffer

				if(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
					pstruEventDetails->dEventTime=pstruEventDetails->dEventTime;
					pstruEventDetails->dPacketSize=pstruNetSimPacket->pstruAppData->dPacketSize;
					pstruEventDetails->nApplicationId=0;
					pstruEventDetails->nProtocolId=pstruNetSimPacket->pstruTransportData->nTransportProtocol;
					pstruEventDetails->nDeviceId=nDeviceId;
					pstruEventDetails->nInterfaceId=0;
					pstruEventDetails->nEventType=TRANSPORT_OUT_EVENT;
					pstruEventDetails->nSubEventType=0;
					pstruEventDetails->pPacket=NULL;
					fnpAddEvent(pstruEventDetails);	
				}
				else
				{
					fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface->pstruSocketBuffer[0]),pstruNetSimPacket,3);
				}	
			}
		}
	}
	return 0;
}
/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A NOTIFICATION message is sent when an error condition is detected. The BGP connection is closed immediately after it is sent.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_BGP_NotificationMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	return 0;
}


