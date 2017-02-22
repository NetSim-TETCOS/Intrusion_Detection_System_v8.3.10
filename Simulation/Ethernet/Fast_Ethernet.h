/************************************************************************************
 * Copyright (C) 2012     
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    Thamothara Kannan                                                      *
 * ---------------------------------------------------------------------------------*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                   *
 *	THIS FILES CONTAIN FASTETHERNET DATASTUCTURE WHICH HAS VARIABLES THAT ARE PROVIDED FOR USERS.    *    
 *																								     *
 *	BY MAKING USE OF THESE VARIABLES THE USER CAN CREATE THEIR OWN PROJECT IN FASTETHERNET 	         *
 *	                                                                                                 *    
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */



#ifndef _FAST_ETHERNET_H_
#define _FAST_ETHERNET_H_
/** To enable spanning tree log file */
#define SPANNING_TREE_LOGFILE	
/** CONFIG_BPDU frame size */
#define CBPDU_PACKET_SIZE 35		

extern const double dMac_Over_Heads;
extern const double dPhy_Over_Heads;

/** Enumeration for Sub event Type*/
 enum enum_Subevent_Type
{
	CARRIER_SENSE = MAC_PROTOCOL_IEEE802_3*100+1,		
	WAIT_FOR_IFG,			
	SPANNING_TREE_PROTOCOL,		
	SWITCHING_TECHNIQUE,
	SWITCH_TABLE_FORMATION,
	SWITCH_FRAME_FORWARDING,
	PHY_SENSE_LINK,
	GET_PACKET=0,  /* Whenever a packet moves from one layer to another layer, a subevent type is assigned Zero*/  
};
/** Enumeration for Switching Technique */
 enum enum_SwitchingTechnique		
{
	STORE_FORWARD=1,
	CUT_THROUGH,		
	FRAGMENT_FREE,
};
/** Enumeration for Physical layer link status */
 enum enum_Link_State
 {
	LINK_DOWN, /* Link is free*/
	LINK_UP, 
 };
/** Enumeration for MAC Layer Packet Type*/
 enum enum_Packet_Type		
{
	CBPDU_PACKET = MAC_PROTOCOL_IEEE802_3*100+1,
	DATA_PACKET = 1,
};
/** Enumeration for Packet Transmission Type*/
 enum enum_Transmission_Type		
{
	UNI_CAST,
	BROAD_CAST,
};
/** Enumeration for  SWITCH Port state*/
enum enum_PortState
{
	DISABLED,
	LISTENING,
	LEARNING,
	FORWARDING,
	BLOCKING
};

/** Enumeration for BPDU frame type */
enum enum_BPDU_Type
{
	CBPDU_TYPE=0,
	TCN_BPDU_TYPE=128	/*As per the ieee 802.3u */
};


SUB_EVENT nSub_Event_Type;

/***************************************MAC LAYER FRAME STRUCTURE *******************************/

/** Frame Format of Configuration Bridge Protocol Data Unit */
struct stru_Bridge_Protocol_Data_Unit   
{
	BPDU_TYPE nBpdu_Type;	
	char *pszRoot_id;	/**< To store the root switch id */	
	char *pszBridge_id;	/**< To store the transmitter switch id */		
	int nRoot_Path_cost; /**< To store the path cost*/	
	NETSIM_ID nPort_id;	 /**< To store the transmitter port id */		
	
	/* This Parameter is used in NetSim to get the device details*/	
	NETSIM_ID nTx_DeviceId;	/**< To store the transmiiter switch no */		

	/* The following Parameters are not used in NetSim currently*/
	/* This can be used in future for Topology Changed Notification 
	BPDU frame for spanning tree calculation*/
	double dMessage_Age;		
	double dMax_Age;		
	double dHello_Time;
	double dForward_delay;
	FLAG nTopology_Change_Acknowledgment;
	FLAG nTopology_Change;
};
/** Packet at Mac layer */
struct stru_Frame_MacLayer				
{	
	MAC_PACKET_TYPE nPacket_Type; /**< 0-CONFIG_BPDU packet, 1 -DATA packet */	
	FRAME_TX_FLAG nTx_Flag;	/**< 0-unicast packet,1- broad cast packet */			
	CONFIG_BPDU *pstruCBPDU;				
};

/*******************************MAC LAYER FRAME STRUCTURE END***********************************/


