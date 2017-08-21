#include "mbed.h"
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "comissioning.h"

/* Function declarations */
//Callback functions for LoRaWAN-lib 
static void McpsConfirm( McpsConfirm_t *McpsConfirm );
static void McpsIndication( McpsIndication_t *McpsIndication );
static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm );
//Helper functions
static int isNetworkJoined( void );
bool sendFrame( uint8_t port, uint8_t* payload, int size );

/*
 * Configuration variables for LoRaWAN-lib
 */
LoRaMacPrimitives_t gLoRaMacPrimitives;
LoRaMacCallback_t gLoRaMacCallbacks;
static uint8_t gDevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t gAppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t gAppKey[] = LORAWAN_APPLICATION_KEY;

/* Debugging methods */
//Serial state indication
Serial gDebugSerial(USBTX, USBRX);

//Device state
enum DevState
{
    DEV_STATE_INIT,
    DEV_STATE_JOIN,
    DEV_STATE_WAIT,
    DEV_STATE_SEND
};
static DevState gDevState = DEV_STATE_INIT;

//Control function of program
int main( void ){
    while(1){
        switch(gDevState){
            case DEV_STATE_INIT:
            {
                MibRequestConfirm_t mibReq;
                LoRaMacStatus_t status;

                gDebugSerial.printf("START: Starting lora node\n");
                
                //Board initilization
                BoardInit();

                /* //HeComm init
                uint8_t key[128] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                heCommSetSessionKey(key, 128); */

                //LoRaWAN MAC layer initialisation
                gLoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
                gLoRaMacPrimitives.MacMcpsIndication = McpsIndication;
                gLoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
                gLoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
                status = LoRaMacInitialization( &gLoRaMacPrimitives, &gLoRaMacCallbacks );
                if( status == LORAMAC_STATUS_OK )
                {
                    gDebugSerial.printf("LoRaMAC: Initialization successful\n");
                    // Initialization successful
                }else{
                    gDebugSerial.printf("LoRaMAC: Initialization FAILED\n");
                    gDebugSerial.printf("LoRaMAC: Trying again in 1 second\n");
                    wait(1);//Wait for 1 second -> try again;
                    gDevState = DEV_STATE_INIT;
                    break;
                }
                //Adaptive datarate
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );
                //Public network
                mibReq.Type = MIB_PUBLIC_NETWORK;
                mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
                LoRaMacMibSetRequestConfirm( &mibReq );
                //ETSI duty cycle control enable
                LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );

                //Class C device
                mibReq.Type = MIB_DEVICE_CLASS;
                mibReq.Param.Class = CLASS_C;
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

                gDevState = DEV_STATE_JOIN;
                break;
            }
            case DEV_STATE_JOIN:
            {
                //LoRaWAN over-the-air activation
                MlmeReq_t mlmeReq;
                LoRaMacStatus_t status;

                // Setup the request type
                mlmeReq.Type = MLME_JOIN;
                // Fill the join parameters
                mlmeReq.Req.Join.DevEui = gDevEui;
                mlmeReq.Req.Join.AppEui = gAppEui;
                mlmeReq.Req.Join.AppKey = gAppKey;
                // Add the number of trials for the join request
                //mlmeReq.Req.Join.NbTrials = 3; //Not available variable in struct MlmeReq_t
                status = LoRaMacMlmeRequest( &mlmeReq );
                if( status == LORAMAC_STATUS_OK )
                {
                    // Join request was send successfully
                    gDebugSerial.printf("LoRaMAC: Join request was send successfully\n");
                } else {
                    gDebugSerial.printf("LoRaMAC: Join request send FAILED\n");
                    gDebugSerial.printf("LoRaMAC: Trying again in 1s\n");
                    wait(1);
                    gDevState = DEV_STATE_JOIN;
                    break;
                }
                gDevState = DEV_STATE_WAIT;
                break;
            }
            case DEV_STATE_WAIT:    //Wait for requests or actions --> indication callback
            {
                wait(1);
                //gDebugSerial.printf("MAIN: waiting\n");
                break;
            }
            case DEV_STATE_SEND:
            {
                /*gDebugSerial.printf("MAIN: sending frame\n");
                uint8_t payload[2] = {0,5};
                while(!sendFrame(2, payload, 2)){
                    gDebugSerial.printf("MAIN: retry sending\n");
                    wait(2);
                }*/
                gDevState = DEV_STATE_WAIT;
                break;
            }
        }
    }
    
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static int isNetworkJoined( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.IsNetworkJoined == true )
        {
            
            gDebugSerial.printf("isJoined: already joined the LoRaWAN network\n");
            return 1;
        }
        else
        {
            gDebugSerial.printf("isJoined: not yet joined the LoRaWAN network!\n");
            return 0;
        }
    }
    return 0;
}


