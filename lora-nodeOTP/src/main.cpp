#include "mbed.h"
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"
#include "comissioning.h"


/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON                    true

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1

#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        true

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 ) 

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT                            15

/*!
 * User application data buffer size
 */
#if ( LORAWAN_CONFIRMED_MSG_ON == 1 )
#define LORAWAN_APP_DATA_SIZE                       6

#else
#define LORAWAN_APP_DATA_SIZE                       1

#endif


















/* Callback functions for LoRaWAN-lib */
static void McpsConfirm( McpsConfirm_t *McpsConfirm );
static void McpsIndication( McpsIndication_t *McpsIndication );
static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm );

static int OnTxNextPacketTimerEvent( void );
/*
 * Configuration variables for LoRaWAN-lib
 */
LoRaMacPrimitives_t LoRaMacPrimitives;
LoRaMacCallback_t LoRaMacCallbacks;
MibRequestConfirm_t mibReq;
LoRaMacStatus_t status;
static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

/* Debugging methods */
//Indication led, indicates if joined a network
DigitalOut ledConnected(LED1);
//Serial state indication
Serial debugSerial(USBTX, USBRX);


//Control function of program
int main( void ){
    LoRaMacPrimitives_t LoRaMacPrimitives;
    LoRaMacCallback_t LoRaMacCallbacks;
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;
    debugSerial.printf("START: Starting lora node\n");
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
        debugSerial.printf("LoRaMAC: Initialization successful\n");
        // Initialization successful
    }else{
        debugSerial.printf("LoRaMAC: Initialization FAILED\n");
        return -1;
    }
    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
    LoRaMacMibSetRequestConfirm( &mibReq );

    LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );

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
        debugSerial.printf("LoRaMAC: Join request was send successfully\n");
    } else {
        debugSerial.printf("LoRaMAC: Join request send FAILED\n");
        return -1;
    }
    Timer t;
    //Processing loop
    while(1)
    {
        t.start();
        while(t.read() < 4){

        }
        t.stop();
        debugSerial.printf("Waited for seconds: %f\n", t.read());
        t.reset();
        //Waiting for requests
        if(!OnTxNextPacketTimerEvent()){
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
                debugSerial.printf("LoRaMAC: Join request was send successfully\n");
            } else {
                debugSerial.printf("LoRaMAC: Join request send FAILED\n");
                
            }
        }
    }
    return 0;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static int OnTxNextPacketTimerEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.IsNetworkJoined == true )
        {
            
            debugSerial.printf("OnTxNextPacketTimerEvent: already joined\n");
            return 1;
        }
        else
        {
            debugSerial.printf("OnTxNextPacketTimerEvent: next try to join!\n");
            return 0;
        }
    }
    return 0;
}


static void McpsConfirm( McpsConfirm_t *McpsConfirm )
{
    debugSerial.printf("McpsConfirm: Status: %i; request: %i\n", McpsConfirm->Status, McpsConfirm->McpsRequest);
    // Implementation of the MCPS-Confirm primitive
    if( McpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        debugSerial.printf("McpsConfirm: status NOT OK!\n");
        return;
    }
    switch( McpsConfirm->McpsRequest ){
        case MCPS_UNCONFIRMED:
        {
            
            break;
        }
        case MCPS_CONFIRMED:
        {
            
            //McpsConfirm->AckReceived;
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        default:
            break;
    }

}

static void McpsIndication( McpsIndication_t *McpsIndication )
{
    debugSerial.printf("McpsIndication: Status: %i; request: %i\n", McpsIndication->Status, McpsIndication->McpsIndication);
    // Implementation of the MCPS-Indication primitive
    if( McpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ){
        debugSerial.printf("McpsIndication: status NOT OK!\n");
        return;
    }

    switch( McpsIndication->McpsIndication ){
        case MCPS_UNCONFIRMED:
        {
            break;
        }
        case MCPS_CONFIRMED:
        {
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }
  switch( McpsIndication->Port ){
    case 1:
        debugSerial.printf("McpsIndication: indication on port 1\n");
        break;
    default:
        debugSerial.printf("McpsIndication: indication on UNKNOWN port\n");
        break;
  }
}

static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm )
{
    debugSerial.printf("MlmeConfirm: Status: %i; request: %i\n", MlmeConfirm->Status, MlmeConfirm->MlmeRequest);
  // Implementation of the MLME-Confirm primitive
  if( MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( MlmeConfirm->MlmeRequest )
        {
            case MLME_JOIN:
            {
                // Status is OK, node has joined the network
                ledConnected = 1;
                debugSerial.printf("LoRaMAC: Node successfully joined network\n");
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