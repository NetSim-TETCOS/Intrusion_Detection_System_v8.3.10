/************************************************************************************
 * Copyright (C) 2013     
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    P.Sathishkumar                                                        *
 * ---------------------------------------------------------------------------------*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                       *
 *	THIS FILES CONTAINS TCP DATASTUCTURE WHICH HAS VARIABLES THAT ARE PROVIDED FOR USERS.*    
 *																						 *
 *	BY MAKING USE OF THESE VARIABLES THE USER CAN CREATE THEIR OWN PROJECT IN TCP 		 *
 *	                                                                                     *    
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _TCP_H_
#define _TCP_H_
#ifdef  __cplusplus
extern "C" {
#endif
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"Metrics.lib")

#include "NetSim_Graph.h"

#define DEFAULT_WINDOW_SIZE 8 ///< Default window size in bytes
#define MIN_TCP_MSS 512 ///< Minimum value for Maximum Segment Size (MSS) in bytes
#define MAX_TCP_MSS 65535 ///< Maximum value for Maximum Segment Size (MSS) in bytes.
#define DEFAULT_TCP_MSS 1460 ///< Default value for Maximum Segment Size (MSS) in bytes.
#define TRANSPORT_TCP_OVERHEADS 20 ///< TCP over heads in bytes
#define MIN_SEQUENCE_NO 0 ///< As per RFC 793 September 1981, [Page 23], Sequence number ranges from 0 to 2**32 - 1 
#define MAX_SEQUENCE_NO 4294967295 ///< As per RFC 793 September 1981, [Page 23], Sequence number ranges from 0 to 2**32 - 1 
#define MAX_ssthresh 65535 ///< Congestion window ans ssthresh initial value as per RFC 2001 January 1997, [Page 2] 
#define INITIAL_RTO_TIME 3*SECOND ///< RTO timer value for the first segment is defaultly set as 3 seconds, as per RFC 1122 October 1989, page no 96
#define MAX_RTO_TIME 60*SECOND ///< Maximum RTO time is 1 minute 
#define MIN_RTO_TIME 10*MILLISECOND ///< Minimum RTO time is 10 ms
#define MSL 120000000 ///< Maximum MSL time is 2 minutes
#define THREE_DUP_ACK 3 ///< This flag is used in Tahoe congestion contol algorithm to retransmit the packet
#define TCP_MAX_PACKET_TRACE_FIELD 8 /**<
										Represents Maximum number of Packet trace fields of TCP.
									*/

#define ISN	0x01
#define IRS 0x01

#define MAX_TTL 255 ///< This is for Transport layer control packets.
#define SND_SEG_SEQ(pPacket) ((struct stru_Segment_Header_TCP*)PACKET_TRXPROTOCOLDATA(pPacket))->ulnSND_SEG_SEQ ///< Macro to get the ulnSND_SEG_SEQ of a packet.
#define SEG_SEQ(pPacket) ((struct stru_Segment_Header_TCP*)PACKET_TRXPROTOCOLDATA(pPacket))->ulnSequence_Number ///< Macro to get the Segment Sequence number.
#define SEG_ACK(pPacket) ((struct stru_Segment_Header_TCP*)PACKET_TRXPROTOCOLDATA(pPacket))->ulnAcknowledgment_Number ///< Macro to get the Acknowledgement of a particular segment.
typedef struct stru_Options_TCP TCP_HEADER_OPTIONS; ///< TCP Segment header options.
typedef struct stru_Segment_Header_TCP SEGMENT_HEADER_TCP; ///< TCP Segment header.
typedef struct stru_TCP_Metrics TCP_METRICS; ///< TCP Metrics
typedef struct stru_TCP_Config_Variable TCP_CONFIG_VARIABLE; ///< To store User Inputs from configuration.xml
typedef struct stru_Transmission_Control_Block TCB;	 ///< As per RFC 793 September 1981, [Page 18], to store TCP variables.

