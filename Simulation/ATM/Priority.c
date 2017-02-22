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

#ifndef PRIORITY_H_
#define PRIORITY_H_
# ifndef ATM_H_
# include "ATM.h"
# endif

_declspec(dllexport) int m_fnPriority(ATM_DATASTRUCTURE** pstruATMData)
{
	ATMDATA* pstruTempData;//Temp data
	int i=0;//Loop counter
	ATMDATA* pstruReturnData; //Return data in case of GET operation, NULL in case of Insert operation
	int nNoOfPriority = 0;//Number of priority
	char pszDummyPriority[10]; //Dummy priority list
	char pszStr[5];
	ATMDATA* pstruInsertData;
	ATMDATA** pstruData;
	pstruData = pstruATM_DATAARRAY;
	pstruInsertData = pstruATM_CURRENTDATA;
	switch((*pstruATMData)->nType) //Perform the operation
	{
	case INSERT: //Insert operation
		//Calculate the number of priority
		while(pszATM_PRIORITYSTRING[i]!='\0')
		{
			if(pszATM_PRIORITYSTRING[i]=='?') nNoOfPriority++;
			i++;
		}
		//Copy priority string to dummy priority
		strcpy(pszDummyPriority,pszATM_PRIORITYSTRING);

		//Loop through each priority type
		for(i=0;i<nNoOfPriority;i++)
		{
			if(i==0)
				strcpy(pszStr,strtok(pszDummyPriority,"?"));
			else
				strcpy(pszStr,strtok(NULL,"?"));
			//Check for priority type & the packet priority type
			if(atoi(pszStr)==pstruInsertData->n_NI_PacketPriority)
			{//Match
				pstruTempData = pstruData[i];

				//Insert data in the list
				//If only one data
				if(pstruTempData ==NULL)
				{
					pstruData[i] = pstruInsertData;
					pstruTempData = pstruData[i];
					pstruATM_CURRENTDATA = NULL;
					return 1;
				}
				//More than one data
				while(pstruTempData->pstru_NextData !=NULL)
				{
					//Move toward next data
					pstruTempData = pstruTempData->pstru_NextData;
				}
				//Insert data in the list
				pstruTempData->pstru_NextData = pstruInsertData;

				pstruATM_CURRENTDATA = NULL;
				return 1;
			}
		}
		break;
	case GET: //Get operation
		{
			//Calculate the number of priority
			while(pszATM_PRIORITYSTRING[i]!='\0')
			{
				if(pszATM_PRIORITYSTRING[i]=='?') nNoOfPriority++;
				i++;
			}
			//Loop through each priority
			for(i=0;i<nNoOfPriority;i++)
			{
				//If there is any data lin the list
				if(pstruData[i]!=NULL)
				{
					//Return head data
					pstruTempData = pstruData[i];
					pstruReturnData = pstruTempData;
					//Make head data of the list = next data
					pstruData[i] = pstruData[i]->pstru_NextData;
					pstruReturnData->pstru_NextData = NULL;

					pstruATM_CURRENTDATA = pstruReturnData;
					return 1;
				}
			}
		break;
		}
	}
	//return NULL;
	pstruATM_CURRENTDATA = NULL;
	return 1;
}


#endif /* PRIORITY_H_ */
