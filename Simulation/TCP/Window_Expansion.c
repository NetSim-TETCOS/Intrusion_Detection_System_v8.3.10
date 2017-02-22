#include "main.h"
#include "TCP.h"

/*---------------------------------------------------------------------------------
* Copyright (C) 2014															  *
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following window expansion function is used when we need to expand the window 
size. Consider a case, according to slow start first we will transmit only one segment. 
then we will wait for the ack. after receving the ack we needs to expand the window size.
this wiondow size expansion depends on the congestion control algorithms. the folllowing 
algorithm is mainly used to perform that. the following function is to know about this 
window expansion. please refer richard w stevence book. 
in that go through the transmission contorl protocol's congestion control algorithm techniques.
if old tahoe congestion control algorithm is used it follows two phases 
1. Slow Start 2. Congestion Avoidance.                                                              
1. Slow Start:
congestion window size increase after receiving each ack.                                          *
2. Congestion Avoidance: 
example. if congestion window = 3. then congestion window increases, 
after getting ack for all the three packet. to perform the above 
the following congestion avoidance count value used. when this value reaches 
the congestion window size value only further window size is increased. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
_declspec (dllexport) int fn_NetSim_TCP_Window_Expansion(TCB *pstruTCP_Connection_Var)
{

	/* check the congestion window is lesser than the ssthreshold value */
	/*Each time another duplicate ACK arrives(in fast recovery), increment cwnd by the segment size.*/

	if(pstruTCP_Connection_Var->un_cwnd < pstruTCP_Connection_Var->un_ssthresh || pstruTCP_Connection_Var->bFastRecovery)
	{
		/* slow start phase. size of the window is doubled every time, 
		when ack segment is received successfully */
		if(pstruTCP_Connection_Var->nWindowExpand_Flag == 0)
		{
			pstruTCP_Connection_Var->un_cwnd += pstruTCP_Connection_Var->nSMSS;
		}
		/* when it reaches the maximum advertised window value don't increment 
		the window size further. set WindowExpand flag as 1*/
		if(pstruTCP_Connection_Var->un_cwnd == pstruTCP_Connection_Var->unAdvertised_Window)
		{
			pstruTCP_Connection_Var->nWindowExpand_Flag = 1;
		}
		return 1;
	}
	else if(pstruTCP_Connection_Var->un_cwnd >= pstruTCP_Connection_Var->un_ssthresh)
	{
		// RFC 2001 ;Congestion avoidance phase dictates that cwnd be incremented 
		// by segsize*segsize/cwnd each time an ACK is received
		if (pstruTCP_Connection_Var->nWindowExpand_Flag == 0) 
		{
			pstruTCP_Connection_Var->un_cwnd+= (pstruTCP_Connection_Var->nSMSS * pstruTCP_Connection_Var->nSMSS) / (pstruTCP_Connection_Var->un_cwnd) ;						
		}

		if (pstruTCP_Connection_Var->un_cwnd  < pstruTCP_Connection_Var->unAdvertised_Window)
			pstruTCP_Connection_Var->nWindowExpand_Flag = 0;
		else
			pstruTCP_Connection_Var->nWindowExpand_Flag = 1;

		return 1;
	}
	else
	{
		fnNetSimError("TCP-- Unknown condition in window expansion function\n");
	}

	return 0;
}