/* Typedef declaration of enumaration */ 
typedef enum enum_TCP_Subevent_Type SUB_EVENT; /**< Enumeration for Sub event type.*/
typedef enum enum_Control_Packet_Type TCP_SEGMENT_TYPE; /**< Enumeration for Control Packet Type.*/ 
typedef enum enum_TCP_Connection_State TCP_CONNECTION_STATE; /**< Enumeration for TCP Connection States. */
typedef enum enum_TCP_Congestion_Algorithm TCP_CONGESTION_CONTROL_ALGORITHM; /**< Enumeration for TCP Congestion control algorithm.*/
typedef enum enum_TCP_isConfigured enum_TCP_IS_CONFIGURED; /**< Enumeration to either Enable or Disable the TCP.*/

/**Enumeration for TCP Segment Type*/
 enum enum_Control_Packet_Type		
{
	TCP_URG = TX_PROTOCOL_TCP*100+1,
	TCP_ACK,
	TCP_PSH,
	TCP_RST,
	TCP_SYN,
	TCP_FIN,
	TCP_SYN_ACK,
	TCP_FIN_DATA,
};

/** Enumeration for TCP Congestion control algorithm*/
enum enum_TCP_Congestion_Algorithm
{
	OLD_TAHOE=1,
	TAHOE,
	RENO,
	NEWRENO,
};


/** Enumeration for TCP Sub event Type*/
 enum enum_TCP_Subevent_Type
{
	TCP_ACTIVE_OPEN = TX_PROTOCOL_TCP*100+1,
	TCP_PASSIVE_OPEN,
	TCP_SEND_SYN_ACK,
	TCP_RECEIVE_SYN_ACK,
	TCP_SEND_SEG, 
	TCP_RECEIVE_SEG,
	TCP_SEND_ACK,
	TCP_RECEIVE_ACK,
	TCP_SEND_FIN_SEG,
	TCP_RECIEVE_FIN_SEG,
	TCP_SEND_FIN_ACK,
	TCP_RECEIVE_FIN_ACK,
	TCP_RETRANSMISSION_TIMEOUT,
	TCP_TIME_WAIT_TIMEOUT,
	TCP_USER_TIME_OUT,
};

/** Enumeration for TCP Connection States.  */
 enum enum_TCP_Connection_State
{
	TCP_CLOSED,
	TCP_LISTEN,
	TCP_SYN_SENT,
	TCP_SYN_RECEIVED,
	TCP_ESTABLISHED,
	TCP_CLOSE_WAIT,
	TCP_FIN_WAIT_1,
	TCP_CLOSING,
	TCP_LAST_ACK,
	TCP_FIN_WAIT_2,
	TCP_TIME_WAIT,
};
/** Enumeration to convey TCP Configuration.*/
enum enum_TCP_isConfigured
{
	DISABLE=0,
	ENABLE=1,
};

/************************TRANSPORT LAYER SEGMENT STRUCTURE START************************/
/**
		~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Options may occupy space at the end of the TCP header and are a
		multiple of 8 bits in length.  All options are included in the
		checksum.  An option may begin on any octet boundary.  There are two
		cases for the format of an option:

		  Case 1:  A single octet of option-kind.

		  Case 2:  An octet of option-kind, an octet of option-length, and
				   the actual option-data octets.

		The option-length counts the two octets of option-kind and
		option-length as well as the option-data octets.

		Note that the list of options may be shorter than the data offset
		field might imply.  The content of the header beyond the
		End-of-Option option must be header padding (i.e., zero).

		A TCP must implement all options.

		~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

struct stru_Options_TCP
{	
	int Kind:8; /**< 
				~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				Currently defined options include (kind indicated in octal):

				  Kind     Length    Meaning
				  ----     ------    -------
				   0         -       End of option list.
				   1         -       No-Operation.
				   2         4       Maximum Segment Size.
				~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				*/	
	int Length:8; /**<
					 The option-length counts the two octets of option-kind and
					option-length as well as the option-data octets.
				*/	
	unsigned int MSS;/**<
						 If this option is present, then it communicates the maximum
						 receive segment size at the TCP which sends this segment.
						 This field must only be sent in the initial connection request
						 (i.e., in segments with the SYN control bit set).
					*/
};

