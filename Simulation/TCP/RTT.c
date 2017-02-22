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

/** This function is used to calculate RTO time  based on jacobson algorithm.
   To provide reliability TCP uses RTO timers. This RTO values are obtained 
   by applying the jacobson algorithm. It uses 3 seconds for the first segment. 
   To know about the first segment we have used received ack count variable 
   in this funtion. */
_declspec (dllexport) int fn_NetSim_TCP_RTT(TCB *pstruTCP_Connection_Var)
{
	if(pstruTCP_Connection_Var->dReceived_ACK_Count == 0)
	{
		/* RTO timer value for the first segment is defaultly set as 3 seconds, 
		as per RFC 1122 October 1989, page no 96 */
		pstruTCP_Connection_Var->dRTO_TIME = INITIAL_RTO_TIME;
	}
	else if (pstruTCP_Connection_Var->dReceived_ACK_Count == 1
		|| pstruTCP_Connection_Var->dSRTT_TIME == 0
		&& pstruTCP_Connection_Var->dSDEVIATION == 0) 
	{

		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  *
		 *                                                              *
		 *	 	SRTT = RTT                                             *
		 *	                                                           *
		 *	 	SDEVIATION = SRTT/2                                    *
		 *	                                                           *
		 *		RTO = SRTT+4*SDEVIATION                                *
		 *	                                                           *
		 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

		pstruTCP_Connection_Var->dRTT_TIME = pstruEventDetails->dEventTime
			- pstruEventDetails->pPacket->pstruTransportData->dStartTime;

		pstruTCP_Connection_Var->dSRTT_TIME = pstruTCP_Connection_Var->dRTT_TIME;

		pstruTCP_Connection_Var->dSDEVIATION = pstruTCP_Connection_Var->dSRTT_TIME / 2;

		pstruTCP_Connection_Var->dRTO_TIME = pstruTCP_Connection_Var->dSRTT_TIME
				+ (4 * pstruTCP_Connection_Var->dSDEVIATION);
	}
	else 
	{
		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  *
		 *                                                              *
		 *	 	SRTT = (1-0.125)*SRTT + 0.125*RTT                      *
		 *	                                                           *
		 *	 	SERROR = RTT - SRTT                                    *
		 *	                                                           *
		 *		SDEIVATION = (1-0.250)*SDEVIATION + 0.250*SERROR       *
		 *	                                                           *
		 *		RTO = SRTT + 4*SDEVIATION       					   *                                                                                     *
		 *	                                                           *
		 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
		pstruTCP_Connection_Var->dRTT_TIME = pstruEventDetails->dEventTime
				- pstruEventDetails->pPacket->pstruTransportData->dStartTime;

		pstruTCP_Connection_Var->dSERROR = pstruTCP_Connection_Var->dRTT_TIME
				- pstruTCP_Connection_Var->dSRTT_TIME;

		pstruTCP_Connection_Var->dSRTT_TIME = (1 - 0.125)
				* (pstruTCP_Connection_Var->dSRTT_TIME) + (0.125
				* pstruTCP_Connection_Var->dRTT_TIME);

		if (pstruTCP_Connection_Var->dSERROR <= 0)
		{
			pstruTCP_Connection_Var->dSERROR
					=- pstruTCP_Connection_Var->dSERROR;
		}
		pstruTCP_Connection_Var->dSDEVIATION = (1 - 0.250)
				* (pstruTCP_Connection_Var->dSDEVIATION) + (0.250
				* pstruTCP_Connection_Var->dSERROR);

		pstruTCP_Connection_Var->dRTO_TIME = pstruTCP_Connection_Var->dSRTT_TIME
				+ (4 * pstruTCP_Connection_Var->dSDEVIATION);

	}
	/* maximum rto expiration value 1 minutes. if rto timer value exceeds 1 minute reset 
	   rto timer value as 1 minute */
	if (pstruTCP_Connection_Var->dRTO_TIME > MAX_RTO_TIME)
	{
		pstruTCP_Connection_Var->dRTO_TIME = MAX_RTO_TIME;
	}
	if(pstruTCP_Connection_Var->dRTO_TIME < MIN_RTO_TIME)
		pstruTCP_Connection_Var->dRTO_TIME = MIN_RTO_TIME;

	return 0;
}
