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
#ifndef _OSPF_H_
#define _OSPF_H_
#ifdef  __cplusplus
extern "C" {
#endif
#define round(x) (int)(x+0.5)
#define MICROSECONDS 1000000
#define ROUTER_DEAD_INTERVAL 40000000
#define HELLO_INTERVAL 10000000
#define HELLO_PACKETSIZE_WITHHEADER 48 //For IPv4
#define DD_PACKETSIZE_WITHHEADER 52 //For IPv4
#define LSR_PACKETSIZE_WITHHEADER 36 //For IPv4
#define LSU_PACKETSIZE_WITHHEADER 64 //For IPv4
#define LSACK_PACKETSIZE_WITHHEADER 44 //For IPv4
	int REFERENCE_BANDWIDTH;
	int nMaxCost;
#define LSA_PACKET_SIZE 36
#define DEFAULT_ROUTER_PRIORITY 1 // Cisco reference
#define SEQUENCE_NUMBER 0x80000001
#define MTU 1500 // For Ethernet and point to point networks

#define _OSPF_Table
	FILE *fpOSPFTable;
	// OSPF
	typedef struct stru_OSPF_Router OSPF_ROUTER;
	typedef struct stru_OSPF_Packet_Header OSPF_PACKET_HEADER;
	typedef struct stru_OSPF_LSA_Header OSPF_LSA_HEADER;

	typedef struct stru_OSPF_Router_LSA ROUTER_LSA;
	typedef struct stru_OSPF_Hello_Packet HELLO_PACKET;
	typedef struct stru_OSPF_DatabaseDescription_Packet DD_PACKET;
	typedef struct stru_OSPF_LinkStateRequest_Packet LSR_PACKET;
	typedef struct stru_OSPF_LinkStateUpdate_Packet LSU_PACKET;
	typedef struct stru_OSPF_LinkStateAcknowledgement_Packet LSA_PACKET;

	typedef struct stru_Router_OSPF_RoutingTable OSPF_ROUTING_TABLE;
	typedef struct stru_OSPF_LinkStateDatabase LSDB;
	typedef struct stru_NeighborIP NEIGHBOR;
	typedef struct stru_LSAs LSAs;
	typedef struct stru_LSR LSR;
	typedef struct stru_RouterLSA_Entry ROUTERLSA_ENTRY;
	typedef struct stru_OSPF_Variables OSPF_VAR;
	typedef struct Stru_OSPF_State OSPF_STATE;
	typedef struct stru_NeighborIPAddress IPADDRESS;

	typedef enum enum_OSPF_PacketType OSPF_PacketType;
	typedef enum enum_OSPF_LSType OSPF_LStype;
	typedef enum enum_OSPF_RouterLink OSPF_RouterLink;
	typedef enum enum_OSPF_Subevent_Type SUB_EVENT_TYPE;
	typedef enum enum_OSPF_PathType OSPF_PathType;
	typedef enum OSPF_States OSPF_STATES;
/// Enumeration for OSPF packet type. As per RFC 2328 April 1998 Page :190
	enum enum_OSPF_PacketType
	{
		HELLO=APP_PROTOCOL_OSPF*100+1,
		DATABASE_DESCRIPTION,
		LINKSTATEREQUEST,
		LINKSTATEUPDATE,
		LINKSTATEACKNOWLEDGEMENT,
	};
/// Enumeration for LS Type. As per RFC 2328 April 1998 Page :204
	enum enum_OSPF_LSType
	{
		ROUTER_LSAs=1,
		NETWORK_LSAs,
		SUMMARY_LSA_Type1s,//ip NETWORK
		SUMMARY_LSAs_Type2,//ASBR
		AS_EXTERNAL_LSAs,
	};
/// Enumeration of router link. As per RFC 2328 April 1998 Page :207
	enum enum_OSPF_RouterLink
	{
		POINT_TO_POINT=1, //Point_To_Point connection to another router
		TRANSIT,		   //Connection to a transit network
		STUB,			   //Connection to a stub network
		VIRTUAL,		   //Virtual link
	};
/// Enumeration of OSPF path type. As per RFC 2328 April 1998 Page :109
	enum enum_OSPF_PathType 
	{
		intra_area,
		inter_area,
		type1_external,
		tpte2_external,
	};
	/* These are the events that can affect our */
	/* StateMachine                             */

	/** Enumeration for OSPF Sub event Types*/

	enum enum_OSPF_Subevent_Type
	{
		//Interface State Machine Subevents
		//Start
		INTERFACEUP=APP_PROTOCOL_OSPF*100+1,
		WAITTIMER,
		BACKUPSEEN,
		NEIGHBORCHANGE,
		LOOPIND,
		UNLOOPIND,
		INTERFACEDOWN,
		//End
		//Neighbor State Machine Subevents
		//Start
		HELLORECEIVED,
		START_NBR,
		TWOWAYRECEIVED,
		NEGOTIATIONDONE,
		EXCHANGEDONE,
		BADLSREQ,
		LOADINGDONE,
		ADJOK,
		SEQNUMBERMISMATCH,
		ONEWAY,
		KILLNBR,
		INACTIVITYTIMER,
		LLDOWN,
		SEND_LSR,
		SEND_LSU,
		SEND_LSA,
		RECEIVE_LSR,
		RECEIVE_LSU,
		RECEIVE_LSA,
		MAXAGE,
		//End
	};

	/** States in OSPF State Machine */
	enum OSPF_States
	{ 
		//Interface States
		Down_Interface,	//Start
		Loopback,
		Waiting, 
		PointToPoint, 
		DRother, 
		Backup, 
		DR, //End 
		//Neighbor States
		Down_Neighbor, //Start
		Start,
		Attempt, 
		Init, 
		TwoWay, 
		Exstart, 
		Exchange, 
		Loading, 
		Full, //End 
		Num_States,
	};
	///These variable values are get from configuration.xml
	struct stru_OSPF_Variables
	{
		//These variable values are get from configuration.xml
		unsigned short int nOSPFVersion;
		int nRtrPri;
		int nLSRefreshtime;
		int nLSMaxage;
		int nHelloInterval;
		bool bInit;

		IPADDRESS *pstruNeighborIP;

		bool nStaticRoutingFlag;
		char* pszFilePath;		//Stores File path
		char* pszFileName;		//Stores File Name

	};
	/** Structure to strore OSPF States. */
	struct Stru_OSPF_State
	{
		OSPF_STATES InterfacePrevState;
		OSPF_STATES InterfaceCurrentState;
		OSPF_STATES NeighborPrevState;
		OSPF_STATES NeighborCurrentState;
	};
	/** Structure to store routing information */
	struct stru_OSPF_Router
	{
		OSPF_ROUTING_TABLE *pstruOSPF_RoutingTable;
		ROUTER_LSA *pstruRouterLSA;
		LSDB *pstruLSDB;
	};

	//OSPF routing table
	//
/// OSPF routing table RFC 2328 April 1998 Page :107
	struct stru_Router_OSPF_RoutingTable
	{		
	NETSIM_IPAddress szDestinationID; ///< The destination's identifier or name i,e OSPF Router ID	
	NETSIM_IPAddress szAddressMask;	 ///< Only defined for networks i,e subnet mask
	unsigned int prefix_len;
	bool bEbit;		//Optional capabilities
	bool bMCbit;	
	bool bNPbit;	
	bool bEAbit;	
	bool bDCbit;	
	unsigned short int nArea;	///< OSPF Area ID	
	OSPF_PathType nPath_type;  ///< Types of paths used to route traffic to the destination	
	unsigned int nCost;		///< The Link state cost of the destination	
	unsigned int nType_2_cost;	///< Only valid for external paths		
	char* pszLinkStateOrigin;	/**<
									Valid only for inter-area paths, this field indicates 
									the LSA(router-LSA or network-LSA) that directly references the destination
								*/		
	NETSIM_IPAddress szNexthop;	///< The outgoing router interface to use  when forwarding traffic to the destination	
	NETSIM_IPAddress szAdvertising_Router; ///< The router advertising the summary LSA or AS-external-LSA that led to this path
	struct stru_Router_OSPF_RoutingTable *pstru_Router_NextEntry;
	};
	/// OSPF - LinkState database
	struct stru_OSPF_LinkStateDatabase
	{
		ROUTER_LSA *pstruRLSA;
	};
	/// OSPF Packet Header RFC 2328 April 1998 Page :190
	struct stru_OSPF_Packet_Header
	{		
		unsigned int nVersion:8; ///< The OSPF version number		
		OSPF_PacketType Type;	///< Packet type		
		unsigned int nPacketLength:16;  ///< The length of the OSPF protocol packet in bytes		
		NETSIM_IPAddress szRouterID;   ///< The Router ID of the packet's source 		
		NETSIM_IPAddress szAreaID;	///< A 32 bit number identifying the area that this packet belongs 		
		unsigned int nCheckSum:16;  /**< 
										The standard IP checksum of the entire contents of the packet, starting with the OSPF 
										packet header but excluding the 64 bit authentication field 
									*/ 		
		unsigned int nAuType:16; ///< Identifies the authentication procedure to be used for the packet		
		unsigned long int nAuthentication;	///< A 64 bit field for use by the authentication scheme
	};
	/// OSPF Hello Packet  RFC 2328 April 1998 Page :192
	struct stru_OSPF_Hello_Packet
	{
		OSPF_PACKET_HEADER *pstruHeader;		
		NETSIM_IPAddress szNetworkMask;	///< The network mask associated with this interface		
		unsigned long int nHelloInterval; ///< The number of seconds between this router's hello packets		
		unsigned int nOptions:8; ///< The optional capabilities supported by the router			
		unsigned int nRtrPri:8;	 ///< Used in designated router election		 		
		unsigned long int nRouterDeadInterval;	///< The number of seconds before declaring a silent router down 
		NETSIM_IPAddress szDesignatedRouter;   ///< The identity of the designated router for this network, in the view of the sending router 
		NETSIM_IPAddress szBackupDesignatedRouter; ///< At the time of system failure, it will act as a designated router
		NEIGHBOR *pstruNeighbor;
	};
	/** Structure to store the Ip addresses of the Neighbours */
	struct stru_NeighborIP
	{
		NETSIM_IPAddress szNeighbor;
		struct stru_NeighborIP *pstruNextNeighborIP;
	};

	/// OSPF Database description Packet RFC 2328 April 1998 Page :195
	struct stru_OSPF_DatabaseDescription_Packet
	{
		OSPF_PACKET_HEADER *pstruHeader_DD;		
		unsigned short int nInterfaceMTU; ///< Maximum transmission unit
		bool bEbit;		
		bool bMCbit;	
		bool bNPbit;	
		bool bEAbit;	
		bool bDCbit;
		bool bIbit;
		bool bMbit;
		bool bMSbit;
		int nDDsequencenumber;
		OSPF_LSA_HEADER *pstruHeader;
	};
	/** Structure to store the LSAs */
	struct stru_LSAs
	{
		OSPF_LSA_HEADER *pstruHeader5;
		struct stru_LSAs *pstruNextLSA;
	};	
	/// OSPF Link State Request Packet RFC 2328 April 1998 Page :196
	struct stru_OSPF_LinkStateRequest_Packet
	{
		OSPF_PACKET_HEADER *pstruHeader_LSR;
		LSR *pstruLSR;
	};
	/** Structure to store the LSR */
	struct stru_LSR
	{
		OSPF_LStype LSType;		///< The LSA type	
		NETSIM_IPAddress szLinkStateID;		///< This field identifies the portion of the internet environment that is being described by the LSA	
		NETSIM_IPAddress szAdvertisingRouter;	///< The Router ID of the router that originated the LSA
		struct stru_LSR *pstruNextLSR;
	};
	/// OSPF Link State Update Packet RFC 2328 April 1998 Page :199
	struct stru_OSPF_LinkStateUpdate_Packet
	{
		OSPF_PACKET_HEADER *pstruHeader_LSU;
		unsigned short int nLSAs;
		ROUTER_LSA *pstruRouterLSA;
	};
/// OSPF Link State Acknowledgement Packet. As per RFC 2328 April 1998 Page :201
	struct stru_OSPF_LinkStateAcknowledgement_Packet
	{
		OSPF_PACKET_HEADER *pstruHeader_OSPFHeader;
		OSPF_LSA_HEADER *pstruHeader_LSA;	
	};

	/// OSPF LSA Header. As per RFC 2328 April 1998 Page:203
	struct stru_OSPF_LSA_Header
	{		
		unsigned int nLSage:16;     ///< The age of the Router LSA            
		unsigned int nOptions:8;	///< The optional capabilities supported by the described portion of the routing domain	
		OSPF_LStype LSType;			///< The LSA type	
		NETSIM_IPAddress szLinkStateID;	///< This field identifies the portion of the internet environment that is being described by the LSA
		NETSIM_IPAddress szAdvertisingRouter; ///< The Router ID of the router that originated the LSA			
		long long int nLS_SequenceNumber;	///< Detects old or duplicate LSAs		
		unsigned int nLS_Checksum:16;	///< The fletcher checksum of the complete contents of the LSA, including the LSA header but excluding the  LS age field		
		unsigned int nLength:16;	///< The length in bytes of the LSA			
	};
	
	// OSPF Router_LSAs
	
	///RFC 2328 April 1998 Page:206
	struct stru_OSPF_Router_LSA
	{
		OSPF_LSA_HEADER *pstruHeader;
		ROUTERLSA_ENTRY *pstrEntry;
		ROUTER_LSA *pstruNextRLSA;
	};
	/** Structure to store the LSA entry details */
	struct stru_RouterLSA_Entry
	{
		bool bbitV;		///< Virtual link endpoint		
		bool bbitE;	 ///< When set, the router is an AS boundary router			
		bool bbitB;	 ///< When set, the router is an area border router		
		unsigned int nLinks:16;	///< The number of router links described in this LSA		
		NETSIM_IPAddress szLinkID;	///< Identifies the object that this router link connects to		
		NETSIM_IPAddress szLinkData;///< Network mask	
		unsigned int prefix_len;
		OSPF_RouterLink RouterLink; 		
		int nTOS:8;	///< The number of different TOS metrics given for this link, not counting the required				
		int nMetric;	///< The cost of using this router link			
		int nTOSIPType;				
		int nTOSMetric;
		struct stru_RouterLSA_Entry *pstru_NextEntry;
	};
	int pcRowCost[50];                                          //for storing cost
	int costofRouters[100];
	int costofRouters_2[100];
	int allpairsshortestpaths [100][100][100];
	int nodes_array[100];
	int fn_NetSim_OSPF_Run_F();
	int fn_NP_OSPF_AddTableEntry(int,int);
	/// This function is for forming the initial LSA for each router
	int fn_NetSim_OSPF_InitialLSA_Creation(NETSIM_ID nDeviceId);
	/// This function is for forming the initial table for each router	
	int fn_NetSim_OSPF_InitialTable_Creation(NETSIM_ID nDeviceId);
	/// This function is for linkstate database creation.
	int fn_NetSim_OSPF_InitialLSDB_Creation(NETSIM_ID nDeviceId);
	/// This function is for router table formation.
	int fn_NetSim_OSPF_RouterTable(NETSIM_ID,NETSIM_IPAddress,NETSIM_IPAddress,NETSIM_IPAddress,int);
	/// This function is used to update the database of router
	int fn_NetSim_OSPF_UpdatingEntriesinRoutingDatabase(struct stru_NetSim_Network *,NETSIM_ID,NETSIM_IPAddress,NETSIM_IPAddress,unsigned int,NETSIM_IPAddress,unsigned int,int,OSPF_PathType);
	/// This function is used to update the RouterLSA of router	
	int fn_NetSim_OSPF_UpdatingEntriesinRouterLSA(NETSIM_ID nIndex,NETSIM_IPAddress szAdvIP,OSPF_LStype Lsatype,int age,int links,long long int sequence,NETSIM_IPAddress szLink,NETSIM_IPAddress szLinkdata,unsigned int prefix_len,int metric);
	int fn_NetSim_OSPFTrace(NETSIM_ID);
	// Functions under lib
	void fn_NetSim_OSPF_ChangeState(struct Stru_OSPF_State *pstruOSPFVar, OSPF_STATES nState,int);
	int fn_NetSim_OSPF_SendDDPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_SendLSAPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	/// Function to Read the static table and assign to the OSPF_TABLE 
	int fn_NetSim_StaticOSPFTable_Read(char* pszOSPFstasticTablePath);
	_declspec(dllexport)int fn_NetSim_OSPF_DesignatedRouterElection(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	_declspec(dllexport)int fn_NetSim_OSPF_DijkstraAlgorithm(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_ReceiveDDPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_ReceiveHelloPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_ReceiveLSRPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_ReceiveLSAPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_ReceiveLSUPackets(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_LSRpacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	_declspec(dllexport)int fn_NetSim_OSPF_LSApacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_Adjacency(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	/****************** NetWorkStack DLL functions declarations *****************************************/
	/** Function for configuring OSPF parameters*/
	_declspec(dllexport) int fn_NetSim_OSPF_Configure_F(void** var);
	/** Function for Intializing OSPF protocol */
	_declspec (dllexport) int fn_NetSim_OSPF_Init(struct stru_NetSim_Network *NETWORK_Formal,NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,char *pszWritePath_Formal,int nVersion_Type,void **fnPointer);
	int fn_NetSim_OSPF_Init_F(struct stru_NetSim_Network * ,NetSim_EVENTDETAILS * ,char * ,char * ,int  ,void **fnPointer);
	/** Function to run OSPF protocol */
	_declspec (dllexport) int fn_NetSim_OSPF_Run();
	/// Function to free the OSPF protocol variable and Unload the primitives
	_declspec(dllexport) int fn_NetSim_OSPF_Finish();
	int fn_NetSim_OSPF_Finish_F();
	/// Return the subevent name with respect to the subevent number for writting event trace
	_declspec (dllexport) char *fn_NetSim_OSPF_Trace(int nSubEvent);
	char *fn_NetSim_OSPF_Trace_F(int nSubEvent);
	/// Function to free the allocated memory for the OSPF packet
	_declspec(dllexport) int fn_NetSim_OSPF_FreePacket(NetSim_PACKET* );
	int fn_NetSim_OSPF_FreePacket_F(NetSim_PACKET* );
	/// Function to copy the OSPF packet from source to destination
	_declspec(dllexport) int fn_NetSim_OSPF_CopyPacket(NetSim_PACKET*  ,NetSim_PACKET* );
	int fn_NetSim_OSPF_CopyPacket_F(NetSim_PACKET* ,NetSim_PACKET* );
	_declspec(dllexport) int fn_NetSim_OSPF_Metrics(char*  );
	int fn_NetSim_OSPF_Metrics_F(char*  );
	int fn_NetSim_OSPF_Hellopacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_LSUpacketformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_OSPF_LinkStateDatbaseformation(struct stru_NetSim_Network *NETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
#ifdef  __cplusplus
}
#endif
#endif /* _OSPF_H_*/


