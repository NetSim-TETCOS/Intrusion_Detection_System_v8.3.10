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

/****************************************************
RFC 4271          Border Gateway Protocol      January 2006
*****************************************************/

#ifndef _NETSIM_BGP_H_
#define _NETSIM_BGP_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "BGP_enum.h"

	//The default values for various parameters in BGP
#define BGP_VERSION											4 /**< Represents the version number of the BGP */
#define BGP_AUTONOMOUS_SYSTEM_NUMBER_DEFAULT				1 /**< Default value of the Autonomus system */
#define BGP_LOCAL_PREFERENCE_DEFAULT						100 /**< Default value of the Local preference as per RFC 4271 */
#define BGP_MULTI_EXIT_DISCRIMINATOR_DEFAULT				0	/**< Default value of the Multi exit discriminatior as per RFC 4271 */
#define BGP_CONNECTRETRYTIMER_DEFAULT						120 /**< Default value of the connect retry timer as per RFC 4271 */
#define BGP_HOLDTIMER_DEFAULT								90*SECOND /**< Default value of the Hold timer as per RFC 4271 */
#define BGP_LARGE_HOLD_TIME_DEFAULT							240*SECOND /**< Default value of the large hold time as per RFC 4271 */
#define BGP_KEEPALIVETIMER_DEFAULT							30*SECOND  /**< Default value of the keep alive timer as per RFC 4271 */
#define BGP_MINASORIGINATIONINTERVAL_DEFAULT				15*SECOND  /**< Default value of the minimum atonomus system origination interval as per RFC 4271 */
#define BGP_MINEXTROUTEADVINTERVAL_DEFAULT					30*SECOND  /**< Default value of the minimum external router advertisement interval as per RFC 4271 */
#define BGP_MININTROUTEADVINTERVAL_DEFAULT					5*SECOND   /**< Default value of the minimum internal router advertisement interval as per RFC 4271*/
#define BGP_DELAYOPENTIMER_DEFAULT							0		   /**< Default value of the Dealy open timer as per RFC 4271 */
#define BGP_IDLEHOLDTIMER_DEFAULT							10*SECOND  /**< Default value of the Idle hold timer as per RFC 4271 */
#define BGP_DESTINATION_PORT								179 /** Destination port number */

	//The BGP Control packet size

