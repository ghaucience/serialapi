/* © 2014 Silicon Laboratories Inc. */

#ifndef SERIAL_API_H_
#define SERIAL_API_H_

#include "TYPES.H"

#define ZW_CONTROLLER
#define ZW_CONTROLLER_BRIDGE

#include "ZW_SerialAPI.h"
#include "ZW_basis_api.h"
#include "ZW_controller_api.h"
#include "ZW_transport_api.h"
#include "ZW_timer_api.h"
#include "zgw_nodemask.h"

/**
 * Structure holding all the which are know from the ZWave application programmers
 * guide. They all serve the essentially same function, except for the ApplicationInitHW,
 * which is always called with  bWakeupReason==0
 */
struct SerialAPI_Callbacks {
  void (*ApplicationCommandHandler)(BYTE  rxStatus,BYTE destNode, BYTE  sourceNode, ZW_APPLICATION_TX_BUFFER *pCmd, BYTE cmdLength);
  void (*ApplicationNodeInformation)(BYTE *deviceOptionsMask,APPL_NODE_TYPE *nodeType,BYTE **nodeParm,BYTE *parmLength );
  void (*ApplicationControllerUpdate)(BYTE bStatus,BYTE bNodeID,BYTE* pCmd,BYTE bLen, BYTE*);
  BYTE (*ApplicationInitHW)(BYTE bWakeupReason);
  BYTE (*ApplicationInitSW)(void );
  void (*ApplicationPoll)(void);
  void (*ApplicationTestPoll)(void);
  void (*ApplicationCommandHandler_Bridge)(BYTE  rxStatus,BYTE destNode, BYTE  sourceNode, ZW_APPLICATION_TX_BUFFER *pCmd, BYTE cmdLength);
  void (*SerialAPIStarted)(BYTE *pData, BYTE pLen);
#if 0
  void (*ApplicationSlaveCommandHandler)(BYTE rxStatus, BYTE destNode, BYTE sourceNode, ZW_APPLICATION_TX_BUFFER *pCmd, BYTE cmdLength);
#endif
};

#define CHIP_DESCRIPTOR_UNINITIALIZED 0x42

/**
 * This chip descriptor contains the version of chip (e.g. 500 or 700).
 * It is populated when serialapi process starts up.
 * Until populated, it contains the value CHIP_DESCRIPTOR_UNINITIALIZED.
 */
struct chip_descriptor {
  uint8_t my_chip_type;
  uint8_t my_chip_version;
};
extern struct chip_descriptor chip_desc;

/*static struct SerialAPI_Callbacks serial_api_callbacks = {
    ApplicationCommandHandler,
    ApplicationNodeInformation,
    ApplicationControllerUpdate,
    ApplicationInitHW,
    ApplicationInitSW,
    ApplicationPoll,
    ApplicationTestPoll,
};*/

/*
 * Initialize the Serial API, with all the callbacks needed for operation.
 */
BOOL SerialAPI_Init(const char* serial_port, const struct SerialAPI_Callbacks* callbacks );
void SerialAPI_Destroy();
/*
 * This must be called from some application main loop. This is what driver the
 * serial api engine. SerialAPI_Poll checks the serial port for new data and
 * handles timers and all other async callbacks. Ie. all Serial API callbacks
 * are called from within this function.
 *
 * TODO: It would be nice to have SerialAPI_Poll return the minimum number of system
 * ticks to parse before a timer times out. In this way we don't need to call
 * SerialAPI_Poll unless we got a receive interrupt from the uart or if a timer
 * has timed out. This means that the system would be able to go into low power
 * mode between SerialAPI_Poll calls.
 */
uint8_t SerialAPI_Poll();

/** Used to indicate that transmissions was not completed due to a
 * SendData that returned false. This is used in some of the higher level sendata
 * calls where lower layer senddata is called async. */
#define TRANSMIT_COMPLETE_ERROR 0xFF

/**
 * Tell upper queue to requeue this frame
 * because ClassicZIPNode is busy sending another frame.
 */
