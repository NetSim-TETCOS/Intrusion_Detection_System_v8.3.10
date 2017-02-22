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

#ifndef ROUNDROBIN_H_
#define ROUNDROBIN_H_
# ifndef ATM_H_
# include "ATM.h"
# endif

_declspec(dllexport) int m_fnRoundRobin(ATM_DATASTRUCTURE** pstruATMData)
{
	ATMDATA* pstruTempData; //Temporary data
	int i=0; //Loop counter
	ATMDATA* pstruReturnData; //Return data. In case of insert, this is NULL
	int nNoOfPriority = 0;//Number of priority
	char pszDummyPriority[10]; //Dummy pririty list
	char pszStr[5];
	static int nLastDataType = -1; //Static variable indicating that the last get operation happens in which list
	ATMDATA* pstruInsertData;
	ATMDATA** pstruData;
	pstruData = pstruATM_DATAARRAY;
	pstruInsertData = pstruATM_CURRENTDATA;
	switch(nATM_OPERATIONTYPE) //Swith for operation
	{
	case INSERT: //Insert operation
		{
			//calculating the number of priority
			while(pszATM_PRIORITYSTRING[i]!='\0')
			{
				if(pszATM_PRIORITYSTRING[i]=='?') nNoOfPriority++;
				i++;
			}
			strcpy(pszDummyPriority,pszATM_PRIORITYSTRING);

			//Loop through each priority
			for(i=0;i<nNoOfPriority;i++)
			{
				if(i==0)
					strcpy(pszStr,strtok(pszDummyPriority,"?"));
				else
					strcpy(pszStr,strtok(NULL,"?"));

				//If priority values and packet priority match then insert the data
				if(atoi(pszStr)==pstruInsertData->n_NI_PacketPriority)
				{
					//Inserting data at the end of the list
					pstruTempData = pstruData[i];
					//If there is no data in the list
					if(pstruTempData ==NULL)
					{
						pstruData[i] = pstruInsertData;
						pstruTempData = pstruData[i];
						pstruATM_CURRENTDATA = NULL;
						return 1;
					}

					//More that one data present
					//Transverse through the list
					while(pstruTempData->pstru_NextData !=NULL)
					{
						pstruTempData = pstruTempData->pstru_NextData;
					}
					//Insert data at the end of list
					pstruTempData->pstru_NextData = pstruInsertData;
					pstruATM_CURRENTDATA = NULL;
					return 1;
				}
			}
			break;
		}
	case GET://Get operation
		{
			//Calculate the number of priority
			while(pszATM_PRIORITYSTRING[i]!='\0')
			{
				if(pszATM_PRIORITYSTRING[i]=='?') nNoOfPriority++;
				i++;
			}
			//Loop till data will found
			while(1)
			{
				//Check for data list
				if(pstruData[nLastDataType+1]!=NULL)
				{
					//Get the head data
					pstruTempData = pstruData[nLastDataType+1];
					pstruReturnData = pstruTempData;
					pstruData[nLastDataType+1] = pstruData[nLastDataType+1]->pstru_NextData;
					pstruReturnData->pstru_NextData = NULL;
					nLastDataType++;
					if(nLastDataType == nNoOfPriority-1) nLastDataType = -1;

					pstruATM_CURRENTDATA = pstruReturnData;
					return 1;

				}
				else
				{//If NULL
					//Move towards next list in circular order
					nLastDataType++;
					if(nLastDataType == nNoOfPriority-1) nLastDataType = -1;
				}
			}
			break;
		}
	}
	//return NULL;
	pstruATM_CURRENTDATA = NULL;
	return 1;
}

#endif /* ROUNDROBIN_H_ */