/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	TCP Segment header, Per RFC 793 September 1981, page number 15.

	TCP Header Format
                                   
		0                   1                   2                   3   
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |          Source Port          |       Destination Port        |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |                        Sequence Number                        |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |                    Acknowledgment Number                      |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |  Data |           |U|A|P|R|S|F|                               |
	   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
	   |       |           |G|K|H|T|N|N|                               |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |           Checksum            |         Urgent Pointer        |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |                    Options                    |    Padding    |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	   |                             data                              |
	   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
struct stru_Segment_Header_TCP
{
	
	unsigned short int usnSource_Port;/**<
										Port number of the source node.*/
	
	unsigned short int usnDestination_Port; /**< 
											Port number of the Destination node.*/
	
	unsigned long int ulnSequence_Number; /**<
											The sequence number of the first data octet in this segment (except when SYN is present). 
											If SYN is present the sequence number is the initial sequence number (ISN) and the first data octet is ISN+1.*/
	
	unsigned long int ulnAcknowledgment_Number; /**<
													If the ACK control bit is set this field contains the value of the next sequence number the sender of the segment is expecting to receive.  
													Once a connection is established this is always sent.*/
	
	int nData_Offset:4; /**< 
						The number of 32 bit words in the TCP Header.  
						This indicates where the data begins.  The TCP header (even one including options) is an integral number of 32 bits long.*/
	
	int nReserved:6; /**<
						Reserved for future use.  Must be zero.*/
	
	bool bURG; /**<
				Urgent Pointer field significant.*/
	
	bool bACK; /**<
				Acknowledgment field significant.*/
	
	bool bPSH; /**<
				Push Function.*/
	
	bool bRST; /**<
				Reset the connection.*/
	
	bool bSYN; /**<
				Synchronize sequence numbers.*/
	
	bool bFIN; /**<
				No more data from sender.*/
	
	unsigned short int nWindow; /**<	
									The number of data octets beginning with the one indicated in the
									acknowledgment field which the sender of this segment is willing to accept.
								*/
	
	unsigned short int usnChecksum; /**<	 
										The checksum field is the 16 bit one's complement of the one's
										complement sum of all 16 bit words in the header and text.  If a
										segment contains an odd number of header and text octets to be
										checksummed, the last octet is padded on the right with zeros to
										form a 16 bit word for checksum purposes.  The pad is not
										transmitted as part of the segment.  While computing the checksum,
										the checksum field itself is replaced with zeros.
									*/
	
	unsigned short int usnUrgent_Pointer; /**<	
												This field communicates the current value of the urgent pointer as a
												positive offset from the sequence number in this segment.  The
												urgent pointer points to the sequence number of the octet following
												the urgent data.  This field is only be interpreted in segments with
												the URG control bit set.
											*/	
	TCP_HEADER_OPTIONS *pstruTCP_Options; /**<	
												Options may occupy space at the end of the TCP header and are a
												multiple of 8 bits in length.  All options are included in the
												checksum.  An option may begin on any octet boundary.
											*/
	
	int nPadding:8; /**<
						The TCP header padding is used to ensure that the TCP header ends
						and data begins on a 32 bit boundary.  The padding is composed of
						zeros.
					*/
	
	bool bReTransmission_Flag; /**<	
								To set the retransmission of a segment (Specific to the NetSim packet trace).
								*/	
	unsigned long int ulnTmp_SEG_SEQ;/**<	
										Used to place the retransmitted segment in the receiver buffer(Specific to the NetSim packet trace).
									*/
	
	unsigned long int ulnSND_SEG_SEQ;/**<	
									Used to delete the segment from retransmission queue(Specific to the NetSim packet trace).
									*/
	
	bool bThreeDuplicateAckFlag; /**<	 
								Used for fast retransmission (Specific to the NetSim packet trace).
								*/	
	int nDupAckCount; /**<	
						To keep track of duplicate acknowledgments (Specific to the NetSim packet trace).
						*/
	
	unsigned int nPrevControlDatatype; /**<
										To get the previous packet's control data type*/
	NETSIM_ID nAppId;
};

