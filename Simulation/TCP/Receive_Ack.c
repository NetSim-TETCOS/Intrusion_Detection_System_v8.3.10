#include "main.h"
#include "TCP.h"
#include "../Application/Application.h"
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

* Author:   Sai Rohan
* Date  :    17-Sep-2014													  *
* --------------------------------------------------------------------------------*/
/**
This function is mainly used to receive the acknowledgement.
*/
int fn_NetSim_TCP_RECEIVE_ACK()
{
	/*Variable to store the received segment */
	NetSim_PACKET *pstruReceived_Segment;
	NETSIM_ID nDeviceID;
	unsigned int nSocketId;
	NETSIM_IPAddress szSourceIP, szDestIP;
	NETSIM_ID usnSourcePort, usnDest_Port;
	unsigned long int ulnTmpSEG_ACK;
	TCB *pstruTCP_Connection_Var;
	int nSucessDeletion, nWindow_Increment_Flag=0, nRetransmission_Flag = 0;
	double dEventTime;
	TCP_CONNECTION_STATE nConnectionState;

	pstruReceived_Segment=pstruEventDetails->pPacket;
	nDeviceID = pstruReceived_Segment->nDestinationId;
	dEventTime = pstruEventDetails->dEventTime;

	szSourceIP = pstruReceived_Segment->pstruNetworkData->szDestIP;
	szDestIP = pstruReceived_Segment->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruReceived_Segment->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruReceived_Segment->pstruTransportData->nSourcePort; 

	ulnTmpSEG_ACK = SEG_ACK(pstruReceived_Segment);

	fn_NetSim_TCP_Check_Connection(nDeviceID,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);

	nConnectionState = pstruTCP_Connection_Var->TCP_Present_Connection_State;
	pstruTCP_Connection_Var->dReceived_ACK_Count++;

	nSucessDeletion = fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(pstruTCP_Connection_Var,pstruReceived_Segment,&nRetransmission_Flag);

	/* don't calculate rtt time for retransmitted segment based on karn's algorithm */
	if(!nRetransmission_Flag)
		fn_NetSim_TCP_RTT(pstruTCP_Connection_Var);

	/* check the ack number to know about duplicate ack */
	fn_NetSim_TCP_Ack_Checking(pstruTCP_Connection_Var);

	nSocketId = fnGetSocketId(pstruTCP_Connection_Var->nAppId,pstruEventDetails->pPacket->nDestinationId,pstruEventDetails->pPacket->nSourceId,usnSourcePort,usnDest_Port);

	/* Free memory for received ack segment */
	if(pstruEventDetails->pPacket)
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);



	/* OLDTAHOE */
	if (pstruTCP_Connection_Var->Congestion_Control_Algorithm == OLD_TAHOE)
	{
		if(!pstruTCP_Connection_Var->bDuplicate_ACK)
		{
			nWindow_Increment_Flag = fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);

			//if(nWindow_Increment_Flag && nConnectionState == TCP_ESTABLISHED)
			if(nConnectionState == TCP_ESTABLISHED)
			{
				/* write transport out event into the event list */
				pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
				pstruEventDetails->dPacketSize = 0;
				pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
				pstruEventDetails->nSubEventType = TCP_SEND_SEG;
				pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
				pstruEventDetails->szOtherDetails = (void*)nSocketId;
				fnpAddEvent(pstruEventDetails);
			}
		}
		
	}
	/* TAHOE */
	else if(pstruTCP_Connection_Var->Congestion_Control_Algorithm == TAHOE)
	{
		
		/*If the number of duplicate ACKs received are 3 then perform fast retransmit */
		if(pstruTCP_Connection_Var->nDup_ACK_Count == THREE_DUP_ACK)
		{
			bool bSegmentAvailable;
			NetSim_PACKET *pstruTmpBuffer;
			NetSim_PACKET *pstruTmpSEG;

			pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

			bSegmentAvailable = fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(pstruTmpBuffer,&pstruTmpSEG,ulnTmpSEG_ACK);

			if(bSegmentAvailable && nConnectionState == TCP_ESTABLISHED)
			{
				NetSim_PACKET *pstruRetransmitSeg;

				fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);

				fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(pstruTCP_Connection_Var,&pstruRetransmitSeg,pstruTmpSEG);

				//Specific to NetSim TCP Metrics
				pstruTCP_Connection_Var->pstruMetrics->nSegFastRetransmitted++;

				/* Add the event details for NETWORK_OUT_EVENT*/
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
				pstruEventDetails->nPacketId = pstruRetransmitSeg->nPacketId;
				pstruEventDetails->nSegmentId = pstruRetransmitSeg->pstruAppData->nSegmentId;
				pstruEventDetails->dPacketSize=pstruRetransmitSeg->pstruTransportData->dPacketSize;
				pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=pstruRetransmitSeg;
				fnpAddEvent(pstruEventDetails);

				/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
				and sub event as TCP_RETRANSMISSION_TIMEOUT */
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
				pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
				pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
				pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruRetransmitSeg);
				fnpAddEvent(pstruEventDetails);	
			}
		}

		if(!pstruTCP_Connection_Var->bDuplicate_ACK && nConnectionState == TCP_ESTABLISHED)
		{
			fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);

			/* write transport out event into the event list */
			pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
			pstruEventDetails->dPacketSize = 0;
			pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType = TCP_SEND_SEG;
			pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
			pstruEventDetails->szOtherDetails=(void*)nSocketId;
			fnpAddEvent(pstruEventDetails);
		}
	}
	else if(pstruTCP_Connection_Var->Congestion_Control_Algorithm == RENO)
	{

		/* if DupAck count is 3 ,goes into fast recovery */
		if(pstruTCP_Connection_Var->nDup_ACK_Count == THREE_DUP_ACK)
		{
			bool bSegmentAvailable;
			NetSim_PACKET *pstruTmpBuffer;
			NetSim_PACKET *pstruTmpSEG;

			pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

			bSegmentAvailable = fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(pstruTmpBuffer,&pstruTmpSEG,ulnTmpSEG_ACK);

			if(bSegmentAvailable && nConnectionState == TCP_ESTABLISHED)
			{
				NetSim_PACKET *pstruRetransmitSeg;
				pstruTCP_Connection_Var->bFastRecovery = true;
				fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);

				//Specific to NetSim TCP Metrics
				pstruTCP_Connection_Var->pstruMetrics->nSegFastRetransmitted++;

				fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(pstruTCP_Connection_Var,&pstruRetransmitSeg,pstruTmpSEG);

				/* Add the event details for NETWORK_OUT_EVENT*/
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
				pstruEventDetails->nPacketId = pstruRetransmitSeg->nPacketId;
				pstruEventDetails->nSegmentId = pstruRetransmitSeg->pstruAppData->nSegmentId;
				pstruEventDetails->dPacketSize=pstruRetransmitSeg->pstruTransportData->dPacketSize;
				pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=pstruRetransmitSeg;
				fnpAddEvent(pstruEventDetails);

				/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
				and sub event as TCP_RETRANSMISSION_TIMEOUT */
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
				pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
				pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
				pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruRetransmitSeg);
				fnpAddEvent(pstruEventDetails);	
			}

			return 0;
		}	

		/*If a duplicate ACK is received when TCP is in Fast Recovery expand the cwnd by one MSS */
		else if(pstruTCP_Connection_Var->nDup_ACK_Count > THREE_DUP_ACK && pstruTCP_Connection_Var->bFastRecovery && nConnectionState == TCP_ESTABLISHED)
		{
			fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);
			/* write transport out event into the event list */
			pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
			pstruEventDetails->dEventTime = dEventTime;
			pstruEventDetails->dPacketSize = 0;
			pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType = TCP_SEND_SEG;
			pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
			pstruEventDetails->szOtherDetails=(void*)nSocketId;
			fnpAddEvent(pstruEventDetails);
			return 0;
		}

		if(!pstruTCP_Connection_Var->bDuplicate_ACK && nConnectionState == TCP_ESTABLISHED)
		{
			/* when a new ACK is received while its in fast recovery, the inflated cwnd is set to ssthresh*/
			if(pstruTCP_Connection_Var->bFastRecovery){
				fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);
			}

			else 
			fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);

			/* write transport out event into the event list */
			pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
			pstruEventDetails->dPacketSize = 0;
			pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType = TCP_SEND_SEG;
			pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;//fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruReceived_Segment);
			pstruEventDetails->dEventTime = dEventTime;
			pstruEventDetails->szOtherDetails=(void*)nSocketId;
			fnpAddEvent(pstruEventDetails);
			return 0;
		}
	}
	else if(pstruTCP_Connection_Var->Congestion_Control_Algorithm == NEWRENO)
	{

		/* if DupAck count is 3 ,goes into fast recovery */
		if(pstruTCP_Connection_Var->nDup_ACK_Count == THREE_DUP_ACK)
		{
			bool bSegmentAvailable;
			NetSim_PACKET *pstruTmpBuffer;
			NetSim_PACKET *pstruTmpSEG;

			pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

			bSegmentAvailable = fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(pstruTmpBuffer,&pstruTmpSEG,ulnTmpSEG_ACK);

			if(bSegmentAvailable && nConnectionState == TCP_ESTABLISHED)
			{
				NetSim_PACKET *pstruRetransmitSeg;

				pstruTCP_Connection_Var->bFastRecovery = true;

				// set the recover packet here
				pstruTCP_Connection_Var->ulnRecover = pstruTCP_Connection_Var->ulnSEG_SEQ;

				fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);

				//Specific to NetSim TCP Metrics
				pstruTCP_Connection_Var->pstruMetrics->nSegFastRetransmitted++;

				fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(pstruTCP_Connection_Var,&pstruRetransmitSeg,pstruTmpSEG);

				/* Add the event details for NETWORK_OUT_EVENT*/
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
				pstruEventDetails->nPacketId = pstruRetransmitSeg->nPacketId;
				pstruEventDetails->nSegmentId = pstruRetransmitSeg->pstruAppData->nSegmentId;
				pstruEventDetails->dPacketSize=pstruRetransmitSeg->pstruTransportData->dPacketSize;
				pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=pstruRetransmitSeg;
				fnpAddEvent(pstruEventDetails);

				/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
				and sub event as TCP_RETRANSMISSION_TIMEOUT */
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
				pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
				pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
				pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruRetransmitSeg);
				fnpAddEvent(pstruEventDetails);	
			}

			return 0;
		}	
		/*If a duplicate ACK is received when TCP is in Fast Recovery expand the cwnd by one MSS */
		else if(pstruTCP_Connection_Var->bDuplicate_ACK && pstruTCP_Connection_Var->bFastRecovery && nConnectionState == TCP_ESTABLISHED)
		{
			fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);
			/* write transport out event into the event list */
			pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
			pstruEventDetails->dEventTime = dEventTime;
			pstruEventDetails->dPacketSize = 0;
			pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType = TCP_SEND_SEG;
			pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
			pstruEventDetails->szOtherDetails=(void*)nSocketId;
			fnpAddEvent(pstruEventDetails);
			return 0;
		}

		if(!pstruTCP_Connection_Var->bDuplicate_ACK && nConnectionState == TCP_ESTABLISHED)
		{
			// When a new ACK is received while its in fast recovery, check for partial ACK
		    // When partial ACK is received, the cwnd is shrunk (new data acknowledged plus one MSS)
			// the lost packet in the window is retransmitted
			if(pstruTCP_Connection_Var->bFastRecovery && pstruTCP_Connection_Var->ulnSEG_ACK <= pstruTCP_Connection_Var->ulnRecover) 
			{ 
				bool bSegmentAvailable;
			    NetSim_PACKET *pstruTmpBuffer;
				NetSim_PACKET *pstruTmpSEG;

				pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

				bSegmentAvailable = fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(pstruTmpBuffer,&pstruTmpSEG,ulnTmpSEG_ACK);

				if(bSegmentAvailable && nConnectionState == TCP_ESTABLISHED)
				{
					NetSim_PACKET *pstruRetransmitSeg;

					// subtract new data ACKed and add one MSS to the cwnd
					fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);

					//Specific to NetSim TCP Metrics
					//pstruTCP_Connection_Var->pstruMetrics->nSegFastRetransmitted++;

					fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(pstruTCP_Connection_Var,&pstruRetransmitSeg,pstruTmpSEG);

					/* Add the event details for NETWORK_OUT_EVENT*/
					pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
					pstruEventDetails->nPacketId = pstruRetransmitSeg->nPacketId;
					pstruEventDetails->nSegmentId = pstruRetransmitSeg->pstruAppData->nSegmentId;
					pstruEventDetails->dPacketSize=pstruRetransmitSeg->pstruTransportData->dPacketSize;
					pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
					pstruEventDetails->nSubEventType=0;
					pstruEventDetails->pPacket=pstruRetransmitSeg;
					fnpAddEvent(pstruEventDetails);

					/* RETRANSMISSION_TIMEOUT Event, Add the event details for TIMER_EVENT 
					and sub event as TCP_RETRANSMISSION_TIMEOUT */
					pstruEventDetails->nEventType = TIMER_EVENT;
					pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
					pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
					pstruEventDetails->dEventTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
					pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruRetransmitSeg);
					fnpAddEvent(pstruEventDetails);	

			    }
			}
			//if a full ACK is received
			else if(pstruTCP_Connection_Var->bFastRecovery){
				fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);
			}

			else 
			fn_NetSim_TCP_Window_Expansion(pstruTCP_Connection_Var);

			///* write transport out event into the event list */
			//Send another segment if possible
			pstruEventDetails->nApplicationId = pstruTCP_Connection_Var->nAppId;
			pstruEventDetails->dPacketSize = 0;
			pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
			pstruEventDetails->nSubEventType = TCP_SEND_SEG;
			pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;//fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruReceived_Segment);
			pstruEventDetails->dEventTime = dEventTime;
			pstruEventDetails->szOtherDetails=(void*)nSocketId;
			fnpAddEvent(pstruEventDetails);
			return 0;
		}
	}
	return 0;
}


