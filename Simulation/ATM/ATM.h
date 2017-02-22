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

# include <stdio.h>
# include <stdlib.h>
# include <windows.h>
# include <string.h>
# include <math.h>

#ifndef ATM_H_
#define ATM_H_
#define _CRTDBG_MAP_ALLOC
//Operation type declaration. Used in Scheduling function
# ifndef INSERT
#define INSERT 1
# endif
# ifndef GET
#define GET 2
# endif

/*Max priority declaration. Used in Scheduling technique. Where, each
 * priority type have different data structure.
 */
# ifndef MAXPRIORITY
# define MAXPRIORITY 2
# endif

# ifndef NODE
# define NODE 1
# endif

# ifndef SWITCH
# define SWITCH 2
#endif
typedef void* (*allocateMem)(unsigned int count, unsigned int size);
typedef void (*freeMem)(void* memory);
/* This data structure define the ATM cell structure*/
typedef struct stru_NetSim_ATMUserCell
{
	int n_NI_CellPayload;	 // Size of the actual data in bytes
	int n_NI_CellOverhead;	 // Size of the header bytes added to the cell to form a 53 bytes of cell. Overhead = (53 - payload).
	int n_NI_Cellflag;		 /* Cell flag value
							  * -1 for starting cell of any packet
							  * 0 for intermediate cell of any packet
							  * 1 for last cell of any packet
							  */
	int n_NI_CellPadd;		 // Cell padding value
	int n_NI_CellSequenceNo; //	Cell No
	int n_NI_CellPriority;	 // Cell priority value
	int n_NI_CellLossPriority;//Cell loss priority value
	double ld_CellSize;	// Cell Size.
	double ld_NI_CellTAT;	// Theoretical Arrival Time. It is used for GCRA algorithm
	double ld_NI_CellArrivalTime; //Actual cell arrival time
	struct stru_NetSim_ATMUserCell *pstru_NI_NextCells; //Pointer to next cell

}USERCELL;

/* The following structure defines the Data packet structure used in the ATM primitives code */
typedef struct stru_NetSim_UserData
{
	int nDataType;	// Data type: Control, voice or data
	double dArrivalTime; // Arrival time of the packet (in microsec)
	USERCELL* pstruATMUserCell; // Cell structure
}USERDATA;

/* The following structure defines the packet structure used in ATM primitives and also provides linkage between NetSim and user code */
typedef struct stru_NetSim_ATMData
{
	void* Var1[5]; //used for linkage
	int Var2[5]; // Used for linkage
	int n_NI_PacketPriority; // Priority of the packet. either 1 or 2.
	struct stru_NetSim_ATMData* pstru_NextData; //Pointer to next data
}ATMDATA;

/* The following structure defines the user structure. Here the user can write their own structure. */
typedef struct stru_NetSim_ATM_UserStructure
{
	int nUserId;
}USERSTRUCTURE;
/* The following data structure defines the ATM structure used in ATM primitives. */
typedef struct stru_NetSim_ATM
{
	double dCurrentSimulationTime;
	int nDeviceType; // Current device type. Node or ATM switch
	int nDeviceId;   // Current device id.
	USERCELL* pstru_NI_CurrentCell; // Currently used ATM cell
	USERCELL* pstru_NI_ReseembleCell; // Reassemple cell of a particular device
	int n_NI_CurrentCellPosition; // Cell position in a packet
	double dPacketsize; // Size of the packet
	int nNoOfCell; // Total number of cell
	int nNoOfCellNotConformed; // Number of non comformed cell
	double ldIncrement; // Increment. Used in GCRA algorithm
	double ldDelayLimit; // Delay limit. Used in GCRA algorithm
	int nCellConformanceStatus; // Status of cell. Used in GCRA algorithm
	double ldLCT; // Last conformance time. Used in CSLBA
	double ldLBCounter; // Buffer count. Used in CSLBA
	char* szPriority; // Priority string. Used in scheduling technique
	int nType; // Operation type. Inssert or get. used in scheduling technique.
	USERDATA* pstruHeadData; // Head data
	ATMDATA* pstruATMDATA[MAXPRIORITY]; // Data list
	ATMDATA* pstruCurrentATMData; // Current ATM data
	USERSTRUCTURE* pstruUserDataStructure;
	allocateMem allocateMemory;
	freeMem freeMemory;
}ATM_DATASTRUCTURE;

ATM_DATASTRUCTURE* pstruATMDataStructure;

// Macro declaration for pointers
# define pstruATM_REASSEMBLECELL (*pstruATMData)->pstru_NI_ReseembleCell
# define pstruATM_CURRENTCELL (*pstruATMData)->pstru_NI_CurrentCell
# define dATM_INCREMENT (*pstruATMData)->ldIncrement
# define nATM_CONFORMANCESTATUS (*pstruATMData)->nCellConformanceStatus
# define dATM_DELAYLIMIT (*pstruATMData)->ldDelayLimit
# define dATM_LASTCONFORMANCETIME (*pstruATMData)->ldLCT
# define dATM_BUFFERCOUNT (*pstruATMData)->ldLBCounter
# define nATM_NONCONFORMEDCELLCOUNT (*pstruATMData)->nNoOfCellNotConformed
# define nATM_OPERATIONTYPE (*pstruATMData)->nType
# define pstruATM_CURRENTDATA (*pstruATMData)->pstruCurrentATMData
# define pstruATM_DATAARRAY (*pstruATMData)->pstruATMDATA
# define pszATM_PRIORITYSTRING (*pstruATMData)->szPriority
#ifndef _ATM_CODE
#define malloc(size) (*pstruATMData)->allocateMemory(1,size)
#define calloc(count,size) (*pstruATMData)->allocateMemory(count,size)
#define free(memory) (*pstruATMData)->freeMemory(memory)
#endif


#endif /* ATM_H_ */
