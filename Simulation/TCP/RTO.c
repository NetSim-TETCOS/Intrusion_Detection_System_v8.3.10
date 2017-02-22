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

* Author:   P.Sathishkumar 
* Date  :    29-Oct-2012
* --------------------------------------------------------------------------------*/

/** 
	whenever RTO timer value expires then exponential timer is called.   
	Maximum RTO timer value is 1 minute, if the retransmitting segment  
	RTO timer value exceeds 1 minute, reset rto timer value as 1 minute.
*/
_declspec (dllexport) int fn_NetSim_TCP_RTO(TCB *pstruTCP_Connection_Var)
{
	/* rto timer value is doubled if the timer value is less than 1 minute */
	pstruTCP_Connection_Var->dRTO_TIME *= 2;

	if (pstruTCP_Connection_Var->dRTO_TIME > MAX_RTO_TIME) 
	{
		pstruTCP_Connection_Var->dRTO_TIME = MAX_RTO_TIME;
	}
	return 0;
}