int nTCPPacketTraceField[TCP_MAX_PACKET_TRACE_FIELD];

/************************TRANSPORT LAYER SEGMENT STRUCTURE END************************/

/**************************TRANSPORT LAYER STRUCTURE START ***************************/

/** 
	Variables required to maintain TCP connection, as per RFC 793 September 1981, page number 19 
	   The maintenance of a TCP connection requires the remembering of several variables.  We conceive
	   of these variables being stored in a connection record called a Transmission Control Block or TCB. 
	   Among the variables stored in the TCB are the local and remote socket numbers,  pointers to the user's
	   send and receive buffers, pointers to the retransmit queue and to the current segment. In addition several
	   variables relating to the send and receive sequence numbers are stored in the TCB.
	   
*/
struct stru_Transmission_Control_Block
{
	NETSIM_IPAddress szSourceIP; ///< Source Node IPAddress.
	NETSIM_IPAddress szDestinationIP; ///< Destination Node IPAddress.
	unsigned short int usnSource_Port; ///< Source Node Port Number.
	unsigned short int usnDestination_Port; ///< Destination Node Port number.
	NETSIM_ID nDevice_ID; ///< Id Number of the Device.
	NETSIM_ID nAppId; ///< Application Id.
	TCP_CONNECTION_STATE TCP_Present_Connection_State; ///< Present State of the TCP Connection.
	TCP_CONNECTION_STATE TCP_Previous_Connection_State; ///< Present State of the Connection.
	
	/* Send Sequence Variables as per RFC 793 page number 18 */	
	unsigned long int ulnSND_UNA; /**< Send unacknowledged */	
	unsigned long int ulnSND_NXT; /**< Send next */	
	unsigned long int ulnSND_WND; /**< Send window */	
	unsigned short int usnSND_UP; /**< Send urgent pointer */	
	unsigned long int ulnSND_WL1; /**< Segment sequence number used for last window update */	
	unsigned long int ulnSND_WL2; /**< Segment acknowledgment number used for last window update */	
	unsigned long int ulnISS; /**< Initial send sequence number */

	/* Receive Sequence Variables as per RFC 793 page number 18 */
	
	unsigned long int ulnRCV_NXT; /**< Receive next */	
	unsigned long int ulnRCV_WND; /**< Receive window */	 
	unsigned short int usnRCV_UP; /**< Receive urgent pointer */	
	unsigned long int ulnIRS; /**< Initial receive sequence number */

	/* Current Segment Variables */

	unsigned long int ulnSEG_SEQ; ///< Segment sequence number
	unsigned long int ulnSEG_ACK; ///< Segment acknowledgment number
	int nSEG_LEN; ///< Segment length 
	unsigned long int ulnSEG_WND; ///< Segment window 
	unsigned short int usnSEG_UP;///< Segment urgent pointer 
	int nSEG_PRC; ///< Segment precedence value
#pragma message(__LOC__"why this is here???")
	unsigned long int ulnFIN_SEG_SEQ; ///< Sequence Number of the FIN segment.




	///Unknown variable
#pragma message(__LOC__"why all this is here???")
	unsigned int unIW; ///< Size of the sender's congestion window after the three-way handshake is completed.
	unsigned int un_cwnd; ///< To store Congestion window size value.
	unsigned int un_rwnd; ///< To store the most recently advertised receiver window size value.
	unsigned int un_ssthresh; ///< To store slow start threshold value
	unsigned int unLW; ///< Size of the cwnd after a TCP sender detects loss using its retransmission timer.
	unsigned int unRW; ///< Size of the cwnd after a TCP restarts transmission after an idle period.
	int nSMSS; ///< To store the size of the largest segment that the sender can transmit.
	int nRMSS; ///< To store the size of the largest segment that the receiver is willing to accept.
	
	/* Variables to store TCP timers */
	double dRTO_TIME;
	double dRTT_TIME;
	double dSRTT_TIME;
	double dSERROR;
	double dSDEVIATION;

