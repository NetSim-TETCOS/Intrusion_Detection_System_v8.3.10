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

/**  The following function is mainly used to delete the segments from retransmission queue.
when TCP gets an ack for already sent segment, it will delete that particular segment
from retransmission queue. */
int fn_NetSim_TCP_Delete_Segement_From_Retransmission_Queue(TCB *pstruTCP_Connection_Var,NetSim_PACKET *pstru_Received_Segment,int *nRetransmissionFlag)
{	
	int nSuccessDeletion = 0;

	/* If Sender Buffer is not NULL */
	if(pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue)
	{
		NetSim_PACKET *pstru_TCP_Tmp_retransmission_queue;
		unsigned long int ulnRCV_SEG_SEQ,ulnTmp_SEG_SEQ, ulnRCV_SEG_ACK,byte;
		SEGMENT_HEADER_TCP *pstruReceivedSegmentHeader;

		pstru_TCP_Tmp_retransmission_queue = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;

		ulnTmp_SEG_SEQ = SEG_SEQ(pstru_TCP_Tmp_retransmission_queue);
		byte = pstru_TCP_Tmp_retransmission_queue->pstruTransportData->dPayload;
		ulnRCV_SEG_ACK = SEG_ACK(pstru_Received_Segment);
		ulnRCV_SEG_SEQ = ulnRCV_SEG_ACK - pstruTCP_Connection_Var->nSEG_LEN;

		pstruReceivedSegmentHeader = pstru_Received_Segment->pstruTransportData->Packet_TransportProtocol;

#pragma message(__LOC__"why this is here???")
		if(pstruReceivedSegmentHeader->bReTransmission_Flag)
			*nRetransmissionFlag = 1;

		while((ulnTmp_SEG_SEQ + byte) <= ulnRCV_SEG_ACK)
		{
			nSuccessDeletion = 1;

#pragma message(__LOC__"why this is here???")
			if(ulnRCV_SEG_SEQ == pstruTCP_Connection_Var->ulnFIN_SEG_SEQ)
				fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_FIN_WAIT_2);

			pstru_TCP_Tmp_retransmission_queue = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue;
			pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue = pstru_TCP_Tmp_retransmission_queue->pstruNextPacket;
			pstru_TCP_Tmp_retransmission_queue->pstruNextPacket = NULL;

			/*delete the segment from tcp retransmission queue */
			fn_NetSim_Packet_FreePacket(pstru_TCP_Tmp_retransmission_queue);

			if (!pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue)
			{
				break;
			}
			ulnTmp_SEG_SEQ = SEG_SEQ(pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue);
			byte = pstruTCP_Connection_Var->pstru_TCP_Retransmission_Queue->pstruTransportData->dPayload;
		}	
	}
	return nSuccessDeletion;
}
