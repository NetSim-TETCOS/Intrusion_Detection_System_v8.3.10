/************************************************************************************

 * Copyright (C) 2010                                                               *

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

#ifndef VSA_H_
#define VSA_H_

# ifndef ATM_H_
# include "ATM.h"
# endif

/*****************  int m_fnVirtualSchedulingAlg(ATM_DATASTRUCTURE**) ************
	Purpose: Check weather incoming cell is Conform or NOT
	Algorithm:
		¢	Declare Necessary variable
		¢	Get delay limit
		¢	Get increment
		¢	If incoming cell == first cell Set TAT ( theoretical arrival time )
		¢	Check for cell conformance
		¢	Return 1 if cell Conform
		¢	Return 0 if cell not Conform

 ****************************************************************************************/
_declspec(dllexport) int m_fnVirtualSchedulingAlg(ATM_DATASTRUCTURE** pstruATMData)
{
	USERCELL *pstru_NP_ATMHeadCell; // Head cell of ATM cell list
	/*	Check the incoming packet is first packet or Not
	 *	If it is a first Packet the set Theoretical Arrival Time is set
     *	with the actual arrival time of first Cell
	 */

	pstru_NP_ATMHeadCell=pstruATM_CURRENTCELL; // Setting head cell = current ATM cell


	// Check for cell sequence number
	if(pstru_NP_ATMHeadCell->n_NI_CellSequenceNo==1)
	{
		/*First cell*/
		//Set the theoretical arrival time = Current cell arrival time
		pstru_NP_ATMHeadCell->ld_NI_CellTAT=pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime;
	}//end of if(first cell)

	/*  If Actual Arrival Time is greater then the Theoretical Arrival Time
	 *	then the cell is conformed
	 */
	if(pstru_NP_ATMHeadCell->ld_NI_CellTAT < pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime)
	{
		/* Cell is conformed */

		// Set the theoretical arrival time  = Current cell arrival time
		pstru_NP_ATMHeadCell->ld_NI_CellTAT = pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime;

		//Set the theoretical arrival time of next cell = current TAT + Increment
		if(pstru_NP_ATMHeadCell->pstru_NI_NextCells!=NULL/* Check for Valid cell */)
			pstru_NP_ATMHeadCell->pstru_NI_NextCells->ld_NI_CellTAT = pstru_NP_ATMHeadCell->ld_NI_CellTAT + dATM_INCREMENT;

		nATM_CONFORMANCESTATUS = 1; //indicating that cell is conformed
		return 1;
	} //End of if(cell conformed)


	else if(pstru_NP_ATMHeadCell->ld_NI_CellTAT > pstru_NP_ATMHeadCell->ld_NI_CellArrivalTime + dATM_DELAYLIMIT)
	{
		/*  Cell is Not Conformed. Because TAT is greater than arrival time + Delay limit */
		nATM_NONCONFORMEDCELLCOUNT++;// Increase the number of cell not conformed by 1

	    //Set the next cell TAT = current cell TAT
		if(pstru_NP_ATMHeadCell->pstru_NI_NextCells!=NULL /* Check for Valid cell */)
			pstru_NP_ATMHeadCell->pstru_NI_NextCells->ld_NI_CellTAT = pstru_NP_ATMHeadCell->ld_NI_CellTAT;

		nATM_CONFORMANCESTATUS = 0; //indicating that cell is not conformed.
		return 1;
	}//End of else if(not conformed)

	else
	{
		/* Cell is conformed becuase, TAT falls in between Arrival time and delay limit */
		//Set the next cell TAT = current cell TAT + Increment
		if(pstru_NP_ATMHeadCell->pstru_NI_NextCells!=NULL /* Check for Valid cell */)
			pstru_NP_ATMHeadCell->pstru_NI_NextCells->ld_NI_CellTAT = pstru_NP_ATMHeadCell->ld_NI_CellTAT + dATM_INCREMENT;

		nATM_CONFORMANCESTATUS = 1;	//indicates cell is conformed
		return 1;
	} //End of else(cell conformed)
} //End of function VSA.


#endif /* VSA_H_ */
