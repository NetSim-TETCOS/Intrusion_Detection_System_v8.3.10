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

* Author:   Sai Rohan
* Date  :    17-Sep-2014														  *
* --------------------------------------------------------------------------------*/
/**
	when the TCP transmits a segment, it will stats a timer(Retransmission Timeout). 
	this function is used to retransmit that particular segment.					 
*/
int fn_NetSim_TCP_Retransmission_Timeout()
{
	bool bSEG_Availability=false;
	unsigned long int ulnTmpSEG_SEQ=0;
	TCB *pstruTCP_Connection_Var=NULL;
	NETSIM_ID nDeviceID;
	NETSIM_IPAddress szSourceIP=NULL;
	NETSIM_IPAddress szDestIP=NULL;
	NETSIM_ID usnSourcePort;
	NETSIM_ID usnDest_Port;
	NetSim_PACKET *pstru_Tmp_RetransmissionSegment=NULL;
	int nThreeDupAckCount=0;
	NetSim_PACKET *pstruTmpBuffer;
	nDeviceID = pstruEventDetails->nDeviceId;


	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;

	fn_NetSim_TCP_Check_Connection(nDeviceID,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);

	if(pstruTCP_Connection_Var)
	{
		pstruTmpBuffer = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

		ulnTmpSEG_SEQ = SEG_SEQ(pstruEventDetails->pPacket);

		bSEG_Availability = fn_NetSim_TCP_Check_Segment_Availability_In_Buffer(pstruTmpBuffer,&pstru_Tmp_RetransmissionSegment,ulnTmpSEG_SEQ,&nThreeDupAckCount);
	}

	
	/*Checks whether the segment is in the re-transmission queue or whether it has already been 
	  retransmitted through fast retransmit and the latest ACK matches the segment to be retransmitted*/
	if(bSEG_Availability && !nThreeDupAckCount && (ulnTmpSEG_SEQ==pstruTCP_Connection_Var->ulnSEG_ACK  || !pstruTCP_Connection_Var->un_cwnd) )
	{
		double start_time = pstruEventDetails->pPacket->pstruTransportData->dStartTime;

		NetSim_PACKET *pstru_Retransmission_Segment=NULL;
		TCP_SEGMENT_TYPE nSegmentType;

		if(pstruEventDetails->pPacket)
		{
			fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
			pstruEventDetails->pPacket = NULL;
		}
		nSegmentType = (TCP_SEGMENT_TYPE)pstru_Tmp_RetransmissionSegment->nControlDataType;

		if(nSegmentType != TCP_SYN && nSegmentType != TCP_SYN_ACK)
		{
			pstruTCP_Connection_Var->pstruMetrics->nTotal_SEG_Retransmitted++;

			fn_NetSim_TCP_Window_Shrinkage(pstruTCP_Connection_Var);

		}
		else
			pstruTCP_Connection_Var->pstruMetrics->unFailedConnectionAttempts++;
		
		/* form the retransmission segment */
		fn_NetSim_TCP_Form_Retransmission_Segment(pstruTCP_Connection_Var,&pstru_Retransmission_Segment,pstru_Tmp_RetransmissionSegment, start_time);
		pstru_Tmp_RetransmissionSegment->pstruTransportData->dStartTime=pstruEventDetails->dEventTime;
		pstru_Tmp_RetransmissionSegment->pstruTransportData->dArrivalTime=pstruEventDetails->dEventTime;
		pstru_Tmp_RetransmissionSegment->pstruTransportData->dEndTime=pstruEventDetails->dEventTime;

		pstruEventDetails->dPacketSize = pstru_Retransmission_Segment->pstruTransportData->dPacketSize;
		pstruEventDetails->nEventType = NETWORK_OUT_EVENT;
		pstruEventDetails->nPacketId = pstru_Retransmission_Segment->nPacketId;

		if(pstru_Retransmission_Segment->pstruAppData)
			pstruEventDetails->nSegmentId = pstru_Retransmission_Segment->pstruAppData->nSegmentId;

		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket = pstru_Retransmission_Segment;
		fnpAddEvent(pstruEventDetails);

		/* Add RETRANSMISSION_TIMEOUT Event */
		/* Add the event details for TIMER_EVENT and sub event as TCP_RETRANSMISSION_TIMEOUT */
		pstruEventDetails->nDeviceId = nDeviceID;
		pstruEventDetails->nEventType = TIMER_EVENT;
		pstruEventDetails->nSubEventType = TCP_RETRANSMISSION_TIMEOUT;
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket);
		pstruEventDetails->dEventTime = pstru_Retransmission_Segment->pstruTransportData->dStartTime + pstruTCP_Connection_Var->dRTO_TIME + 1;
		pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstru_Retransmission_Segment);
		fnpAddEvent(pstruEventDetails);
	}

	/* Special case where there are multiple errors within the same window.
	NetSim adds another time out event for those timeouts that occur for
	packets that are in retransmission queue and whose seq no is > current_ack_value
	*/
	else if(bSEG_Availability && !nThreeDupAckCount && ulnTmpSEG_SEQ>pstruTCP_Connection_Var->ulnSEG_ACK )
	{
		double rto = pstruEventDetails->dEventTime-pstruEventDetails->pPacket->pstruTransportData->dStartTime;
		pstruEventDetails->pPacket->pstruTransportData->dStartTime =pstruEventDetails->dEventTime;
		pstruEventDetails->dEventTime += rto;
		fnpAddEvent(pstruEventDetails);
	}
	return 0;
}

