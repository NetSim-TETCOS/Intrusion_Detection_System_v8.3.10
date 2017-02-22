/************************************************************************************
* Copyright (C) 2014                                                               *
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
#include "List.h"
#include "IP.h"
#include "Protocol.h"
#include "NetSim_utility.h"

_declspec(dllexport) int fn_NetSim_IP_VPN_Init();
_declspec(dllexport) int fn_NetSim_IP_FirewallConfig(NETSIM_ID nDeviceId);
NETWORK_LAYER_PROTOCOL fnGetLocalNetworkProtocol(NetSim_EVENTDETAILS* pstruEventDetails);
int fn_NetSim_IP_RoutePacketViaStaticEntry();
int fn_NetSim_IP_RoutePacket();
int freeVPN(void* vpn);
int freeDNS(void* dns);
int freeVPNPacket(void* vpnPacket);
void* copyVPNPacket(void* vpnPacket);


//ICMP function
_declspec(dllexport) int fn_NetSim_IP_ICMP_GenerateDstUnreachableMsg();
_declspec(dllexport) int fn_NetSim_IP_ICMP_EchoRequest();
_declspec(dllexport) int fn_NetSim_IP_ICMP_EchoReply();
_declspec(dllexport) int fn_NetSim_IP_ICMP_ProcessRouterAdvertisement();
_declspec(dllexport) int fn_NetSim_IP_ICMP_ProcessDestUnreachableMsg();
_declspec(dllexport) int fn_NetSim_IP_ICMP_Init();
_declspec(dllexport) int fn_NetSim_IP_ICMP_POLL();
_declspec(dllexport) int fn_NetSim_IP_ICMP_AdvertiseRouter();
_declspec(dllexport) int ICMP_CHECKSTATE(NETSIM_IPAddress ip);


_declspec(dllexport) int fn_NetSim_IP_Firewall(NETSIM_ID nDeviceId,NetSim_PACKET* packet);
_declspec(dllexport) int fn_NetSim_IP_VPN_Run();
int fn_NetSim_IP_ConfigStaticIPTable(char* szVal);

/**
This functon initializes the IP parameters. 
*/
_declspec(dllexport) int fn_NetSim_IP_Init(struct stru_NetSim_Network *NETWORK_Formal,
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,
	char *pszAppPath_Formal,
	char *pszWritePath_Formal,
	int nVersion_Type,
	void **fnPointer)
{
	NETSIM_ID loop;
	if(nVersion_Type/10 != VERSION)
	{
		printf("IP---Version number mismatch\nDll Version=%d\nNetSim Version=%d\nFileName=%s\nLine=%d\n",VERSION,nVersion_Type/10,__FILE__,__LINE__);
		exit(0);
	}
	fnDNS=dns_query;
	ipMetrics = calloc(NETWORK->nDeviceCount,sizeof* ipMetrics);
	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer)
		{
			NETSIM_ID nInterface;
			unsigned int i;
			IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->ipVar;
			devVar->nGatewayId = calloc(devVar->nGatewayCount,sizeof* devVar->nGatewayId);
			for(i=0;i<devVar->nGatewayCount;i++)
				devVar->nGatewayId[i] = fn_NetSim_Stack_GetDeviceId_asIP(devVar->GatewayIPAddress[i],&nInterface);
			if(devVar && devVar->nFirewallStatus)
			{
				//read the firewall info
				fn_NetSim_IP_FirewallConfig(loop+1);
			}
			ipMetrics[loop] = calloc(1,sizeof* ipMetrics[loop]);
			ipMetrics[loop]->nDeviceId = fn_NetSim_Stack_GetConfigIdOfDeviceById(loop+1);
		}
	}

	//Initiallize the ICMP
	fn_NetSim_IP_ICMP_Init();
	//Initiallize the VPN
	fn_NetSim_IP_VPN_Init();
	return 1;
}
/**	
This function is called by NetworkStack.dll, whenever the event gets triggered	
inside the NetworkStack.dll for IP.It includes NETWORK_OUT,NETWORK_IN and TIMER_EVENT.		
*/
_declspec(dllexport) int fn_NetSim_IP_Run()
{
	switch(pstruEventDetails->nEventType)
	{
	case NETWORK_OUT_EVENT:
		{
			NetSim_PACKET* packet = pstruEventDetails->pPacket;
			NETWORK_LAYER_PROTOCOL nLocalNetworkProtcol;
			NETSIM_IPAddress dest;
			nLocalNetworkProtcol = fnGetLocalNetworkProtocol(pstruEventDetails);
			if(nLocalNetworkProtcol)
			{
				fnCallProtocol(nLocalNetworkProtcol);
				return 0;
			}
			if(packet->pstruNetworkData->nTTL == 0)
			{
				//TTL expire drop the packet
				packet->nPacketStatus = PacketStatus_TTL_Expired;
				fn_NetSim_WritePacketTrace(packet);
				fn_NetSim_Packet_FreePacket(packet);
				pstruEventDetails->pPacket=NULL;
				ipMetrics[pstruEventDetails->nDeviceId-1]->nTTLDrop++;
				return 0;
			}
			//set the time
			packet->pstruNetworkData->dStartTime = pstruEventDetails->dEventTime;
			packet->pstruNetworkData->dArrivalTime = pstruEventDetails->dEventTime;
			//Set the payload
			if(packet->pstruTransportData)
				packet->pstruNetworkData->dPayload = packet->pstruTransportData->dPacketSize;
			//Call NAT
			dest = packet->pstruNetworkData->szDestIP;
			fn_NetSim_NAT_NetworkOut(pstruEventDetails->nDeviceId,packet);

			if(packet->pstruNetworkData->szNextHopIp == NULL)
			{
				//First route via static ip route table
				fn_NetSim_IP_RoutePacketViaStaticEntry();
			}
			if(packet->pstruNetworkData->szNextHopIp == NULL)
			{
				//routing function
				//First route via routing function
				if(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->routerFunction)
					DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->routerFunction();
				packet = pstruEventDetails->pPacket;
				if(packet && packet->pstruNetworkData->szNextHopIp == NULL)
				{
					//Route via routing protocol
					if(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId)
						fnCallProtocol(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId);

				}
				packet = pstruEventDetails->pPacket;
				if(packet && packet->pstruNetworkData->szNextHopIp == NULL)
				{
					//Route via IP forwarding table
					fn_NetSim_IP_RoutePacket();
				}
				packet = pstruEventDetails->pPacket;
				if(packet && packet->pstruNetworkData->szNextHopIp == NULL)
				{
					ipMetrics[pstruEventDetails->nDeviceId-1]->nPacketDiscarded++;
					//Generate ICMP dst unreachable message
					fn_NetSim_IP_ICMP_GenerateDstUnreachableMsg();
					//fn_NetSim_Packet_FreePacket(packet); //routing fails
					packet = NULL;
				}
				if(!packet)
					return 0; //routing fails
			}
			packet->pstruNetworkData->szDestIP = dest;
			//check for firewall
			if(!fn_NetSim_IP_Firewall(pstruEventDetails->nDeviceId,packet))
			{
				ipMetrics[pstruEventDetails->nDeviceId-1]->nFirewallBlocked++;
				fn_NetSim_Packet_FreePacket(packet);
				pstruEventDetails->pPacket=NULL;
				return 0;
			}
			//IP function
			//Add IP header
			if(!pstruEventDetails->nInterfaceId)
				pstruEventDetails->nInterfaceId = fn_NetSim_Stack_GetInterfaceIdFromIP(pstruEventDetails->nDeviceId,packet->pstruNetworkData->szGatewayIP);


			packet->pstruNetworkData->dOverhead += IPV4_HEADER_SIZE;
			packet->pstruNetworkData->dPacketSize = packet->pstruNetworkData->dOverhead +
				packet->pstruNetworkData->dPayload;
			//Set the end time
			packet->pstruNetworkData->dEndTime = pstruEventDetails->dEventTime;
			packet->pstruNetworkData->nNetworkProtocol = DEVICE_INTERFACE(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)->nProtocolId;
			packet->nTransmitterId = pstruEventDetails->nDeviceId;

			if(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->nLocalNetworkProtocol==PROTOCOL_VPN)
				fn_NetSim_IP_VPN_Run();
			if(pstruEventDetails->pPacket==NULL)
				return 0;
			//Increment the count
			ipMetrics[pstruEventDetails->nDeviceId-1]->nPacketSent++;

			if(wireshark_trace.convert_sim_to_real_packet)
			{
				wireshark_trace.convert_sim_to_real_packet(packet,
					wireshark_trace.pcapWriterlist[pstruEventDetails->nDeviceId-1],
					pstruEventDetails->dEventTime);
			}

			if(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->nLocalNetworkProtocol)
			{
				//Call the local network protcol
				fnCallProtocol(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->nLocalNetworkProtocol);
			}
			else
			{
				NetSim_BUFFER* buffer = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->pstruAccessInterface->pstruAccessBuffer;
				packet->pstruMacData->szSourceMac = _strdup(fn_NetSim_Stack_GetMacAddressFromIP(packet->pstruNetworkData->szGatewayIP));
				packet->pstruMacData->szDestMac = _strdup(fn_NetSim_Stack_GetMacAddressFromIP(packet->pstruNetworkData->szNextHopIp));


				if(!fn_NetSim_GetBufferStatus(buffer))
				{
					//Add the MAC out event
					pstruEventDetails->dPacketSize = packet->pstruNetworkData->dPacketSize;
					if(packet->pstruAppData)
					{
						pstruEventDetails->nApplicationId = packet->pstruAppData->nApplicationId;
						pstruEventDetails->nSegmentId = packet->pstruAppData->nSegmentId;
					}
					pstruEventDetails->nEventType = MAC_OUT_EVENT;
					pstruEventDetails->nPacketId = packet->nPacketId;
					pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->pPacket = NULL;
					pstruEventDetails->szOtherDetails = NULL;
					fnpAddEvent(pstruEventDetails);
				}
				fn_NetSim_Packet_AddPacketToList(buffer,packet,0);
			}
		}
		break;
	case NETWORK_IN_EVENT:
		{
			NetSim_PACKET* packet = pstruEventDetails->pPacket;
			if(pstruEventDetails->nInterfaceId && NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->nLocalNetworkProtocol)
			{
				//Call the local network protcol
				fnCallProtocol(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->nLocalNetworkProtocol);
			}
			if(pstruEventDetails->pPacket==NULL)
				return 0;
			packet = pstruEventDetails->pPacket;
			//Decrease the TTL
			packet->pstruNetworkData->nTTL--;
			if(wireshark_trace.convert_sim_to_real_packet)
			{
				wireshark_trace.convert_sim_to_real_packet(packet,
					wireshark_trace.pcapWriterlist[pstruEventDetails->nDeviceId-1],
					pstruEventDetails->dEventTime);
			}

			//Reduce the IP overhead
			packet->pstruNetworkData->dOverhead -= IPV4_HEADER_SIZE;
			packet->pstruNetworkData->dPacketSize -= IPV4_HEADER_SIZE;
			if((((IP_DEVVAR*)DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipVar)->nVPNStatus == VPN_SERVER ||
				((IP_DEVVAR*)DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipVar)->nVPNStatus == VPN_CLIENT) &&
				packet->pstruNetworkData->nPacketFlag==PACKET_VPN)
				fn_NetSim_IP_VPN_Run();
			packet = pstruEventDetails->pPacket;
			if(!packet)
				return 0;
			//call routing protocol
			if(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId)
				fnCallProtocol(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId);
			packet=pstruEventDetails->pPacket;
			if(pstruEventDetails->pPacket)
				switch(pstruEventDetails->pPacket->nControlDataType)
			{
				case PACKET_ICMP_ECHORequest:
					fn_NetSim_IP_ICMP_EchoRequest();
					break;
				case PACKET_ICMP_ECHOReply:
					fn_NetSim_IP_ICMP_EchoReply();
					break;
				case PACKET_ROUTER_ADVERTISEMENT:
					fn_NetSim_IP_ICMP_ProcessRouterAdvertisement();
					break;
				case PACKET_ICMP_DstUnreachableMsg:
					fn_NetSim_IP_ICMP_ProcessDestUnreachableMsg();
					break;
			}
			packet = pstruEventDetails->pPacket;
			if(packet)
			{
				//Call NAT
				//fn_NetSim_NAT_NetworkIn(pstruEventDetails->nDeviceId,packet);
				if(packet->nDestinationId == pstruEventDetails->nDeviceId ||
					packet->nDestinationId == 0/*Broadcast*/)
				{
					//Add transport in event
					pstruEventDetails->dPacketSize = packet->pstruNetworkData->dPacketSize;
					pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,packet);
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->szOtherDetails = NULL;
					fnpAddEvent(pstruEventDetails);

					//Increment the received count
					ipMetrics[pstruEventDetails->nDeviceId-1]->nPacketReceived++;

				}
				else
				{
					//Add network out event to reroute the packet
					packet->pstruNetworkData->szNextHopIp = NULL;
					packet->pstruNetworkData->szGatewayIP = NULL;
					pstruEventDetails->dPacketSize = packet->pstruNetworkData->dPacketSize;
					pstruEventDetails->nEventType = NETWORK_OUT_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nProtocolId = NW_PROTOCOL_IPV4;
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->szOtherDetails = NULL;
					fnpAddEvent(pstruEventDetails);

					//Increment the forwarded count
					ipMetrics[pstruEventDetails->nDeviceId-1]->nPacketForwarded++;
				}
			}
		}
		break;
	case TIMER_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case EVENT_ICMP_POLL:
				fn_NetSim_IP_ICMP_POLL();
				break;
			case EVENT_ADVERTISE_ROUTER:
				fn_NetSim_IP_ICMP_AdvertiseRouter();
				break;
			default:
				{
					char error[BUFSIZ];
					sprintf(error,"Unknown sub event %d for IP",pstruEventDetails->nSubEventType);
					fnNetSimError(error);
				}
				break;
			}
		}
		break;
	default:
		{
			char error[BUFSIZ];
			sprintf(error,"Unknown event %d for IP",pstruEventDetails->nEventType);
			fnNetSimError(error);
		}
		break;
	}
	return 1;
}
/**
This function is called by NetworkStack.dll, once simulation end to free the 
allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_IP_Finish()
{
	NETSIM_ID i;
	for(i=0;i<NETWORK->nDeviceCount;i++)
	{
		if(DEVICE_NWLAYER(i+1))
		{
			IP_ROUTINGTABLE* table=DEVICE_NWLAYER(i+1)->ipRoutingTables;
			IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[i]->pstruNetworkLayer->ipVar;
			if(devVar)
			{
				free(devVar->blockedIP);
				free(devVar->firewallConfig);
				free(devVar->GatewayIPAddress);
				free(devVar->nGatewayId);
				free(devVar->nGatewayState);
				free(devVar->nInterfaceId);
				freeVPN(devVar->vpn);
				freeDNS(devVar->dnsList);
				free(devVar);
			}
			while(table)
			{
				LIST_FREE(&table,table);
			}
		}
		free(ipMetrics[i]);
	}
	free(ipMetrics);
	return 1;
}
/**
This function is called by NetworkStack.dll, while writing the evnt trace 
to get the sub event as a string.
*/
_declspec(dllexport) char* fn_NetSim_IP_Trace(NETSIM_ID nSubeventid)
{
	switch(nSubeventid)
	{
	case EVENT_ICMP_POLL:
		return "ICMP_POLL";
	case EVENT_ADVERTISE_ROUTER:
		return "ICMP_Advertise_Router";
	default:
		return "IP_UNKNOWN_SUBEVENT";
	}
}
/**
This function is called by NetworkStack.dll, while configuring the device 
NETWORK layer for IP protocol.	
*/
_declspec(dllexport) int fn_NetSim_IP_Configure(void** var)
{
	IP_ROUTINGTABLE* ipRoutingTable;
	FILE* fpConfigLog;
	void* xmlNetSimNode;
	NETSIM_ID nDeviceId=0;
	NETSIM_ID nInterfaceId=0;
	LAYER_TYPE nLayerType=0;
	char* szVal;
	IP_DEVVAR* devVar;
	fpConfigLog = var[0];
	xmlNetSimNode = var[2];
	if(var[3])
		nDeviceId = *((NETSIM_ID*)var[3]);
	if(var[4])
		nInterfaceId = *((NETSIM_ID*)var[4]);
	if(var[5])
		nLayerType = *((LAYER_TYPE*)var[5]);
	if(nDeviceId)
	{
		devVar=NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipVar;
		if(devVar==NULL)
		{
			devVar=calloc(1,sizeof* devVar);
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipVar = devVar;
		}
	}
	if(nDeviceId && nInterfaceId)
	{
		int iptype=0;
		NetSim_BUFFER* pstruBuffer;
		int nDeviceType;
		nDeviceType = DEVICE_TYPE(nDeviceId);
		if(nDeviceType == ROUTER)
		{
			pstruBuffer = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->pstruAccessInterface->pstruAccessBuffer;
			//Configure the Buffer size
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"BUFFER_SIZE",1);
			if(szVal)
				pstruBuffer->dMaxBufferSize = atoi(szVal);
			else
				pstruBuffer->dMaxBufferSize=8;//8 MB
			free(szVal);

			//Configure the Scheduling type
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"SCHEDULING_TYPE",1);
			if(szVal)
			{
				_strupr(szVal);
				if(strcmp(szVal,"FIFO")==0)
					pstruBuffer->nSchedulingType=SCHEDULING_FIFO;
				else if(strcmp(szVal,"PRIORITY")==0)
					pstruBuffer->nSchedulingType=SCHEDULING_PRIORITY;
				else if(strcmp(szVal,"ROUND ROBIN")==0)
					pstruBuffer->nSchedulingType=SCHEDULING_ROUNDROBIN;
				else if(strcmp(szVal,"WFQ")==0)
					pstruBuffer->nSchedulingType=SCHEDULING_WFQ;
				else
					pstruBuffer->nSchedulingType=SCHEDULING_FIFO;
			}
			else
				pstruBuffer->nSchedulingType=SCHEDULING_FIFO;
			free(szVal);
		}
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->nProtocolId == NW_PROTOCOL_IPV4)
			iptype = 4;
		else if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->nProtocolId == NW_PROTOCOL_IPV6)
			iptype = 6;
		//Configure the IP address
		szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"IP_ADDRESS",1);
		NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress = STR_TO_IP(szVal,iptype);
		free(szVal);

		if(iptype == 4)
		{
			//Configure the subnet mask
			szVal= fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"SUBNET_MASK",1);
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szSubnetMask = STR_TO_IP4(szVal);
			free(szVal);
		}
		else if(iptype == 6)
		{
			//Configure the subnet mask
			szVal= fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"PREFIX_LENGTH",1);
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->prefix_len = atoi(szVal);
			free(szVal);
		}
		else
			return -1;

		//Configure the default gateway
		szVal= fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"DEFAULT_GATEWAY",0);
		if(szVal)
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szDefaultGateWay = STR_TO_IP(szVal,iptype);
		free(szVal);

		ipRoutingTable = IPROUTINGTABLE_ALLOC();
		ipRoutingTable->gateway = on_link;
		ipRoutingTable->Interface = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress;
		ipRoutingTable->nInterfaceId = nInterfaceId;
		ipRoutingTable->Metric = ONLINK_METRIC;
		ipRoutingTable->netMask = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szSubnetMask;
		ipRoutingTable->networkDestination = IP_NETWORK_ADDRESS(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress,
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szSubnetMask,
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->prefix_len);
		ipRoutingTable->prefix_len = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->prefix_len;

		list_add(&(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipRoutingTables),ipRoutingTable,ipRoutingTable->ele->offset,iptable_add_check);

		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szDefaultGateWay &&
			IP_COMPARE(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress,
			NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szDefaultGateWay))
		{
			ipRoutingTable = IPROUTINGTABLE_ALLOC();
			ipRoutingTable->gateway = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szDefaultGateWay;
			ipRoutingTable->Interface = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress;
			ipRoutingTable->nInterfaceId = nInterfaceId;
			ipRoutingTable->Metric = DEFAULT_METRIC;
			if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress->type == 4)
			{
				ipRoutingTable->netMask = STR_TO_IP4("0.0.0.0");
				ipRoutingTable->networkDestination = STR_TO_IP4("0.0.0.0");
			}
			else if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress->type == 6)
			{
				ipRoutingTable->prefix_len = 0;
				ipRoutingTable->networkDestination = STR_TO_IP6("0:0:0:0:0:0:0:0");
			}
			ipRoutingTable->type = RoutingType_DEFAULT;
			list_add(&(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipRoutingTables),ipRoutingTable,ipRoutingTable->ele->offset,iptable_add_check);
			devVar->nGatewayCount++;
			devVar->GatewayIPAddress = realloc(devVar->GatewayIPAddress,devVar->nGatewayCount*(sizeof* devVar->GatewayIPAddress));
			devVar->GatewayIPAddress[devVar->nGatewayCount-1] = IP_COPY(ipRoutingTable->gateway);
			devVar->nGatewayState = realloc(devVar->nGatewayState,devVar->nGatewayCount*(sizeof* devVar->nGatewayState));
			devVar->nGatewayState[devVar->nGatewayCount-1] = GATEWAYSTATE_UP;
			devVar->nInterfaceId = realloc(devVar->nInterfaceId,devVar->nGatewayCount*(sizeof* devVar->nInterfaceId));
			devVar->nInterfaceId[devVar->nGatewayCount-1] = nInterfaceId;			
		}
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->nInterfaceType != INTERFACE_WAN_ROUTER)
		{
			//Add entry for broadcast
			ipRoutingTable = IPROUTINGTABLE_ALLOC();
			ipRoutingTable->gateway = on_link;
			ipRoutingTable->Interface = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress;
			ipRoutingTable->nInterfaceId = nInterfaceId;
			ipRoutingTable->Metric = DEFAULT_METRIC;
			if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress->type == 4)
			{
				ipRoutingTable->netMask = STR_TO_IP4("255.255.255.255");
				ipRoutingTable->networkDestination = STR_TO_IP4("255.255.255.255");
			}
			else if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->szAddress->type == 6)
			{
				ipRoutingTable->networkDestination = STR_TO_IP6("FF00:0:0:0:0:0:0:0");
				ipRoutingTable->prefix_len = 8;
			}
			list_add(&(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipRoutingTables),ipRoutingTable,ipRoutingTable->ele->offset,iptable_add_check);
		}
	}
	else if(nDeviceId)
	{
		//Configure the firewall status
		szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"FIREWALL_STATUS",0);
		if(szVal && _strupr(szVal) && !strcmp(szVal,"TRUE"))
		{
			devVar->nFirewallStatus=1;
			free(szVal);
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"FIREWALL_CONFIG",1);
			if(szVal)
				devVar->firewallConfig=szVal;
			else
				devVar->nFirewallStatus=0;
		}
		else
			free(szVal);
		//Configure the ICMP property
		szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"ICMP_CONTINUOUS_POLLING_TIME",0);
		if(szVal)
		{
			devVar->nICMPPollingTime = atoi(szVal);
			free(szVal);
		}
		szVal=fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"ROUTER_ADVERTISEMENT",0);
		if(szVal)
		{
			_strupr(szVal);
			if(!strcmp(szVal,"TRUE"))
				devVar->nRouterAdvertisementFlag = 1;
			free(szVal);
		}
		if(devVar->nRouterAdvertisementFlag)
		{
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"ROUTER_ADVERTISEMENT_MIN_INTERVAL",0);
			if(szVal)
			{
				devVar->nRouterAdverMinInterval = atoi(szVal);
				free(szVal);
			}
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"ROUTER_ADVERTISEMENT_MAX_INTERVAL",0);
			if(szVal)
			{
				devVar->nRouterAdverMaxInterval = atoi(szVal);
				free(szVal);
			}
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"ROUTER_ADVERTISEMENT_LIFE_TIME",0);
			if(szVal)
			{
				devVar->nRouterAdverLifeTime = atoi(szVal);
				free(szVal);
			}
		}
		//Configure the VPN
		szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"VPN_STATUS",0);
		if(szVal)
		{
			_strupr(szVal);
			if(!strcmp(szVal,"SERVER"))
			{
				devVar->nVPNStatus=VPN_SERVER;
				free(szVal);
				szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"IP_POOL_START",1);
				if(szVal)
				{
					devVar->ipPoolStart=STR_TO_IP4(szVal);
				}
				else
					devVar->nVPNStatus=VPN_DISABLE;
				free(szVal);
				szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"IP_POOL_END",1);
				if(szVal)
				{
					devVar->ipPoolEnd=STR_TO_IP4(szVal);
				}
				else
					devVar->nVPNStatus=VPN_DISABLE;
				free(szVal);
				szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"IP_POOL_MASK",1);
				if(szVal)
				{
					devVar->ipPoolMask=STR_TO_IP4(szVal);
				}
				else
					devVar->nVPNStatus=VPN_DISABLE;
				free(szVal);
			}
			else if(!strcmp(szVal,"CLIENT"))
			{
				devVar->nVPNStatus=VPN_CLIENT;
				free(szVal);
				szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"SERVER_IP",1);
				if(szVal)
				{
					devVar->serverIP=STR_TO_IP4(szVal);
				}
				else
					devVar->nVPNStatus=VPN_DISABLE;
				free(szVal);
			}
			else
				free(szVal);
		}
	}
	else
	{
		//configure the static ip forwardng table
		void* child = fn_NetSim_xmlGetChildElement(xmlNetSimNode,"STATIC_IP_FORWARDING_TABLE",0);
		if(child)
		{
			char* szVal = fn_NetSim_xmlConfig_GetVal(child,"FILE",0);
			if(szVal)
			{
				fn_NetSim_IP_ConfigStaticIPTable(szVal);
			}
		}
	}
	return 1;
}
_declspec(dllexport) int fn_NetSim_IP_ConfigurePrimitives(void* var)
{
	return 1;
}
/** 
This function is to free the memory allocated for packets of this protocol.
*/
_declspec(dllexport) int fn_NetSim_IP_FreePacket(NetSim_PACKET* pstruPacket)
{
	if(pstruPacket->pstruNetworkData->nPacketFlag==PACKET_VPN)
	{
		freeVPNPacket(pstruPacket->pstruNetworkData->Packet_NetworkProtocol);
	}
	return 1;
}
/**
This function is to copy the IP information from source to the destination.
*/
_declspec(dllexport) int fn_NetSim_IP_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	if(pstruSrcPacket->pstruNetworkData->nPacketFlag==PACKET_VPN)
		pstruDestPacket->pstruNetworkData->Packet_NetworkProtocol=copyVPNPacket(pstruSrcPacket->pstruNetworkData->Packet_NetworkProtocol);
	return 1;
}
/**
This function write the Metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_IP_Metrics(char* szMetrics)
{
	NETSIM_ID loop;
	FILE* fp = fopen(szMetrics,"a+");
	fprintf(fp,"\n\n#IP Metrics\n");
	fprintf(fp,"Device ID\tPacket Sent\tPacket Received\tPacket Forwarded\tPacket Discarded\tTTL Expire\tFirewall Blocked\n");
	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(ipMetrics[loop])
		{
			fprintf(fp,"%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				ipMetrics[loop]->nDeviceId,
				ipMetrics[loop]->nPacketSent,
				ipMetrics[loop]->nPacketReceived,
				ipMetrics[loop]->nPacketForwarded,
				ipMetrics[loop]->nPacketDiscarded,
				ipMetrics[loop]->nTTLDrop,
				ipMetrics[loop]->nFirewallBlocked);
		}
	}
	fprintf(fp,"\n\n#IP Forwarding Table\n");
	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer &&
			NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->ipRoutingTables)
		{
			IP_ROUTINGTABLE* routeTable=NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->ipRoutingTables;
			fprintf(fp,"##%s\n",NETWORK->ppstruDeviceList[loop]->szDeviceName);
			fprintf(fp,"Network Destination\tNetMask/Prefix_len\tGateway\tInterface\tMetric\tType\n");
			while(routeTable)
			{
				char ipStr[_NETSIM_IP_LEN];
				IP_TO_STR(routeTable->networkDestination,ipStr);
				fprintf(fp,"%s\t",ipStr);
				if(routeTable->networkDestination->type == 4)
				{
					IP_TO_STR(routeTable->netMask,ipStr);
					fprintf(fp,"%s\t",ipStr);
				}
				else if(routeTable->networkDestination->type == 6)
				{
					fprintf(fp,"%d\t",routeTable->prefix_len);
				}
				if(routeTable->gateway)
				{
					IP_TO_STR(routeTable->gateway,ipStr);
					fprintf(fp,"%s\t",ipStr);
				}
				else
					fprintf(fp,"on-link\t");
				IP_TO_STR(routeTable->Interface,ipStr);
				fprintf(fp,"%s\t",ipStr);
				fprintf(fp,"%d\t",routeTable->Metric);
				switch(routeTable->type)
				{
				case RoutingType_DEFAULT:
					fprintf(fp,"Default");
					break;
				case RoutingType_STATIC:
					fprintf(fp,"Static");
					break;
				default:
					fprintf(fp,"-");
					break;
				}
				fprintf(fp,"\n");
				routeTable = LIST_NEXT(routeTable);
			}
			fprintf(fp,"\n");
		}
	}
	fclose(fp);
	return 1;
}
int IP_packetTraceFiledFlag[4]={0,0,0,0};
char pszTrace[BUFSIZ];
/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_IP_ConfigPacketTrace(const void* xmlNetSimNode)
{
	char* szStatus;
	*pszTrace=0;
	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"SOURCE_IP");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		IP_packetTraceFiledFlag[0] = 1;
		strcat(pszTrace,"SOURCE_IP,");
	}
	else
		IP_packetTraceFiledFlag[0] = 0;
	free(szStatus);
	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"DESTINATION_IP");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		IP_packetTraceFiledFlag[1] = 1;
		strcat(pszTrace,"DESTINATION_IP,");
	}
	else
		IP_packetTraceFiledFlag[1] = 0;
	free(szStatus);
	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"GATEWAY_IP");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		IP_packetTraceFiledFlag[2] = 1;
		strcat(pszTrace,"GATEWAY_IP,");
	}
	else
		IP_packetTraceFiledFlag[2] = 0;
	free(szStatus);
	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"NEXT_HOP_IP");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		IP_packetTraceFiledFlag[3] = 1;
		strcat(pszTrace,"NEXT_HOP_IP,");
	}
	else
		IP_packetTraceFiledFlag[3] = 0;
	free(szStatus);
	return pszTrace;
}
/**
This function will return the string to write packet trace.																									
*/
_declspec(dllexport) int fn_NetSim_IP_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	int i=0;
	char ip[_NETSIM_IP_LEN];
	if(pstruPacket->pstruNetworkData==NULL)
		return 0;
	*pszTrace=0;
	*ppszTrace = calloc(BUFSIZ,sizeof(char));
	if(IP_packetTraceFiledFlag[i++] == 1)
	{
		if(pstruPacket->pstruNetworkData->szSourceIP)
			IP_TO_STR(pstruPacket->pstruNetworkData->szSourceIP,ip);
		else
			strcpy(ip,"-");
		sprintf(pszTrace,"%s%s,",pszTrace,ip);
	}
	if(IP_packetTraceFiledFlag[i++] == 1)
	{
		if(pstruPacket->pstruNetworkData->szDestIP)
			IP_TO_STR(pstruPacket->pstruNetworkData->szDestIP,ip);
		else
			strcpy(ip,"-");
		sprintf(pszTrace,"%s%s,",pszTrace,ip);
	}
	if(IP_packetTraceFiledFlag[i++] == 1)
	{
		if(pstruPacket->pstruNetworkData->szGatewayIP)
			IP_TO_STR(pstruPacket->pstruNetworkData->szGatewayIP,ip);
		else
			strcpy(ip,"-");
		sprintf(pszTrace,"%s%s,",pszTrace,ip);
	}
	if(IP_packetTraceFiledFlag[i++] == 1)
	{
		if(pstruPacket->pstruNetworkData->szNextHopIp)
			IP_TO_STR(pstruPacket->pstruNetworkData->szNextHopIp,ip);
		else
			strcpy(ip,"-");
		sprintf(pszTrace,"%s%s,",pszTrace,ip);
	}
	strcpy(*ppszTrace,pszTrace);
	return 1;
}
/** This function is used to route the packet */
int fn_NetSim_IP_RoutePacket()
{
	NETSIM_ID i;
	struct stru_NetSim_Packet_NetworkLayer* networkData = pstruEventDetails->pPacket->pstruNetworkData;
	unsigned int metric = 9999;
	NETSIM_IPAddress network1;
	NETSIM_IPAddress network2;
	IP_ROUTINGTABLE* matchedTable=NULL;
	IP_ROUTINGTABLE* routingTable = DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipRoutingTables;
	NETSIM_IPAddress ip = networkData->szDestIP;
	while(routingTable)
	{
		if(!IP_COMPARE(ip,routingTable->networkDestination) && ICMP_CHECKSTATE(ip))
		{
			matchedTable = routingTable;
			break;
		}
		if(ip->type == routingTable->networkDestination->type)
		{
			network1 = IP_NETWORK_ADDRESS(ip,routingTable->netMask,routingTable->prefix_len);
			network2 = routingTable->networkDestination;
			if(!IP_COMPARE(network1,network2))
			{
				if(metric > routingTable->Metric && ICMP_CHECKSTATE(routingTable->gateway))
				{
					matchedTable = routingTable;
					metric = routingTable->Metric;
				}
			}
		}
		routingTable = LIST_NEXT(routingTable);
	}
	if(matchedTable)
	{
		if(matchedTable->gateway)
		{
			networkData->szNextHopIp = matchedTable->gateway;
		}
		else
			networkData->szNextHopIp = networkData->szDestIP;
		networkData->szGatewayIP = matchedTable->Interface;
		pstruEventDetails->nInterfaceId = matchedTable->nInterfaceId;
		if(!matchedTable->nGatewayId && matchedTable->gateway)
			matchedTable->nGatewayId = fn_NetSim_Stack_GetDeviceId_asIP(matchedTable->gateway,&i);
		if(matchedTable->nGatewayId)
			pstruEventDetails->pPacket->nReceiverId = matchedTable->nGatewayId;
		else
			pstruEventDetails->pPacket->nReceiverId = pstruEventDetails->pPacket->nDestinationId;
	}
	return 1;
}
/** This function is to configure static ip table */
int fn_NetSim_IP_ConfigStaticIPTable(char* szVal)
{
	char buffer[50][50];
	NETSIM_ID nDevId=0;
	char* temp;
	char input[BUFSIZ];
	char* destination;
	char* mask;
	char* gateway;
	int metric=0;
	NETSIM_ID interfaceId=0;
	FILE* fp = fopen(szVal,"r");
	if(fp==NULL)
	{
		perror(szVal);
		fnNetSimError("Unable to open file");
		return 0;
	}
	while(fgets(input,BUFSIZ,fp))
	{
		temp=input;
		temp=lskip(temp);
		if(*temp == '#')
		{
			temp++;
			temp=lskip(temp);
			strtok(temp,"=");
			rstrip(temp);
			_strupr(temp);
			if(!strcmp(temp,"DEVICE ID"))
			{
				temp+=strlen(temp);
				temp++;
				temp=lskip(temp);
				rstrip(temp);
				nDevId=atoi(temp);
			}
		}
		else
		{
			NETSIM_ID i;
			_strupr(temp);
			for(i=0;i<50,*temp;temp+=strlen(buffer[i])+1,i++)
			{
				temp=lskip(temp);
				sscanf(temp,"%s",buffer[i]);
			}
			*buffer[i]=0;
			for(i=0;*buffer[i];i++)
			{
				if(!strcmp(buffer[i],"ROUTE"))
					continue;
				else if(!strcmp(buffer[i],"ADD"))
				{
					destination = buffer[i+1];
					i++;
					continue;
				}
				else if(!strcmp(buffer[i],"MASK"))
				{
					mask=buffer[i+1];
					i++;
					continue;
				}
				else if(!strcmp(buffer[i],"METRICS"))
				{
					metric = atoi(buffer[i+1]);
					i++;
					continue;
				}
				else if(!strcmp(buffer[i],"IF"))
				{
					interfaceId = atoi(buffer[i+1]);
					i++;
					continue;
				}
				else
				{
					gateway=buffer[i];
				}
			}
			if(nDevId)
			{
				IP_ROUTINGTABLE** table = (IP_ROUTINGTABLE**)(&(NETWORK->ppstruDeviceList[nDevId-1]->pstruNetworkLayer->ipRoutingTables));
				IP_ROUTINGTABLE* newTable = IPROUTINGTABLE_ALLOC();
				newTable->gateway = STR_TO_IP4(gateway);
				newTable->Metric = metric;
				newTable->Interface = DEVICE_INTERFACE(nDevId,interfaceId)->szAddress;
				newTable->netMask = STR_TO_IP4(mask);
				newTable->networkDestination = STR_TO_IP4(destination);
				newTable->nGatewayId = fn_NetSim_Stack_GetDeviceId_asIP(newTable->gateway,&i);
				newTable->nInterfaceId = interfaceId;
				newTable->type = RoutingType_STATIC;
				IPROUTINGTABLE_ADD(table,newTable,iptable_add_check);
			}
			mask=0;
			interfaceId=0;
			gateway=NULL;
			destination=NULL;
			mask=NULL;
		}
	}
	return 0;
}

