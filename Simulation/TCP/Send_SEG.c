#include "main.h"
#include "TCP.h"
/*--------------------------------------------------------------------------------*
* Copyright (C) 2013															  *
*																				  *
* TETCOS, Bangalore. India                                                        *
																				  *
* Tetcos owns the intellectual property rights in the Product and its content.    *
* The copying, redistribution, reselling or publication of any or all of the      *
* Product or its content without express prior written consent of Tetcos is       *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.     *

* Author:   P.Sathishkumar														  *
* Date  :    29-Oct-2012														  *
* --------------------------------------------------------------------------------*/
/** 
When TRANSPORT_OUT Event is triggered, if subevent type is 0(zero) then by default this function is called.
Connection creation, forming retransmission queue and adding the Retransmission time out event and NETWORK_OUT Event will be performed from this function only. 
*/
int fn_NetSim_TCP_SEND_SEG()
{
	TCB *pstruTCP_Connection_Var;
	NetSim_PACKET *pstru_Received_Segment, *pstru_Tmp_Segment;
	NetSim_SOCKETINTERFACE *pstru_Socket_Interface;
	NETSIM_ID nDeviceId, nAppId;
	unsigned int nSocketId;
	NETSIM_IPAddress szSourceIP, szDestIP;
	NETSIM_ID usnSourcePort, usnDest_Port;
	unsigned int unNo_of_Seg_in_Queue=0, unSEG_Count=0, unSEG_to_Transmit=0;
	double dEventtime=0.0;
	double byte;

	nAppId = pstruEventDetails->nApplicationId;
	nSocketId = (unsigned int)pstruEventDetails->szOtherDetails;
	/* Get the socket interface address and assign to local variable */
	pstru_Socket_Interface = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface;
	pstru_Tmp_Segment = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;

	if(pstru_Tmp_Segment)
	{
		nDeviceId = pstru_Tmp_Segment->nSourceId;
		szSourceIP = pstru_Tmp_Segment->pstruNetworkData->szSourceIP;
		szDestIP = pstru_Tmp_Segment->pstruNetworkData->szDestIP;
		usnSourcePort = pstru_Tmp_Segment->pstruTransportData->nSourcePort;
		usnDest_Port = pstru_Tmp_Segment->pstruTransportData->nDestinationPort; 

		/* If connection is already available for recevied segment then get the TCB */
		fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);

		if(!pstruTCP_Connection_Var)
		{
			printf("TCP-- Connection not available for the device %d\n",nDeviceId);
			return 1;
		}

		/* Check number of segments available in Retransmission queue */
		unNo_of_Seg_in_Queue = fn_NetSim_TCP_Check_No_Of_Segment_Available_In_Buffer(pstruTCP_Connection_Var,pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue);

		/* If number of segment in queue is less than the congestion window, 
		then send the segment */
		if(unNo_of_Seg_in_Queue <= pstruTCP_Connection_Var->un_cwnd )
		{
			unSEG_to_Transmit = pstruTCP_Connection_Var->un_cwnd - unNo_of_Seg_in_Queue;
			dEventtime = pstruEventDetails->dEventTime;

			/* Get the segment from the socket buffer and assign to the local variable to send */
			pstru_Received_Segment = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;
			byte = pstru_Received_Segment->pstruAppData->dPacketSize;

			//nagle's algorithm.
			while(unSEG_Count + byte <= unSEG_to_Transmit)
			{
				unSEG_Count += byte;
				
				pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist = pstru_Received_Segment->pstruNextPacket;
				pstru_Received_Segment->pstruNextPacket = NULL;
			

				/* Add payload and overheads */
				pstru_Received_Segment->pstruTransportData->dPayload = pstru_Received_Segment->pstruAppData->dPacketSize;
				pstru_Received_Segment->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;

				/* Assign the packet size */
				pstru_Received_Segment->pstruTransportData->dPacketSize = pstru_Received_Segment->pstruTransportData->dPayload + pstru_Received_Segment->pstruTransportData->dOverhead;

				/* Update TransportLayer segment time */
				pstru_Received_Segment->pstruTransportData->dArrivalTime = dEventtime;
				pstru_Received_Segment->pstruTransportData->dStartTime = dEventtime;
				pstru_Received_Segment->pstruTransportData->dEndTime = dEventtime;
				pstru_Received_Segment->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;		

				if(!pstruTCP_Connection_Var->dReceived_ACK_Count)
				{
					/* For first Segment Transmission calculate the RTO time */
					/* This function is used to calculate the RTO time */
					fn_NetSim_TCP_RTT(pstruTCP_Connection_Var);
					pstruTCP_Connection_Var->dReceived_ACK_Count++;
				}

				/* Add TCP header information */
				fn_NetSim_TCP_Header(pstruTCP_Connection_Var,pstru_Received_Segment);

				/* Add received segment to Retransmission Queue */
				fn_NetSim_TCP_AddSegmentTo_RetransmissionQueue(pstruTCP_Connection_Var,pstru_Received_Segment);

				/* Add the event details for NETWORK_OUT_EVENT*/
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
				pstruEventDetails->nPacketId = pstru_Received_Segment->nPacketId;
				pstruEventDetails->nSegmentId = pstru_Received_Segment->pstruAppData->nSegmentId;
				pstruEventDetails->dPacketSize=pstru_Received_Segment->pstruTransportData->dPacketSize;
				pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
				pstruEventDetails->nSubEventType=ZERO;
				pstruEventDetails->dEventTime = dEventtime;
				pstruEventDetails->pPacket=pstru_Received_Segment;
				fnpAddEvent(pstruEventDetails);

				fn_NetSim_TCP_Write_CongestionWindowData(pstruEventDetails->pPacket);

				pstruTCP_Connection_Var->pstruMetrics->nTotal_SEG_Transmitted++;
				
				/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
				and sub event as TCP_RETRANSMISSION_TIMEOUT */
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket);
				pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
				pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstru_Received_Segment);
				fnpAddEvent(pstruEventDetails);

				pstru_Received_Segment = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;
				if(pstru_Received_Segment)
				{
					byte = pstru_Received_Segment->pstruAppData->dPacketSize;
				}
				else
					break;
			}
		}
	}
	return 0;
}


