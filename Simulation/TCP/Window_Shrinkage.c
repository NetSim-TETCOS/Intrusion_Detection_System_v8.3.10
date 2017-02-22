#include "main.h"
#include "TCP.h"

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

* Author:   Sai Rohan
* Date  :    17-Sep-2014
* --------------------------------------------------------------------------------*/
/**
The following function mainly involved in the window shrinkage when there 
is a retransmission. This function is common for Old tahoe and for tahoe.
In old Tahoe it is called upon RTO expiration
In Tahoe it is called during RTO expiration or on receipt of 3 dup acks
*/
#pragma warning(disable:4102)
_declspec (dllexport) int fn_NetSim_TCP_Window_Shrinkage(TCB *pstruTCP_Connection_Var)
{

	unsigned int nMin_ssthresh=0;

	if(pstruTCP_Connection_Var->Congestion_Control_Algorithm == NEWRENO &&pstruTCP_Connection_Var->bFastRecovery && !pstruTCP_Connection_Var->bDuplicate_ACK && pstruTCP_Connection_Var->ulnSEG_ACK <= pstruTCP_Connection_Var->ulnRecover)
	{
		/* if a partial ack is received, shrink the cwnd by the number of new data acknowledged and add one MSS  */
		  pstruTCP_Connection_Var->un_cwnd = pstruTCP_Connection_Var->un_cwnd - pstruTCP_Connection_Var->ulnNewData + pstruTCP_Connection_Var->nSMSS;
	      return 0;
	}


	if(pstruTCP_Connection_Var->Congestion_Control_Algorithm == RENO || pstruTCP_Connection_Var->Congestion_Control_Algorithm == NEWRENO)
	{
		if(pstruTCP_Connection_Var->bFastRecovery && !pstruTCP_Connection_Var->bDuplicate_ACK)
		{
EXIT_FAST_RECOVERY:
			/*TCP Reno: When a new ack is received, exit fast recovery, set cwnd to threshold */
			pstruTCP_Connection_Var->un_cwnd = pstruTCP_Connection_Var->un_ssthresh;              //during fast recovery, exit fast  
			pstruTCP_Connection_Var->bFastRecovery = false;                                       //recovery and set cwnd to ssthresh
			pstruTCP_Connection_Var->unCongestionAvoidance_Count = 0;
			if (pstruTCP_Connection_Var->un_cwnd < pstruTCP_Connection_Var->unAdvertised_Window)
				pstruTCP_Connection_Var->nWindowExpand_Flag = 0;
			else
				pstruTCP_Connection_Var->nWindowExpand_Flag = 1;
			
			return 0;
		}
		if(pstruTCP_Connection_Var->nDup_ACK_Count == THREE_DUP_ACK)
		{

FAST_RECOVERY:
			/* While going into Fast Recovery set threshold to half of cwnd plus 3*MSS */
			pstruTCP_Connection_Var->un_ssthresh = ((int)pstruTCP_Connection_Var->un_cwnd / (2*pstruTCP_Connection_Var->nSMSS))*pstruTCP_Connection_Var->nSMSS;
			nMin_ssthresh = 2 * pstruTCP_Connection_Var->nSMSS;

			if(pstruTCP_Connection_Var->un_ssthresh < nMin_ssthresh)
				pstruTCP_Connection_Var->un_ssthresh = nMin_ssthresh;

			pstruTCP_Connection_Var->un_cwnd = pstruTCP_Connection_Var->un_ssthresh + 3*pstruTCP_Connection_Var->nSMSS;

			if (pstruTCP_Connection_Var->un_cwnd < pstruTCP_Connection_Var->unAdvertised_Window)
				pstruTCP_Connection_Var->nWindowExpand_Flag = 0;
			else
				pstruTCP_Connection_Var->nWindowExpand_Flag = 1;

			return 0;
		}
		else
		{
			goto SLOW_START;// a timer out event
		}
	} 			
	else //Other congestion control algorithm
	{ 
SLOW_START:	

		/* minimum threshold value should be 2*SEG size */
		nMin_ssthresh = 2 * pstruTCP_Connection_Var->nSMSS;
		
		// if retransmitted packet is errored
		pstruTCP_Connection_Var->bFastRecovery = false; 
		
		pstruTCP_Connection_Var->un_ssthresh = ((int)pstruTCP_Connection_Var->un_cwnd / (2*pstruTCP_Connection_Var->nSMSS))*pstruTCP_Connection_Var->nSMSS;
		
		if (pstruTCP_Connection_Var->un_ssthresh < nMin_ssthresh) 
		{
			pstruTCP_Connection_Var->un_ssthresh = nMin_ssthresh;
		}

		/* set the congestion window size to one segment size */
		pstruTCP_Connection_Var->un_cwnd = pstruTCP_Connection_Var->nSMSS;

		if (pstruTCP_Connection_Var->un_cwnd == pstruTCP_Connection_Var->unAdvertised_Window)
		{
			pstruTCP_Connection_Var->nWindowExpand_Flag = 1;
		}
		else
		{
			pstruTCP_Connection_Var->nWindowExpand_Flag = 0;
		}
		pstruTCP_Connection_Var->unCongestionAvoidance_Count = 0;
		return 0;
	}
}