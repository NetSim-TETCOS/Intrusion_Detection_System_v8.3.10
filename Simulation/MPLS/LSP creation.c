//LSP (Label Switched Path) creation



 

#include<stdio.h>

#include<stdlib.h>

#include<String.h>

#define MAXIP 50

 

/* This structure is for Label mapping message

 * It has Message type, Message id, FEC, Ingress router id, Label Path vector structure, explicit route structure elements

 */

typedef struct LabelMapping

{

            char szMessageType[20];

            int nMessageId;

            char ForwardEquivalenceClass[MAXIP];

            int nIngressRouterId;

            int nLabel;

            //When label mapping message is received by a router, router adds its id in this path vector structure

            struct PathVectorOfLabelMapping

            {

                           int nPath;

                           struct PathVectorOfLabelMapping *nextentry;

 

            }*pstruPathVector,*pstruReference;

            /* This structure is for the explicit route of mapping message

            * This Explicit structure is used only for CR-LDP

            */

            struct struExplicitRouteOfMapping

             {

                        int nRouterId;

                        struct struExplicitRouteOfMapping *pstrunextentry;

             }*pstruExplicitRoute,*pstruExplicitReference;

}Mapping;

 

/* This function is used to create LSP (Label Switched Path)

 * LSP is created by reversing the path vector value of Label mapping message

 */

_declspec(dllexport)struct PathVectorOfLabelMapping * fnNPMPLSLSPCreation(struct PathVectorOfLabelMapping *pstruPathVector)

{

            // Create temporary objects for PathVectorOfLabelMapping structure

            struct PathVectorOfLabelMapping *pstruTemp,*pstruTemp1;

            struct PathVectorOfLabelMapping *pstruReverse = NULL;

 

            // Get the path vector of label mapping message

            pstruTemp = pstruPathVector;

            // follow the steps until path vector value becomes NULL

            while(pstruTemp != NULL)

            {

                        // Allocate the memory for pstruTemp1 object

                        pstruTemp1 = (struct PathVectorOfLabelMapping *)malloc(sizeof(struct PathVectorOfLabelMapping));

                        // Assign the path value

                        pstruTemp1->nPath = pstruTemp->nPath;

                        if(pstruReverse == NULL)

                        { // First time pstruReverse would be NULL

                                    // Assign the next entry of pstruTemp1 object as NULL

                                    pstruTemp1->nextentry = NULL;

                                    // Assign pstruTemp1 to pstruReverse

                                    pstruReverse = pstruTemp1;

                        }

                        else

                        {

                                    // Assign the next entry of pstruTemp1 object as pstruReverse

                                    pstruTemp1->nextentry = pstruReverse;

                                    // Assign pstruTemp1 to pstruReverse

                                    pstruReverse = pstruTemp1;

                        }

                        // Move to next entry

                        pstruTemp = pstruTemp->nextentry;

            }

            // Return the reversed path ( i.e LSP)

            return pstruReverse;

 

}

 