#define TRANSMIT_COMPLETE_REQUEUE 0xFE
/**
 * Tell upper queue to requeue this frame
 * because ClassicZIPNode cannot send to PAN side because
 * NetworkManagament or Mailbox are busy, or because the destination
 * node is being probed.
 * The frame should be re-queued to long-queue or Mailbox. */
#define TRANSMIT_COMPLETE_REQUEUE_QUEUED 0xFD


BYTE SerialAPI_GetInitData( BYTE *ver, BYTE *capabilities, BYTE *len, BYTE *nodesList,BYTE* chip_type,BYTE* chip_version );

BOOL SerialAPI_GetRandom(BYTE count, BYTE* randomBytes);

/** Look up the cached chip type and chip version.
 * Only valid after \ref SerialAPI_GetInitData() has been called.
 * 
 * \param type Return argument for the chip type.
 * \param version Return argument for the chip version. 
 */
void SerialAPI_GetChipTypeAndVersion(uint8_t *type, uint8_t *version);

/* Long timers */
BYTE ZW_LTimerStart(void (*func)(),unsigned long timerTicks, BYTE repeats);
BYTE ZW_LTimerRestart(BYTE handle);
BYTE ZW_LTimerCancel(BYTE handle);


/* Access hardware AES encrypter. ext_input,ext_output,ext_key are all 16 byte arrays
 * Returns true if routine went well. */
BOOL SerialAPI_AES128_Encrypt(const BYTE *ext_input, BYTE *ext_output, const BYTE *ext_key);

/*FIXME Don't know why this is not defined elsewhere */
BOOL ZW_EnableSUC( BYTE state, BYTE capabilities );

void
SerialAPI_ApplicationSlaveNodeInformation(BYTE dstNode, BYTE listening,
    APPL_NODE_TYPE nodeType, BYTE *nodeParm, BYTE parmLength );

void SerialAPI_ApplicationNodeInformation( BYTE listening, APPL_NODE_TYPE nodeType, BYTE *nodeParm, BYTE parmLength );

void Get_SerialAPI_AppVersion(uint8_t *major, uint8_t *minor);

/**
 * Enable APM mode
 */
void ZW_AutoProgrammingEnable(void);


void ZW_GetRoutingInfo_old( BYTE bNodeID, BYTE *buf, BYTE bRemoveBad, BYTE bRemoveNonReps );

void
ZW_AddNodeToNetworkSmartStart(BYTE bMode, BYTE *dsk,
                    VOID_CALLBACKFUNC(completedFunc)(auto LEARN_INFO*));

void
ZW_GetBackgroundRSSI(BYTE *rssi_values, BYTE *values_length);


/**
  * \ingroup ZWCMD
  * Transmit data buffer to a list of Z-Wave Nodes (multicast frame) in bridge mode.
  *
  * Invokes the ZW_SendDataMulti_Bridge() function on the Z-Wave chip via serial API.
  *
  * This function should be used also when the gateway is the source node because the
  * function ZW_SendDataMulti() is not available on a chip with bridge controller
  * library (which is what the gateway uses).
  *
  * NB: This function is named differently from the actual Z-Wave API function.
  *     This is intentional since pre-processor defines in ZW_transport_api.h
  *     re-maps ZW_SendDataMulti and ZW_SendDataMulti_Bridge.
  *
  * \param[in] srcNodeID      Source nodeID - if 0xFF then controller is set as source
  * \param[in] dstNodeMask    Node mask where the bits corresponding to the destination node IDs are set.
  * \param[in] data           Data buffer pointer
  * \param[in] dataLength     Data buffer length
  * \param[in] txOptions      Transmit option flags
  * \param[in] completedFunc  Transmit completed call back function
  * \return FALSE if transmitter queue overflow
  */
BYTE SerialAPI_ZW_SendDataMulti_Bridge(BYTE srcNodeID,
                                       nodemask_t dstNodeMask,
                                       BYTE *data,
                                       BYTE dataLength,
                                       BYTE txOptions,
                                       VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus));

/**
 * \ingroup BASIS
 * *
 * Enable watchdog and start kicking it in the Z-Wave chip
 * \serialapi{
 * HOST -> ZW: REQ | 0xD2
 * }
 *
 */
void SerialAPI_WatchdogStart();


#endif /* SERIAL_API_H_ */