/******************************* MAC LAYER STRUCTURE START ******************************/
/** To store the Switch table */
struct stru_Device_SwitchTable			
{	
	int nCost;	/**< path cast between two switch */			
	NETSIM_ID nOut_Port;/**< Output port */		
	NETSIM_ID nNode_Id;	/**< Connected Node Id */	
	char *pszPort_MAC_Address;	/**< Switch port mac address */		
	
};
/** To store the Bridge Data */
struct stru_BridgeData
{	
	char *pszDesignated_root;	/**< To store the root switch id */	
	char *pszBridge_id;	/**< To store the transmitter switch id */		
	NETSIM_ID nRoot_Port;	
	int nRoot_Path_cost; /**< To store the path cost */		
	NETSIM_ID nPort_id;	/**< To store the transmitter port id */			
	
/* This Parameter is used in NetSim to get the device structure*/
	/** To store the transmiiter switch no */
	NETSIM_ID nTx_DeviceId;			
	char *pszTx_Bridge_id;
	/* The following Parameters are not used in NetSim currently*/
	/* This can be used in future for Topology changed Notification 
	BPDU frame for spanning tree calculation*/	
	double dBridge_Max_Age;		
	double dBridge_Hello_Time;
	double dBridge_Forward_delay;
	double dTopology_Change_Time;
	double dHold_Time;
	FLAG nTopology_Change_Detected;
	FLAG nTopology_Change;
};
/** To store the Bridge port details*/
struct stru_PortData
{
	int nPort_id;	/**< To store the bridge port id */	
	PORT_STATE nPort_state; /**< It will indicate Port state */
	int nPath_Cost;
	char *pszDesignated_root;
	char *pszDesignated_bridge;
	int nDesignated_port;
	int nDesignated_cost;
	/* The following Parameters are not used in NetSim currently*/
	/* This can be used in future for Topology changed Notification 
	BPDU frame for spanning tree calculation*/	
	FLAG nTopology_Change_Acknowledge;
	FLAG nConfig_pending;
	FLAG nChange_Detection_Enabled;
};

/** To store the Device Physical layer variables */
struct stru_Device_PhysicalLayer
{	
	int nCost;	
	double dData_Start_Time;	/**< To find the start time of the next data */		
	double dData_End_Time;		/**< To find the end time of the next data */	
	double dDataErrored;		/**< Store the error data */	
	LINK_STATE nLinkState;		/**< To find link is idle(0) or busy(1) */		
	char *pszEthernet_Standard;	/**< To find the Etherenet standard*/		
	
};
/** To store the Device variables*/
struct stru_Device_Variable
{
	char *pszSwitch_Priority;	
	SWITCHING_TECHNIQUE_TYPES nSwitchingTechnique;	/**< Different switching techniques */
	SWITCH_TABLE **ppstruSwitchTable;
	char *pstruSpanningTreeProtocol;
	char *pstruEthernetStandard;
	double dSwitchingLatency;	/**<
									Stores the switching latency 802.3u  
									Store and Forward	Latency = 21.7 micro sec
									Cut Through			Latency = 0.19 micro sec
									Fragment Free		Latency = 0.91 micro sec
								*/  
	double dTotalLatency;
	BRIDGE_DATA *pstruBridgeVar;
};
/* Frame Structure variable declaration*/
FRAME_MAC *pstruMac_Layer_Frame;
CONFIG_BPDU *pstruCBPDU;
/* Device Structure variable declaration*/
SWITCH_TABLE **ppstruSwitchTable;
DEVICE_VARIABLES *pstruDevice;
BRIDGE_DATA *pstruBridge_Var;
DEVICE_PHYSICALLAYER *pstruDevice_Phy_Layer;
PORT_DATA *pstruPortVar;



/******************************* MAC LAYER STRUCTURE END ******************************/

/******************************* MAC LAYER FUNCATION DECLARATION ******************************/
/** 
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	This function is used to add the given string into head or tail of the original string 
		 pszString is original string 
		 pszAddstring need to be add 
		 ncount number of time to add 
		 nPosition -- head/ tail 
		 return concatinated string 
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
char *fn_NetSim_FastEthernet_Addstring(char *pszString,char *pszAddstring,int nCount,int nPosition);

/** Forward the frame from MAC_IN to next layer */
_declspec(dllexport) int fn_NetSim_FastEthernet_Frame_Forward_MacIn(NetSim_EVENTDETAILS *);

