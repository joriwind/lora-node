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
MibRequestConfirm_t mibReq;
LoRaMacStatus_t status;
static uint8_t DevEui[] = LORAWAN_DEV_EUI;
static uint8_t AppEui[] = LORAWAN_APP_EUI;
static uint8_t AppKey[] = LORAWAN_APP_KEY;

//Indication led, indicates if joined a network
DigitalOut ledConnected(LED1);


//Control function of program
int main( void ){

    //Board initilization
    BoardInit();
    ledConnected = 0;

    //LoRaWAN MAC layer initialisation
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
    status = LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks );
    if( status == LORAMAC_STATUS_OK )
    {
        // Initialization successful
    }else{
        return -1;
    }
    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
    LoRaMacMibSetRequestConfirm( &mibReq );

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 ) 
    LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
    LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
    LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
    LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
    LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
    LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
    LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );

    mibReq.Type = MIB_RX2_CHANNEL;
    mibReq.Param.Rx2Channel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
    LoRaMacMibSetRequestConfirm( &mibReq );
#endif


    //LoRaWAN over-the-air activation
    MlmeReq_t mlmeReq;
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

    //Processing loop
    while(1)
    {
        //Waiting for requests
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
                ledConnected = 1;
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