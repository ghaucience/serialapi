/* © 2014 Silicon Laboratories Inc.
 */

#ifndef ZW_ZIPAPPLICATION_H_
#define ZW_ZIPAPPLICATION_H_

#include <TYPES.H>
#include <ZW_typedefs.h>
#include "ZW_basis_api.h"
#include "ZW_classcmd_ex.h"
#include "ZW_SendDataAppl.h"

extern BYTE IPNIF[];
extern BYTE IPNIFLen;
extern BYTE IPSecureClasses[];
extern BYTE IPnSecureClasses;

extern BYTE MyNIF[];
extern BYTE MyNIFLen;
#define NIF ((NODEINFO*) MyNIF)
#define CLASSES ((BYTE*)&MyNIF[sizeof(NODEINFO)])
#define NUMCLASSES (MyNIFLen - sizeof(NODEINFO))
extern BYTE nSecureClassesPAN;
extern BYTE SecureClassesPAN[];
void ApplicationDefaultSet();
void ApplicationInitProtocols(void);

/**
 * Call if the one of the NIFs has been dynamically updated.
 */
void CommandClassesUpdated() ;
/** Add more command classes to NIF and update Z-Wave target.
 * Make sure to add less than sizeof(MyNIF) CCs in total. */
void AddUnsocDestCCsToGW(BYTE *ccList, BYTE ccCount);
void AddSecureUnsocDestCCsToGW(BYTE *ccList, BYTE ccCount);


/**
 * Send a nodelist to the unsolicited destination, if
 * \ref should_send_nodelist is true.
 */
void send_nodelist();

/**
 * Called when SerialAPI restarts (typically when power cycled)
 *
 * pData contains the following data:
 *   ZW->HOST: bWakeupReason | bWatchdogStarted | deviceOptionMask |
 *          nodeType_generic | nodeType_specific | cmdClassLength | cmdClass[]
 */
void SerialAPIStarted(uint8_t *pData, uint8_t length);

/**
 * Set the pre-inclusion NIF, which is an non-seucre nif.
 * \param target_scheme The scheme we wish to present
 */
void SetPreInclusionNIF(security_scheme_t target_scheme);

/**
 * Stop the timer which probes for new nodes.
 */
void StopNewNodeProbeTimer();

#endif /* ZW_ZIPAPPLICATION_H_ */
