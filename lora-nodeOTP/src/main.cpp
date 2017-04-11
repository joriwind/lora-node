#include "mbed.h"
#include "board.h"
#include "LoRaMac.h"
#include "comissioning.h"

/* Callback functions for LoRaWAN-lib */
static void McpsConfirm( McpsConfirm_t *McpsConfirm );
static void McpsIndication( McpsIndication_t *McpsIndication );
static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm );

/*
 * Configuration variables for LoRaWAN-lib
 */
LoRaMacPrimitives_t LoRaMacPrimitives;
LoRaMacCallback_t LoRaMacCallbacks;
LoRaMacStatus_t Status;


static uint8_t DevEui[] = LORAWAN_DEV_EUI;
static uint8_t AppEui[] = LORAWAN_APP_EUI;
static uint8_t AppKey[] = LORAWAN_APP_KEY;

//Control function of program
int main( void ){
    MlmeReq_t mlmeReq;
    LoRaMacStatus_t status;

    //LoRaWAN MAC layer initialisation
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
    Status = LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks );
    if( Status == LORAMAC_STATUS_OK )
    {
        // Initialization successful
    }

    //LoRaWAN over-the-air activation
    // Setup the request type
    mlmeReq.Type = MLME_JOIN;
    // Fill the join parameters
    mlmeReq.Req.Join.DevEui = DevEui;
    mlmeReq.Req.Join.AppEui = AppEui;
    mlmeReq.Req.Join.AppKey = AppKey;
    // Add the number of trials for the join request
    //mlmeReq.Req.Join.NbTrials = 3; //Not available variable in struct MlmeReq_t
    status = LoRaMacMlmeRequest( &mlmeReq );
    if( status == LORAMAC_STATUS_OK )
    {
        // Join request was send successfully
    }
}


static void McpsConfirm( McpsConfirm_t *McpsConfirm )
{
  // Implementation of the MCPS-Confirm primitive
}

static void McpsIndication( McpsIndication_t *McpsIndication )
{
  // Implementation of the MCPS-Indication primitive
}

static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm )
{
  // Implementation of the MLME-Confirm primitive
  if( MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( MlmeConfirm->MlmeRequest )
        {
            case MLME_JOIN:
            {
                // Status is OK, node has joined the network
                break;
            }
            case MLME_LINK_CHECK:
            {
                // Check DemodMargin
                // Check NbGateways
                break;
            }
            default:
                break;
        }
    }
}