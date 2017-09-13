#include "mbed.h"
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "comissioning.h"

//CoAP includes
#include <string>
//#include "FEATURE_COMMON_PAL/mbed-coap/mbed-coap/sn_coap_protocol.h"
//#include "FEATURE_COMMON_PAL/mbed-coap/mbed-coap/sn_coap_header.h"

#include "mbed-coap/sn_coap_protocol.h"
#include "mbed-coap/sn_coap_header.h"

//Object security
#include "cn-cbor/cn-cbor.h"
#include "obj-sec.h"


/* Function declarations */
//Callback functions for LoRaWAN-lib 
static void McpsConfirm( McpsConfirm_t *McpsConfirm );
static void McpsIndication( McpsIndication_t *McpsIndication );
static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm );
//Helper functions
static int isNetworkJoined( void );
bool sendFrame( uint8_t port, uint8_t* payload, int size );

extern "C" void * calloc_fn(size_t count, size_t size, void *context);
extern "C" void free_fn(void *ptr, void *context);

int16_t compileResponse(uint8_t *resp, size_t respSize, uint8_t *rxBuffer, size_t rxBufferSize);

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

#define BUTTON_ENABLE 1
#ifdef BUTTON_ENABLE
//Add button to send messages to fog
InterruptIn button(USER_BUTTON);
#endif

//Device state
enum DevState
{
    DEV_STATE_INIT,
    DEV_STATE_JOIN,
    DEV_STATE_WAIT,
    DEV_STATE_SEND
};
static DevState gDevState = DEV_STATE_INIT;

struct coap_s* coapHandle;
coap_version_e coapVersion = COAP_VERSION_1;
 
// CoAP HAL
void* coap_malloc(uint16_t size) {
    printf("Coap: malloc size: %u\r\n", size);
    return malloc(size);
}
 
void coap_free(void* addr) {
    printf("Coap: free %p\r\n", addr);
    free(addr);
}


// tx_cb and rx_cb are not used in this program
uint8_t coap_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d) {
    printf("coap tx cb\n");
    return 0;
}
 
int8_t coap_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c) {
    printf("coap rx cb\n");
    return 0;
}

int heapSize()
{
   char   stackVariable;
   void   *heap;
   int result;
   heap  = malloc(4);
   if(heap == 0){
       return 0;
   }
   result  = &stackVariable - (char *)heap;
   free(heap);
   return result;
}

