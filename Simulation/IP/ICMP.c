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

/***********************************************************************

Network Working Group                                          J. Postel
Request for Comments:  792                                           ISI
                                                          September 1981
Updates:  RFCs 777, 760
Updates:  IENs 109, 128

                   INTERNET CONTROL MESSAGE PROTOCOL

                         DARPA INTERNET PROGRAM
                         PROTOCOL SPECIFICATION

*************************************************************************/
#include "main.h"
#include "List.h"
#include "IP.h"
#include "ICMP.h"
_declspec(dllexport) NetSim_PACKET* fn_NetSim_IP_ICMP_GenerateEchoRequest(NETSIM_ID source,
															   NETSIM_ID dest,
															   NETSIM_IPAddress srcIP,
															   NETSIM_IPAddress destIP,
															   double time,
															   void* data,
															   unsigned int size,
															   unsigned int ttl);
/** This fucntion is to initialize the ICMP parameters */
_declspec(dllexport) int fn_NetSim_IP_ICMP_Init()
{
	NETSIM_ID i;
	for(i=0;i<NETWORK->nDeviceCount;i++)
	{
		if(NETWORK->ppstruDeviceList[i]->pstruNetworkLayer)
		{
			IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[i]->pstruNetworkLayer->ipVar;
			if(devVar && devVar->nICMPPollingTime)
			{
				//Create timer event for poll
				memset(pstruEventDetails,0,sizeof* pstruEventDetails);
				pstruEventDetails->dEventTime = devVar->nICMPPollingTime*SECOND;
				pstruEventDetails->nDeviceId = NETWORK->ppstruDeviceList[i]->nDeviceId;
				pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[i]->nDeviceType;
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nProtocolId = NW_PROTOCOL_IPV4;
				pstruEventDetails->nSubEventType = EVENT_ICMP_POLL;
				fnpAddEvent(pstruEventDetails);
			}
			if(devVar->nRouterAdvertisementFlag)
			{
				//Create timer event for router advertisement
				memset(pstruEventDetails,0,sizeof* pstruEventDetails);
				pstruEventDetails->dEventTime = 0;
				pstruEventDetails->nDeviceId = NETWORK->ppstruDeviceList[i]->nDeviceId;
				pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[i]->nDeviceType;
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nProtocolId = NW_PROTOCOL_IPV4;
				pstruEventDetails->nSubEventType = EVENT_ADVERTISE_ROUTER;
				fnpAddEvent(pstruEventDetails);
			}
		}
	}
	return 1;
}
/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Operations Manager uses a high-performance, asynchronous ICMP poller. 
	The ICMP poller performs at a consistent rate that is independent of poll response times.
	Operations Manager achieves this using two asynchronous threads: 
	one that sends polls and one that receives polls. Because the send and receive threads 
	operate asynchronously, slow response times or excessive timeouts do not affect the polling rate.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec(dllexport) int fn_NetSim_IP_ICMP_POLL()
{
	unsigned int i;
	double time=pstruEventDetails->dEventTime;
	IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruNetworkLayer->ipVar;
	//Add the next ICMP poll event
	pstruEventDetails->dEventTime += devVar->nICMPPollingTime*SECOND;
	fnpAddEvent(pstruEventDetails);
	for(i=0;i<devVar->nGatewayCount;i++)
	{
		NetSim_PACKET* packet;
		if(devVar->nGatewayState[i] == GATEWAYSTATE_NOTIFICATION_PENDING)
			devVar->nGatewayState[i] = GATEWAYSTATE_DOWN;
		else if(devVar->nGatewayState[i] == GATEWAYSTATE_UP)
			devVar->nGatewayState[i] = GATEWAYSTATE_NOTIFICATION_PENDING;
		//Send ICMP echo request to gateway
		packet = fn_NetSim_IP_ICMP_GenerateEchoRequest(pstruEventDetails->nDeviceId,
			devVar->nGatewayId[i],
			NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[devVar->nInterfaceId[i]-1]->szAddress,
			devVar->GatewayIPAddress[i],
			pstruEventDetails->dEventTime,
			NULL,
			0,
			1);
		//Generate Network out event to transmit
		pstruEventDetails->dEventTime=time;
		pstruEventDetails->dPacketSize = fnGetPacketSize(packet);
		pstruEventDetails->nEventType = NETWORK_OUT_EVENT;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket = packet;
		fnpAddEvent(pstruEventDetails);
	}
	
	return 1;
}
/** This function is used to genrate echo request */
_declspec(dllexport) NetSim_PACKET* fn_NetSim_IP_ICMP_GenerateEchoRequest(NETSIM_ID source,
															   NETSIM_ID dest,
															   NETSIM_IPAddress srcIP,
															   NETSIM_IPAddress destIP,
															   double time,
															   void* data,
															   unsigned int size,
															   unsigned int ttl)
{
	ICMP_ECHO* echo = calloc(1,sizeof* echo);
	NetSim_PACKET* packet = fn_NetSim_Packet_CreatePacket(NETWORK_LAYER);
	packet->nDestinationId = dest;
	packet->nControlDataType = PACKET_ICMP_ECHORequest;
	packet->nPacketId =0;
	packet->nPacketPriority = Priority_Low;
	packet->nPacketType = PacketType_Control;
	packet->nSourceId = source;
	packet->pstruNetworkData->dArrivalTime = time;
	packet->pstruNetworkData->dStartTime = time;
	packet->pstruNetworkData->dEndTime = time;
	packet->pstruNetworkData->dOverhead = 8;
	packet->pstruNetworkData->dPayload = size;
	packet->pstruNetworkData->dPacketSize = 8+size;
	packet->pstruNetworkData->nNetworkProtocol = NW_PROTOCOL_IPV4;
	packet->pstruNetworkData->nTTL = ttl+1;
	packet->pstruNetworkData->szDestIP = IP_COPY(destIP);
	packet->pstruNetworkData->szSourceIP = IP_COPY(srcIP);
	packet->pstruNetworkData->Packet_NetworkProtocol = echo;
	echo->Type = 8;
	echo->Data = data;
	return packet;
}
/// This function is to process the echo request.
_declspec(dllexport) int fn_NetSim_IP_ICMP_EchoRequest()
{
	//generate echo reply packet
	NetSim_PACKET* request = pstruEventDetails->pPacket;
	NetSim_PACKET* reply = fn_NetSim_Packet_CreatePacket(NETWORK_LAYER);
	ICMP_ECHO* echo = calloc(1,sizeof* echo);
	reply->nControlDataType = PACKET_ICMP_ECHOReply;
	reply->nDestinationId = request->nSourceId;
	reply->nPacketType = PacketType_Control;
	reply->nSourceId = pstruEventDetails->nDeviceId;
	reply->pstruNetworkData->dArrivalTime = pstruEventDetails->dEventTime;
	reply->pstruNetworkData->dEndTime = pstruEventDetails->dEventTime;
	reply->pstruNetworkData->dOverhead = 8;
	reply->pstruNetworkData->dPayload = request->pstruNetworkData->dPayload;
	reply->pstruNetworkData->dPacketSize = 8+request->pstruNetworkData->dPayload;
	reply->pstruNetworkData->dStartTime = pstruEventDetails->dEventTime;
	reply->pstruNetworkData->nNetworkProtocol = NW_PROTOCOL_IPV4;
	reply->pstruNetworkData->nTTL = 255;
	reply->pstruNetworkData->Packet_NetworkProtocol = echo;
	reply->pstruNetworkData->szDestIP=IP_COPY(request->pstruNetworkData->szSourceIP);
	reply->pstruNetworkData->szSourceIP=IP_COPY(request->pstruNetworkData->szDestIP);
	echo->Type=0;
	echo->Data = ((ICMP_ECHO*)request->pstruNetworkData->Packet_NetworkProtocol)->Data;
	pstruEventDetails->pPacket=reply;
	//Free the request packet
	fn_NetSim_Packet_FreePacket(request);
	return 1;
}
/// The data received in the echo message must be returned in the echo reply message.
_declspec(dllexport) int fn_NetSim_IP_ICMP_EchoReply()
{
	IP_DEVVAR* devVar = DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipVar;
	unsigned int i;
	for(i=0;i<devVar->nGatewayCount;i++)
	{
		if(!IP_COMPARE(devVar->GatewayIPAddress[i],pstruEventDetails->pPacket->pstruNetworkData->szSourceIP))
		{
			devVar->nGatewayState[i] = GATEWAYSTATE_UP;
			break;
		}
	}
	//free the reply packet
	fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
	pstruEventDetails->pPacket = NULL;
	return 1;
}
/** This function is to check the gateway state */
_declspec(dllexport) int ICMP_CHECKSTATE(NETSIM_IPAddress ip)
{
	IP_DEVVAR* devVar = DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipVar;
	unsigned int i;
	for(i=0;ip && i<devVar->nGatewayCount;i++)
		if(!IP_COMPARE(devVar->GatewayIPAddress[i],ip))
		{
			if(devVar->nGatewayState[i] == GATEWAYSTATE_DOWN)
				return 0;
			break;
		}
		return 1;
}
unsigned long advertiseseed1=12345678;
unsigned long advertiseseed2=23456789;
/**
   The ICMP router discovery messages are called "Router Advertisements"
   and "Router Solicitations".  Each router periodically multicasts a
   Router Advertisement from each of its multicast interfaces,
   announcing the IP address(es) of that interface.  Hosts discover the
   addresses of their neighboring routers simply by listening for
   advertisements.  When a host attached to a multicast link starts up,
   it may multicast a Router Solicitation to ask for immediate
   advertisements, rather than waiting for the next periodic ones to
   arrive; if (and only if) no advertisements are forthcoming, the host
   may retransmit the solicitation a small number of times, but then
   must desist from sending any more solicitations.  Any routers that
   subsequently start up, or that were not discovered because of packet
   loss or temporary link partitioning, are eventually discovered by
   reception of their periodic (unsolicited) advertisements.  (Links
   that suffer high packet loss rates or frequent partitioning are
   accommodated by increasing the rate of advertisements, rather than
   increasing the number of solicitations that hosts are permitted to
   send.)
 */
