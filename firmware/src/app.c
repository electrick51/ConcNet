/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "app.h"
#include "system_config.h"
#include "system/ports/sys_ports.h"
#include "peripheral/devcon/plib_devcon.h"
#include "peripheral/ports/plib_ports.h"
#include "peripheral/int/plib_int.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
typedef  unsigned char uint8_t;

static uint8_t txIndex = 0, rxIndex = 0, count, DeviceAddress;
static uint8_t chksm = 0, cs = 0, csLength = 0;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*
*
*
*   GetChecksum - Calculates the checksum for C4
*
*/
uint8_t GetChecksum(uint8_t data[], uint8_t length)
{
    
    uint8_t cs = 1;  // Seed cs
    uint8_t x;

    for(x = 0; x < length - 1; x++)
    {
         cs += data[x];
    }
         

    return cs;
}


uint8_t pLength; 
uint8_t chks = 0, pchksm = 0;
uint8_t x;
    


/* TODO:  Add any necessary local functions.
*/
//uint8_t data[20];
uint8_t CheckForValidPacket(uint8_t *packet)
{
   pLength = packet[1];
    
    chks = GetChecksum(packet, pLength);
    pLength--;
    pchksm = packet[pLength];
    
    if(chks == pchksm)
    {
        return pLength++;
    }  
    
    return 0;
}



    
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    DRV_TMR0_Start();
    
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

uint8_t length, stopLength;

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
            
//                appData.state = APP_STATE_SERVICE_TASKS;
                appData.state = APP_STATE_RX;
                
                
            }
            break;
        }

//        case APP_STATE_SERVICE_TASKS:
//        {
//        
//              
//            break;
//        }

        /* TODO: implement your application state machine.*/
        case APP_STATE_PRINT_PACKET:
        {
            uint8_t x;
            
            for(x = 0; x < csLength + 1; x++)
            {
                if(!(DRV_USART_TRANSFER_STATUS_TRANSMIT_FULL & DRV_USART1_TransferStatus()) )
                {
                    DRV_USART1_WriteByte(appData.RxBuff[x]);  // send modified byte
    //                appData.state = APP_STATE_RX;           // change state to RX and wait for next received byte
                }
                
            }
            for(x = 0; x < 15; x++)
            {
                if(!(DRV_USART_TRANSFER_STATUS_TRANSMIT_FULL & DRV_USART1_TransferStatus()) )
                {
                    DRV_USART1_WriteByte(0xCC);  // send modified byte

                }
            }
            
            appData.state = APP_STATE_RX;           // change state to RX and wait for next received byte
            rxIndex = 0;
            break;
        }

        case APP_STATE_PROCESS_PACKET:
        {
            csLength = CheckForValidPacket(appData.RxBuff);
            if(csLength > 0)
            {
                appData.state = APP_STATE_PRINT_PACKET;            // change state to TX
            }
            else
            {
                rxIndex = 0;
                appData.state = APP_STATE_RX;
            }
            break;
        }

        
        case APP_STATE_RX:                              // USART receive state
        {   
            // if byte received in USART instance 0 (USART1 in this case)
            if (!DRV_USART0_ReceiverBufferIsEmpty())
            {
               appData.rx_byte = DRV_USART0_ReadByte(); // read received byte
               appData.tx_byte = appData.rx_byte ;   // modifying received byte confirms it was received
//               appData.tx_byteBus = appData.rx_byte;
               if(rxIndex < 30)
               {
                   if(rxIndex > 3)
                   {
                       appData.state = APP_STATE_PROCESS_PACKET;
                    }

                   appData.RxBuff[rxIndex++] = appData.rx_byte;
               }
               else
               {
                   rxIndex = 0;
               }
               
               
            }
            
            if(!DRV_USART1_ReceiverBufferIsEmpty())
            {
                appData.rx_byteBus = DRV_USART1_ReadByte(); // read received byte
               appData.tx_byteBus = appData.rx_byteBus ;   // modifying received byte confirms it was received
               appData.state = APP_STATE_TX;            // change state to TX
            }
            break;
        }
 
        case APP_STATE_TX:                              // USART transmit state
        {
            // make sure the transmit buffer is not full before trying to write byte 
            if(!(DRV_USART_TRANSFER_STATUS_TRANSMIT_FULL & DRV_USART0_TransferStatus()) )
            {
                DRV_USART0_WriteByte(appData.tx_byte);  // send modified byte
                appData.state = APP_STATE_RX;           // change state to RX and wait for next received byte
            }
            
            if(!(DRV_USART_TRANSFER_STATUS_TRANSMIT_FULL & DRV_USART1_TransferStatus()) )
            {
                DRV_USART1_WriteByte(appData.tx_byteBus);  // send modified byte
                appData.state = APP_STATE_RX;           // change state to RX and wait for next received byte
            }
            break;
        }                
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
