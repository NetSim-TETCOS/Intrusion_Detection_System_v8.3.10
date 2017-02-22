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

#ifndef FIFO_H_
#define FIFO_H_
# ifndef ATM_H_
# include "ATM.h"
# endif


_declspec(dllexport) int m_fnFIFO(ATM_DATASTRUCTURE** pstruATMData)
{
	ATMDATA* pstruTempData; //Temporary data
	ATMDATA* pstruReturnData; //Return data
	ATMDATA* pstruInsertData; //Inserted data
	switch(nATM_OPERATIONTYPE)
	{
	case INSERT:
		pstruInsertData = pstruATM_CURRENTDATA; //Get the
		pstruTempData = pstruATM_DATAARRAY[0]; //Initializing temp data
		if(pstruTempData ==NULL) //Checking for there is any data in temp data list
		{
			pstruATM_DATAARRAY[0] = pstruInsertData; //First data to assign directly
			pstruTempData = pstruATM_DATAARRAY[0];
			return 1; //Return NULL in case of insert operation
		}
		//There is data in the data list
		//Moving to the end of the list
		while(pstruTempData->pstru_NextData !=NULL)
		{
			pstruTempData = pstruTempData->pstru_NextData;
		}
		//Inserting data
		pstruTempData->pstru_NextData = pstruInsertData;
		return 1; //Return NULL in case of insert operation
		break;
	case GET:
		{
			//In case of get operation return head data from the list
			pstruTempData = pstruATM_DATAARRAY[0];
			pstruReturnData = pstruTempData;

			//Move head data to next data
			pstruATM_DATAARRAY[0] = pstruATM_DATAARRAY[0]->pstru_NextData;
		//	pstruTempData = pstruTempData->pstruNext;
			pstruReturnData->pstru_NextData = NULL;
			pstruATM_CURRENTDATA = pstruReturnData;
			return 1; //Return head data
		break;
		}
	}
	return 0;
}


#endif /* FIFO_H_ */
