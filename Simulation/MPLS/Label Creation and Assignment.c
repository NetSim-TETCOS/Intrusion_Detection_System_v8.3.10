 

//Label Creation and Assignment


 

#include<stdio.h>

#include<stdlib.h>

#include<string.h>

#define MAXIP 50

 

/* This structure is for Label pool of router

 * It has FEc, QoS, Label elements

 */

typedef struct struLabelPool

{

            char lnForwardingEquivalenceClass[MAXIP];

            char lnQos[MAXIP];

            int nLabel;

            struct struLabelPool* pstruNextentry;

}LABELPOOL;

 

unsigned long ul_NP_MPLS_LabelSeed1 = 1391973145;

unsigned long ul_NP_MPLS_LabelSeed2 = 1391973145;

int nRandomNumber;

 

int fn_NP_MPLS_RandomNumber(long,int *, unsigned long *, unsigned long *);

 

/*This function is used to create label and assign the label into Label pool of current router

 * The label creation is implementation specific

 */

_declspec(dllexport)int fnNPMPLSLabelAssigning(char lnFEC[MAXIP],char lnQOS[MAXIP],LABELPOOL **pstruMPLSLabelPool, LABELPOOL **pstruReferenceLabelPool)

{

            // Create temporary object for Label pool structure

            LABELPOOL *pstruTempLabelPool;

            // Allocate memory

            pstruTempLabelPool = (LABELPOOL *)malloc(sizeof(LABELPOOL));

            // Call this function to get a random number

            fn_NP_MPLS_RandomNumber(50,&nRandomNumber,&ul_NP_MPLS_LabelSeed1,&ul_NP_MPLS_LabelSeed2);

            // If the random number is less than 1 than call the function again and again until the random number is greater than 0

            while(nRandomNumber <= 0)

            {

                        fn_NP_MPLS_RandomNumber(50,&nRandomNumber,&ul_NP_MPLS_LabelSeed1,&ul_NP_MPLS_LabelSeed2);

            }

            // Assign the FEC value

            strcpy(pstruTempLabelPool->lnForwardingEquivalenceClass,lnFEC);

            // Assign the Qos value

            strcpy(pstruTempLabelPool->lnQos,lnQOS);

            // Assign the random number

            pstruTempLabelPool->nLabel = nRandomNumber;

            pstruTempLabelPool->pstruNextentry = NULL;

            // Assign the temporary object into the Label pool of current router

            if(*pstruMPLSLabelPool == NULL)

            {

                        *pstruMPLSLabelPool = pstruTempLabelPool;

                        *pstruReferenceLabelPool = pstruTempLabelPool;

            }

            else

            {

                        (*pstruReferenceLabelPool)->pstruNextentry = pstruTempLabelPool;

                        *pstruReferenceLabelPool = pstruTempLabelPool;

            }

            // return the random  number

            return nRandomNumber;

 

}

/* This function is random number generator function*/

 

int fn_NP_MPLS_RandomNumber(long lm,int *ldRandNo,unsigned long *ulseed1,unsigned long *ulseed2)

{

            unsigned long ulTempSeed1;

            unsigned long ulTempSeed2;

            long ldTemp;

            ulTempSeed1= *ulseed1;

            ulTempSeed2 = *ulseed2;

            *ulseed1 = (unsigned long)36969*(unsigned long)((ulTempSeed1 & 65535)+(ulTempSeed1 >> 16));

            *ulseed2 = (unsigned long)30963*(unsigned long)((ulTempSeed2 & 65535)+(ulTempSeed2 >> 16));

            ldTemp = (long)((*ulseed1)<<16)+(long)(*ulseed2);

            *ldRandNo = (int)((ldTemp % (long)(lm+1)));

            return 0;

}

 
