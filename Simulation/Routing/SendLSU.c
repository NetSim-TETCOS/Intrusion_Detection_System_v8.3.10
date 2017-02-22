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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function is used to forward the data to Network out and RIP packet to Transport in
1. Check the packet type is data or not
2.If it is data,Check the destination network address in the routing database
If the destination is present in the database, update the output port and Next hop IPaddress
3.if the destination ip address is not in the table then forward the packet to the default 
gateway
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
*/
int fn_NetSim_OSPF_LSUpacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails)
{
	LSU_PACKET *pstruLSUpacket;
	DEVICE_ROUTER *pstruRouter;
	OSPF_ROUTING_TABLE *pstruRoutingTable;
	NetSim_PACKET *pstruControlpacket,*pstruControlpacket_temp;
	LSDB *pstruLSDB;
	int nLink_Id=0,nLoop;
	NETSIM_ID nConnectedDevId=0,nDeviceid;
	NETSIM_ID nConnectedInterfaceId=0;
	NETSIM_ID nInterfaceId;
	double dBandwidth;
	ROUTERLSA_ENTRY *pstruTempEntry,*pstruNewEntry,*pstruCurrentEntry;
	int nInterfaceCount,nCost;
	ROUTER_LSA *pstruRouterLSA;
	NETWORK=pstruNETWORK;
	pstruControlpacket_temp=pstruEventDetails->pPacket;
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
				nInterfaceId = nLoop+1;
				pstruLSUpacket=calloc(1,sizeof(struct stru_OSPF_LinkStateUpdate_Packet));
				pstruLSUpacket->pstruHeader_LSU=calloc(1,sizeof(struct stru_OSPF_Packet_Header));
				pstruLSUpacket->pstruRouterLSA = calloc(1,sizeof(struct stru_OSPF_Router_LSA));
				pstruLSUpacket->pstruRouterLSA->pstruHeader = calloc(1,sizeof(struct stru_OSPF_LSA_Header));
				pstruLSDB=((DEVICE_ROUTER*)NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar)->pstruRoutingTables->pstruOSPFTables->pstruLSDB;
				pstruRouterLSA = pstruLSDB->pstruRLSA;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->LSType = pstruRouterLSA->pstruHeader->LSType;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->nLength = pstruRouterLSA->pstruHeader->nLength;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->nLS_Checksum = pstruRouterLSA->pstruHeader->nLS_Checksum;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->nLS_SequenceNumber = pstruRouterLSA->pstruHeader->nLS_SequenceNumber;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->nLSage = pstruRouterLSA->pstruHeader->nLSage;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->nOptions = pstruRouterLSA->pstruHeader->nOptions;
				pstruLSUpacket->pstruRouterLSA->pstruHeader->szAdvertisingRouter =IP_COPY(pstruRouterLSA->pstruHeader->szAdvertisingRouter);
				pstruLSUpacket->pstruRouterLSA->pstruHeader->szLinkStateID =IP_COPY(pstruRouterLSA->pstruHeader->szLinkStateID);
				pstruTempEntry = pstruRouterLSA->pstrEntry;	
				dBandwidth=NETWORK->ppstruNetSimLinks[nLink_Id-1]->puniMedProp.pstruWiredLink.dDataRateUp;
				nCost=(int)round(REFERENCE_BANDWIDTH/dBandwidth);
				
				pstruRoutingTable = ((DEVICE_ROUTER*)NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar)->pstruRoutingTables->pstruOSPFTables->pstruOSPF_RoutingTable;
				while(pstruRoutingTable)
				{
					pstruNewEntry = calloc(1,sizeof(struct stru_RouterLSA_Entry));
					pstruNewEntry->nMetric = pstruRoutingTable->nCost+nCost;
					pstruNewEntry->nTOS = pstruTempEntry->nTOS;
					pstruNewEntry->szLinkData = IP_COPY(pstruRoutingTable->szAddressMask);
					pstruNewEntry->prefix_len = pstruRoutingTable->prefix_len;
					pstruNewEntry->szLinkID = IP_COPY(pstruRoutingTable->szDestinationID);
					if(pstruLSUpacket->pstruRouterLSA->pstrEntry)
					{
						pstruCurrentEntry =  pstruLSUpacket->pstruRouterLSA->pstrEntry;
						while(pstruCurrentEntry->pstru_NextEntry)
							pstruCurrentEntry = pstruCurrentEntry->pstru_NextEntry;
						pstruCurrentEntry->pstru_NextEntry = pstruNewEntry;
					}
					else
					{
						pstruLSUpacket->pstruRouterLSA->pstrEntry = pstruNewEntry;
					}
					pstruRoutingTable = pstruRoutingTable->pstru_Router_NextEntry;
				}
				
				
				
				//Fill the header details in LSU packet
				pstruLSUpacket->pstruHeader_LSU->nVersion=((DEVICE_ROUTER*)(NETWORK->ppstruDeviceList[nDeviceid-1]->pstruApplicationLayer->routingVar))->uniInteriorRouting.struOSPF.nOSPFVersion;
				pstruLSUpacket->pstruHeader_LSU->Type=LINKSTATEUPDATE;
				pstruLSUpacket->pstruHeader_LSU->szRouterID=IP_COPY(NETWORK->ppstruDeviceList[nDeviceid-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress);
				pstruLSUpacket->pstruHeader_LSU->szAreaID=STR_TO_IP4("0.0.0.0");
				pstruLSUpacket->nLSAs=1;
				pstruLSDB->pstruRLSA->pstruHeader->nLS_SequenceNumber=pstruLSDB->pstruRLSA->pstruHeader->nLS_SequenceNumber+1;					
				nLink_Id=fn_NetSim_Stack_GetConnectedDevice(nDeviceid,nInterfaceId,&nConnectedDevId,&nConnectedInterfaceId);
				pstruControlpacket=fn_NetSim_Packet_CreatePacket(APPLICATION_LAYER);
				pstruControlpacket->nSourceId=nDeviceid;
				pstruControlpacket->nDestinationId=nConnectedDevId;
				pstruControlpacket->nTransmitterId=nDeviceid;
				pstruControlpacket->nReceiverId=nConnectedDevId;
				pstruControlpacket->nPacketType=PacketType_Control;
				pstruControlpacket->nControlDataType=LINKSTATEUPDATE;
				pstruControlpacket->nPacketPriority=Priority_High;
				pstruControlpacket->nPacketId=0;
				pstruControlpacket->pstruAppData->dArrivalTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dStartTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dEndTime=pstruEventDetails->dEventTime;
				pstruControlpacket->pstruAppData->dPayload=LSU_PACKETSIZE_WITHHEADER;
				pstruControlpacket->pstruAppData->dOverhead=0;
				pstruControlpacket->pstruAppData->dPacketSize=pstruControlpacket->pstruAppData->dPayload+pstruControlpacket->pstruAppData->dOverhead;
				pstruControlpacket->pstruAppData->nApplicationProtocol=APP_PROTOCOL_OSPF;
				pstruControlpacket->pstruAppData->Packet_AppProtocol=pstruLSUpacket;
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
			}
		}
	}
	return 0;
}
