/************************************************************************************
* Copyright (C) 2013                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "Mobility.h"
static unsigned int nCallBackCount=0;
/** Function to configure the mobility model for all the devices */
_declspec(dllexport) int fn_NetSim_Mobility_Configure(void** var)
{
	void* xmlNetSimNode=var[2];
	NETSIM_ID nDeviceId=*((NETSIM_ID*)var[3]);
	NETSIM_ID nInterfaceId = *((NETSIM_ID*)var[4]);
	LAYER_TYPE nLayerType = *((LAYER_TYPE*)var[5]);
	char* szVal;
	MOBILITY_VAR* pstruMobilityVar=(MOBILITY_VAR*)calloc(1,sizeof* pstruMobilityVar);
	NETWORK=var[1];
	NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->pstruMobVar = pstruMobilityVar;
	//Configure the Model
	getXmlVar(&szVal,MODEL,xmlNetSimNode,1,STRING,Mobility);

	if(!strcmp(szVal,"NO_MOBILITY"))
	{
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_NOMOBILITY;
		return 0;
	}
	else if(!strcmp(szVal,"RANDOM_WALK"))
	{
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_RANDOMWALK;
	}
	else if(!strcmp(szVal,"RANDOM_WAY_POINT"))
	{
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_RANDOMWAYPOINT;
	}
	else if(!strcmp(szVal,"FILE_BASED_MOBILITY"))
	{
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_FILEBASEDMOBILITY;
	}
	else if(!strcmp(szVal,"GROUP_MOBILITY"))
	{
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_GROUP;
	}
	else
	{
		xmlError("Unknown mobility model",szVal,xmlNetSimNode);
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_NOMOBILITY;
		return -1;
	}

	if(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
	{
		//IF node has FileBasedMobility
		pstruMobilityVar->dLastTime = 0;
		return 1;
	}
	//Get the velocity
	szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"VELOCITY",1);
	pstruMobilityVar->dVelocity = atof(szVal);
	NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->dAvgSpeed = atof(szVal);
	free(szVal);
	switch(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType)
	{
	case MobilityModel_RANDOMWAYPOINT:
		{
			//Get the pause time
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"PAUSE_TIME",1);
			pstruMobilityVar->dPauseTime = atof(szVal);
			pstruMobilityVar->dLastTime = -1000000*atof(szVal);
			free(szVal);
		}
		break;
	case MobilityModel_GROUP:
		{
			getXmlVar(&pstruMobilityVar->nGroupId,GROUP_ID,xmlNetSimNode,1,INT,Mobility);
			add_to_group(pstruMobilityVar->nGroupId,nDeviceId);
		}
		break;
	}

	//Save the seed value
	pstruMobilityVar->ulSeed1 = DEVICE(nDeviceId)->ulSeed[0];
	pstruMobilityVar->ulSeed2 = DEVICE(nDeviceId)->ulSeed[1];
	return 1;
}
/** Function to initialize the parameters of positions for all nodes*/
_declspec(dllexport) int fn_NetSim_Mobility_Init(struct stru_NetSim_Network *NETWORK_Formal,
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,
	char *pszAppPath_Formal,
	char *pszWritePath_Formal,
	int nVersion_Type,
	void **fnPointer)
{
	NETSIM_ID nLoop;
	NETWORK = NETWORK_Formal;		//Get the Network structure from NetworkStack	
	pstruEventDetails=pstruEventDetails_Formal;	//Get the Eventdetails from NetworkStack	
	pszAppPath=pszAppPath_Formal;	//Get the application path from NetworkStack	
	pszIOPath=pszWritePath_Formal;	//Get the write path from NetworkStack	
	nVersion_Type = nVersion_Type; //Get the version type from NetworkStack

	srand(1); //Initiallize seed as 1 for same result for every run

	if(nVersion_Type/10 != VERSION)
	{
		printf("Mobility---Version number mismatch\nDll Version=%d\nNetSim Version=%d\nFileName=%s\nLine=%d\n",VERSION,nVersion_Type/10,__FILE__,__LINE__);
		exit(0);
	}

	{
		char* temp=NULL;
		temp = getenv("NETSIM_SIM_AREA_X");
		if(temp)
		{
			if(atoi(temp))
				dSimulationArea_X = atoi(temp);
			else
				dSimulationArea_X = 0xFFFFFFFF;
		}
		else
			dSimulationArea_X = 0xFFFFFFFF;
		temp=NULL;
		temp = getenv("NETSIM_SIM_AREA_Y");
		if(temp)
		{
			if(atoi(temp))
				dSimulationArea_Y = atoi(temp);
			else
				dSimulationArea_Y = 0xFFFFFFFF;
		}
		else
			dSimulationArea_Y = 0xFFFFFFFF;
	}
	for(nLoop=0;nLoop<NETWORK->nDeviceCount;nLoop++)
	{
		if(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility)
		{
			MOBILITY_VAR* var = (MOBILITY_VAR*)NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruMobVar;
			var->ulSeed1 = NETWORK->ppstruDeviceList[nLoop]->ulSeed[0];
			var->ulSeed2 = NETWORK->ppstruDeviceList[nLoop]->ulSeed[1];
			switch(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->nMobilityType)
			{
			case MobilityModel_NOMOBILITY:
				//No action
				break;
			case MobilityModel_FILEBASEDMOBILITY:
				{
					//IF node has FileBasedMobility
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					FileBasedMobilityInitializeNodePositions(NETWORK->nDeviceCount, nLoop);
					pstruEventDetails->dEventTime = 0;
					pstruEventDetails->dPacketSize = 0;
					pstruEventDetails->nApplicationId = 0;
					pstruEventDetails->nDeviceId = nLoop+1;
					pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nLoop]->nDeviceType;
					pstruEventDetails->nEventType = TIMER_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nPacketId = 0;
					pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->pPacket = NULL;
					fnpAddEvent(pstruEventDetails);
				}
				break;
			case MobilityModel_RANDOMWALK:
			case MobilityModel_RANDOMWAYPOINT:
				{
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					pstruEventDetails->dEventTime = 0;
					pstruEventDetails->dPacketSize = 0;
					pstruEventDetails->nApplicationId = 0;
					pstruEventDetails->nDeviceId = nLoop+1;
					pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nLoop]->nDeviceType;
					pstruEventDetails->nEventType = TIMER_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nPacketId = 0;
					pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->pPacket = NULL;
					fnpAddEvent(pstruEventDetails);
				}
				break;
			case MobilityModel_GROUP:
				{
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);	
				}
				break;
			default:
				{
					fnNetSimError("Unknown mobility model %d in fn_NetSim_Mobility_Init\n",NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->nMobilityType);
				}
				break;
			} //End switch(mobility_type)
		}// End if(device_mobility)
	}// End for(loop_all_device)

	fn_NetSim_Mobility_Group_init();
	return 1;
}
/** This function is to free the memory space allocated for the variables that are used in mobily */
_declspec(dllexport) int fn_NetSim_Mobility_Finish()
{

	NETSIM_ID loop;

	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility)
		{
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruMobVar)
				free((MOBILITY_VAR*)NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruMobVar);
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruCurrentPosition)
				free(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruCurrentPosition);
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruNextPosition)
				free(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruNextPosition);
		}
	}
	FileBasedMobilityPointersFree();
	free(fnMobilityCallBack);
	return 1;
}
/** This function is used to change the positions of the devices over simualation. At the end of this function cummulativereceivedpower[][] will be updated. */
_declspec(dllexport) int fn_NetSim_Mobility_Run()
{
	switch(pstruEventDetails->nSubEventType)
	{
	case MOVE_GROUP:
		fn_NetSim_MoveGroup();
		break;
	default:
		{
			unsigned int nLoop;
			double X = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->X;
			double Y = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->Y;
			double vel = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->dAvgSpeed;
			MOBILITY_VAR* pstruMobilityVar=(MOBILITY_VAR*)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruMobVar;
			double dPresentTime = pstruMobilityVar->dLastTime;
			if(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility && NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
			{
				//IF node has FileBasedMobility
				if(dPresentTime == 0)
				{
					//Animate the nodes for initial positions
					fn_NetSim_Anim_MoveNode(pstruEventDetails->nDeviceId,
						(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->X,
						(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->Y,
						pstruEventDetails->dEventTime);
				}

				FileBasedMobility(pstruEventDetails->nDeviceId, dPresentTime);

			}
			else
			{
				memcpy(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruNextPosition,sizeof* NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition);
				if(pstruMobilityVar->dLastTime+pstruMobilityVar->dPauseTime*1000000<pstruEventDetails->dEventTime+1000000)
				{
					fn_NMo_RandomPoint(&X,&Y,vel,&pstruMobilityVar->ulSeed1,&pstruMobilityVar->ulSeed2);
					while(X > dSimulationArea_X || X < 0 || Y < 0 || Y > dSimulationArea_Y)
					{
						X = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->X;
						Y = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->Y;
						fn_NMo_RandomPoint(&X,&Y,vel,&pstruMobilityVar->ulSeed1,&pstruMobilityVar->ulSeed2);
					}
					NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruNextPosition->X = X;
					NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruNextPosition->Y = Y;
					//store the last time
					pstruMobilityVar->dLastTime = pstruEventDetails->dEventTime+1000000;
				}
				//update the device position
				memcpy(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition,NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition,sizeof* NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition);
				fn_NetSim_Anim_MoveNode(pstruEventDetails->nDeviceId,
					(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->X,
					(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->Y,
					pstruEventDetails->dEventTime);
				//Add event for next point 
				pstruEventDetails->dEventTime+=1000000;
				fnpAddEvent(pstruEventDetails);
				pstruEventDetails->dEventTime-=1000000;
			}

			//call all the callback function
			for(nLoop=0;nLoop<nCallBackCount;nLoop++)
			{
				fnMobilityCallBack[nLoop](pstruEventDetails->nDeviceId);
			}
		}
		break;
	}
	return 1;
};

_declspec(dllexport) char* fn_NetSim_Mobility_Trace(NETSIM_ID id)
{
	switch(id)
	{
	case MOVE_GROUP:
		return "MOVE_GROUP";
		break;
	}
	return "";
};
_declspec(dllexport) int fn_NetSim_Mobility_FreePacket()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_CopyPacket()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_Metrics()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_ConfigurePrimitives()
{
	return 1;
};
_declspec(dllexport) char* fn_NetSim_Mobility_ConfigPacketTrace()
{
	return "";
};
_declspec(dllexport) char* fn_NetSim_Mobility_WritePacketTrace()
{
	return "";
};
/** This function is used to genreate the random point */
int fn_NMo_RandomPoint(double* X, double* Y,double velocity,unsigned long *pulSeed1, unsigned long *pulSeed2)
{
	int min;
	int max;
	int ldRandNo;

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1,pulSeed2);

	min = (int)(*X- ldRandNo % ((int)velocity +1));

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1,pulSeed2);

	max = (int)(*X+ ldRandNo % ((int)velocity +1));

	if(min > max)
	{
		*X = max +(int)((min-max+1)*rand()/(RAND_MAX+1.0));
	}
	else
	{
		*X = min + (int)((max-min+1)*rand()/(RAND_MAX+1.0));
	}

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1,pulSeed2);

	min = (int)(*Y- ldRandNo % ((int)velocity +1));

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1,pulSeed2);

	max = (int)(*Y+ ldRandNo % ((int)velocity +1));

	if(min > max)
	{
		*Y = max +(int)((min-max+1)*rand()/(RAND_MAX+1.0));
	}
	else
	{
		*Y = min + (int)((max-min+1)*rand()/(RAND_MAX+1.0));
	}
	return 1;
}

_declspec(dllexport) int fnMobilityRegisterCallBackFunction(_fnMobilityCallBack fnCallBack)
{
	if(!nCallBackCount)
	{
		fnMobilityCallBack = (_fnMobilityCallBack*)calloc(1,sizeof* fnMobilityCallBack);
	}
	else
	{
		fnMobilityCallBack = (_fnMobilityCallBack*)realloc(fnMobilityCallBack,(nCallBackCount+1)*sizeof* fnMobilityCallBack);
	}
	fnMobilityCallBack[nCallBackCount] = fnCallBack;
	nCallBackCount++;
	return 0;
}