	unsigned int unAdvertised_Window; ///< To store Advertised window size value
	TCP_CONGESTION_CONTROL_ALGORITHM Congestion_Control_Algorithm; ///< To know about congestion control algorithm 
	int nWindowExpand_Flag; ///< This is mainly used to know about whether to increment the congestion window or not.
	bool bDuplicate_ACK; ///< To check duplicate ack.
	unsigned int unCongestionAvoidance_Count; ///< This is mainly useful to follow congestion avoidance phase.
	unsigned long int ulnPrevious_SEG_ACK; ///< This is to store the previous Segment's Ack.
	unsigned long int ulnExpected_SEG_SEQ; ///< Stores the expected segment sequence number.
	unsigned long int ulnRecover; ///< Stores the last sent sequence number after 3 duplicate Acks come.
	unsigned long int ulnNewData;
	int nDup_ACK_Count; ///< Stores the duplicate ack count.
	double dReceived_ACK_Count; ///< Stores received ack count.
	bool bFastRecovery; ///< Flag to check whether FastRecovery is enabled or not.
	bool bRENO; ///< Flag to check whether RENO is enabled or not.
	NetSim_PACKET* pstru_TCP_Retransmission_Queue;	///< As per RFC 793 page no 9. TCP, before send the data, puts a copy on retransmission queue.
	NetSim_PACKET* pstru_TCP_Receiver_Buffer; ///< Buffer at receiver end.
	struct stru_Transmission_Control_Block* pstru_Previous_Connection_Var; ///< Pointer to Store the previous TCB.
	struct stru_Transmission_Control_Block* pstru_Next_Connection_Var; ///< Pointer to Store the next TCB.
	TCP_METRICS* pstruMetrics; ///< TCP Metrics variable to store the Connection Metrics.

	bool isFinSend;
};

/* This is NetSim specific sructure to calculate metrics. */
struct stru_TCP_Metrics
{
	NETSIM_IPAddress szSourceIP; ///< Source Node IPAddress.
	NETSIM_IPAddress szDestinationIP; ///< Destination Node IPAddress.
	unsigned short int usnSource_Port; ///< Source Node Port number.
	unsigned short int usnDestination_Port; ///< Destination Port number.
	unsigned int unActiveOpens; ///< Number of Active Opens.
	unsigned int unPassiveOpens; ///< Number of Passive Opens.
	unsigned int unFailedConnectionAttempts; ///< Number of attempts failed to connect.
	unsigned int unResetConnections; ///< Number of Connection Resets.
	unsigned int unCurrentConnections; ///< Number of CurrentConnections.
	int nPureControlSegSent; ///< Number of Pure Control Segments sent.
	int nPureControlSegReceived; ///< Number of Pure Control Segments Received.
	int nAckSent; ///< Number of Ack Sent.
	int nTotal_ACK_Received; ///< Total number of ack received.
	int nTotal_SEG_Transmitted; ///< Total number of Segments Transmitted.
	int nTotal_SEG_Received; ///< Total number of Segments received.
	int nSegInRetransmissionQueue; ///< Number of segments in the retransmission queue.
	int nSegInReceiverBuffer; ///< Number of segments in the Receiver buffer.
	int nTotal_SEG_Retransmitted; ///< Total number of segments retransmitted.
	int nSegFastRetransmitted; ///< Number of fast retransmitted segments.
	int nTotal_SEG_Recovered; ///< Total number of segments recovered.
	int nTotal_Dup_SEG_Received; ///< Total number of duplicate segments received.
	int nTotal_Dup_ACK_Received;	///< Total number of Duplicate ack received.
	int nReliability;
	struct stru_TCP_Metrics* pstruNextMetrics;
};

/** This is NetSim specific structure to store the user inputs from Configuration.xml */
struct stru_TCP_Config_Variable
{
	int nWindowSize;
	TCP_CONGESTION_CONTROL_ALGORITHM CongestionControlAlgorithm;
	int nTCP_MSS;
};


TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables; /**< NetSim specific global variable for configuration */
TCP_METRICS* g_pstruTCPMetrics; /**< NetSim specific global variable for  metrics */


