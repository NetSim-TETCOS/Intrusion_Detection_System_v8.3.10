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
#ifndef _NETSIM_IP_H_
#define _NETSIM_IP_H_
#ifdef  __cplusplus
extern "C" {
#endif
#ifndef _NETSIM_IP_LIB_
#pragma comment(lib,"IP.lib")
#endif

#define on_link NULL
#define DEFAULT_METRIC 999
#define ONLINK_METRIC 300
#define IPV4_HEADER_SIZE 20
#define PROTOCOL_VPN 1
#define VPN_METRIC 200

	typedef struct stru_NetSim_IPRoutingTable IP_ROUTINGTABLE;
	typedef struct stru_IP_DevVar IP_DEVVAR;
	/// Enumeration for routing types
	typedef enum
	{
		RoutingType_DEFAULT=1,
		RoutingType_STATIC,
	}ROUTING_TYPE;
	/// Enumeration for IP control packets
	typedef enum
	{
		PACKET_ICMP_DstUnreachableMsg=NW_PROTOCOL_IPV4*100+1,
		PACKET_ICMP_ECHORequest,
		PACKET_ICMP_ECHOReply,
		PACKET_ROUTER_ADVERTISEMENT,
		PACKET_VPN,
	}IP_CONTROL_PACKET;
	/// Enumeration for ip subevents.
	typedef enum
	{
		EVENT_ICMP_POLL=NW_PROTOCOL_IPV4*100+1,
		EVENT_ADVERTISE_ROUTER,
	}IP_SUBEVENT;

	//http://www.cisco.com/en/US/docs/net_mgmt/ciscoworks_ip_communications_operations_manager/1.0/user/guide/SNMPInfo.html#wp1024287
	/// Enumeration for IP gadeway states.
	typedef enum
	{
		GATEWAYSTATE_UP,
		GATEWAYSTATE_DOWN,
		GATEWAYSTATE_NOTIFICATION_PENDING,
		GATEWAYSTATE_CLEARANCE_PENDING,
	}IP_GATEWAYSTATE;
	/// Enumeration for VPN states.
	typedef enum
	{
		VPN_DISABLE=0,
		VPN_SERVER,
		VPN_CLIENT,
	}VPN_STATE;
/// Structure to store ip routing table
struct stru_NetSim_IPRoutingTable
{
	NETSIM_IPAddress networkDestination;
	NETSIM_IPAddress netMask;
	NETSIM_IPAddress gateway;
	NETSIM_IPAddress Interface;
	unsigned int prefix_len;
	unsigned int Metric;
	ROUTING_TYPE type;
	_ele* ele;
	//NetSim specific
	NETSIM_ID nInterfaceId;
	NETSIM_ID nGatewayId;
};
/*
3.1.  Internet Header Format

  A summary of the contents of the internet header follows:

                                    
    0                   1                   2                   3   
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version|  IHL  |Type of Service|          Total Length         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Identification        |Flags|      Fragment Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Time to Live |    Protocol   |         Header Checksum       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination Address                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                    Example Internet Datagram Header

                               Figure 4.
*/
/*typedef struct stru_IP_Option
{
	unsigned int optType;
	unsigned int optLen;
	void* option;
	struct stru_IP_Option* next;
}IP_OPTION;
struct stru_NetSim_IPHeader
{
	unsigned int version:4;
	unsigned int IHL:4;
	unsigned int TypeofService:8;
	unsigned int TotalLength:16;
	unsigned int Identification:16;
	unsigned int flags:3;
	unsigned int FragmentOffset:13;
	unsigned int TimetoLive:8;
	unsigned int Protocol:8;
	unsigned int HeaderChecksum:16;
	NETSIM_IPAddress SourceAddress;
	NETSIM_IPAddress DestinationAddess;
	IP_OPTION* options;
	unsigned int Padding;
};*/

/// Structure to store the device ip details
struct stru_IP_DevVar
{
	//Firewall variable
	int nFirewallStatus;
	char* firewallConfig;
	NETSIM_IPAddress* blockedIP;
	unsigned int blockedIPCount;
	
	//ICMP Variable
	//Router Advertisement
	unsigned int nRouterAdvertisementFlag; 
	unsigned int nRouterAdverMinInterval;
	unsigned int nRouterAdverMaxInterval;
	unsigned int nRouterAdverLifeTime;
	//ICMP POLL
	unsigned int nICMPPollingTime;
	unsigned int nGatewayCount;
	NETSIM_IPAddress* GatewayIPAddress;
	NETSIM_ID* nGatewayId;
	NETSIM_ID* nInterfaceId;
	IP_GATEWAYSTATE* nGatewayState;

	//VPN variable
	VPN_STATE nVPNStatus;	/*	0--Disable
								1--Server
								2--Client */
	//Client variable
	NETSIM_IPAddress serverIP;
	//Server variable
	NETSIM_IPAddress ipPoolStart;
	NETSIM_IPAddress ipPoolEnd;
	NETSIM_IPAddress ipPoolMask;
	void* vpn;
	void* dnsList;

	//DHCP
	void* dhcp;
};
/// Structure to store the IP metrics.
struct stru_IP_Metrics
{
	NETSIM_ID nDeviceId;
	unsigned int nPacketSent;
	unsigned int nPacketReceived;
	unsigned int nPacketForwarded;
	unsigned int nPacketDiscarded;
	unsigned int nFirewallBlocked;
	unsigned int nTTLDrop;
};
struct stru_IP_Metrics** ipMetrics;
/// Data structure for dns.
typedef struct stru_dnsList
{
	NETSIM_ID deviceId;
	NETSIM_IPAddress ip;
	_ele* ele;
}DNS;
#define DNS_ALLOC() (struct stru_dnsList*)list_alloc(sizeof(struct stru_dnsList),offsetof(struct stru_dnsList,ele))

#define IPROUTINGTABLE_ALLOC() (IP_ROUTINGTABLE*)list_alloc(sizeof(IP_ROUTINGTABLE),offsetof(IP_ROUTINGTABLE,ele))
#define IPROUTINGTABLE_ADD(table,current,fun) list_add(table,current,offsetof(IP_ROUTINGTABLE,ele),fun)


//Lib function
int iptable_add_check(IP_ROUTINGTABLE* current,IP_ROUTINGTABLE* mem);
IP_ROUTINGTABLE* iptable_check(IP_ROUTINGTABLE** table,NETSIM_IPAddress dest,NETSIM_IPAddress subnet);
int iptable_change(IP_ROUTINGTABLE** table,
				NETSIM_IPAddress dest,
				NETSIM_IPAddress subnet,
				NETSIM_IPAddress gateway,
				NETSIM_IPAddress interfaceIp,
				NETSIM_ID interfaceId,
				unsigned int metric);
int iptable_delete(IP_ROUTINGTABLE** table,
				   NETSIM_IPAddress dest);
int iptable_add(IP_ROUTINGTABLE** table,
				NETSIM_IPAddress dest,
				NETSIM_IPAddress subnet,
				unsigned int prefix_len,
				NETSIM_IPAddress gateway,
				NETSIM_IPAddress interfaceIp,
				NETSIM_ID interfaceId,
				unsigned int metric);
int iptable_print(FILE* fp,IP_ROUTINGTABLE* routeTable);
_declspec(dllexport) NETSIM_IPAddress dns_query(NETSIM_ID nDeviceId,NETSIM_ID id);

//NAT
int fn_NetSim_NAT_NetworkOut(NETSIM_ID ndev,NetSim_PACKET* packet);
int fn_NetSim_NAT_NetworkIn(NETSIM_ID ndev,NetSim_PACKET* packet);


#ifdef  __cplusplus
}
#endif
#endif