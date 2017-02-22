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

#ifndef REASSEMBLY_H_
#define REASSEMBLY_H_
# ifndef ATM_H_
# include "ATM.h"
# endif


/*******  int m_fnAtmAdaptionLayerReassembly(ATM_DATASTRUCTURE**)  ***************
	Purpose: Form reassemble cell list.
			 Called in data link in event of Node and Router.

	Algorithm:

		¢	If reassemble cell is NULL allocate memory for reassemble cell
		¢	Initialize the reassemble cell
		¢	Add atm cell payload , overhead , padding , to the reassemble cell payload , overhead and  padding

  *****************************************************************************************/
_declspec(dllexport) int m_fnAtmAdaptionLayerReassembly(ATM_DATASTRUCTURE** pstruATMData)
{
	if(pstruATM_REASSEMBLECELL == NULL /* Reassemble cell is NULl */\
			|| pstruATM_CURRENTCELL->n_NI_Cellflag == -1/*first cell of a packet arrives*/)
	{

		/************* Initializing reassemble cell **********/

		//Allocate memory for reassemble cell
		pstruATM_REASSEMBLECELL= (USERCELL*)malloc(sizeof(USERCELL));

		// Initialize cell overhead = 0
		pstruATM_REASSEMBLECELL ->n_NI_CellOverhead = 0;

		//Initialize the cell padding = 0
		pstruATM_REASSEMBLECELL -> n_NI_CellPadd = 0;

		//Initialize the cell payload = 0
		pstruATM_REASSEMBLECELL -> n_NI_CellPayload = 0;

		//Next cell = NULL
		pstruATM_REASSEMBLECELL -> pstru_NI_NextCells = NULL;

		/********** Initialization end ***********************/
	}

	while(pstruATM_CURRENTCELL!=NULL) // transverse through the Cell list till last cell encounters
	{
		if(pstruATM_CURRENTCELL->n_NI_Cellflag==0/*Cell flag = 0 means intermediate cell*/ ||\
				pstruATM_CURRENTCELL->n_NI_Cellflag == -1) /*  cell flag == -1 means first cell*/
		{
			//Add cell overhead to reassemble cell overhead, this over head also contains padding.
			pstruATM_REASSEMBLECELL->n_NI_CellOverhead =\
			pstruATM_REASSEMBLECELL->n_NI_CellOverhead +\
											pstruATM_CURRENTCELL->n_NI_CellOverhead +\
											pstruATM_CURRENTCELL->n_NI_CellPadd;

			// Add cell payload to reassemble cell payload
			pstruATM_REASSEMBLECELL->n_NI_CellPayload =\
			pstruATM_REASSEMBLECELL->n_NI_CellPayload +\
										pstruATM_CURRENTCELL->n_NI_CellPayload; // Adding payload

			//Add cell padding to reassemble cell padding
			pstruATM_REASSEMBLECELL->n_NI_CellPadd =\
			pstruATM_REASSEMBLECELL->n_NI_CellPadd +\
										pstruATM_CURRENTCELL->n_NI_CellPadd;  // Adding padding
			//Mark the cell position = 0
			(*pstruATMData)->n_NI_CurrentCellPosition = 0;
			return 1;


		}//end of if(first or intermediate cell)

		if(pstruATM_CURRENTCELL->n_NI_Cellflag==1) // cell flag == 1 for last cell
		{
			//Add cell overhead to reassemble cell overhead. This over head also contain padding
			pstruATM_REASSEMBLECELL->n_NI_CellOverhead =\
			pstruATM_REASSEMBLECELL->n_NI_CellOverhead +\
										pstruATM_CURRENTCELL->n_NI_CellOverhead +\
										pstruATM_CURRENTCELL->n_NI_CellPadd;

			//Adding cell payload to reassemble cell payload
			pstruATM_REASSEMBLECELL->n_NI_CellPayload =\
			pstruATM_REASSEMBLECELL->n_NI_CellPayload +\
										pstruATM_CURRENTCELL->n_NI_CellPayload;

			//Adding padding value to reassemble cell padding
			pstruATM_REASSEMBLECELL->n_NI_CellPadd =\
			pstruATM_REASSEMBLECELL->n_NI_CellPadd +\
										pstruATM_CURRENTCELL->n_NI_CellPadd;

			// set cell position = 1 for last cell
			(*pstruATMData)->n_NI_CurrentCellPosition = 1;
			return 1;

		}//end of if(cell flag == 1)

		//Move reassemble cell to next cell
		pstruATM_REASSEMBLECELL=pstruATM_REASSEMBLECELL->pstru_NI_NextCells;

	}//End of while(transversing through cell list)
	return 1;
}//End of function reassembly


#endif /* REASSEMBLY_H_ */
