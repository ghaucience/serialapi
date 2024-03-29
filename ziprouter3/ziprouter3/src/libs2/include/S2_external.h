/* © 2017 Silicon Laboratories Inc.
 */
/*
 * S2_external.h
 *
 *  Created on: Sep 24, 2015
 *      Author: aes
 */

#ifndef INCLUDE_S2_EXTERNAL_H_
#define INCLUDE_S2_EXTERNAL_H_

#include "S2.h"
/**
 * \ingroup S2trans
 * \defgroup s2external Extenal functions used by S2
 *
 * All of these function must be implemented elsewhere. These are
 * the hooks that libs2 use for actually sending frames, and to timer tracking.
 * @{
 */
#ifdef SINGLE_CONTEXT
#define S2_send_done_event(a,b) __S2_send_done_event(b)
#define S2_msg_received_event(a,b,c,d) __S2_msg_received_event(b,c,d)
#define S2_send_frame(a,b,c,d) __S2_send_frame(b,c,d)
#define S2_send_frame_multi(a,b,c,d) __S2_send_frame_multi(b,c,d)
#define S2_set_timeout(a,b) __S2_set_timeout(b)
#endif
/**
 * Emitted when the security engine is done.
 * Note that this is also emitted when the security layer sends a SECURE_COMMANDS_SUPPORTED_REPORT
 *
 * \param ctxt the S2 context
 * \param status The status of the S2 transmisison
 */
void S2_send_done_event(struct S2* ctxt, s2_tx_status_t status);

/**
 * Emitted when a messages has been received and decrypted
 * \param ctxt the S2 context
 * \param peer Transaction parameters. peer->class_id will indicate which security class was used for
 * decryption.
 * \param buf The received message
 * \param len The length of the received message
 */
void S2_msg_received_event(struct S2* ctxt,s2_connection_t* peer , uint8_t* buf, uint16_t len);


/**
 * Must be implemented elsewhere maps to ZW_SendData or ZW_SendDataBridge note that ctxt is
 * passed as a handle. The ctxt MUST be provided when the \ref S2_send_frame_done_notify is called
 *
 * \param ctxt the S2 context
 * \param peer Transaction parameters.
 * \param buf The received message
 * \param len The length of the received message
 *
 */
uint8_t S2_send_frame(struct S2* ctxt,const s2_connection_t* peer, uint8_t* buf, uint16_t len);

#ifdef ZW_CONTROLLER
/**
 * Send without emitting a callback into S2 and by caching the buffer
 *
 * Must be implemented elsewhere maps to ZW_SendData or ZW_SendDataBridge note that ctxt is
 * passed as a handle. The ctxt MUST be provided when the \ref S2_send_frame_done_notify is called.
 *
 * \param ctxt the S2 context
 * \param peer Transaction parameters.
 * \param buf The received message
 * \param len The length of the received message
 *
 */
uint8_t S2_send_frame_no_cb(struct S2* ctxt,const s2_connection_t* peer, uint8_t* buf, uint16_t len);
#endif

/**
 * This must send a broadcast frame.
 * \ref S2_send_frame_done_notify must be called when transmisison is done.
 *
 * \param ctxt the S2 context
 * \param peer Transaction parameters.
 * \param buf The received message
 * \param len The length of the received message
 */
uint8_t S2_send_frame_multi(struct S2* ctxt,s2_connection_t* peer, uint8_t* buf, uint16_t len);

/**
 * Must be implemented elsewhere maps to ZW_TimerStart. Note that this must start the same timer every time.
 * Ie. two calls to this function must reset the timer to a new value. On timeout \ref S2_timeout_notify must be called.
 *
 * \param ctxt the S2 context
 * \param interval Timeout in milliseconds.
 */
void S2_set_timeout(struct S2* ctxt, uint32_t interval);


/**
 * Get a number of bytes from a hardware random number generator
 *
 * \param buf pointer to buffer where the random bytes must be written.
 * \param len number of bytes to write.
 */
void S2_get_hw_random(uint8_t *buf, uint8_t len);

/**
 * Get the command classes supported by S2
 * \param lnode The node id to which the frame was sent.
 * \param class_id the security class this request was on
 * \param cmdClasses points to the first command class
 * \param length the length of the command class list
 */
void S2_get_commands_supported(uint8_t lnode, uint8_t class_id, const uint8_t ** cmdClasses, uint8_t* length);

/**
 * External function for printing debug output
 * Takes printf arguments
 */
void S2_dbg_printf(const char *str, ...);

/**
 * Makes time in ms available to LibS2
 * \return Timer tick in MS
 */
#ifdef ZIPGW
unsigned long clock_time(void);
#else
uint16_t clock_time(void);
#endif

/**
 * @}
 */
#endif /* INCLUDE_S2_EXTERNAL_H_ */
