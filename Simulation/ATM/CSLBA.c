/************************************************************************************

 * Copyright (C) 2012                                                               *

 * TETCOS, Bangalore. India                                                         *

 *                                                                                  *

 * Tetcos owns the intellectual property rights in the Product and its content.     *

 * The copying, redistribution, reselling or publication of any or all of the       *

 * Product or its content without express prior written consent of Tetcos is        *

 * prohibited. Ownership and / or any other right relating to the software and all *

 * intellectual property rights therein shall remain at all times with Tetcos.      *

 *                                                                                  *

 * Author:    Shashi Kant Suman                                                        *

 *                                                                                  *

 * ---------------------------------------------------------------------------------*/

#ifndef CSLBA_H_
#define CSLBA_H_
# ifndef ATM_H_
# include "ATM.h"
# endif
/**************  int m_fnContinuousStateLeakyBucketAlg(ATM_DATASTRUCTURE**) ***************
	Purpose: Check whether incoming cell conforms to traffic rate or NOT

	Algorithm:
		¢	Declare Necessary variable
		¢	If incoming cell = first cell set LCT ( last Conformance time) == arrival time
		¢	Check for cell Conformance
		¢	set conformance flag = 1 if cell conforms
		¢	set conformance flag = 0 if cell does not conform


***********************************************************************************************/
_declspec(dllexport) int m_fnContinuousStateLeakyBucketAlg(ATM_DATASTRUCTURE** pstruATMData)
{
	double ldTempBuffer=0.0;	// Temporary buffer
	USERCELL *pstru_NP_ATMHeadCell;// ATM head cell

	pstru_NP_ATMHeadCell= (*pstruATMData)->pstru_NI_CurrentCell; //Setting ATM head cell = Current cell

	/*	Condition for checking whether the received packet is First or not
	 *	If it is the first packet then set Last Conformance cell to Arrival
	 *	time of current cell
	 */
	if(pstru_NP_ATMHeadCell->n_NI_CellSequenceNo == 1 /* First Cell */)
	{
		dATM_LASTCONFORMANCETIME = pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime;  /* First cell is always conformed
																 * so, last conformance time =
																 * first Cell arrival time
																 */
		dATM_BUFFERCOUNT = 0.0;	// First cell, so buffer is empty.
	}//End of if(first cell)

	/*  Calculation the current data in the Bucket or Buffer  */
	ldTempBuffer = dATM_BUFFERCOUNT - (pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime - dATM_LASTCONFORMANCETIME);

	/* Check for valid buffer data */
	if(ldTempBuffer < 0.0)
    {   ldTempBuffer=0.0;	// Set the current buffer size = 0.
		dATM_BUFFERCOUNT = dATM_INCREMENT;	// Increase the buffer value by increment.
		dATM_LASTCONFORMANCETIME = pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime;	/* This cell conforms, so
																 * last conformance time =
																 * Current cell arrival time
																 */
		nATM_CONFORMANCESTATUS = 1; //1 indicates that current cell conforms
		return 1;
	}
    else if(ldTempBuffer>dATM_DELAYLIMIT)
	{
    	//Current buffer size is greater than maximum delay, so cell does not conform
        nATM_NONCONFORMEDCELLCOUNT++;	//Increase the number of cells not conformed by 1.
        nATM_CONFORMANCESTATUS = 0; //0 indicates that current cell does not conformed.
		return 1;
	} //End of else if

    else
	{
    	//Current buffer size is less that delay limit, so cell conforms
		dATM_BUFFERCOUNT = ldTempBuffer + dATM_INCREMENT;	//Increase the buffer size by increment
		dATM_LASTCONFORMANCETIME = pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime;  /* This cell conforms, so
																 * Last conformance time =
																 * Current cell arrival time
																 */
		nATM_CONFORMANCESTATUS = 1; //1 indicates that current cell conforms
		 return 1;
    }//End of else
}//End of CSLBA function


#endif /* CSLBA_H_ */