/**
 This function is used to check whether the retransmission timeout segment  
 is available in retransmission queue or not. If the segment is available, that   
 needs to be retransmitted, return true if available otherwise return false. 
*/
bool fn_NetSim_TCP_Check_Segment_Availability_In_Buffer_For_ThreeDupAck(NetSim_PACKET *pstruBuffer,NetSim_PACKET **pstruRetransmissionSegment,unsigned long int ulnSEG_SEQ)
{
	if(pstruBuffer)
	{
		unsigned long int ulnTmpSEG_SEQ;
		SEGMENT_HEADER_TCP *pstruSEGHdr;
		NetSim_PACKET *pstruTmpBuffer;
		pstruTmpBuffer = pstruBuffer;
		ulnTmpSEG_SEQ = SEG_SEQ(pstruTmpBuffer);

		while(ulnSEG_SEQ != ulnTmpSEG_SEQ)
		{
			pstruTmpBuffer = pstruTmpBuffer->pstruNextPacket;
			if(!pstruTmpBuffer)
				return false;
			ulnTmpSEG_SEQ = SEG_SEQ(pstruTmpBuffer);
		}
		
		pstruSEGHdr = pstruTmpBuffer->pstruTransportData->Packet_TransportProtocol;

		if(!pstruSEGHdr->bReTransmission_Flag)
		{
			(*pstruRetransmissionSegment) = pstruTmpBuffer;
			pstruSEGHdr = pstruTmpBuffer->pstruTransportData->Packet_TransportProtocol;
			pstruSEGHdr->bReTransmission_Flag = true;
			pstruSEGHdr->bThreeDuplicateAckFlag = true;
			return true;
		}
	}
	return false;
}
/**
This function is to form the retransmission segment for three duplicate acknowledgements.
*/
int fn_NetSim_TCP_Form_RetransmissionSeg_For_Three_Duplicate_Ack(TCB *pstruTCP_Connection_Var,NetSim_PACKET **pstruRetransmitSeg,NetSim_PACKET *pstruTmpSeg)
{
	SEGMENT_HEADER_TCP *pstruSegmentHeader;
	*pstruRetransmitSeg = fn_NetSim_Packet_CopyPacket(pstruTmpSeg);
	pstruSegmentHeader = (*pstruRetransmitSeg)->pstruTransportData->Packet_TransportProtocol;
	pstruSegmentHeader->bReTransmission_Flag = true;
	pstruSegmentHeader->bThreeDuplicateAckFlag = true;
	
	(*pstruRetransmitSeg)->dEventTime = pstruEventDetails->dEventTime;
	(*pstruRetransmitSeg)->pstruTransportData->dArrivalTime = pstruEventDetails->dEventTime;
	(*pstruRetransmitSeg)->pstruTransportData->dStartTime = pstruEventDetails->dEventTime;
	(*pstruRetransmitSeg)->pstruTransportData->dEndTime = pstruEventDetails->dEventTime;
	return 0;
}

/**
This function is to check the Retrasmission queue for the duplicate segment. If the duplicate segment is found update the duplicate ack count and return 1 else return 0.
*/
int	fn_NetSim_TCP_Update_DupAckCount_In_Buffer(TCB *pstruTCP_Connection_Var,unsigned long int ulnSEG_SEQ)
{
	if(pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue)
	{
		NetSim_PACKET *pstruTmpBuffer;
		unsigned long int ulnTmpSEG_SEQ;
		SEGMENT_HEADER_TCP *pstruTmpHdr;

		pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;
		ulnTmpSEG_SEQ = SEG_SEQ(pstruTmpBuffer);

		while(ulnSEG_SEQ != ulnTmpSEG_SEQ)
		{
			pstruTmpBuffer = pstruTmpBuffer->pstruNextPacket;
			if(!pstruTmpBuffer)
				return 0;
			ulnTmpSEG_SEQ = SEG_SEQ(pstruTmpBuffer);
		}
		pstruTmpHdr = pstruTmpBuffer->pstruTransportData->Packet_TransportProtocol;
		pstruTmpHdr->nDupAckCount = pstruTCP_Connection_Var->nDup_ACK_Count;
		return 1;
	}
	return 0;
}

