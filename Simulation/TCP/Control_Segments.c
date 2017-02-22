#include "main.h"
#include "TCP.h"
#include "../Application/Application.h"
/*---------------------------------------------------------------------------------
* Copyright (C) 2013															  *
*																				  *
* TETCOS, Bangalore. India                                                        *
*
* Tetcos owns the intellectual property rights in the Product and its content.    *
* The copying, redistribution, reselling or publication of any or all of the      *
* Product or its content without express prior written consent of Tetcos is       *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.     *

* Author:   P.Sathishkumar 
* Date  :    21-May-2013
* --------------------------------------------------------------------------------*/
/**
This function is called by the Sender of the data. TCP_SYN packet is initiated and transmitted to
the receiver of the data.
*/
int fn_NetSim_TCP_ACTIVE_OPEN(unsigned short int usnSourcePort, unsigned short int usnDest_Port, NETSIM_IPAddress szSourceIP, NETSIM_IPAddress szDestIP,NETSIM_ID nDeviceID,TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables,TCP_METRICS ** g_pstruTCPMetrics)
{
	NETSIM_ID nSourceID;
	NETSIM_ID nDestID;
	TCB *pstru_Connection_Var;
	TCB *pstruTmp_Connection_Var;
	NetSim_PACKET *pstru_SYN_SEG;
	double dEventtime;
	/* Variable to store TCP Segment header */
	SEGMENT_HEADER_TCP *pstruTCP_Segment_Header;	

	dEventtime = pstruEventDetails->dEventTime;
	nSourceID = pstruEventDetails->pPacket->nSourceId;
	nDestID = pstruEventDetails->pPacket->nDestinationId;

	pstru_Connection_Var = (TCB*)calloc(1,sizeof(TCB));

	pstru_Connection_Var->szSourceIP = IP_COPY(szSourceIP);
	pstru_Connection_Var->szDestinationIP = IP_COPY(szDestIP);
	pstru_Connection_Var->usnSource_Port = usnSourcePort;
	pstru_Connection_Var->usnDestination_Port = usnDest_Port;

	pstru_Connection_Var->nDevice_ID = nDeviceID;
	pstru_Connection_Var->nAppId = pstruEventDetails->pPacket->pstruAppData->nApplicationId;

	//Get the value from config variable and assign to TCP connection variable
	pstru_Connection_Var->nSMSS = g_ppstru_TCP_Config_Variables[nSourceID-1]->nTCP_MSS;
	pstru_Connection_Var->Congestion_Control_Algorithm = g_ppstru_TCP_Config_Variables[nSourceID-1]->CongestionControlAlgorithm;

	pstru_Connection_Var->bFastRecovery=false;
	pstru_Connection_Var->ulnNewData= 0;

	/* Specific to NetSim Metrics */
	pstru_Connection_Var->pstruMetrics = (TCP_METRICS*)calloc(1,sizeof* pstru_Connection_Var->pstruMetrics);
	pstru_Connection_Var->pstruMetrics->pstruNextMetrics = (*g_pstruTCPMetrics);
	(*g_pstruTCPMetrics) = pstru_Connection_Var->pstruMetrics;
	pstru_Connection_Var->pstruMetrics->szSourceIP = IP_COPY(szSourceIP);
	pstru_Connection_Var->pstruMetrics->szDestinationIP = IP_COPY(szDestIP);
	pstru_Connection_Var->pstruMetrics->usnSource_Port = usnSourcePort;
	pstru_Connection_Var->pstruMetrics->usnDestination_Port = usnDest_Port;

	//change the connection state
	pstru_Connection_Var->TCP_Previous_Connection_State = TCP_CLOSED;
	fn_NetSim_TCP_Change_Connection_State(pstru_Connection_Var,TCP_SYN_SENT);

	//Assign the connection to the device variable 
	if(NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar)
	{
		pstruTmp_Connection_Var = (TCB*)NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar;
		while(pstruTmp_Connection_Var->pstru_Next_Connection_Var != NULL)
		{
			pstruTmp_Connection_Var = pstruTmp_Connection_Var->pstru_Next_Connection_Var;
		}
		pstruTmp_Connection_Var->pstru_Next_Connection_Var = pstru_Connection_Var;
		pstru_Connection_Var->pstru_Previous_Connection_Var = pstruTmp_Connection_Var;
	}
	else
		NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar = pstru_Connection_Var;

	//Create the Syn Segment
	pstru_SYN_SEG = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);

	/* Add payload and overheads */
	pstru_SYN_SEG->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;

	/* Assign the packet size */
	pstru_SYN_SEG->pstruTransportData->dPacketSize = pstru_SYN_SEG->pstruTransportData->dPayload + pstru_SYN_SEG->pstruTransportData->dOverhead;

	pstru_SYN_SEG->nSourceId = nSourceID;
	pstru_SYN_SEG->nDestinationId = nDestID;

	/* Update TransportLayer segment time */
	pstru_SYN_SEG->pstruTransportData->dArrivalTime = dEventtime;
	pstru_SYN_SEG->pstruTransportData->dStartTime = dEventtime;
	pstru_SYN_SEG->pstruTransportData->dEndTime = dEventtime;
	pstru_SYN_SEG->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;	

	pstru_SYN_SEG->pstruTransportData->nSourcePort = usnSourcePort;
	pstru_SYN_SEG->pstruTransportData->nDestinationPort = usnDest_Port;

	pstru_SYN_SEG->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstru_SYN_SEG->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstru_SYN_SEG->pstruNetworkData->nTTL = MAX_TTL; // Set TTL value

	pstru_SYN_SEG->dEventTime = dEventtime;
	pstru_SYN_SEG->nPacketType = PacketType_Control;
	pstru_SYN_SEG->nControlDataType = TCP_SYN;
	pstru_SYN_SEG->nTransmitterId = nDeviceID;

	/* For first segment Transmission calculate the RTO time */
	/* This function is used to calculate the RTO time */
	fn_NetSim_TCP_RTT(pstru_Connection_Var);

	pstru_Connection_Var->nSEG_LEN = 0;//(int) pstru_SYN_SEG->pstruTransportData->dPacketSize;
	/* Assign the sequence number to TCB send sequence variables */
	pstru_Connection_Var->ulnISS = ISN;
	pstru_Connection_Var->ulnSEG_SEQ = pstru_Connection_Var->ulnISS;
	pstru_Connection_Var->ulnSND_UNA = pstru_Connection_Var->ulnSEG_SEQ;
	pstru_Connection_Var->ulnSND_NXT = pstru_Connection_Var->ulnSEG_SEQ + pstru_Connection_Var->nSEG_LEN;

	/* Add TCP header information */
	/* Allocate memory for TCP Segment header */
	pstruTCP_Segment_Header = (SEGMENT_HEADER_TCP*)calloc(1,sizeof(SEGMENT_HEADER_TCP));
	// Allocate memory for header options
	pstruTCP_Segment_Header->pstruTCP_Options = (TCP_HEADER_OPTIONS*)calloc(1,sizeof(TCP_HEADER_OPTIONS));

	/* Fill the TCP Header information */
	pstruTCP_Segment_Header->usnSource_Port = usnSourcePort;
	pstruTCP_Segment_Header->usnDestination_Port = usnDest_Port;
	pstruTCP_Segment_Header->ulnSequence_Number = pstru_Connection_Var->ulnISS;
	pstruTCP_Segment_Header->ulnAcknowledgment_Number = pstru_Connection_Var->ulnSEG_ACK;
	pstruTCP_Segment_Header->nData_Offset = 0;
	pstruTCP_Segment_Header->nReserved = 0;
	pstruTCP_Segment_Header->bURG = false;
	pstruTCP_Segment_Header->bACK = false;
	pstruTCP_Segment_Header->bPSH = false;
	pstruTCP_Segment_Header->bRST = false;
	pstruTCP_Segment_Header->bSYN = true;
	pstruTCP_Segment_Header->bFIN = false;
	pstruTCP_Segment_Header->usnChecksum = 0;
	pstruTCP_Segment_Header->usnUrgent_Pointer = 0;
	pstruTCP_Segment_Header->nWindow = (unsigned short)pstru_Connection_Var->un_cwnd;

	// TCP Options
	pstruTCP_Segment_Header->pstruTCP_Options->Kind = 2;
	pstruTCP_Segment_Header->pstruTCP_Options->Length = 4;
	pstruTCP_Segment_Header->pstruTCP_Options->MSS = pstru_Connection_Var->nSMSS;
	pstruTCP_Segment_Header->nPadding = 0;

	/* Specific to packet trace */
	pstruTCP_Segment_Header->nAppId = pstru_Connection_Var->nAppId;

	pstru_SYN_SEG->pstruTransportData->Packet_TransportProtocol = pstruTCP_Segment_Header;

	/* Add received segment to Retransmission Queue */
	fn_NetSim_TCP_AddSegmentTo_RetransmissionQueue(pstru_Connection_Var,pstru_SYN_SEG);

	//Free the segment which is coppied for active open connection
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	//Specific to NetSim TCP Metrics
	pstru_Connection_Var->pstruMetrics->nPureControlSegSent++;
	pstru_Connection_Var->pstruMetrics->unActiveOpens++;

	/* Add the event details for NETWORK_OUT_EVENT*/
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nPacketId = pstru_SYN_SEG->nPacketId;
	pstruEventDetails->dPacketSize=pstru_SYN_SEG->pstruTransportData->dPacketSize;
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType=0;
	pstruEventDetails->dEventTime = dEventtime;
	pstruEventDetails->pPacket=pstru_SYN_SEG;
	fnpAddEvent(pstruEventDetails);	

	fn_NetSim_TCP_Init_CongestionWindowGraph(pstruEventDetails->nApplicationId,pstru_SYN_SEG->nSourceId,pstru_SYN_SEG->nDestinationId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port);
	
	/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
	and sub event as TCP_RETRANSMISSION_TIMEOUT */
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstru_SYN_SEG);
	pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstru_Connection_Var->dRTO_TIME + 1;
	pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstru_SYN_SEG);
	fnpAddEvent(pstruEventDetails);	 

	return 0;
}
/**
This function is called by the receiver of the data to send the ack for syn packet of the sender.
Initiate one syn packet and send to the sender.
*/
int fn_NetSim_TCP_PASSIVE_OPEN(unsigned short int usnSourcePort, unsigned short int usnDest_Port, NETSIM_IPAddress szSourceIP, NETSIM_IPAddress szDestIP,TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables,TCP_METRICS** g_pstruTCPMetrics)
{
	NETSIM_ID nSourceID, nDestID;
	TCB *pstru_Connection_Var, *pstruTmp_Connection_Var;
	NetSim_PACKET *pstru_SYN_ACK_SEG;
	double dEventtime;
	/* Variable to store TCP Segment header */
	SEGMENT_HEADER_TCP *pstruTCP_Segment_Header;
	SEGMENT_HEADER_TCP *pstruTCP_Received_Syn_Header;


	dEventtime = pstruEventDetails->dEventTime;
	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	pstruTCP_Received_Syn_Header = (SEGMENT_HEADER_TCP*)pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

	pstru_Connection_Var = (TCB*)calloc(1,sizeof(TCB));

	//Assign the IP address and Port number to the TCP connection variable
	pstru_Connection_Var->szSourceIP = IP_COPY(szSourceIP);
	pstru_Connection_Var->szDestinationIP = IP_COPY(szDestIP);
	pstru_Connection_Var->usnSource_Port = usnSourcePort;
	pstru_Connection_Var->usnDestination_Port = usnDest_Port;
	pstru_Connection_Var->nDevice_ID = nSourceID;
	pstru_Connection_Var->nAppId = pstruTCP_Received_Syn_Header->nAppId;

	//Get the value from config variable and assign to the TCP connection variable
	pstru_Connection_Var->nSMSS = g_ppstru_TCP_Config_Variables[nSourceID-1]->nTCP_MSS;
	pstru_Connection_Var->Congestion_Control_Algorithm = g_ppstru_TCP_Config_Variables[nDestID-1]->CongestionControlAlgorithm;

	//Get the sender side TCP MSS and assign to the TCP connection variable
	pstru_Connection_Var->nRMSS = pstruTCP_Received_Syn_Header->pstruTCP_Options->MSS;

	pstru_Connection_Var->bFastRecovery=false;
	pstru_Connection_Var->ulnNewData= 0;
	/* Specific to NetSim Metrics */
	pstru_Connection_Var->pstruMetrics = (TCP_METRICS*)calloc(1,sizeof* pstru_Connection_Var->pstruMetrics);
	pstru_Connection_Var->pstruMetrics->pstruNextMetrics = (*g_pstruTCPMetrics);
	(*g_pstruTCPMetrics) = pstru_Connection_Var->pstruMetrics;
	pstru_Connection_Var->pstruMetrics->szSourceIP = IP_COPY(szSourceIP);
	pstru_Connection_Var->pstruMetrics->szDestinationIP = IP_COPY(szDestIP);
	pstru_Connection_Var->pstruMetrics->usnSource_Port = usnSourcePort;
	pstru_Connection_Var->pstruMetrics->usnDestination_Port = usnDest_Port;

	//Change the connection state
	pstru_Connection_Var->TCP_Previous_Connection_State = TCP_LISTEN;
	fn_NetSim_TCP_Change_Connection_State(pstru_Connection_Var,TCP_SYN_RECEIVED);

	//Assign the connection to the device variable 
	if(NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar)
	{
		pstruTmp_Connection_Var = (TCB*)NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar;
		while(pstruTmp_Connection_Var->pstru_Next_Connection_Var != NULL)
		{
			pstruTmp_Connection_Var = pstruTmp_Connection_Var->pstru_Next_Connection_Var;
		}
		pstruTmp_Connection_Var->pstru_Next_Connection_Var = pstru_Connection_Var;
		pstru_Connection_Var->pstru_Previous_Connection_Var = pstruTmp_Connection_Var;
	}
	else
		NETWORK->ppstruDeviceList[nSourceID-1]->pstruTransportLayer->TCPVar = pstru_Connection_Var;

	//Create the segment Syn and Ack for received Syn segment
	pstru_SYN_ACK_SEG = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);

	/* Add payload and overheads */
	pstru_SYN_ACK_SEG->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;

	/* Assign the packet size */
	pstru_SYN_ACK_SEG->pstruTransportData->dPacketSize = pstru_SYN_ACK_SEG->pstruTransportData->dPayload + pstru_SYN_ACK_SEG->pstruTransportData->dOverhead;

	/* Update TransportLayer segment time */
	pstru_SYN_ACK_SEG->pstruTransportData->dArrivalTime = dEventtime;
	//To calculate the RTT time
	pstru_SYN_ACK_SEG->pstruTransportData->dStartTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime;
	pstru_SYN_ACK_SEG->pstruTransportData->dEndTime = dEventtime;

	pstru_SYN_ACK_SEG->nSourceId = nSourceID;
	pstru_SYN_ACK_SEG->nDestinationId = nDestID;
	pstru_SYN_ACK_SEG->pstruTransportData->nSourcePort = usnSourcePort;
	pstru_SYN_ACK_SEG->pstruTransportData->nDestinationPort = usnDest_Port;
	pstru_SYN_ACK_SEG->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;	

	pstru_SYN_ACK_SEG->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstru_SYN_ACK_SEG->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstru_SYN_ACK_SEG->pstruNetworkData->nTTL = MAX_TTL;

	pstru_SYN_ACK_SEG->dEventTime = dEventtime;
	pstru_SYN_ACK_SEG->nPacketType = PacketType_Control;
	pstru_SYN_ACK_SEG->nControlDataType = TCP_SYN_ACK;
	pstru_SYN_ACK_SEG->nTransmitterId = nSourceID;

	/* For first Segment Transmission calculate the RTO time */
	/* This function is used to calculate the RTO time */
	fn_NetSim_TCP_RTT(pstru_Connection_Var);

	pstru_Connection_Var->nSEG_LEN = (int) (pstru_SYN_ACK_SEG->pstruTransportData->dPacketSize-pstru_SYN_ACK_SEG->pstruTransportData->dOverhead);
	/* Assign the sequence number to TCB segment sequence variables */
	pstru_Connection_Var->ulnIRS = IRS;
	pstru_Connection_Var->ulnSEG_SEQ = pstru_Connection_Var->ulnIRS;
	pstru_Connection_Var->ulnSEG_ACK = pstruTCP_Received_Syn_Header->ulnSequence_Number + pstru_Connection_Var->nSEG_LEN+1;
	pstru_Connection_Var->ulnSND_NXT = pstru_Connection_Var->ulnSEG_SEQ + pstru_Connection_Var->nSEG_LEN;
	pstru_Connection_Var->ulnSND_UNA = pstru_Connection_Var->ulnSEG_SEQ;
	pstru_Connection_Var->ulnRCV_NXT = pstru_Connection_Var->ulnSEG_ACK;
	//To place the segment in receiver buffer
	pstru_Connection_Var->ulnExpected_SEG_SEQ = pstru_Connection_Var->ulnRCV_NXT;

	/* Add TCP header information */
	/* Allocate memory for TCP Segment header */
	pstruTCP_Segment_Header = (SEGMENT_HEADER_TCP*)calloc(1,sizeof(SEGMENT_HEADER_TCP));
	pstruTCP_Segment_Header->pstruTCP_Options = (TCP_HEADER_OPTIONS*)calloc(1,sizeof(TCP_HEADER_OPTIONS));

	/* Fill the TCP Header information */
	pstruTCP_Segment_Header->usnSource_Port = pstru_Connection_Var->usnSource_Port;
	pstruTCP_Segment_Header->usnDestination_Port = pstru_Connection_Var->usnDestination_Port;
	pstruTCP_Segment_Header->ulnSequence_Number = pstru_Connection_Var->ulnIRS;
	pstruTCP_Segment_Header->ulnAcknowledgment_Number = pstru_Connection_Var->ulnSEG_ACK;
	pstruTCP_Segment_Header->nData_Offset = 0;
	pstruTCP_Segment_Header->nReserved = 0;
	pstruTCP_Segment_Header->bURG = false;
	pstruTCP_Segment_Header->bACK = true;
	pstruTCP_Segment_Header->bPSH = false;
	pstruTCP_Segment_Header->bRST = false;
	pstruTCP_Segment_Header->bSYN = true;
	pstruTCP_Segment_Header->bFIN = false;
	pstruTCP_Segment_Header->nWindow = 0;
	pstruTCP_Segment_Header->usnChecksum = 0;
	pstruTCP_Segment_Header->usnUrgent_Pointer = 0;
	// TCP options
	pstruTCP_Segment_Header->pstruTCP_Options->Kind = 2;
	pstruTCP_Segment_Header->pstruTCP_Options->Length = 4;
	pstruTCP_Segment_Header->pstruTCP_Options->MSS = pstru_Connection_Var->nSMSS;
	pstruTCP_Segment_Header->nPadding = 0;

	//To delete the segment from retransmission queue
	pstruTCP_Segment_Header->ulnSND_SEG_SEQ = pstruTCP_Received_Syn_Header->ulnSequence_Number;

	pstru_SYN_ACK_SEG->pstruTransportData->Packet_TransportProtocol = pstruTCP_Segment_Header;

	/* Add received segment to Retransmission Queue */
	fn_NetSim_TCP_AddSegmentTo_RetransmissionQueue(pstru_Connection_Var,pstru_SYN_ACK_SEG);

	//Free the received syn segment
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	// Specific to NetSim TCP Metrics
	pstru_Connection_Var->pstruMetrics->nPureControlSegReceived++;
	pstru_Connection_Var->pstruMetrics->nPureControlSegSent++;
	pstru_Connection_Var->pstruMetrics->unPassiveOpens++;

	/* Add the event details for NETWORK_OUT_EVENT*/
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nPacketId = pstru_SYN_ACK_SEG->nPacketId;
	pstruEventDetails->dPacketSize=pstru_SYN_ACK_SEG->pstruTransportData->dPacketSize;
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType=0;
	pstruEventDetails->dEventTime = dEventtime;
	pstruEventDetails->pPacket=pstru_SYN_ACK_SEG;
	fnpAddEvent(pstruEventDetails);	

	/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
	and sub event as TCP_RETRANSMISSION_TIMEOUT */
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstru_SYN_ACK_SEG);
	pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstru_Connection_Var->dRTO_TIME + 1;
	pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstru_SYN_ACK_SEG);
	fnpAddEvent(pstruEventDetails);	

	return 0;
}
/**
This function is called by the sender to send the ack for syn packet of the receiver. Also sends data segment after 1 second.
*/
int fn_NetSim_TCP_SEND_ACK_FOR_SYN(TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables)
{
	TCB* pstru_Connection_Var;
	NETSIM_ID nSourceID, nDestID, nDeviceId;
	unsigned int nSocketId;
	NetSim_PACKET *pstru_ACK_SEG;
	double dEventtime;
	/* Variable to store TCP Segment header */
	SEGMENT_HEADER_TCP *pstruTCP_Segment_Header, *pstruTCP_Received_Syn_Header;
	int nSucessDeletion, nRetransmission_Flag=0;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;

	nDeviceId = pstruEventDetails->pPacket->nDestinationId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort; 

	fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstru_Connection_Var);

	if(!pstru_Connection_Var)
	{
		printf("TCP-- Connection not available for the device %d\n",nDeviceId);
		return 1;
	}

	dEventtime = pstruEventDetails->dEventTime;
	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;	
	pstruTCP_Received_Syn_Header = pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

	nSucessDeletion = fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(pstru_Connection_Var,pstruEventDetails->pPacket,&nRetransmission_Flag);

	// Get the mss of remote tcp
	pstru_Connection_Var->nRMSS = pstruTCP_Received_Syn_Header->pstruTCP_Options->MSS;

	//Create the ack segment for received syn segment
	pstru_ACK_SEG = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);

	/* Add payload and overheads */
	pstru_ACK_SEG->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;

	/* Assign the packet size */
	pstru_ACK_SEG->pstruTransportData->dPacketSize = pstru_ACK_SEG->pstruTransportData->dPayload + pstru_ACK_SEG->pstruTransportData->dOverhead;
	pstru_ACK_SEG->nSourceId = nSourceID;
	pstru_ACK_SEG->nDestinationId = nDestID;
	/* Update TransportLayer segment time */
	pstru_ACK_SEG->pstruTransportData->dArrivalTime = dEventtime;
	//To calculate the RTT time
	pstru_ACK_SEG->pstruTransportData->dStartTime = pstruEventDetails->pPacket->pstruTransportData->dArrivalTime;
	pstru_ACK_SEG->pstruTransportData->dEndTime = dEventtime;

	pstru_ACK_SEG->pstruTransportData->nSourcePort = usnSourcePort;
	pstru_ACK_SEG->pstruTransportData->nDestinationPort = usnDest_Port;
	pstru_ACK_SEG->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;	

	pstru_ACK_SEG->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstru_ACK_SEG->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstru_ACK_SEG->pstruNetworkData->nTTL = MAX_TTL;

	pstru_ACK_SEG->dEventTime = dEventtime;
	pstru_ACK_SEG->nPacketType = PacketType_Control;
	pstru_ACK_SEG->nControlDataType = TCP_ACK;
	pstru_ACK_SEG->nTransmitterId = nDeviceId;

	pstru_Connection_Var->nSEG_LEN = (int) (pstru_ACK_SEG->pstruTransportData->dPacketSize-pstru_ACK_SEG->pstruTransportData->dOverhead);
	/* Assign the sequence number to TCB sequence variables */
	pstru_Connection_Var->ulnSEG_SEQ = pstruTCP_Received_Syn_Header->ulnAcknowledgment_Number;
	pstru_Connection_Var->ulnSEG_ACK = pstruTCP_Received_Syn_Header->ulnAcknowledgment_Number;
	// To calculate seq no for data packet
	pstru_Connection_Var->ulnSND_NXT = pstru_Connection_Var->ulnSEG_SEQ;
	pstru_Connection_Var->ulnRCV_NXT = pstru_Connection_Var->ulnSEG_ACK;
	pstru_Connection_Var->ulnExpected_SEG_SEQ = pstru_Connection_Var->ulnRCV_NXT;

	//change the connection state
	fn_NetSim_TCP_Change_Connection_State(pstru_Connection_Var,TCP_ESTABLISHED);

	/* Add TCP header information */
	/* Allocate memory for TCP Segment header */
	pstruTCP_Segment_Header = fnpAllocateMemory(1,sizeof(SEGMENT_HEADER_TCP));

	// To delete the segment from retransmission queue
	pstruTCP_Segment_Header->ulnSND_SEG_SEQ = pstruTCP_Received_Syn_Header->ulnSequence_Number;

	/* Fill TCP Header information */
	pstruTCP_Segment_Header->usnSource_Port = usnSourcePort;
	pstruTCP_Segment_Header->usnDestination_Port = usnDest_Port;
	pstruTCP_Segment_Header->ulnSequence_Number = pstru_Connection_Var->ulnSEG_SEQ;
	pstruTCP_Segment_Header->ulnAcknowledgment_Number = pstru_Connection_Var->ulnSEG_ACK;
	pstruTCP_Segment_Header->nData_Offset = 0;
	pstruTCP_Segment_Header->nReserved = 0;
	pstruTCP_Segment_Header->bURG = false;
	pstruTCP_Segment_Header->bACK = true;
	pstruTCP_Segment_Header->bPSH = false;
	pstruTCP_Segment_Header->bRST = false;
	pstruTCP_Segment_Header->bSYN = false;
	pstruTCP_Segment_Header->bFIN = false;
	pstruTCP_Segment_Header->nWindow = (unsigned short)pstru_Connection_Var->nSMSS;
	pstruTCP_Segment_Header->usnChecksum = 0;
	pstruTCP_Segment_Header->usnUrgent_Pointer = 0;
	//pstruTCP_Segment_Header->ulnOptions = 0;
	pstruTCP_Segment_Header->nPadding = 0;

	//If the receiver mss is less than the sender mss then choose the minimum mss value between them
	if(pstru_Connection_Var->nSMSS > pstru_Connection_Var->nRMSS)
		pstru_Connection_Var->nSMSS = pstru_Connection_Var->nRMSS;

	//changed for delete seg from Retransmission queue
	pstru_Connection_Var->nSEG_LEN = pstru_Connection_Var->nSMSS;

	pstru_Connection_Var->un_cwnd =pstru_Connection_Var->nSMSS; /* Bytes. Per RFC 2001 January 1997, page number 3 */
	pstru_Connection_Var->un_ssthresh = MAX_ssthresh; /* Bytes. Per RFC 2001 January 1997, page number 3 */
	pstru_Connection_Var->unAdvertised_Window = g_ppstru_TCP_Config_Variables[nSourceID-1]->nWindowSize * pstru_Connection_Var->nSMSS;

	pstru_ACK_SEG->pstruTransportData->Packet_TransportProtocol = pstruTCP_Segment_Header;

	/* Free memory for received ack segment */
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	//Specific to NetSim TCP Metrics
	pstru_Connection_Var->pstruMetrics->nPureControlSegReceived++;
	pstru_Connection_Var->pstruMetrics->nAckSent++;

	/* Add the event details for NETWORK_OUT_EVENT*/
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nPacketId = pstru_ACK_SEG->nPacketId;
	pstruEventDetails->dPacketSize=pstru_ACK_SEG->pstruTransportData->dPacketSize;
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType=0;
	pstruEventDetails->dEventTime = dEventtime;
	pstruEventDetails->pPacket=pstru_ACK_SEG;
	fnpAddEvent(pstruEventDetails);	

	// After send the ack for syn, delay 1 micro second to send the segment
	pstruEventDetails->dEventTime = dEventtime + 1;
	pstruEventDetails->nApplicationId = pstru_Connection_Var->nAppId;
	pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
	pstruEventDetails->nSubEventType = TCP_SEND_SEG;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket);
	nSocketId = fnGetSocketId(pstruEventDetails->nApplicationId,nSourceID,nDestID,usnSourcePort,usnDest_Port);
	pstruEventDetails->szOtherDetails = (void*)nSocketId;
	fnpAddEvent(pstruEventDetails);	 

	

	return 0;
}
/**
This function is called by the receiver to receive the ack for its syn. Receiver also updates its connection information.
*/
int fn_NetSim_TCP_RECEIVE_ACK_FOR_SYN(TCP_CONFIG_VARIABLE **g_ppstru_TCP_Config_Variables)
{
	TCB *pstruConnectionVar;
	NETSIM_ID nSrcID, nDestID, nDeviceId;
	int nRetransmission_Flag=0;
	int nSuccessDeletion;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;

	nSrcID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	nDeviceId = nSrcID;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

	fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruConnectionVar);

	if(!pstruConnectionVar)
	{
		printf("TCP-- Connection not available for the device %d\n",nDeviceId);
		return 1;
	}

	nSuccessDeletion = fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(pstruConnectionVar,pstruEventDetails->pPacket,&nRetransmission_Flag);

	//Choose the minimum mss value between sender and receiver
	if(pstruConnectionVar->nSMSS > pstruConnectionVar->nRMSS)
		pstruConnectionVar->nSMSS = pstruConnectionVar->nRMSS;

	//changed for delete the seg from retransmission queue
	pstruConnectionVar->nSEG_LEN = pstruConnectionVar->nSMSS;

	pstruConnectionVar->un_cwnd = pstruConnectionVar->nSMSS; /* Bytes. Per RFC 2001 January 1997, page number 3 */
	pstruConnectionVar->un_ssthresh = MAX_ssthresh; /* Bytes. Per RFC 2001 January 1997, page number 3 */
	pstruConnectionVar->unAdvertised_Window = g_ppstru_TCP_Config_Variables[nDestID-1]->nWindowSize * pstruConnectionVar->nSMSS;

	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	//Specific to NetSim TCP Metrics
	pstruConnectionVar->pstruMetrics->nTotal_ACK_Received++;

	//change the connection state
	fn_NetSim_TCP_Change_Connection_State(pstruConnectionVar,TCP_ESTABLISHED);

	// After send the ack for syn, send the segment
	pstruEventDetails->nApplicationId = pstruConnectionVar->nAppId;
	pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
	pstruEventDetails->nSubEventType = TCP_SEND_SEG;
	pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
	pstruEventDetails->szOtherDetails = (void*)fnGetSocketId(pstruConnectionVar->nAppId,pstruEventDetails->nDeviceId,nDestID,usnSourcePort,usnDest_Port);
	fnpAddEvent(pstruEventDetails);	 

	return 0;
}
/**
This function is called by the receiver in response to the last segment from the sender.
*/
int fn_NetSim_TCP_SEND_FIN_SEG()
{
	NETSIM_ID nSourceID, nDestID;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	NetSim_PACKET *pstruFIN_SEG;
	SEGMENT_HEADER_TCP *pstruFIN_Header;
	TCB *pstruTCP_Connection_Var;
	double dEventTime;

	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

	dEventTime = pstruEventDetails->dEventTime;

	fn_NetSim_TCP_Check_Connection(nSourceID, szSourceIP, szDestIP, usnSourcePort, usnDest_Port, &pstruTCP_Connection_Var);

	fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_LAST_ACK);

	if(!pstruTCP_Connection_Var)
	{
		printf("TCP-- Connection not available for the device %d\n",nSourceID);
		return 1;
	}

	pstruFIN_SEG = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);
	pstruFIN_Header = fnpAllocateMemory(1,sizeof(SEGMENT_HEADER_TCP));

	pstruFIN_SEG->dEventTime = dEventTime;
	pstruFIN_SEG->nControlDataType = TCP_FIN;

	pstruFIN_SEG->nSourceId = nSourceID;
	pstruFIN_SEG->nDestinationId = nDestID;
	pstruFIN_SEG->nTransmitterId = nSourceID;

	pstruFIN_SEG->nPacketType = PacketType_Control;
	pstruFIN_SEG->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstruFIN_SEG->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstruFIN_SEG->pstruTransportData->nSourcePort = usnSourcePort;
	pstruFIN_SEG->pstruTransportData->nDestinationPort = usnDest_Port;
	pstruFIN_SEG->pstruNetworkData->nTTL = MAX_TTL; // Set TTL.

	pstruFIN_SEG->pstruTransportData->dArrivalTime = dEventTime;
	pstruFIN_SEG->pstruTransportData->dEndTime = dEventTime;
	pstruFIN_SEG->pstruTransportData->dStartTime = dEventTime;
	pstruFIN_SEG->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;
	pstruFIN_SEG->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;
	pstruFIN_SEG->pstruTransportData->dPacketSize = pstruFIN_SEG->pstruTransportData->dPayload + pstruFIN_SEG->pstruTransportData->dOverhead;

	/* Fill TCP Header information */
	pstruFIN_Header->usnSource_Port = usnSourcePort;
	pstruFIN_Header->usnDestination_Port = usnDest_Port;
	pstruFIN_Header->ulnSequence_Number = IRS+1;//pstruTCP_Connection_Var->ulnSEG_SEQ;
	pstruFIN_Header->ulnAcknowledgment_Number = pstruTCP_Connection_Var->ulnSEG_ACK;
	pstruFIN_Header->nData_Offset = 0;
	pstruFIN_Header->nReserved = 0;
	pstruFIN_Header->bURG = false;
	pstruFIN_Header->bACK = false;
	pstruFIN_Header->bPSH = false;
	pstruFIN_Header->bRST = false;
	pstruFIN_Header->bSYN = false;
	pstruFIN_Header->bFIN = true;

	pstruFIN_SEG->pstruTransportData->Packet_TransportProtocol = pstruFIN_Header;

	pstruTCP_Connection_Var->ulnFIN_SEG_SEQ = pstruTCP_Connection_Var->ulnSEG_SEQ;

	/* Add received segment to Retransmission Queue */
	fn_NetSim_TCP_AddSegmentTo_RetransmissionQueue(pstruTCP_Connection_Var,pstruFIN_SEG);

	//Specific to NetSim TCP Metrics
	pstruTCP_Connection_Var->pstruMetrics->nPureControlSegSent++;

	//Free the received segment
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	/* form the event details to add Network out event */
	pstruEventDetails->nDeviceId = nSourceID;
	pstruEventDetails->dPacketSize = pstruFIN_SEG->pstruTransportData->dPacketSize;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType = 0;
	pstruEventDetails->pPacket =  pstruFIN_SEG;
	fnpAddEvent(pstruEventDetails);

	/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
	and sub event as TCP_RETRANSMISSION_TIMEOUT */
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
	pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
	pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
	pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruFIN_SEG);
	fnpAddEvent(pstruEventDetails);	 

	return 0;
}
/**
This function is called by the sender to receive the FIN segment from the receiver.
*/
int fn_NetSim_TCP_RECEIVE_FIN_SEG()
{
	NETSIM_ID nSourceID, nDestID;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	TCB *pstruTCP_ConnectionVar;

	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

	fn_NetSim_TCP_Check_Connection(nSourceID,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_ConnectionVar);

	if(!pstruTCP_ConnectionVar)
	{
		printf("TCP-- Connection not available for the device %d\n",nSourceID);
		return 1;
	}

	//Specific to NetSim TCP Metrics
	pstruTCP_ConnectionVar->pstruMetrics->nPureControlSegReceived++;

	/* form the event details to add Transport out event and subevent as TCP_SEND_FIN_ACK */
	pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
	pstruEventDetails->nSubEventType = TCP_SEND_FIN_ACK;
	pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
	fnpAddEvent(pstruEventDetails);	

	return 0;
}
/**
This function is called by the sender to send the ack for FIN segment of the receiver.
*/
int fn_NetSim_TCP_SEND_FIN_ACK()
{
	TCB *pstruTCP_Connection_Var;
	NETSIM_ID nSourceID, nDestID;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	NetSim_PACKET *pstruFIN_ACK;
	SEGMENT_HEADER_TCP *pstruFIN_Header,*fin;
	double dEventTime;

	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;
	fin = pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

	dEventTime = pstruEventDetails->dEventTime;

	fn_NetSim_TCP_Check_Connection(nSourceID, szSourceIP, szDestIP, usnSourcePort, usnDest_Port, &pstruTCP_Connection_Var);

	if(!pstruTCP_Connection_Var)
	{
		printf("Connection not available\n");
		return 1;
	}

	fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_TIME_WAIT);

	pstruFIN_ACK = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);
	pstruFIN_Header = fnpAllocateMemory(1,sizeof(SEGMENT_HEADER_TCP));

	pstruFIN_ACK->dEventTime = dEventTime;
	pstruFIN_ACK->nPacketType = PacketType_Control;
	pstruFIN_ACK->nControlDataType = TCP_ACK;

	pstruFIN_ACK->nSourceId = nSourceID;
	pstruFIN_ACK->nDestinationId = nDestID;
	pstruFIN_ACK->nTransmitterId = nSourceID;

	pstruFIN_ACK->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstruFIN_ACK->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstruFIN_ACK->pstruTransportData->nSourcePort = usnSourcePort;
	pstruFIN_ACK->pstruTransportData->nDestinationPort = usnDest_Port;
	pstruFIN_ACK->pstruNetworkData->nTTL = MAX_TTL;

	pstruFIN_ACK->pstruTransportData->dArrivalTime = dEventTime;
	pstruFIN_ACK->pstruTransportData->dEndTime = dEventTime;
	pstruFIN_ACK->pstruTransportData->dStartTime = dEventTime;
	pstruFIN_ACK->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;
	pstruFIN_ACK->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;
	pstruFIN_ACK->pstruTransportData->dPacketSize = pstruFIN_ACK->pstruTransportData->dPayload + pstruFIN_ACK->pstruTransportData->dOverhead;

	/* Set the source port number */
	pstruFIN_Header->usnSource_Port = usnSourcePort;
	/* Set the destination port number */
	pstruFIN_Header->usnDestination_Port = usnDest_Port;
	pstruFIN_Header->ulnSequence_Number = IRS+2;
	pstruFIN_Header->ulnAcknowledgment_Number = fin->ulnSequence_Number+1;
	pstruFIN_Header->nWindow = MAX_TCP_MSS;
	pstruFIN_Header->bACK = true;

	pstruFIN_ACK->pstruTransportData->Packet_TransportProtocol = pstruFIN_Header;

	//Free the received segment
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	//Specific to NetSim TCP Metrics
	pstruTCP_Connection_Var->pstruMetrics->nAckSent++;

	/* form the event details to add Network out event */
	pstruEventDetails->nDeviceId = nSourceID;
	pstruEventDetails->dPacketSize = pstruFIN_ACK->pstruTransportData->dPacketSize;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType = 0;
	pstruEventDetails->pPacket =  pstruFIN_ACK;
	fnpAddEvent(pstruEventDetails);

	pstruEventDetails->nDeviceId = nSourceID;
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nSubEventType = TCP_TIME_WAIT_TIMEOUT;
	pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruFIN_ACK);
	pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
	pstruEventDetails->dEventTime = dEventTime + (MSL * 2);// 4 minutes time wait time
	fnpAddEvent(pstruEventDetails);	 

	return 0;
}
/**
This function is called by the receiver to receive the FIN_ACK from the sender.
In this function receiver colses its connection.
*/
int fn_NetSim_TCP_RECEIVE_FIN_ACK()
{
	TCB *pstruTCP_ConnectionVar;
	NETSIM_ID nDeviceId;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	int nRetransmission_Flag;

	nDeviceId = pstruEventDetails->pPacket->nDestinationId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

	fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_ConnectionVar);

	if(!pstruTCP_ConnectionVar)
	{
		if(pstruEventDetails->pPacket)
			fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

		printf("TCP-- Connection not available\n");
		return 1;
	}

	//Specific to NetSim TCP Metrics
	pstruTCP_ConnectionVar->pstruMetrics->nTotal_ACK_Received++;

	fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(pstruTCP_ConnectionVar,pstruEventDetails->pPacket,&nRetransmission_Flag);
	fn_NetSim_TCP_Change_Connection_State(pstruTCP_ConnectionVar,TCP_CLOSED);
	fn_NetSim_TCP_Delete_TCB(pstruTCP_ConnectionVar,pstruTCP_ConnectionVar->nDevice_ID);

	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	return 0;
}