/*Graph*/
//Statistics
bool nCongestionWindowGraphFlag;
bool nCongestionWindowGraphRealTime;
typedef struct tcp_graph
{
	NETSIM_ID source;
	NETSIM_ID dest;
	NETSIM_ID app;
	int id;
	struct stru_GenericGraph* genericTCPGraph;
	struct tcp_graph* next;
}TCPGRAPH,*PTCPGRAPH;
PTCPGRAPH tcpgraph;
int fn_NetSim_TCP_Init_CongestionWindowGraph(NETSIM_ID nAppId,NETSIM_ID src,NETSIM_ID dest,NETSIM_IPAddress srcIP,NETSIM_IPAddress dstIP,NETSIM_ID srcPort,NETSIM_ID dstPort);
int fn_NetSim_TCP_Write_CongestionWindowData(NetSim_PACKET* packet);
/**************************TRANSPORT LAYER STRUCTURE END ****************************/


/** Function to retransmit segment when retransmission timeout for a segment */
int fn_NetSim_TCP_Retransmission_Timeout();
/** Function to check the connection */
//int fn_NetSim_TCP_Check_Connection(NETSIM_ID,NETSIM_IPAddress,NETSIM_IPAddress,unsigned short int,unsigned short int,TCB **);
/** Function to create TCB(connection) */
int fn_NetSim_TCP_Create_TCB(TCB **,NetSim_PACKET *);
/** Function to delete TCB(connection) */
//int fn_NetSim_TCP_Delete_TCB(TCB *,NETSIM_ID);
/** Function to form TCP Header information */
int fn_NetSim_TCP_Header(TCB *,NetSim_PACKET *);
/** Function to add the segment to retransmission queue befor transmitting the segment */
int fn_NetSim_TCP_AddSegmentTo_RetransmissionQueue(TCB *,NetSim_PACKET *);
/** Function to add the segment to receiver buffer */
int fn_NetSim_TCP_AddSegmentTo_Receiver_Buffer(TCB *,NetSim_PACKET *);
/** Function to check the particular segment in retransmission queue for retransmitting 
that segment, if the segment is available then it will return that segment */
bool fn_NetSim_TCP_Check_Segment_Availability_In_Buffer(NetSim_PACKET * ,NetSim_PACKET ** ,unsigned long int,int*);
bool fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(NetSim_PACKET * ,NetSim_PACKET ** ,unsigned long int);
int	fn_NetSim_TCP_Update_DupAckCount_In_Buffer(TCB*,unsigned long int);
/** Function to delete the segment from retransmission queue when the ack is received */
int fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(TCB *,NetSim_PACKET *,int *);
/** Function to add the Application event when it reaches the advertised window */
int fn_NetSim_TCP_Delete_Segments_From_Receiver_Buffer(TCB *,unsigned int *);
/** Function to generate the ack segment */
int fn_NetSim_TCP_Generate_Ack(TCB *,NetSim_PACKET *);
/** Function to form the retransmission segment */
int fn_NetSim_TCP_Form_Retransmission_Segment(TCB *,NetSim_PACKET **,NetSim_PACKET *,double);
/* Function to form the retransmission segment when three duplicate ack is received,
this function is used in TAHOE congestion control algorithm */
int fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(TCB*,NetSim_PACKET**,NetSim_PACKET*);
/** Function to check number of segments available in sender/receiver buffer */
unsigned int fn_NetSim_TCP_Check_No_Of_Segment_Available_In_Buffer(TCB *,NetSim_PACKET *);
/** Function to clear the time for timeout segment */
int fn_NetSim_TCP_Clear_Timers(TCB *);

//Function to change the connection state
int fn_NetSim_TCP_Change_Connection_State(TCB *, TCP_CONNECTION_STATE);
///Function to update the state for packet trace
int fn_NetSim_TCP_State_PacketTrace(TCP_CONNECTION_STATE, char*);

int fn_NetSim_TCP_Check_Connection(NETSIM_ID,NETSIM_IPAddress,NETSIM_IPAddress,NETSIM_ID,NETSIM_ID,TCB **); 