/**
 This function is used to form Retransmission timeout segment						
 is available in retransmission queue or not. It the segment is available, that   
 needs to be retransmitted, it will return 1, otherwise it will return 0. 		
*/
int	fn_NetSim_TCP_Form_Retransmission_Segment(TCB *pstruTCP_Connection_Var,NetSim_PACKET **pstru_Retransmission_Segment,NetSim_PACKET *pstru_Tmp_RetransmissionSegment,double start_time)
{
	SEGMENT_HEADER_TCP *pstruSegmentHeader,*pstruSegHeaderofBuffer;
	
	//update the retransmission flag of sender buffer segment, this will be useful in tahoe algorithm.
	pstruSegHeaderofBuffer = pstru_Tmp_RetransmissionSegment->pstruTransportData->Packet_TransportProtocol;
	pstruSegHeaderofBuffer->bReTransmission_Flag = true;

	*pstru_Retransmission_Segment = fn_NetSim_Packet_CopyPacket(pstru_Tmp_RetransmissionSegment);

	pstruSegmentHeader = (*pstru_Retransmission_Segment)->pstruTransportData->Packet_TransportProtocol;
	pstruSegmentHeader->bReTransmission_Flag = true;
	if(pstruTCP_Connection_Var->isFinSend==true)
	{
		pstruSegmentHeader->bFIN = true;
		pstruTCP_Connection_Var->isFinSend=true;
		(*pstru_Retransmission_Segment)->nControlDataType = TCP_FIN_DATA;
		pstruTCP_Connection_Var->ulnFIN_SEG_SEQ = pstruTCP_Connection_Var->ulnSEG_SEQ;
		fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_FIN_WAIT_1);
	}
	(*pstru_Retransmission_Segment)->dEventTime = pstruEventDetails->dEventTime;
	(*pstru_Retransmission_Segment)->pstruTransportData->dArrivalTime = pstruEventDetails->dEventTime;
	(*pstru_Retransmission_Segment)->pstruTransportData->dStartTime = pstruEventDetails->dEventTime;
	(*pstru_Retransmission_Segment)->pstruTransportData->dEndTime = pstruEventDetails->dEventTime;
	pstruTCP_Connection_Var->dRTO_TIME = pstruEventDetails->dEventTime - start_time;
	
	//Calculate the RTO time
	fn_NetSim_TCP_RTO(pstruTCP_Connection_Var);

	//Reset the TCP timers
	pstruTCP_Connection_Var->dSDEVIATION = 0.0;
	pstruTCP_Connection_Var->dSRTT_TIME = 0.0;

	return 0;
}

