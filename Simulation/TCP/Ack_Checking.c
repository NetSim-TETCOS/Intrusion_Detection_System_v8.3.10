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
	This function is mainly used to know about the duplicate acknowledgment. 
	TCP performs the ACK checking to know about the Duplicate ACK. This Duplicate    
	ACK checking is mainly used in Tahoe Congestion Control Algorithm.
*/

_declspec (dllexport) int fn_NetSim_TCP_Ack_Checking(TCB *pstruTCP_Connection_Var)
{

	NetSim_PACKET *pstruReceived_Segment=NULL;
	SEGMENT_HEADER_TCP *pstruTCP_Segment_Header=NULL;
	
	/* Get the Ack segment and assign to the local variable */
	pstruReceived_Segment = pstruEventDetails->pPacket;
	/* Get the Ack segment header and assign to the local variable to know about 
	the ack number*/
	pstruTCP_Segment_Header = pstruReceived_Segment->pstruTransportData->Packet_TransportProtocol;

	/* Get the current segment ack number and assign to the TCB current segment variable */
	pstruTCP_Connection_Var->ulnSEG_ACK = pstruTCP_Segment_Header->ulnAcknowledgment_Number;
	
	/* If previous ack number and received ack number not same means,  this is not a duplicate
	ack, so set the previous ack number as received ack number */
	
	if((pstruTCP_Connection_Var->ulnPrevious_SEG_ACK != 0) && (pstruTCP_Connection_Var->ulnPrevious_SEG_ACK != pstruTCP_Connection_Var->ulnSEG_ACK))
	{
		pstruTCP_Connection_Var->ulnNewData = pstruTCP_Connection_Var->ulnSEG_ACK - pstruTCP_Connection_Var->ulnPrevious_SEG_ACK;
		pstruTCP_Connection_Var->ulnPrevious_SEG_ACK = pstruTCP_Connection_Var->ulnSEG_ACK;
		pstruTCP_Connection_Var->bDuplicate_ACK = false;
		pstruTCP_Connection_Var->nDup_ACK_Count = 0;
		//specific to NetSim metrics
		pstruTCP_Connection_Var->pstruMetrics->nTotal_ACK_Received++;
	}
	/* If previous ack number and received ack number both are same then 
	this is duplicate ack */
	else if(pstruTCP_Connection_Var->ulnPrevious_SEG_ACK == pstruTCP_Connection_Var->ulnSEG_ACK)
	{
		pstruTCP_Connection_Var->ulnNewData = 0;
		pstruTCP_Connection_Var->nDup_ACK_Count++;
		pstruTCP_Connection_Var->bDuplicate_ACK = true;
		pstruTCP_Connection_Var->ulnPrevious_SEG_ACK = pstruTCP_Connection_Var->ulnSEG_ACK;
		//specific to NetSim metrics
		pstruTCP_Connection_Var->pstruMetrics->nTotal_Dup_ACK_Received++;
	}
	/* If first ack received  set the received ack number as previous ack number */
	else 
	{
		pstruTCP_Connection_Var->ulnPrevious_SEG_ACK = pstruTCP_Connection_Var->ulnSEG_ACK;
		pstruTCP_Connection_Var->bDuplicate_ACK = false;
		pstruTCP_Connection_Var->nDup_ACK_Count = 0;
		//specific to NetSim metrics
		pstruTCP_Connection_Var->pstruMetrics->nTotal_ACK_Received++;
	}
	return 0;
}