/** 
This function is used to fill the TCP header information. As per RFC 793 page no.14
*/
int fn_NetSim_TCP_Header(TCB *pstruTCP_Connection_Var,NetSim_PACKET *pstru_Received_Segment)
{
	/* Variable to store TCP Segment header */
	SEGMENT_HEADER_TCP *pstruTCP_Segment_Header;	
	unsigned short int usnSourcePort;
	unsigned short int usnDestinationPort;
	/* Allocate memory for TCP Segment header */
	pstruTCP_Segment_Header = fnpAllocateMemory(1,sizeof(SEGMENT_HEADER_TCP));
	usnSourcePort = pstru_Received_Segment->pstruTransportData->nSourcePort;
	usnDestinationPort = pstru_Received_Segment->pstruTransportData->nDestinationPort;

	pstruTCP_Connection_Var->ulnSEG_SEQ = pstruTCP_Connection_Var->ulnSND_NXT;

	/* Fill TCP Header information */
	pstruTCP_Segment_Header->usnSource_Port = usnSourcePort;
	pstruTCP_Segment_Header->usnDestination_Port = usnDestinationPort;
	pstruTCP_Segment_Header->ulnSequence_Number = pstruTCP_Connection_Var->ulnSEG_SEQ;
	pstruTCP_Segment_Header->ulnAcknowledgment_Number = 0;
	pstruTCP_Segment_Header->nData_Offset = 0;
	pstruTCP_Segment_Header->nReserved = 0;
	pstruTCP_Segment_Header->bURG = false;
	pstruTCP_Segment_Header->bACK = false;
	pstruTCP_Segment_Header->bPSH = false;
	pstruTCP_Segment_Header->bRST = false;
	pstruTCP_Segment_Header->bSYN = false;
	pstruTCP_Segment_Header->nPrevControlDatatype = pstru_Received_Segment->nControlDataType;
	if(pstru_Received_Segment->pstruAppData->nAppEndFlag)
	{
		pstruTCP_Segment_Header->bFIN = true;
		pstruTCP_Connection_Var->isFinSend=true;
		pstru_Received_Segment->nControlDataType = TCP_FIN_DATA;
		pstruTCP_Connection_Var->ulnFIN_SEG_SEQ = pstruTCP_Connection_Var->ulnSEG_SEQ;
		fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_FIN_WAIT_1);
	}
	else if(pstruTCP_Connection_Var->isFinSend==true)
	{
		pstruTCP_Segment_Header->bFIN = true;
		pstruTCP_Connection_Var->isFinSend=true;
		pstru_Received_Segment->nControlDataType = TCP_FIN_DATA;
		pstruTCP_Connection_Var->ulnFIN_SEG_SEQ = pstruTCP_Connection_Var->ulnSEG_SEQ;
		fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_FIN_WAIT_1);
	}
	else
		pstruTCP_Segment_Header->bFIN = false;
	
	pstruTCP_Segment_Header->nWindow = (unsigned short)pstruTCP_Connection_Var->un_cwnd;
	pstruTCP_Segment_Header->usnChecksum = 0;
	pstruTCP_Segment_Header->usnUrgent_Pointer = 0;
	pstruTCP_Segment_Header->nPadding = 0;

	/* Assign the sequence number to TCB send sequence variables */
	pstruTCP_Connection_Var->ulnSND_NXT = pstruTCP_Connection_Var->ulnSEG_SEQ + pstru_Received_Segment->pstruTransportData->dPayload;

	pstru_Received_Segment->pstruTransportData->Packet_TransportProtocol = pstruTCP_Segment_Header;
	return 0;
}
