//LIB (Label Information Base) Creation

 


 

#include<stdio.h>

#include<stdlib.h>

#include<String.h>

#define MAXIP 50

 

/* This structure is for LIB of router

 * It has FEC, next hop and label number elements

 */

typedef struct LabelInformationBase

{

            char lnForwardingEquivalenceClass[MAXIP];

            char lnNextHop[MAXIP];

    int nLabelNumber;

    struct LabelInformationBase *pstruNextentry;

}LIB;

 

/* This function is used add the entries in LIB*/

_declspec(dllexport)int fnNPMPLSLIBFormation(char lnFEC[MAXIP],char lnNextHop[MAXIP],int nIncomingLabel,LIB **pstruMPLSLIB,LIB **pstruMPLSReferenceLIB)

{

            // Create temporary object for LIB structure

            LIB *pstruTempLIB;

            // Allocate memory for that object

            pstruTempLIB = (LIB*)malloc(sizeof(LIB));

            // Assign FEC

            strcpy(pstruTempLIB->lnForwardingEquivalenceClass,lnFEC);

            // Assign next hop

            strcpy(pstruTempLIB->lnNextHop,lnNextHop);

            // Assign Incoming label

            pstruTempLIB->nLabelNumber = nIncomingLabel;

            pstruTempLIB->pstruNextentry = NULL;

 

            // Assign the temporary object in the LIB of current router

            if(*pstruMPLSLIB == NULL)

            {

                        *pstruMPLSLIB = pstruTempLIB;

                        *pstruMPLSReferenceLIB = pstruTempLIB;

            }

            else

            {

                        (*pstruMPLSReferenceLIB)->pstruNextentry = pstruTempLIB;

                        *pstruMPLSReferenceLIB= pstruTempLIB;

            }

            return 0;

}

 