static void McpsConfirm( McpsConfirm_t *McpsConfirm )
{
    gDebugSerial.printf("McpsConfirm: Status: %i; request: %i\n", McpsConfirm->Status, McpsConfirm->McpsRequest);
    // Implementation of the MCPS-Confirm primitive
    if( McpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        gDebugSerial.printf("McpsConfirm: status NOT OK!\n");
    }else{
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

}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    gDebugSerial.printf("McpsIndication: Status: %i; request: %i\n", mcpsIndication->Status, mcpsIndication->McpsIndication);
    // Implementation of the MCPS-Indication primitive
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ){
        gDebugSerial.printf("McpsIndication: status NOT OK!\n");
        return;
    }

    switch( mcpsIndication->McpsIndication ){
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
  switch( mcpsIndication->Port ){ //requests or actions
    case 1:
        gDebugSerial.printf("McpsIndication: indication on port 1\n");
        break;
    case 20:
        {
            gDebugSerial.printf("McpsIndication: indication on port 20\n");
            uint8_t payload[5] = {116,101,115,116,10};
            sendFrame(20, payload, 5);
            break;
        }
    case 254:   //HeComm commands
        {
            gDebugSerial.printf("McpsIndication: command on HeCOMM; Rx:%i, BufSize: %i\n", mcpsIndication->RxData, mcpsIndication->BufferSize);
            if(mcpsIndication->RxData == true){
                if(mcpsIndication->BufferSize == 128){
                    gDebugSerial.printf("McpsIndication: command on HeCOMM; configured new key!\n");
                    heCommSetSessionKey(mcpsIndication->Buffer, mcpsIndication->BufferSize);
                }
            }
            break;
            
        }
    case 255:   //HeComm communication
        {
            gDebugSerial.printf("McpsIndication: request on HeCOMM\n");
            uint8_t payload[5] = {116,101,115,116,10};
            sendFrame(255, payload, 5);
            break;
        }
    default:
        gDebugSerial.printf("McpsIndication: indication on UNKNOWN port: %i\n", mcpsIndication->Port);
        break;
  }
}

static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm )
{
    gDebugSerial.printf("MlmeConfirm: Status: %i; request: %i\n", MlmeConfirm->Status, MlmeConfirm->MlmeRequest);
    // Implementation of the MLME-Confirm primitive
    if( MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ){
        switch( MlmeConfirm->MlmeRequest )
        {
            case MLME_JOIN:
                // Status is OK, node has joined the network
                gDebugSerial.printf("LoRaMAC: Node successfully joined network\n");
                break;
            
            default:
                break;
        }
    }else{ 
    }
}


bool sendFrame( uint8_t port, uint8_t* payload, int size )
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    
    if( LoRaMacQueryTxPossible( size, &txInfo ) != LORAMAC_STATUS_OK )
    {
        gDebugSerial.printf("sendFrame: LoRaMAC bussy!\n");
        gDebugSerial.printf("sendFrame: flushing mac\n!", size);
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = DR_0;

    }
    else
    {

        mcpsReq.Type = MCPS_CONFIRMED;  //Confirmed message
        mcpsReq.Req.Confirmed.fPort = port;
        mcpsReq.Req.Confirmed.fBuffer = payload;
        mcpsReq.Req.Confirmed.fBufferSize = size;
        mcpsReq.Req.Confirmed.NbTrials = 8;
        mcpsReq.Req.Confirmed.Datarate = DR_0;
    }

    //gDebugSerial.printf("sendFrame: Sending message, payload size %i\n!", size);
    if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
    {
        if(mcpsReq.Type == MCPS_UNCONFIRMED){
            return false;
        }
    }else{
        gDebugSerial.printf("sendFrame: DID NOT SEND!\n");
    }
    return true;
}