int fn_NetSim_TCP_Delete_TCB(TCB *,NETSIM_ID);

int fn_NetSim_TCP_SEND_SEG();
int fn_NetSim_TCP_SEND_ACK_FOR_SYN(TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables);
int fn_NetSim_TCP_SEND_FIN_SEG();
int fn_NetSim_TCP_SEND_FIN_ACK();
int fn_NetSim_TCP_ACTIVE_OPEN(unsigned short int usnSourcePort, unsigned short int usnDest_Port, NETSIM_IPAddress szSourceIP, NETSIM_IPAddress szDestIP,NETSIM_ID nDeviceID,TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables,TCP_METRICS ** g_pstruTCPMetrics);
int fn_NetSim_TCP_PASSIVE_OPEN(unsigned short int usnSourcePort, unsigned short int usnDest_Port, NETSIM_IPAddress szSourceIP, NETSIM_IPAddress szDestIP,TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables,TCP_METRICS** g_pstruTCPMetrics);
int fn_NetSim_TCP_RECEIVE_SEG();
int fn_NetSim_TCP_SEND_ACK();
int fn_NetSim_TCP_RECEIVE_ACK();
int fn_NetSim_TCP_RECEIVE_ACK_FOR_SYN(TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables);
int fn_NetSim_TCP_RECEIVE_FIN_SEG();
int fn_NetSim_TCP_RECEIVE_FIN_ACK();
_declspec (dllexport) int fn_NetSim_TCP_RTT(TCB *pstruTCP_Connection_Var);

/****************** NetWorkStack DLL functions declarations *****************************************/
/** Function for configuring TCP parameters*/
_declspec(dllexport) int fn_NetSim_TCP_Configure(void** var);
int fn_NetSim_TCP_Configure_F(void** var);
/** Function for Intializing TCP protocol */
_declspec (dllexport) int fn_NetSim_TCP_Init(struct stru_NetSim_Network *NETWORK_Formal,NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,char *pszWritePath_Formal,int nVersion_Type,void **fnPointer);
int fn_NetSim_TCP_Init_F(struct stru_NetSim_Network *,NetSim_EVENTDETAILS *,char *,char *,int ,void **fnPointer);
/** Function to run TCP protocol */
_declspec (dllexport) int fn_NetSim_TCP_Run();
///Function to free the TCP protocol variable and Unload the primitives
_declspec(dllexport) int fn_NetSim_TCP_Finish();
int fn_NetSim_TCP_Finish_F();
///Return the subevent name with respect to the subevent number for writting event trace
_declspec (dllexport) char *fn_NetSim_TCP_Trace(int nSubEvent);
char *fn_NetSim_TCP_Trace_F(int nSubEvent);
/// Function to free the allocated memory for the TCP packet
_declspec(dllexport) int fn_NetSim_TCP_FreePacket(NetSim_PACKET* );
int fn_NetSim_TCP_FreePacket_F(NetSim_PACKET* );
/// Function to copy the TCP packet from source to destination
_declspec(dllexport) int fn_NetSim_TCP_CopyPacket(NetSim_PACKET* ,NetSim_PACKET* );
int fn_NetSim_TCP_CopyPacket_F(NetSim_PACKET* ,NetSim_PACKET* );
/// Function to write TCP Metrics into Metrics.txt
_declspec(dllexport) int fn_NetSim_TCP_Metrics(char *);
int fn_NetSim_TCP_Metrics_F(char *);
_declspec (dllexport) int fn_NetSim_TCP_Window_Shrinkage(TCB *pstruTCP_Connection_Var);
_declspec (dllexport) int fn_NetSim_TCP_Window_Expansion(TCB *pstruTCP_Connection_Var);
_declspec (dllexport) int fn_NetSim_TCP_RTO(TCB *pstruTCP_Connection_Var);
_declspec (dllexport) int fn_NetSim_TCP_Ack_Checking(TCB *pstruTCP_Connection_Var);
#ifdef  __cplusplus
}
#endif
#endif