/** 
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	To check switch buffer for over flow 
	arg 2 -- packet size
	arg 3-- port number
	return 2 -- buffer overflow
	return 1 -- buffer underflow
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_FastEthernet_Switch_BufferCheck(NetSim_EVENTDETAILS *,double,int);

/** Store the Ethernet config file variables in to the respective structure*/
int fn_NetSim_FastEthernet_Config(char *pszSection,char *pszName,char *pszValue,int nLineNo);

 /** To free the MAC protocol variable and Unload the primitives*/
_declspec(dllexport) int fn_NetSim_Ethernet_Finish();
_declspec (dllexport) int fn_NetSim_Ethernet_Finish_F();
/** To find spanning tree*/
int  fn_NetSim_FastEthernet_Spanning_Tree_Protocol(NetSim_EVENTDETAILS *);
/**  To Transmit the config BDPU*/
int fn_NetSim_FastEthernet_TransmitConfig_BPDU(NETSIM_ID ,NetSim_EVENTDETAILS *);

/** To initialize the Bridge Variables*/
int fn_NetSim_FastEthernet_Initialisation();
/** To initialize the Bridge port Variables*/
int fn_NetSim_FastEthernet_Initialize_port(NETSIM_ID);
/** To Display the spanning tree */
int fn_NetSim_FastEthernet_Switch_Display_Spanning_Tree();
/** To initialize the Network structure*/
_declspec (dllexport) int fn_NetSim_Ethernet_Init(struct stru_NetSim_Network *NETWORK_Formal,NetSim_EVENTDETAILS *pstruEventDetails_Formal,const char *pszAppPath_Formal,const char *pszIOPath_Formal,int nVersion_Type);
_declspec (dllexport) int fn_NetSim_Ethernet_Init_F(struct stru_NetSim_Network *,NetSim_EVENTDETAILS *,const char *,const char *,int);

/* Fast Ethernet functionality */
_declspec (dllexport) int fn_NetSim_Ethernet_Run();
/*Free the mac layer packet */
_declspec (dllexport) int fn_NetSim_Ethernet_FreePacket(NetSim_PACKET *);
_declspec (dllexport) int fn_NetSim_Ethernet_FreePacket_F(NetSim_PACKET *);
/*Copy the mac layer packet */
_declspec(dllexport) int fn_NetSim_Ethernet_CopyPacket(NetSim_PACKET *,NetSim_PACKET *);
_declspec (dllexport) int fn_NetSim_Ethernet_CopyPacket_F(NetSim_PACKET *,NetSim_PACKET *);
/** To return the subevent name with respect to the subevent number for writting event trace*/
_declspec (dllexport) char *fn_NetSim_Ethernet_Trace(int nSubEvent);
_declspec (dllexport) char *fn_NetSim_Ethernet_Trace_F(int nSubEvent);
/** To move the frame to link*/
_declspec(dllexport) int fn_NetSim_FastEthernet_Physical_Out_Event();
/** To receive the frame from the Link*/
_declspec(dllexport) int fn_NetSim_FastEthernet_Physical_In_Event();
/* To down the link state after frame Transmitted*/
_declspec(dllexport) int fn_NetSim_FastEthernet_Phy_Sense_Link();
/** Fast ethernet configuration function*/
_declspec(dllexport) int fn_NetSim_Ethernet_Configure(void** var);
int fn_NetSim_Ethernet_Configure_F(void** var);	
int fn_NetSim_FastEthernet_Carrier_Sense(NetSim_EVENTDETAILS *pstruEventDetails);
int fn_NetSim_FastEthernet_IFG(NetSim_EVENTDETAILS* pstruEventDetails);
int fn_NetSim_FastEthernet_Switching_Techniques(NetSim_EVENTDETAILS *pstruEventDetails);
int fn_NetSim_FastEthernet_Switch_Table_Formation(NetSim_EVENTDETAILS *pstruEventDetails);
int fn_NetSim_FastEthernet_Switch_Frame_Forwarding(NetSim_EVENTDETAILS *pstruEventDetails);
int fn_NetSim_FastEthernet_Config_bpdu_generation();
int fn_NetSim_FastEthernet_ReceiveConfig_BPDU(int nPort_No,FRAME_MAC *pstruConfig, NetSim_EVENTDETAILS * pstruEventDetails);
#endif /* _FAST_ETHERNET_H_*/