#define MAXIMUM_MESSAGE_SIZE 4096	///< Minimum size of the message in bytes as per RFC 4271
#define BGP_HEADER_SIZE 19	///< Header size in bytes as per RFC 4271
#define BGP_OPEN_MESSAGE_SIZE 29	///< Open message size in bytes, without including the optional parameter size as per RFC 4271
#define BGP_UPDATE_MESSAGE_SIZE 23	///< Update message size in bytes as per RFC 4271
#define BGP_KEEPALIVE_MESSAGE_SIZE 19	///< Keep alive message size in bytes as per RFC 4271
#define BGP_NOTIFICATION_MESSAGE_SIZE 21 ///< Notification message size in bytes as per RFC 4271
#define IP_LENGTH _NETSIM_IP_LEN
	//typedef for the enumeration
	typedef enum enum_BGP_Subevent BGP_SUBEVENT;
	typedef enum enum_BGP_ControlPacket BGP_CONTROL_PACKET;
	typedef	enum enum_Attribute_Type_Codes BGP_ATTRIBUTE_TYPE_CODES;
	typedef enum enum_BGP_FSMStates BGP_STATES;
	//typedef for the structures
	typedef struct stru_NetSim_BGP_Open BGP_OPEN;
	typedef struct stru_NetSim_BGP_Update BGP_UPDATE;
	typedef	struct stru_NetSim_BGP_Keepalive BGP_KEEPALIVE;
	typedef struct stru_NetSim_BGP_Notification BGP_NOTIFICATION;
	typedef struct stru_NetSim_BGP_Header BGP_HEADER;
	typedef struct stru_NetSim_BGP_Withdrawn_Routes WITHDRAWN_ROUTES;
	typedef struct struAS_PATH ASPATH; 
	typedef struct stru_PathAttributes PATH_ATTRIBUTES;
	typedef struct stru_NLRI NLRI;
	typedef struct stru_BGP_RoutingTable BGP_ROUTING_TABLE;
	typedef struct stru_BGP_Router BGP_ROUTING_INFORMATION_BASE;
	/// BGP FSM States
	enum enum_BGP_FSMStates
	{
		Idle=1,
		Connect,
		Active,
		OpenSent,
		OpenConfirm,
		Established,
	};
	/// BGP Control Packets
	enum enum_BGP_ControlPacket
	{
		ctrlPacket_OPEN=APP_PROTOCOL_BGP*100+1,
		ctrlPacket_UPDATE,
		ctrlPacket_KEEPALIVE,
		ctrlPacket_NOTIFICATION,
	};
	enum enum_Attribute_Type_Codes
	{
		ORIGIN=1,
		AS_PATH,
		NEXT_HOP,
		MULTI_EXIT_DISC,
		LOCAL_PREF,
		ATOMIC_AGGREGATE,
		AGGREGATOR,
	};

	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Message Header Format

	Each message has a fixed-size header.  There may or may not be a data
	portion following the header, depending on the message type.  The
	layout of these fields is shown below:

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                                                               +
	|                                                               |
	+                                                               +
	|                           Marker                              |
	+                                                               +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|          Length               |      Type     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/
	struct stru_NetSim_BGP_Header
	{
		long double Marker;  ///< This 16-octet field is included for compatibility; it MUST be set to all ones.
		unsigned short int Length; /**<
											This 2-octet unsigned integer indicates the total length of the
											message, including the header in octets.  Thus, it allows one to locate the (Marker field of the) next message in the TCP
											stream.  The value of the Length field MUST always be at least 19 and no greater than 4096, and MAY be further constrained,
											depending on the message type.  "padding" of extra data after the message is not allowed.  Therefore, the Length field MUST
											have the smallest value required, given the rest of the message.
								   */
		unsigned short int Type:8; /**<
										This 1-octet unsigned integer indicates the type code of the message.  
										This document defines the following type codes:

											1 - OPEN
											2 - UPDATE
											3 - NOTIFICATION
											4 - KEEPALIVE
								   */
	};
	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	OPEN Message Format

	In addition to the fixed-size BGP header, the OPEN message contains
	the following fields:

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+
	|    Version    |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     My Autonomous System      |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Hold Time           |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                         BGP Identifier                        |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	| Opt Parm Len  |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	|             Optional Parameters (variable)                    |
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/

	struct stru_NetSim_BGP_Open
	{
		BGP_HEADER *pstruHeader; 
		unsigned short int Version:8; ///< This 1-octet unsigned integer indicates the protocol version number of the message.  The current BGP version number is 4.
		unsigned short int My_Autonomous_System; ///< This 2-octet unsigned integer indicates the Autonomous System number of the sender.
		double Hold_Time; /**< 
								This 2-octet unsigned integer indicates the number of seconds the sender proposes for the value of the Hold Timer.  Upon
								receipt of an OPEN message, a BGP speaker MUST calculate the value of the Hold Timer by using the smaller of its configured
								Hold Time and the Hold Time received in the OPEN message.  The Hold Time MUST be either zero or at least three seconds.  An
								implementation MAY reject connections on the basis of the Hold Time.  The calculated value indicates the maximum number of
								seconds that may elapse between the receipt of successive KEEPALIVE and/or UPDATE messages from the sender.
						  */
		NETSIM_IPAddress BGP_Identifier; /**<
												This 4-octet unsigned integer indicates the BGP Identifier of the sender.  A given BGP speaker sets the value of its BGP
												Identifier to an IP address that is assigned to that BGP speaker.  The value of the BGP Identifier is determined upon
												startup and is the same for every local interface and BGP peer.
										 */
		unsigned short int Optional_Parameters_Length:8; /**<
															This 1-octet unsigned integer indicates the total length of the Optional Parameters field in octets.  If the value of this
															field is zero, no Optional Parameters are present.
														 */
		unsigned short int Parameter_Type:8; /**<
												 This field contains a list of optional parameters, in which each parameter is encoded as a <Parameter Type, Parameter
												 Length, Parameter Value> triplet.
											 */
		unsigned short int Parameter_Length:8;
	};

	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	This is a variable-length field that contains a list of IP address prefixes for the routes that are being withdrawn from
    service.  Each IP address prefix is encoded as a 2-tuple of the form <length, prefix>, whose fields are described below:

                  +---------------------------+
                  |   Length (1 octet)        |
                  +---------------------------+
                  |   Prefix (variable)       |
                  +---------------------------+
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/
	struct stru_NetSim_BGP_Withdrawn_Routes
	{
		unsigned short int Length:8; 
		NETSIM_IPAddress szIPaddress;
		struct stru_NetSim_BGP_Withdrawn_Routes *pstruNextRoute;
	};
	/**
	AS_PATH is a well-known mandatory attribute that is composed of a sequence of AS path segments. 
	Each AS path segment is represented by a triple  <path segment type, path segment length, path segment value>.
	*/ 
	struct struAS_PATH 
	{
		unsigned int path_segment_type;
		unsigned int path_segment_length;
		unsigned int path_segment_value;
	};
	/**
	 A variable-length sequence of path attributes is present in every UPDATE message, except for an UPDATE message that carries
     only the withdrawn routes.  Each path attribute is a triple <attribute type, attribute length, attribute value> of variable
     length.
	*/
	struct stru_PathAttributes
	{
		unsigned short int AttrFlags:8;
		BGP_ATTRIBUTE_TYPE_CODES nAttributetypecode;
		unsigned int attribute_length;
	};
	/// Network Layer Reachability Information.
	struct stru_NLRI
	{
		unsigned short int Length:8;
		NETSIM_IPAddress Prefix;
		unsigned int ORIGIN;
		int nASpathlength;
		int *nAs_path;
		NETSIM_IPAddress NEXT_HOP;
		unsigned int MULTI_EXIT_DISC;
		unsigned int LOCAL_PREF ;
		unsigned int ATOMIC_AGGREGATE;
		unsigned int AGGREGATOR;
		struct stru_NLRI *pstruNextNLRI;
		NETSIM_IPAddress bgpSubnetMask;
		unsigned int prefix_len;
	};
	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	UPDATE Message Format

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
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/
	struct stru_NetSim_BGP_Update
	{
		BGP_HEADER *pstruHeader;
		unsigned short int Withdrawn_Routes_Length;
		WITHDRAWN_ROUTES *pstruWithdrawn_routes;
		unsigned short int Total_PathAttribute_Length;
		PATH_ATTRIBUTES *pstruPathAttribute;
		NLRI *pstruNLRI;
	};

	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	KEEPALIVE Message Format
	A KEEPALIVE message consists of only the message header and has a length of 19 octets.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/
	struct stru_NetSim_BGP_Keepalive
	{
		BGP_HEADER *pstruHeader;
	};

	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	NOTIFICATION Message Format

	A NOTIFICATION message is sent when an error condition is detected.
	The BGP connection is closed immediately after it is sent.
	In addition to the fixed-size BGP header, the NOTIFICATION message
	contains the following fields:

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	| Error code    | Error subcode |   Data (variable)             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/

	struct stru_NetSim_BGP_Notification
	{
		BGP_HEADER *pstruHeader;
		unsigned short int Error_Code:8;
		unsigned short int Error_subcode:8;
		unsigned int Data;
	};

	/// The structure to store the input parameters of BGP

	struct stru_BGP
	{
		unsigned int nAutonomous_system_number;
		unsigned int nLocal_preference;
		unsigned int Multi_Exit_Discriminator;
		double dConnectRetryTimerConfigured;
		double dHoldTimerConfigured;
		double dLarge_Hold_timeConfigured ;
		double dKeepaliveTimerConfigured;
		double dMinimum_ASOriginationIntervalTimerConfigured;
		double dMinExtRouteAdvIntervalConfigured;
		double dMinIntRouteAdvIntervalConfigured;
		double dDelayOpenTimerConfigured;
		double dIdleHoldTimerConfigured;
		//BGP statistics
		//
		unsigned int bgpPeerInUpdates;			
		unsigned int bgpPeerOutUpdates;			
		unsigned int bgpPeerInTotalMessages;	
		unsigned int bgpPeerOutTotalMessages;	
		unsigned int bgpPeerLastError;															
		unsigned int bgpPeerFsmEstablishedTransitions;
		double bgpPeerFsmEstablishedTime;			
		double bgpPeerConnectRetryInterval;	
		double bgpPeerMinASOriginationInterval;
		double bgpPeerHoldTime;					
		double bgpPeerKeepAlive;	
		double bgpPeerInUpdateElapsedTime;
		unsigned int bgpPeerAdminStatus;
		double bgpPeerMinExtRouteAdvInterval;
		BGP_STATES nBGPState;
	};

	/// BGP routing table

	struct stru_BGP_RoutingTable
	{
		NETSIM_IPAddress bgpPeerIdentifier;		///< The BGP Identifier of this entry's BGP peer
		BGP_STATES bgpPeerState;				///< The BGP peer connection state
		/**
		idle(1),
		connect(2),
		active(3),
		opensent(4),
		openconfirm(5),
		established(6)*/
		unsigned int bgpPeerAdminStatus;		/**< The desired state of the BGP connection.
												A transition from 'stop' to 'start' will
												cause the BGP Start Event to be generated.
												A transition from 'start' to 'stop' will
												cause the BGP Stop Event to be generated.
												This parameter can be used to restart BGP
												peer connections.*/
		unsigned int bgpPeerNegotiatedVersion; ///< The negotiated version of BGP running
		//between the two peers
		NETSIM_IPAddress bgpPeerLocalAddr;	   ///< The local IP address of this entry's BGP
		//connection
		unsigned int bgpPeerLocalPort;		   ///< The local port for the TCP connection
		// between the BGP peers
		NETSIM_IPAddress bgpPeerRemoteAddr;    ///< The remote IP address of this entry's BGP peer
		unsigned int bgpPeerRemotePort;		   /**< The remote port for the TCP connection
											   between the BGP peers.  Note that the
											   objects bgpPeerLocalAddr,
											   bgpPeerLocalPort, bgpPeerRemoteAddr and
											   bgpPeerRemotePort provide the appropriate
											   reference to the standard MIB TCP
											   connection table*/
		unsigned int bgpPeerRemoteAs;			///< The remote autonomous system number
		unsigned int bgpPeerInUpdates;			/**< The number of BGP UPDATE messages
												received on this connection.  This object
												should be initialized to zero (0) when the
												connection is established*/
		unsigned int bgpPeerOutUpdates;			/**< The number of BGP UPDATE messages
												transmitted on this connection.  This
												object should be initialized to zero (0)
												when the connection is established*/
		unsigned int bgpPeerInTotalMessages;	/**< The total number of messages received
												from the remote peer on this connection.
												This object should be initialized to zero
												when the connection is established*/
		unsigned int bgpPeerOutTotalMessages;	/*The total number of messages transmitted to
												the remote peer on this connection.  This
												object should be initialized to zero when
												the connection is established*/
		unsigned int bgpPeerLastError;			/**< The last error code and subcode seen by this
												peer on this connection.  If no error has
												occurred, this field is zero.  Otherwise, the
												first byte of this two byte OCTET STRING
												contains the error code, and the second byte
												contains the subcode*/
		unsigned int bgpPeerFsmEstablishedTransitions;/**< The total number of times the BGP FSM
													  transitioned into the established state*/
		double bgpPeerFsmEstablishedTime;			/**< This timer indicates how long (in
													seconds) this peer has been in the
													Established state or how long
													since this peer was last in the
													Established state.  It is set to zero when
													a new peer is configured or the router is
													booted*/
		double bgpPeerConnectRetryInterval;			/**< Time interval in seconds for the
													ConnectRetry timer.  The suggested value
													for this timer is 120 seconds*/
		double bgpPeerHoldTime;						/**< Time interval in seconds for the Hold
													Timer established with the peer.  The
													value of this object is calculated by this
													BGP speaker by using the smaller of the
													value in bgpPeerHoldTimeConfigured and the
													Hold Time received in the OPEN message.
													This value must be at lease three seconds
													if it is not zero (0) in which case the
													Hold Timer has not been established with
													the peer, or, the value of
													bgpPeerHoldTimeConfigured is zero (0)*/
		double bgpPeerKeepAlive;					/**< Time interval in seconds for the KeepAlive
													timer established with the peer.  The value
													of this object is calculated by this BGP
													speaker such that, when compared with
													bgpPeerHoldTime, it has the same
													proportion as what
													bgpPeerKeepAliveConfigured has when
													compared with bgpPeerHoldTimeConfigured.
													If the value of this object is zero (0),
													it indicates that the KeepAlive timer has
													not been established with the peer, or,
													the value of bgpPeerKeepAliveConfigured is
													zero (0)*/		
		double bgpPeerHoldTimeConfigured;			/**< Time interval in seconds for the Hold Time
													configured for this BGP speaker with this
													peer.  This value is placed in an OPEN
													message sent to this peer by this BGP
													speaker, and is compared with the Hold
													Time field in an OPEN message received
													from the peer when determining the Hold
													Time (bgpPeerHoldTime) with the peer.
													This value must not be less than three
													seconds if it is not zero (0) in which
													case the Hold Time is NOT to be
													established with the peer.  The suggested
													value for this timer is 90 seconds.*/
		double bgpPeerKeepAliveConfigured;			/**< Time interval in seconds for the
													KeepAlive timer configured for this BGP
													speaker with this peer.  The value of this
													object will only determine the
													KEEPALIVE messages' frequency relative to
													the value specified in
													bgpPeerHoldTimeConfigured; the actual
													time interval for the KEEPALIVE messages
													is indicated by bgpPeerKeepAlive.  A
													reasonable maximum value for this timer
													would be configured to be one
													third of that of
													bgpPeerHoldTimeConfigured.
													If the value of this object is zero (0),
													no periodical KEEPALIVE messages are sent
													to the peer after the BGP connection has
													been established.  The suggested value for
													this timer is 30 seconds.*/
		double bgpPeerMinASOriginationInterval;		/**< Time interval in seconds for the
													MinASOriginationInterval timer.
													The suggested value for this timer is 15
													seconds*/
		double bgpPeerMinExtRouteAdvInterval;      /**< Time interval in seconds for the
												   MinRouteAdvertisementInterval timer.
												   The suggested value for this timer is 30
												   seconds*/
		double bgpPeerInUpdateElapsedTime;			/**< Elapsed time in seconds since the last BGP
													UPDATE message was received from the peer.
													Each time bgpPeerInUpdates is incremented,
													the value of this object is set to zero
													(0)*/
		NETSIM_IPAddress bgpNextHop;
		NETSIM_IPAddress bgpSubnetMask;
		int nASpathlength;
		struct stru_BGP_RoutingTable *pstruNextEntry;
	};
	/// Data structure for BGP router
	struct stru_BGP_Router 
	{
		BGP_ROUTING_TABLE *pstruBGPTable;
		BGP_ROUTING_TABLE *pstruBGPAdj_RIB_IN;   
		BGP_ROUTING_TABLE *pstruBGPAdj_RIB_OUT;
		BGP_ROUTING_TABLE *pstruBGPAdj_LOC_RIB;
	};

	NETSIM_ID fn_NetSim_FindAS(NETSIM_ID nDeviceId);
	int fn_NetSim_BGP_Run_F();

	/// This function for forming the initial table for each router	
	int fn_NetSim_BGP_InitialTable_Creation(NETSIM_ID nDeviceId);
	/// This function is to initialize the RIB for each router	
	int fn_NetSim_BGP_InitializingRIB(NETSIM_ID nDeviceId);
	// This function is used to update the routing entries in the database	
	int fn_NetSim_BGP_UpdatingEntriesinRoutingDatabase(struct stru_NetSim_Network *NETWORK,NETSIM_ID nIndex,\
		NETSIM_ID nInterfaceid,NETSIM_IPAddress bgpPeerRemoteAddr,\
		unsigned int bgpPeerRemoteAs,NETSIM_IPAddress bgpNextHop,NETSIM_IPAddress bgpSubnetMask,int nRIBFlag,int nASpathlength);
	int fn_NetSim_BGP_KeepAliveMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_BGP_OpenMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_BGP_PathVectorRouting(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	int fn_NetSim_BGP_UpdateMessageformation(struct stru_NetSim_Network *pstruNETWORK,NetSim_EVENTDETAILS *pstruEventDetails);
	/****************** NetWorkStack DLL functions declarations *****************************************/
	/** Function for configuring BGP parameters*/
	_declspec(dllexport) int fn_NetSim_BGP_Configure_F(void** var);
	/** Function for Intializing BGP protocol */
	_declspec (dllexport) int fn_NetSim_BGP_Init(struct stru_NetSim_Network *NETWORK_Formal,NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,char *pszWritePath_Formal,int nVersion_Type,void **fnPointer);
	int fn_NetSim_BGP_Init_F(struct stru_NetSim_Network * ,NetSim_EVENTDETAILS * ,char * ,char * ,int  ,void **fnPointer);
	/** Function to run BGP protocol */
	_declspec (dllexport) int fn_NetSim_BGP_Run();
	/// Function to free the BGP protocol variable 
	_declspec(dllexport) int fn_NetSim_BGP_Finish();
	int fn_NetSim_BGP_Finish_F();
	/// Return the subevent name with respect to the subevent number for writting event trace
	_declspec (dllexport) char *fn_NetSim_BGP_Trace(NETSIM_ID nSubEvent);
	char *fn_NetSim_BGP_Trace_F(NETSIM_ID nSubEvent);
	/// Function to free the allocated memory for the BGP packet
	_declspec(dllexport) int fn_NetSim_BGP_FreePacket(NetSim_PACKET* );
	int fn_NetSim_BGP_FreePacket_F(NetSim_PACKET* );
	/// Function to copy the BGP packet from source to destination
	_declspec(dllexport) int fn_NetSim_BGP_CopyPacket(const NetSim_PACKET*  ,const NetSim_PACKET* );
	int fn_NetSim_BGP_CopyPacket_F(const NetSim_PACKET* ,const NetSim_PACKET* );

	_declspec(dllexport) int fn_NetSim_BGP_Metrics(char*  );
	int fn_NetSim_BGP_Metrics_F(char*  );

#ifdef  __cplusplus
}
#endif
#endif