#ifdef BUTTON_ENABLE
void sendDebugMessage() {
    printf("USER BUTTON interrupt, sending message... \r\n");
    uint8_t payload[5] = {116,101,115,116,10};

    if(!sendFrame(255, payload, sizeof(payload))){
        printf("Did not send!\r\n");
    }else{
        printf("Successfully send message\r\n");
    }
}
#endif


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
                
                //Initialise security before coap messages
                cn_cbor_context context = {.calloc_func = &calloc_fn, .free_func = &free_fn, .context = NULL};
                objsec_init(context);
                printf("Objsec sucessfully inited\n");

                //Configure CoAP
                // Initialize the CoAP protocol handle, pointing to local implementations on malloc/free/tx/rx functions
                coapHandle = sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);

            #ifdef BUTTON_ENABLE
                button.rise(&sendDebugMessage);  // call toggle function on the rising edge
            #endif

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
                //gDevState = DEV_STATE_SEND;
                break;
            }
            case DEV_STATE_WAIT:    //Wait for requests or actions --> indication callback
            {
                
                //gDebugSerial.printf("MAIN: waiting\n");
                break;
            }
            case DEV_STATE_SEND:
            {
                
                


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
                    //heCommSetSessionKey(mcpsIndication->Buffer, mcpsIndication->BufferSize);
                    if(mcpsIndication->BufferSize != 16){
                        printf("Key lenght does not match!: %u\r\n", mcpsIndication->BufferSize);
                    }
                    objsec_set_key(mcpsIndication->Buffer);
                }
            }
            break;
            
        }
    case 255:   //HeComm communication
        {
            uint8_t buffer[128];
            gDebugSerial.printf("McpsIndication: request on HeCOMM\n");
            
            int16_t length = compileResponse(buffer, 128, mcpsIndication->Buffer, mcpsIndication->BufferSize);
            if(length < 0){
                printf("Unable to compile response\r\n");
                break;
            }
            if(!sendFrame(255, buffer, length)){
                printf("Did not send reply!\r\n");
            }else{
                printf("Successfully send reply\r\n");
            }
            gDevState = DEV_STATE_SEND;
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

int16_t compileResponse(uint8_t *resp, size_t respSize, uint8_t *rxBuffer, size_t rxBufferSize){
    const char* payload = "Hello world!";
    uint8_t buffer[128];
    size_t bufferSize = 128;
    size_t payloadLength = 12;
    int16_t encryptedLength = 0;
    uint16_t messageLength = 0;
    sn_coap_hdr_s *coap_res_ptr = NULL;

    //Expect coap packet
    printf("Parsing CoAP...\n");
    sn_coap_hdr_s* parsed = sn_coap_parser(coapHandle, rxBufferSize, rxBuffer, &coapVersion);
    
    if(parsed != NULL){
        //We expect the payload to be a string
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);

        printf("\tmsg_id:           %d\n", parsed->msg_id);
        printf("\tmsg_code:         %d\n", parsed->msg_code);
        printf("\tcontent_format:   %d\n", parsed->content_format);
        printf("\tpayload_len:      %d\n", parsed->payload_len);
        printf("\tpayload:          %s\n", payload.c_str());
        printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr); 
    }else{
        printf("Did not receive a correct coap packet!\r\n");
        return -1;
    }
    //TODO: check if request contains the right resource, for now always send the resource

    gDebugSerial.printf("Compiling response...\n");
    // Path to the resource we want to retrieve

    //Encrypting message
    
    printf("Encrypting... provided buffer of size: %u\r\n", bufferSize);
    encryptedLength = encrypt(buffer,bufferSize, (const uint8_t *) payload,payloadLength);
    if(0 > encryptedLength){
        printf("Did not encrypt correctly\r\n");
        goto errorReturn;
    }else{
        printf("Encrypted message of size: %u\r\n", encryptedLength);
    }
    //Creating coap header/packet
    // See ns_coap_header.h
    //sn_coap_hdr_s *coap_res_ptr = (sn_coap_hdr_s*)calloc(sizeof(sn_coap_hdr_s), 1);
    //sn_coap_hdr_s *coap_res_ptr = sn_coap_parser_alloc_message(coapHandle);
    coap_res_ptr = sn_coap_build_response(coapHandle,parsed,COAP_MSG_CODE_RESPONSE_VALID );
    if(NULL == coap_res_ptr){
        printf("Unable to allocate COAP message\r\n");
        goto errorReturn;
    }
    /* coap_res_ptr->uri_path_ptr = (uint8_t*)coap_uri_path;       // Path
    coap_res_ptr->uri_path_len = strlen(coap_uri_path);
    coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_GET;         // CoAP method */
    coap_res_ptr->content_format = COAP_CT_TEXT_PLAIN;          // CoAP content type
    coap_res_ptr->payload_len = encryptedLength;                              // Body length
    coap_res_ptr->payload_ptr = buffer;                              // Body pointer
    //coap_res_ptr->options_list_ptr = 0;                         // Optional: options list
    // Message ID is used to track request->response patterns, because we're using UDP (so everything is unconfirmed).
    // See the receive code to verify that we get the same message ID back
    //coap_res_ptr->msg_id = 7;
    
    // Calculate the CoAP message size, allocate the memory and build the message
    messageLength = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);

    if(!messageLength){
        printf("Unable to calculate coap packet size\n");
        goto errorReturn;
    }
    if(messageLength > respSize){
        printf("Response buffer too small\r\n");
        goto errorReturn;
    }
    printf("Calculated message length: %d bytes\n", messageLength);
    
    messageLength = sn_coap_builder(resp, coap_res_ptr);    

    sn_coap_parser_release_allocated_coap_msg_mem(coapHandle, parsed);
    sn_coap_parser_release_allocated_coap_msg_mem(coapHandle, coap_res_ptr);
    return messageLength;
    

errorReturn:
    //Compile negative response
    coap_res_ptr = sn_coap_build_response(coapHandle,parsed,COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR);
    if(NULL == coap_res_ptr){
        printf("Unable to allocate bad response COAP message\r\n");
        return -1;
    }
    messageLength = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);
    
    if(!messageLength){
        printf("Unable to calculate coap packet size\n");
    }else{
        if(messageLength > respSize){
            printf("Response buffer too small\r\n");
            messageLength = -1;
        }else{
            printf("Calculated message length: %d bytes\n", messageLength);
            
            messageLength = sn_coap_builder(resp, coap_res_ptr);    
        }
    }
    sn_coap_parser_release_allocated_coap_msg_mem(coapHandle, parsed);
    sn_coap_parser_release_allocated_coap_msg_mem(coapHandle, coap_res_ptr);
    return messageLength;
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
            return true;
        }
        return true;
    }else{
        gDebugSerial.printf("sendFrame: DID NOT SEND!\n");
    }
    return false;
}

//typedef void* (*cn_calloc_func)(size_t count, size_t size, void *context);
void * calloc_fn(size_t count, size_t size, void *context){
    //return mbed_ualloc(size * count, [UALLOC_TRAITS_NEVER_FREE, UALLOC_TRAITS_ZERO_FILL]);
    printf("fn calloc: s: %u, c: %u\r\n", size, count);
    return calloc(count, size);
}

//typedef void (*cn_free_func)(void *ptr, void *context);
void free_fn(void *ptr, void *context){
    //mbed_ufree(ptr);
    printf("fn free: p: %p\r\n", ptr);
    free(ptr);
}