_declspec(dllexport) int fn_NetSim_IP_ICMP_AdvertiseRouter()
{
	ICMP_RouterAdvertisement* adver=calloc(1,sizeof* adver);
	double time=pstruEventDetails->dEventTime;
	NetSim_PACKET* packet;
	NETSIM_ID nDeviceId=pstruEventDetails->nDeviceId;
	NETSIM_ID i;
	IP_DEVVAR* devVar = DEVICE_NWLAYER(nDeviceId)->ipVar;
	//Add next event for router advertisement
	pstruEventDetails->dEventTime += (fn_NetSim_Utilities_GenerateRandomNo(&advertiseseed1,&advertiseseed2)/NETSIM_RAND_MAX*(devVar->nRouterAdverMaxInterval-devVar->nRouterAdverMinInterval)+devVar->nRouterAdverMinInterval)*SECOND;
	fnpAddEvent(pstruEventDetails);
	//Generate router advertisement
	packet = fn_NetSim_Packet_CreatePacket(NETWORK_LAYER);
	packet->dEventTime = time;
	packet->nControlDataType = PACKET_ROUTER_ADVERTISEMENT;
	packet->nDestinationId = 0;
	packet->nPacketType = PacketType_Control;
	packet->nReceiverId=0;
	packet->nSourceId=pstruEventDetails->nDeviceId;
	packet->nTransmitterId=pstruEventDetails->nDeviceId;
	packet->pstruNetworkData->dArrivalTime =time;
	packet->pstruNetworkData->dEndTime=time;
	packet->pstruNetworkData->dStartTime=time;
	packet->pstruNetworkData->dOverhead=16;
	packet->pstruNetworkData->dPacketSize=16;
	packet->pstruNetworkData->dPayload=0;
	packet->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
	packet->pstruNetworkData->nTTL=2;
	packet->pstruNetworkData->szDestIP=STR_TO_IP4("255.255.255.255");
	packet->pstruNetworkData->Packet_NetworkProtocol=adver;
	adver->Type=9;
	adver->AddrEntrySize=2;
	adver->Lifetime = devVar->nRouterAdverLifeTime;
	//count the num of address
	for(i=0;i<NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;i++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i])
		{
			if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType && (NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType != INTERFACE_WAN_ROUTER && NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType != INTERFACE_VIRTUAL))
			{
				adver->NumAddrs++;
				adver->RouterAddress = realloc(adver->RouterAddress,(sizeof* adver->RouterAddress)*adver->NumAddrs);
				adver->RouterAddress[adver->NumAddrs-1] = IP_COPY(DEVICE_NWADDRESS(nDeviceId,i+1));
			}
		}
	}
	for(i=0;i<NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;i++)
	{
		if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i])
		{
			if(NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType && (NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType != INTERFACE_WAN_ROUTER && NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[i]->nInterfaceType != INTERFACE_VIRTUAL))
			{
				NetSim_PACKET* temp=fn_NetSim_Packet_CopyPacket(packet);
				//Create network out event to transmit
				temp->pstruNetworkData->szGatewayIP = IP_COPY(DEVICE_NWADDRESS(nDeviceId,i+1));
				temp->pstruNetworkData->szSourceIP =  IP_COPY(DEVICE_NWADDRESS(nDeviceId,i+1));
				temp->pstruNetworkData->szNextHopIp = temp->pstruNetworkData->szDestIP;
				pstruEventDetails->dEventTime=time;
				pstruEventDetails->nInterfaceId=i+1;
				pstruEventDetails->dPacketSize=16;
				pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=temp;
				fnpAddEvent(pstruEventDetails);
			}
		}
	}
	return 1;
}
/**
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   The router discovery messages do not constitute a routing protocol:
   they enable hosts to discover the existence of neighboring routers,
   but not which router is best to reach a particular destination.  If a
   host chooses a poor first-hop router for a particular destination, it
   should receive an ICMP Redirect from that router, identifying a
   better one.
   A Router Advertisement includes a "preference level" for each
   advertised router address.  When a host must choose a default router
   address (i.e., when, for a particular destination, the host has not
   been redirected or configured to use a specific router address), it
   is expected to choose from those router addresses that have the
   highest preference level.
   A Router Advertisement also includes a "lifetime" field, specifying
   the maximum length of time that the advertised addresses are to be
   considered as valid router addresses by hosts, in the absence of
   further advertisements.  This is used to ensure that hosts eventually
   forget about routers that fail, become unreachable, or stop acting as
   routers.
   The default advertising rate is once every 7 to 10 minutes, and the
   default lifetime is 30 minutes.
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec(dllexport) int fn_NetSim_IP_ICMP_ProcessRouterAdvertisement()
{
	int flag=0;
	NETSIM_ID nDeviceId=pstruEventDetails->nDeviceId;
	IP_ROUTINGTABLE* table=DEVICE_NWLAYER(nDeviceId)->ipRoutingTables;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ICMP_RouterAdvertisement* adver=PACKET_NWPROTOCOLDATA(packet);
	//Get the source IP
	NETSIM_IPAddress src=PACKET_NWDATA(packet)->szSourceIP;	
	NETSIM_IPAddress ip=STR_TO_IP4("0.0.0.0");
	while(table)
	{
		if(table->gateway)
		if(!IP_COMPARE(src,table->gateway) && !IP_COMPARE(table->networkDestination,ip) && !IP_COMPARE(table->netMask,ip))
		{
			//entry found
			flag=1;
			break;
		}
		table=LIST_NEXT(table);
	}
	if(!table)
	{
		//Create new entry
		iptable_add((IP_ROUTINGTABLE**)(&(DEVICE_NWLAYER(nDeviceId)->ipRoutingTables)),
			ip,ip,0,src,DEVICE_NWADDRESS(nDeviceId,pstruEventDetails->nInterfaceId),
			pstruEventDetails->nInterfaceId,DEFAULT_METRIC);

	}
	fn_NetSim_Packet_FreePacket(packet);
	pstruEventDetails->pPacket = NULL;
	return 1;
}
/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	If, according to the information in the gateway's routing tables,
      the network specified in the internet destination field of a
      datagram is unreachable, e.g., the distance to the network is
      infinity, the gateway sends a destination unreachable message to
      the internet source host of the datagram.  In addition, in some
      networks, the gateway may be able to determine if the internet
      destination host is unreachable.  Gateways in these networks may
      send destination unreachable messages to the source host when the
      destination host is unreachable.

      If, in the destination host, the IP module cannot deliver the
      datagram  because the indicated protocol module or process port is
      not active, the destination host may send a destination
      unreachable message to the source host.

      Another case is when a datagram must be fragmented to be forwarded
      by a gateway yet the Don't Fragment flag is on.  In this case the
      gateway must discard the datagram and return a destination
      unreachable message.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec(dllexport) int fn_NetSim_IP_ICMP_GenerateDstUnreachableMsg()
{
	NetSim_PACKET* orgPacket=pstruEventDetails->pPacket;
	NetSim_PACKET* packet = fn_NetSim_Packet_CreatePacket(NETWORK_LAYER);
	ICMP_DestinationUnreachableMessage* message=calloc(1,sizeof* message);
	message->Type=3;
	message->code=1;//host unreachable
	message->InternetHeader=orgPacket;
	packet->pstruNetworkData->szDestIP = IP_COPY(orgPacket->pstruNetworkData->szSourceIP);
	packet->nControlDataType = PACKET_ICMP_DstUnreachableMsg;
	packet->nDestinationId=orgPacket->nSourceId;
	packet->nPacketType=PacketType_Control;
	packet->nSourceId=pstruEventDetails->nDeviceId;
	packet->nTransmitterId=pstruEventDetails->nDeviceId;
	packet->pstruNetworkData->dArrivalTime=pstruEventDetails->dEventTime;
	packet->pstruNetworkData->dEndTime=pstruEventDetails->dEventTime;
	packet->pstruNetworkData->dStartTime=pstruEventDetails->dEventTime;
	packet->pstruNetworkData->dOverhead=24+IPV4_HEADER_SIZE;
	packet->pstruNetworkData->dPacketSize=24+IPV4_HEADER_SIZE;
	packet->pstruNetworkData->dPayload=0;
	packet->pstruNetworkData->nNetworkProtocol=NW_PROTOCOL_IPV4;
	packet->pstruNetworkData->nTTL=255;
	packet->pstruNetworkData->Packet_NetworkProtocol=message;
	packet->pstruNetworkData->szSourceIP=fn_NetSim_Stack_GetFirstIPAddressAsId(pstruEventDetails->nDeviceId,0);
	//Add the network in event
	pstruEventDetails->dPacketSize=24+IPV4_HEADER_SIZE;
	pstruEventDetails->nApplicationId=0;
	pstruEventDetails->nEventType=NETWORK_IN_EVENT;
	pstruEventDetails->nInterfaceId=0;
	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nProtocolId=NW_PROTOCOL_IPV4;
	pstruEventDetails->nSegmentId=0;
	pstruEventDetails->nSubEventType=0;
	pstruEventDetails->pPacket=packet;
	fnpAddEvent(pstruEventDetails);
	return 1;
}
/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	If, according to the information in the gateway's routing tables,
      the network specified in the internet destination field of a
      datagram is unreachable, e.g., the distance to the network is
      infinity, the gateway sends a destination unreachable message to
      the internet source host of the datagram.  In addition, in some
      networks, the gateway may be able to determine if the internet
      destination host is unreachable.  Gateways in these networks may
      send destination unreachable messages to the source host when the
      destination host is unreachable.

      If, in the destination host, the IP module cannot deliver the
      datagram  because the indicated protocol module or process port is
      not active, the destination host may send a destination
      unreachable message to the source host.

      Another case is when a datagram must be fragmented to be forwarded
      by a gateway yet the Don't Fragment flag is on.  In this case the
      gateway must discard the datagram and return a destination
      unreachable message.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec(dllexport) int fn_NetSim_IP_ICMP_ProcessDestUnreachableMsg()
{
	IP_ROUTINGTABLE* table=DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipRoutingTables;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ICMP_DestinationUnreachableMessage* message=packet->pstruNetworkData->Packet_NetworkProtocol;
	
	while(table)
	{
		if(!IP_COMPARE(table->networkDestination,PACKET_NWDATA(((NetSim_PACKET*)(message->InternetHeader)))->szDestIP))
		{
			LIST_FREE(&(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipRoutingTables),table);
			table = DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->ipRoutingTables;
			continue;
		}
		table=LIST_NEXT(table);
	}
	fn_NetSim_Stack_CallICMPErrorFun(message->InternetHeader,pstruEventDetails->nDeviceId,3);
	if(pstruEventDetails->pPacket->nDestinationId == pstruEventDetails->nDeviceId)
	{
		NetSim_PACKET* p=message->InternetHeader;
		pstruEventDetails->pPacket=NULL;
		fn_NetSim_Packet_FreePacket(p);
		free(message);
		fn_NetSim_Packet_FreePacket(packet);
	}
	return 0;
}