int fn_NetSim_IP_RoutePacketViaStaticEntry()
{
	NETSIM_ID i;
	struct stru_NetSim_Packet_NetworkLayer* networkData = pstruEventDetails->pPacket->pstruNetworkData;
	unsigned int metric = 9999;
	NETSIM_IPAddress network1;
	NETSIM_IPAddress network2;
	IP_ROUTINGTABLE* matchedTable=NULL;
	IP_ROUTINGTABLE* routingTable = DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipRoutingTables;
	NETSIM_IPAddress ip = networkData->szDestIP;
	while(routingTable)
	{
		if(ip->type != routingTable->networkDestination->type)
		{
			routingTable = LIST_NEXT(routingTable);
			continue;
		}
		if(!IP_COMPARE(ip,routingTable->networkDestination) &&
			routingTable->type == RoutingType_STATIC)
		{
			matchedTable = routingTable;
			break;
		}
		network1 = IP_NETWORK_ADDRESS(ip,routingTable->netMask,routingTable->prefix_len);
		network2 = routingTable->networkDestination;
		if(!IP_COMPARE(network1,network2) &&
			routingTable->type == RoutingType_STATIC)
		{
			if(metric > routingTable->Metric)
			{
				matchedTable = routingTable;
				metric = routingTable->Metric;
			}
		}
		routingTable = LIST_NEXT(routingTable);
	}
	if(matchedTable)
	{
		if(matchedTable->gateway)
		{
			networkData->szNextHopIp = matchedTable->gateway;
		}
		else
			networkData->szNextHopIp = networkData->szDestIP;
		networkData->szGatewayIP = matchedTable->Interface;
		pstruEventDetails->nInterfaceId = matchedTable->nInterfaceId;
		if(!matchedTable->nGatewayId && matchedTable->gateway)
			matchedTable->nGatewayId = fn_NetSim_Stack_GetDeviceId_asIP(matchedTable->gateway,&i);
		if(matchedTable->nGatewayId)
			pstruEventDetails->pPacket->nReceiverId = matchedTable->nGatewayId;
		else
			pstruEventDetails->pPacket->nReceiverId = pstruEventDetails->pPacket->nDestinationId;
	}
	return 1;
}
/** This function is to get the local network protocol */
NETWORK_LAYER_PROTOCOL fnGetLocalNetworkProtocol(NetSim_EVENTDETAILS* pstruEventDetails)
{
	switch(pstruEventDetails->nSubEventType/100)
	{
	case NW_PROTOCOL_ARP:
		return NW_PROTOCOL_ARP;
	case NW_PROTOCOL_MPLS:
		return NW_PROTOCOL_MPLS;
	}
	switch(pstruEventDetails->pPacket->nControlDataType/100)
	{
	case NW_PROTOCOL_ARP:
		return NW_PROTOCOL_ARP;
	case NW_PROTOCOL_MPLS:
		return NW_PROTOCOL_MPLS;
	}
	return 0;
}

