#include "zwave.h"
#include "log.h"

#include "Serialapi.h"
#include "zwave_device.h"
#include "zwave_device_storage.h"
#include "zwave_class_cmd.h"
#include "zwave_ccdb.h"

#include "ZW_classcmd.h"
#include "schedule.h"
#include "system.h"
#include "ZW_basis_api.h"
#include "amber_mtksdk.h"

static stZWaveEnv_t ze = {
	0
};

static int include_exclude_ing = 0;


void zwave_device_state_task_func(void *arg);


void zwave_device_state_change(stZWaveDevice_t *zd, int new);
int zs_class_version_get_success(stZWaveDevice_t *zd, unsigned char class, int versoin);
int zs_class_version_get_failed(stZWaveDevice_t *zd, unsigned char class);
int zs_class_version_get_timeout(stZWaveDevice_t *zd);
int zs_class_version_get(unsigned char nid, unsigned char class);


static void ApplicationCommandHandler(
		BYTE  rxStatus,                   /*IN  Frame header info */
#if defined(ZW_CONTROLLER) && !defined(ZW_CONTROLLER_STATIC) && !defined(ZW_CONTROLLER_BRIDGE)
		BYTE  destNode,                  /*IN  Frame destination ID, only valid when frame is not Multicast*/
#endif
		BYTE  sourceNode,                 /*IN  Command sender Node ID */
		ZW_APPLICATION_TX_BUFFER *pCmd,  /*IN  Payload from the received frame, the union
																			 should be used to access the fields*/
		BYTE   cmdLength); /*CC_REENTRANT_ARG*/                /*IN  Number of command bytes including the command */
void ApplicationNodeInformation(BYTE *deviceOptionsMask, /*OUT Bitmask with application options    */
		APPL_NODE_TYPE *nodeType, /*OUT  Device type Generic and Specific   */
		BYTE **nodeParm, /*OUT  Device parameter buffer pointer    */
		BYTE *parmLength /*OUT  Number of Device parameter bytes   */
		); 
void ApplicationControllerUpdate(BYTE bStatus, /*IN  Status event */
		BYTE bNodeID, /*IN  Node id of the node that send node info */
		BYTE* pCmd, /*IN  Pointer to Application Node information */
		BYTE bLen /*IN  Node info length                        */
		);
void ApplicationCommandHandler_Bridge(BYTE  rxStatus,BYTE destNode, BYTE  sourceNode, ZW_APPLICATION_TX_BUFFER *pCmd, BYTE cmdLength);
void SerialAPIStarted(BYTE *pData, BYTE pLen);


static const struct SerialAPI_Callbacks serial_api_callbacks = {
	.ApplicationCommandHandler		= ApplicationCommandHandler,
	.ApplicationNodeInformation		= ApplicationNodeInformation,
	.ApplicationControllerUpdate	= ApplicationControllerUpdate,
	.ApplicationInitHW						= NULL,
	.ApplicationInitSW						= NULL,
	.ApplicationPoll							= NULL,
	.ApplicationTestPoll					= NULL,
	.ApplicationCommandHandler_Bridge	= ApplicationCommandHandler_Bridge,
	.SerialAPIStarted							= SerialAPIStarted,
};

int		zwave_init(void *_th, void *_fet, char *dev, int buad) {
	ze.th = _th;
	ze.fet = _fet;
	log_debug(" ");
	
	timer_init(&ze.step_timer, zwave_run);
	lockqueue_init(&ze.eq);
	
#if 0
	ze.fd = 0;
	file_event_reg(ze.fet, ze.fd, zwave_in, NULL, NULL);
#else
	strcpy(ze.dev, dev);
	ze.buad = buad;

	system_delete_netinfo("/etc/config/dusun/zwdev/netinfo");
	if (!SerialAPI_Init(dev, &serial_api_callbacks)) {
		system("echo 0 > /sys/class/gpio/gpio40/value; sleep 1; echo 1 > /sys/class/gpio/gpio40/value; sleep 5;");
		
		log_warn("SerialAPI_Init Failed!");
		exit (0);
	}
	system_write_netinfo("/etc/config/dusun/zwdev/netinfo", 0, 0);

	char zver[64];
	if (ZW_Version((BYTE *)zver) == 0) {
		log_warn("SerialAPI_Init Failed!");
		exit (0);
	}
	log_debug("zwave version:%s", zver);

	BYTE region = ZW_RFRegionGet();
	log_debug("region: %02X", region&0xff);
		BYTE rflvl = ZW_RFPowerLevelGet();
	log_debug("rf lvl:%02X", rflvl&0xff);

	TX_POWER_LEVEL txlvl = ZW_TXPowerLevelGet();
	log_debug("normal:%02X, measure:%02X", txlvl.normal&0xff, txlvl.measured0dBm&0xff);

	txlvl.normal = 0x00;
	txlvl.measured0dBm = 0x20;
	ZW_TXPowerLevelSet(txlvl);
	
	int region_file = system_get_zwave_region();
	if (region_file < 0) {
		log_warn("no correct region setting, use default in chip:%02X", region);
		system_set_zwave_region(region&0xff);
	} else {
		log_info("region_file:%02X, region:%02X", region_file&0xff, region&0xff);
		if ((region_file&0xff) != (region&0xff)) {
			log_info("setting region from :%02X -> %02X", region&0xff, region_file&0xff);
			zwave_region(region_file);
			sleep(1);
			ZW_SoftReset();
		} else {
			log_info("holding the region:%02X", region&0xff);
		}
	}

	extern int SerialAPI_GetFd();
	ze.fd = SerialAPI_GetFd();
	file_event_reg(ze.fet, ze.fd, zwave_in, NULL, NULL);

	int zwave_remove_failed_node_start();
	zwave_remove_failed_node_start();


	void parse_type_load();
	parse_type_load();

	void zwave_device_state_init();
	zwave_device_state_init();

	extern void uproto_dusun_start_up_rpt_devlist();
	uproto_dusun_start_up_rpt_devlist();
#endif

	return 0;

}
int		zwave_step() {

	timer_cancel(ze.th, &ze.step_timer);
	timer_set(ze.th, &ze.step_timer, 10);
	return 0;
}
int		zwave_push(stEvent_t *e) {

	lockqueue_push(&ze.eq, e);
	zwave_step();
	return 0;
}
void	zwave_run(struct timer *timer) {
	stEvent_t *e;

	if (!lockqueue_pop(&ze.eq, (void**)&e)) {
		return;
	}

	if (e != NULL) {
		FREE(e);
	}

	zwave_step();

}
void	zwave_in(void *arg, int fd) {
	log_debug(" ");

	extern int SerialAPI_GetFd();
	int _fd = SerialAPI_GetFd();
	if (_fd != ze.fd) {
		log_warn("re watch the serial fd : %d , orgin:%d", _fd, ze.fd);
		file_event_unreg(ze.fet, ze.fd, zwave_in, NULL, NULL);
		ze.fd = _fd;
		file_event_reg(ze.fet, ze.fd, zwave_in, NULL, NULL);
	}

	while (SerialAPI_Poll() > 0);
	log_debug("over");
}
/////////////////////////////////////////////////////////////////////////////
typedef struct stClassCmdHanlder {
	unsigned char class;
	unsigned char cmd;
	int						(*handler)(unsigned char sourceNode, unsigned char *pCmd, unsigned cmdLength);
} stClassCmdHanlder_t;

int version_report_hanlder(unsigned sourceNode,  unsigned char *pCmd, unsigned cmdLength);
int assoc_grps_report_handler(unsigned sourceNode, unsigned char *pCmd, unsigned cmdLength);
int assoc_report_handler(unsigned sourceNode, unsigned char *pCmd, unsigned cmdLength);
int class_cmd_generic_handler(unsigned sourceNode, unsigned char *pCmd, unsigned cmdLength);
static stClassCmdHanlder_t cchs[] = {
	{COMMAND_CLASS_VERSION,			VERSION_COMMAND_CLASS_REPORT,		version_report_hanlder},
	{COMMAND_CLASS_ASSOCIATION, ASSOCIATION_GROUPINGS_REPORT,		assoc_grps_report_handler},
	{COMMAND_CLASS_ASSOCIATION, ASSOCIATION_REPORT,							assoc_report_handler},
};

stClassCmdHanlder_t *class_cmd_handler_search(unsigned class, unsigned char cmd) {
	int i = 0;
	int cnt = sizeof(cchs)/sizeof(cchs[0]);
	for (i = 0; i < cnt; i++) {
		stClassCmdHanlder_t *cch = &cchs[i];
		if ((cch->class&0xff) != (class&0xff)) {
			continue;
		}
		if ((cch->cmd&0xff) != (cmd&0xff)) {
			continue;
		}
		return cch;
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
static void ApplicationCommandHandler(
		BYTE  rxStatus,                   /*IN  Frame header info */
#if defined(ZW_CONTROLLER) && !defined(ZW_CONTROLLER_STATIC) && !defined(ZW_CONTROLLER_BRIDGE)
		BYTE  destNode,                  /*IN  Frame destination ID, only valid when frame is not Multicast*/
#endif
		BYTE  sourceNode,                 /*IN  Command sender Node ID */
		ZW_APPLICATION_TX_BUFFER *pCmd,  /*IN  Payload from the received frame, the union
																			 should be used to access the fields*/
		BYTE   cmdLength) /*CC_REENTRANT_ARG*/                /*IN  Number of command bytes including the command */
{
		__ApplicationControllerUpdate(rxStatus, 
		#if defined(ZW_CONTROLLER) && !defined(ZW_CONTROLLER_STATIC) && !defined(ZW_CONTROLLER_BRIDGE)
		destNode, 
		#endif
		sourceNode, pCmd, cmdLength);
#if 0
	printf("ApplicationCommandHandler node %d class %d size %d\n",sourceNode,pCmd->ZW_Common.cmdClass,cmdLength);
	if(pCmd->ZW_Common.cmdClass == COMMAND_CLASS_SIMPLE_AV_CONTROL) {
		switch (pCmd->ZW_Common.cmd)
		{
			case SIMPLE_AV_CONTROL_SET:
				{
					unsigned short index;
					unsigned short avCommand = pCmd->ZW_SimpleAvControlSet1byteFrame.variantgroup1.command1;
					avCommand <<=8;
					avCommand |= pCmd->ZW_SimpleAvControlSet1byteFrame.variantgroup1.command2;
					for (index = 0; avCmdStrings[index].avCmd != 0;index++)
					{
						if (avCommand == avCmdStrings[index].avCmd)
						{
							break;
						}
					}
					printf("SIMPLE_AV_CONTROL_SET \n");
					printf("%s ",avCmdStrings[index].str);
					if ((pCmd->ZW_SimpleAvControlSet1byteFrame.properties1 & 0x07) == 0)
					{
						printf("Attribute :Key Down\n");
					}
					else  if ((pCmd->ZW_SimpleAvControlSet1byteFrame.properties1 & 0x07) == 1)
					{
						printf("Attribute :Key Up\n");
					}
					else  if ((pCmd->ZW_SimpleAvControlSet1byteFrame.properties1 & 0x07) == 2)
					{
						printf("Attribute :Keep Alive\n");
					}
					else
					{
						printf("Attribute :UNKNOWN \n");
					}

				}
				break;
		}
	}
#else
	log_debug( " ");
	char* buf = (char *)pCmd;

	log_debug_hex("pCmd:", (char *)buf, cmdLength&0xff);
	unsigned char class = buf[0]&0xff;
	unsigned char cmd		= buf[1]&0xff;
	char *data					= buf + 2;
	unsigned len				= cmdLength - 2;
	log_debug("class:%02X, cmd:%02X", class&0xff, cmd&0xff);
	log_debug_hex("data:", data, len&0xff);

	if (include_exclude_ing == 0) {
		system_led_shot("zigbee");
	}

	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd == NULL) {
		log_warn("no such dev!");
		return;
	}
	int online = device_get_online(zd);
	zd->last = schedue_current();
	if ( online == 0) {
		uproto_rpt_status_dusun(device_get_extaddr(zd));
	}
	class_cmd_generic_handler(sourceNode, (unsigned char *)pCmd, cmdLength);
	stClassCmdHanlder_t *handler = class_cmd_handler_search(pCmd->ZW_Common.cmdClass, pCmd->ZW_Common.cmd);
	if (handler != NULL) {
		handler->handler(sourceNode, (unsigned char *)pCmd, cmdLength);
	} 

	if (zd->state == DS_WORKED) {
		uproto_rpt_cmd_dusun(device_get_extaddr(zd), 0, class&0xff, cmd&0xff, data , len);
	}

	/** TODO REPORT stand*/
#endif
}

void ApplicationNodeInformation(BYTE *deviceOptionsMask, /*OUT Bitmask with application options    */
		APPL_NODE_TYPE *nodeType, /*OUT  Device type Generic and Specific   */
		BYTE **nodeParm, /*OUT  Device parameter buffer pointer    */
		BYTE *parmLength /*OUT  Number of Device parameter bytes   */
		) {
	log_debug( " ");
	 __ApplicationNodeInformation(deviceOptionsMask, nodeType, nodeParm, parmLength);
#if 0
	/* this is a listening node and it supports optional CommandClasses */
	*deviceOptionsMask = APPLICATION_NODEINFO_LISTENING
		| APPLICATION_NODEINFO_OPTIONAL_FUNCTIONALITY;
	nodeType->generic = MyType.generic; /* Generic device type */
	nodeType->specific = MyType.specific; /* Specific class */
	*nodeParm = (BYTE*)MyClasses; /* Send list of known command classes. */
	*parmLength = sizeof(MyClasses); /* Set length*/
#endif
}

void ApplicationControllerUpdate(BYTE bStatus, /*IN  Status event */
		BYTE bNodeID, /*IN  Node id of the node that send node info */
		BYTE* pCmd, /*IN  Pointer to Application Node information */
		BYTE bLen /*IN  Node info length                        */
		) {
	log_debug( " ");
	__ApplicationControllerUpdate(bStatus, bNodeID, pCmd, bLen);
#if 0
	printf("Got node info from node %d\n",bNodeID);
#endif
}
void ApplicationCommandHandler_Bridge(BYTE  rxStatus,BYTE destNode, BYTE  sourceNode, ZW_APPLICATION_TX_BUFFER *pCmd, BYTE cmdLength) {
	log_debug( " ");
	char* buf = (char *)pCmd;

	log_debug_hex("pCmd:", (char *)buf, cmdLength&0xff);
	unsigned char class = buf[0]&0xff;
	unsigned char cmd		= buf[1]&0xff;
	char *data					= buf + 2;
	unsigned len				= cmdLength - 2;
	log_debug("class:%02X, cmd:%02X", class&0xff, cmd&0xff);
	log_debug_hex("data:", data, len&0xff);

#if 1
	if (include_exclude_ing == 0) {
		system_led_shot("zigbee");
	}
	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd == NULL) {
		log_warn("no such dev!");
		return;
	}

	int online = device_get_online(zd);
	zd->last = schedue_current();
	if ( online == 0) {
		uproto_rpt_status_dusun(device_get_extaddr(zd));
	}

	class_cmd_generic_handler(sourceNode, (unsigned char *)pCmd, cmdLength);
	stClassCmdHanlder_t *handler = class_cmd_handler_search(pCmd->ZW_Common.cmdClass, pCmd->ZW_Common.cmd);
	if (handler != NULL) {
		handler->handler(sourceNode, (unsigned char *)pCmd, cmdLength);
	} 

	if (zd->state == DS_WORKED) {
		uproto_rpt_cmd_dusun(device_get_extaddr(zd), 0, class&0xff, cmd&0xff, data , len);
	}
#else
	switch (pCmd->ZW_Common.cmdClass) {
		case COMMAND_CLASS_VERSION: {
			switch (pCmd->ZW_Common.cmd) {
				case VERSION_COMMAND_CLASS_REPORT: {
					ZW_VERSION_COMMAND_CLASS_REPORT_FRAME *zv = (ZW_VERSION_COMMAND_CLASS_REPORT_FRAME*)pCmd;
					int version = zv->commandClassVersion&0xff;
					unsigned char reqclass = zv->requestedCommandClass&0xff;

					stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
					if (zd != NULL) {
						stZWaveClass_t *zc = device_get_class(zd, 0, reqclass);
						if (zc != NULL) {
							if (!zwave_ccdb_exsit()) {
								if (zc->version < 0) {
									zc->version = version;
									stZWClass_t *c = zcc_get_class(zc->classid, zc->version);
									if (c != NULL) {
										char cmds[MAX_CMD_NUM];
										int cmdcnt = zcc_get_class_cmd_rpt(c, cmds);
										if (cmdcnt > 0) {
											device_add_cmds(zc, cmdcnt, cmds);
										}
									}
								} 
							} else {
								if (zc->version < 0) {
									zc->version = version;

									char cmds[MAX_CMD_NUM];
									int cmdcnt = zwave_ccdb_get_class_cmd_rpt(zc->classid, version, cmds);
									if (cmdcnt > 0) {
										device_add_cmds(zc, cmdcnt, cmds);
									}
								} 
							}
						}
						zs_class_version_get_success(zd, reqclass, version);
					}
				}
				break;
			}
		}
		break;

		case COMMAND_CLASS_ASSOCIATION: {
			switch (pCmd->ZW_Common.cmd) {
				case ASSOCIATION_GROUPINGS_REPORT: {
					ZW_ASSOCIATION_GROUPINGS_REPORT_FRAME *zg = (ZW_ASSOCIATION_GROUPINGS_REPORT_FRAME*)pCmd;
					char ars = zg->supportedGroupings;
					stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
					if (zd != NULL) {
						unsigned char class = zg->cmdClass&0xff;
						stZWaveClass_t *zcls = device_get_class(zd, 0, class);
						if (zcls != NULL) {
							unsigned char cmd = zg->cmd&0xff;
							stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmd);
							if (zcmd != NULL) {
								device_update_cmds_data(zcmd, &ars, 1);
								//ds_update_cmd_data(zcmd);
								int zs_assoc_grp_get_success(stZWaveDevice_t *zd);
								zs_assoc_grp_get_success(zd);
							}
						}
					}
				}
				break;
				case ASSOCIATION_REPORT: {
					ZW_ASSOCIATION_REPORT_1BYTE_FRAME *zar1 = (ZW_ASSOCIATION_REPORT_1BYTE_FRAME*)pCmd;
					char anid = zar1->nodeid1&0xff;
					stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
					if (zd != NULL) {
						unsigned char class = zar1->cmdClass&0xff;
						stZWaveClass_t *zcls = device_get_class(zd, 0, class);
						if (zcls != NULL) {
							unsigned char cmd = zar1->cmd&0xff;
							stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmd);
							if (zcmd != NULL && (anid == 0x01)) {
								device_update_cmds_data(zcmd, &anid, 1);
								zs_assoc_success(zd);
							}
						}
					}
				}
				break;
			}
		}
		break;

		default:
		break;
	}
#endif
}
void SerialAPIStarted(BYTE *pData, BYTE pLen) {
	log_debug( " ");
}



///////////////////////////////////////////////////////////////////////////////
static void AddNodeStatusUpdate(LEARN_INFO* inf) {
	log_debug("AddNodeStatusUpdate status=%d info len %d\n",inf->bStatus,inf->bLen);

	switch(inf->bStatus) {
		case ADD_NODE_STATUS_LEARN_READY:
			include_exclude_ing = 1;
			system_led_blink("zigbee", 500, 500);
			break;
		case ADD_NODE_STATUS_NODE_FOUND:
			break;
		case ADD_NODE_STATUS_ADDING_SLAVE:
		case ADD_NODE_STATUS_ADDING_CONTROLLER:
			if(inf->bLen) {
				log_debug("Node added with nodeid %d", inf->bSource );
				{
					log_debug("LEARN INFO: status:%02X", inf->bStatus&0xff);
					log_debug("LEARN INFO: source:%02X", inf->bSource&0xff);
					log_debug("LEARN CMDS: basic:%02x, generic:%02X, specific:%02X\n", inf->pCmd[0]&0xff, inf->pCmd[1]&0xff, inf->pCmd[2]&0xff);
					log_debug_hex("LEARN CLAS:", inf->pCmd + 3, inf->bLen-3);
					device_add(inf->bSource, inf->pCmd[0]&0xff, inf->pCmd[1]&0xff,inf->pCmd[2]&0xff, 0, 0, (inf->bLen-3)&0xff,	(char *)(inf->pCmd+3));
					stZWaveDevice_t *zd = device_get_by_nodeid(inf->bSource&0xff);
					if (zd != NULL) {
						log_debug("start device state func for %02X", zd->bNodeID&0xff);
						schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
					} else {
						log_warn("zd is null!");
					}
				}
			}
			break;
		case ADD_NODE_STATUS_PROTOCOL_DONE:
			ZW_AddNodeToNetwork(ADD_NODE_STOP,AddNodeStatusUpdate);
			{
				log_debug("LEARN INFO: status:%02X\n", inf->bStatus&0xff);
				log_debug("LEARN INFO: source:%02X\n", inf->bSource&0xff);
			}
			break;
		case ADD_NODE_STATUS_DONE:
		case ADD_NODE_STATUS_FAILED:
		case ADD_NODE_STATUS_NOT_PRIMARY:
			include_exclude_ing = 0;
			system_led_off("zigbee");
			break;
	}
}


void RemoveNodeStatusUpdate(LEARN_INFO* inf) {
	log_debug("RemoveNodeStatusUpdate status=%d, inf->nodeid:%02X\n",inf->bStatus, inf->bSource&0xff);

	switch(inf->bStatus) {
		case ADD_NODE_STATUS_LEARN_READY:
			system_led_blink("zigbee", 500, 500);
			include_exclude_ing = 1;
			break;
		case REMOVE_NODE_STATUS_NODE_FOUND:
			break;
		case REMOVE_NODE_STATUS_REMOVING_SLAVE:
			{
				log_debug("Removing node %d\n",inf->bSource);
				stZWaveDevice_t *zd = device_get_by_nodeid(inf->bSource&0xff);
				if (zd != NULL) {
					schedue_del(&zd->task);
					ds_del_device(zd);
					device_del(inf->bSource);
					uproto_rpt_unregister_dusun(device_get_extaddr(zd));
				}
			}
			break;
		case REMOVE_NODE_STATUS_DONE:
		case REMOVE_NODE_STATUS_FAILED:
			include_exclude_ing = 0;
			system_led_off("zigbee");
			break;
	}

}

void SetDefaultUpdate(){
	log_debug("Set Default done\n");
}


///////////////////////////////////////////////////////////////////////
int zwave_include() {
	ZW_AddNodeToNetwork(ADD_NODE_ANY|ADD_NODE_OPTION_NETWORK_WIDE,AddNodeStatusUpdate);
	return 0;
}

int zwave_exclude() {
	ZW_RemoveNodeFromNetwork(REMOVE_NODE_ANY,RemoveNodeStatusUpdate);
	return 0;
}

int zwave_reboot() {
	ZW_SoftReset();
	return 0;
}

int zwave_region(int region) {
#if 0
**       EU          -    0x00
**       US          -    0x01
**       ANZ         -    0x02
**       HK          -    0x03
**       Malaysia    -    0x04
**       India       -    0x05
**       Israel      -    0x06
**       Russia      -    0x07
**       China       -    0x08
**       Japan       -    0x20
**       Korea       -    0x21
#endif
	
	switch (region&0xff) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x20:
		case 0x21:
			{
			int retreg = ZW_RFRegionSet(region&0xff)&0xff;
			system_set_zwave_region(region&0xff);
			return retreg;
			}
		default:
			log_warn("not support region:%08X", region&0xff);
			break;
	}

	return -1;
}

static stSchduleTask_t remove_failed_node_task;
void zwave_remove_failed_node_complete(BYTE ret) {
	log_debug(" ret:%02X", ret);
}
void zwave_remove_failed_node_func(void *arg) {
	int i = 0;
	extern stZWaveCache_t zc;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);
	for (i = 0; i < cnt; i++) {
		stZWaveDevice_t *zd = &zc.devs[i];
		if (zd->used == 0) {
			continue;
		}
		if (zd->state != DS_REMOVED) {
			continue;
		}
	
		int ret = ZW_RemoveFailedNode(zd->bNodeID, zwave_remove_failed_node_complete);
		if (ret == ZW_FAILED_NODE_NOT_FOUND) {
			schedue_del(&zd->task);
			ds_del_device(zd);
			device_del(zd->bNodeID);
			schedue_add(&remove_failed_node_task, 200, zwave_remove_failed_node_func, NULL);
			//uproto_rpt_unregister_dusun(device_get_extaddr(zd));
			return;
		}
		
		schedue_add(&remove_failed_node_task, 8000, zwave_remove_failed_node_func, NULL);
	}
}
int zwave_remove_failed_node_start() {
	schedue_add(&remove_failed_node_task, 200, zwave_remove_failed_node_func, NULL);
	return 0;
}
int zwave_remove_failed_node(char *mac) {
	/* TODO */
	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		return 0;
	}

	zd->state = DS_REMOVED;
	int offset = (char *)&zd->state - (char *)zd;
	ds_update_dev_member(zd, offset, (char *)&zd->state, sizeof(zd->state));

	zwave_remove_failed_node_start();
	return 0;
}

int zwave_version(char *version) {
	char ver[64];
	if (ZW_Version((BYTE *)ver) == 0) {
		return -1;
	}
	strcpy(version, ver);
	return 0;
}

int zwave_info(int *homeid, char *nodeid) {
	static int _homeid = 0x00;
	static char _nodeid = 0x00;

	if (_homeid == 0 || _nodeid == 0) {
		MemoryGetID((BYTE *)&homeid, (BYTE *)&_nodeid);
	}

	*homeid	= _homeid;
	*nodeid = _nodeid;

	return 0;
}

void zwave_class_cmd_completed(BYTE x, TX_STATUS_TYPE *status) {
	log_debug("x:%02X", x&0xff);
}
int zwave_class_cmd(unsigned char nid, unsigned char class, unsigned char cmd, char *data, int len,
										void (*completed)(BYTE x, TX_STATUS_TYPE *status)) {
	char buf[256] = {0};
	buf[0] = class&0xff;
	buf[1] = cmd&0xff;
	if (len > 0) {
		memcpy(buf + 2, data, len);
	}
	unsigned char options = 0x25;

	void (*f)(BYTE, TX_STATUS_TYPE *) = NULL;
	if (completed == NULL) {
		f = zwave_class_cmd_completed;
	} else {
		f = completed;
	}

	//if (ZW_SendData((BYTE)nid, (BYTE *)buf, 2 + len, options, zwave_class_cmd_completed) == FALSE) {
	if (ZW_SendData((BYTE)nid, (BYTE *)buf, 2 + len, options, f) == FALSE) {
		log_warn("full tx queue!");
		return -1;
	}
	return 0;
}
int zwave_ep_class_cmd(unsigned char nid, int ep,  unsigned char class, unsigned char cmd, char *data, int len,
										void (*completed)(BYTE x, TX_STATUS_TYPE *status)) {
	log_debug("len:%d, data:%p, class:%02X, cmd:%02x", len, data, class&0xff, cmd&0xff);
	
	char buf[256] = {0};
	if (ep == 0 || ep == 1) {
		log_debug("main");
		log_debug_hex("buf:", data, len);
		zwave_class_cmd(nid, class, cmd, data, len, completed);
	} else {
		log_debug("other");
		buf[0] = 0x00;
		buf[1] = (ep)&0xff;
		buf[2] = class;
		buf[3] = cmd;
		class = 0x60;
		cmd   = 0x0D;
		if (len > 0) {
			memcpy(&buf[4], data, len);
			len = len + 4;
		} else {
			len = 4;
		}
		log_debug_hex("buf:", buf, len);
		zwave_class_cmd(nid, class, cmd, buf, len, completed);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
int version_report_hanlder(unsigned sourceNode, unsigned char *pCmd, unsigned cmdLength) {
	log_debug(" ");
	ZW_VERSION_COMMAND_CLASS_REPORT_FRAME *zv = (ZW_VERSION_COMMAND_CLASS_REPORT_FRAME*)pCmd;
	int version = zv->commandClassVersion&0xff;
	unsigned char reqclass = zv->requestedCommandClass&0xff;

	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd != NULL) {
		stZWaveClass_t *zc = device_get_class(zd, 0, reqclass);
		if (zc != NULL) {
			if (!zwave_ccdb_exsit()) {
				if (zc->version < 0) {
					zc->version = version;
					stZWClass_t *c = zcc_get_class(zc->classid, zc->version);
					if (c != NULL) {
						char cmds[MAX_CMD_NUM];
						int cmdcnt = zcc_get_class_cmd_rpt(c, cmds);
						if (cmdcnt > 0) {
							device_add_cmds(zc, cmdcnt, cmds);
						}
					}
				} 
			} else {
				if (zc->version < 0) {
					zc->version = version;

					char cmds[MAX_CMD_NUM];
					int cmdcnt = zwave_ccdb_get_class_cmd_rpt(zc->classid, version, cmds);
					if (cmdcnt > 0) {
						device_add_cmds(zc, cmdcnt, cmds);
					}
				} 
			}
		}
		zs_class_version_get_success(zd, reqclass, version);
	}
	return 0;
}
int assoc_grps_report_handler(unsigned sourceNode, unsigned char *pCmd, unsigned cmdLength) {
	log_debug(" ");
	ZW_ASSOCIATION_GROUPINGS_REPORT_FRAME *zg = (ZW_ASSOCIATION_GROUPINGS_REPORT_FRAME*)pCmd;
	//char ars = zg->supportedGroupings;
	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd != NULL) {
		unsigned char class = zg->cmdClass&0xff;
		stZWaveClass_t *zcls = device_get_class(zd, 0, class);
		if (zcls != NULL) {
			unsigned char cmd = zg->cmd&0xff;
			stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmd);
			if (zcmd != NULL) {
				//device_update_cmds_data(zcmd, &ars, 1);
				//ds_update_cmd_data(zcmd);
				int zs_assoc_grp_get_success(stZWaveDevice_t *zd);
				zs_assoc_grp_get_success(zd);
			}
		}
	}
	return 0;
}
int assoc_report_handler(unsigned sourceNode,  unsigned char *pCmd, unsigned cmdLength) {
	log_debug(" ");
	ZW_ASSOCIATION_REPORT_1BYTE_FRAME *zar1 = (ZW_ASSOCIATION_REPORT_1BYTE_FRAME*)pCmd;
	char anid = zar1->nodeid1&0xff;
	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd != NULL) {
		unsigned char class = zar1->cmdClass&0xff;
		stZWaveClass_t *zcls = device_get_class(zd, 0, class);
		if (zcls != NULL) {
			unsigned char cmd = zar1->cmd&0xff;
			stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmd);
			if (zcmd != NULL && (anid == 0x01)) {
				//device_update_cmds_data(zcmd, &anid, 1);
				int zs_assoc_success(stZWaveDevice_t *zd);
				zs_assoc_success(zd);
			}
		}
	}
	return 0;
}

int class_cmd_generic_handler(unsigned sourceNode, unsigned char *_pCmd, unsigned cmdLength) {
	log_debug(" ");
	stZWaveDevice_t *zd = device_get_by_nodeid(sourceNode);
	if (zd != NULL) {
		ZW_APPLICATION_TX_BUFFER *pCmd = (ZW_APPLICATION_TX_BUFFER*)_pCmd;
		unsigned char class = pCmd->ZW_Common.cmdClass&0xff;
		stZWaveClass_t *zcls = device_get_class(zd, 0, class);
		if (zcls != NULL) {
			unsigned char cmd = pCmd->ZW_Common.cmd&0xff;
			stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmd);
			if (zcmd != NULL) {
				device_update_cmds_data(zcmd, (char *)(_pCmd + 2), (cmdLength - 2)&0xff);
				if (zd->state == DS_WORKED) {
					ds_update_cmd_data(zcmd);
				}

				if (zd->state == DS_READ_RPT_VALUEING) {
					device_set_read_rpt_cmd_by_class_cmd(zd, class, cmd);
				}

				int zs_read_rpt_cmd_success(stZWaveDevice_t *zd);
				zs_read_rpt_cmd_success(zd);
			}
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
void zwave_device_state_change(stZWaveDevice_t *zd, int new) {
	log_debug("state form [%02X] -> [%02X]", zd->state&0xff, new&0xff);
	zd->state = new;
}

int zs_class_version_get_success(stZWaveDevice_t *zd, unsigned char class, int versoin) {
	if (zd->state != DS_READ_VERSION_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_VERSION_SUCCSSS);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_class_version_get_failed(stZWaveDevice_t *zd, unsigned char class) {
	if (zd->state != DS_READ_VERSION_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_VERSION_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_class_version_get_timeout(stZWaveDevice_t *zd) {
	if (zd->state != DS_READ_VERSION_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_VERSION_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_class_version_get(unsigned char nid, unsigned char class) {
	return zwave_class_cmd(nid, 0x86, 0x13, (char *)&class, 1, NULL);
}


int zs_asso_failed(stZWaveDevice_t *zd) {
	if (zd->state != DS_ASSOCIATING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_ASSOCIATE_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_assoc_success(stZWaveDevice_t *zd) {
	if (zd->state != DS_ASSOCIATING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_ASSOCIATE_SUCCESS);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_assoc_timeout(stZWaveDevice_t *zd) {
	if (zd->state != DS_ASSOCIATING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_ASSOCIATE_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
/*
void zs_assoc_completed_callback(BYTE x, TX_STATUS_TYPE *ts) {
	char buf[2] = {gid, 0x01};
	int ret = zwave_class_cmd(zd->bNodeID, 0x85, 0x01, (char *)buf, 2, zs_assoc_completed_callback);
	if (ret != 0) {
		return -1;
	}
	return 0;
}
*/
int zs_assoc(stZWaveDevice_t *zd, unsigned char gid) {
	char buf[2] = {gid, 0x01};
	int ret = zwave_class_cmd(zd->bNodeID, 0x85, 0x01, buf, 2, NULL);
	if (ret != 0) {
		return -1;
	}
	ret = zwave_class_cmd(zd->bNodeID, 0x85, 0x02, (char *)&gid, 1, NULL);
	if (ret != 0) {
		return -2;
	}
	return 0;
}



int zs_assoc_grp_get_failed(stZWaveDevice_t *zd) {
	if (zd->state != DS_READ_ASSOC_GRP_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_ASSOC_GRP_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_assoc_grp_get_success(stZWaveDevice_t *zd) {
	if (zd->state != DS_READ_ASSOC_GRP_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_ASSOC_GRP_SUCCESS);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_assoc_grp_get_timeout(void *arg) {
	stZWaveDevice_t *zd = (stZWaveDevice_t *)arg;
	if (zd->state != DS_READ_ASSOC_GRP_ING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_ASSOC_GRP_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_assoc_grp_get(stZWaveDevice_t *zd) {
	int ret = zwave_class_cmd(zd->bNodeID, 0x85, 0x05, NULL, 0, NULL);
	if (ret != 0) {
		return -1;
	}
	return 0;
}

int zs_read_rpt_cmd_faild(stZWaveDevice_t *zd) {
	if (zd->state != DS_READ_RPT_VALUEING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_RPT_VALUE_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_read_rpt_cmd_success(stZWaveDevice_t *zd) {
	if (zd->state != DS_READ_RPT_VALUEING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_RPT_VALUE_SUCCESS);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_read_rpt_cmd_timeout(void *arg) {
	stZWaveDevice_t *zd = (stZWaveDevice_t *)arg;
	if (zd->state != DS_READ_RPT_VALUEING) {
		return 0;
	}
	zwave_device_state_change(zd, DS_READ_RPT_VALUE_FAILED);
	schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
	return 0;
}
int zs_read_rpt_cmd(stZWaveDevice_t *zd, unsigned char class, unsigned char cmd) {
	int ret = zwave_class_cmd(zd->bNodeID, class, cmd, NULL, 0, NULL);
	if (ret != 0) {
		return -1;
	}
	return 0;
}


void zwave_device_state_task_func(void *arg) {
	stZWaveDevice_t *zd = (stZWaveDevice_t *)arg;
	log_debug( " ");
	if (zd == NULL) {
		return;
	}
	if (zd->used == 0) {
		return;
	}

	switch (zd->state) {
		case DS_ADDED:
			zwave_device_state_change(zd, DS_READ_VERSION);
			zd->trycnt = 0;
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;
		case DS_READ_VERSION:
			{
				int class = device_get_class_no_version(zd);
				if (class < 0) {
					zd->trycnt = 0;
					zwave_device_state_change(zd, DS_READ_ASSOC_GRP);
					schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
				} else {
					zd->tvar = class;
					if (zs_class_version_get(zd->bNodeID, class&0xff) != 0) {
						zwave_device_state_change(zd, DS_READ_VERSION_FAILED);
						schedue_add(&zd->task, 1000, zwave_device_state_task_func, zd);
					} else {
						zwave_device_state_change(zd, DS_READ_VERSION_ING);
						schedue_add(&zd->task, 2500, zs_class_version_get_timeout, zd);
					}
				}
			}
			break;
		case DS_READ_VERSION_ING:
			break;
		case DS_READ_VERSION_SUCCSSS:	
			zwave_device_state_change(zd, DS_READ_VERSION);
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;
		case DS_READ_VERSION_FAILED:
			if (zd->trycnt < 3) {
				zd->trycnt++;
			} else {
				log_warn("try get class(%02X) version failed, default to 0", zd->tvar&0xff);
				stZWaveClass_t *zc = device_get_class(zd, 0, zd->tvar&0xff);
				if (zc != NULL) {
					zc->version = 0;
					zd->trycnt = 0;
				}
			}
			zwave_device_state_change(zd, DS_READ_VERSION);
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;

		case DS_READ_ASSOC_GRP:
			if (!device_exsit_assoc_class(zd)) {
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			} else {
				if (zs_assoc_grp_get(zd) != 0) {
					zwave_device_state_change(zd, DS_READ_ASSOC_GRP_FAILED);
					schedue_add(&zd->task, 1000, zwave_device_state_task_func, zd);
				} else {
					zwave_device_state_change(zd, DS_READ_ASSOC_GRP_ING);
					schedue_add(&zd->task, 2500, zs_assoc_grp_get_timeout, zd);
				}
			}
			break;
		case DS_READ_ASSOC_GRP_ING:	
			break;
		case DS_READ_ASSOC_GRP_SUCCESS:
			zd->trycnt = 0;
			zwave_device_state_change(zd, DS_ASSOCIATE);
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;
		case DS_READ_ASSOC_GRP_FAILED:
			if (zd->trycnt < 3) {
				zd->trycnt++;
				zwave_device_state_change(zd, DS_READ_ASSOC_GRP);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			} else {
				zwave_device_state_change(zd, DS_ASSOCIATE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
				zd->trycnt = 0;
			}
			break;

		case DS_ASSOCIATE:
			if (!device_exsit_assoc_class(zd)) {
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			} else {
				/* TODO assoc all  group */
				if (zs_assoc(zd, 0x01) != 0) {
					zwave_device_state_change(zd, DS_ASSOCIATE_FAILED);
					schedue_add(&zd->task, 1000, zwave_device_state_task_func, zd);
				} else {
					zwave_device_state_change(zd, DS_ASSOCIATING);
					schedue_add(&zd->task, 2500, zs_assoc_timeout, zd);
				}
			}
			break;
		case DS_ASSOCIATING:
			break;
		case DS_ASSOCIATE_SUCCESS:
			zwave_device_state_change(zd, DS_READ_RPT_VALUE);
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;
		case DS_ASSOCIATE_FAILED:
			if (zd->trycnt < 3) {
				zd->trycnt++;
				zwave_device_state_change(zd, DS_ASSOCIATE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			} else {
				zd->trycnt = 0;
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			}
			break;

		case DS_READ_RPT_VALUE:
#if 1
			{
				static stReadRptCmd_t rrcs[] = {
					{0x80, 0x02, 0x03, 0}, // battery	
					/*{0x80, 0x02, 0x04, 0}, // battery	 */
					{0x30, 0x02, 0x03, 0}, // sensor type binary */
					{0x25, 0x02, 0x03, 0}, 
					{0x31, 0x04, 0x05, 0},
					{0x86, 0x11, 0x12, 0}, // version
					{0x73, 0x02, 0x03, 0}, // version
					{0x71, 0x07, 0x08, 0}, // notification */
					{0x60, 0x07, 0x08, 0},
					/*{0x71, 0x30, 0}, // notification */
					/* {0x84, 0x04, 0x04, 0}, // wake up interval set	 */
					{0x84, 0x05, 0x06, 0}, // wake up interval get
				};
				if (zd->rrcs_size == 0) {
					log_debug("set init rrcs to device!!! ");
					zd->rrcs_size = sizeof(rrcs)/sizeof(rrcs[0]);
					memcpy(&zd->rrcs[0], &rrcs[0], sizeof(rrcs));
				}	 

				unsigned char class =0x00, cmd = 0x00;
				if (device_get_read_rpt_cmd(zd, &class, &cmd) < 0) {
					zd->trycnt = 0;
					zwave_device_state_change(zd, DS_WORKED);
					ds_add_device(zd);
					schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
					uproto_rpt_register_dusun(device_get_extaddr(zd));
				} else {
					log_debug("read class:%02X, cmd:%02X...", class&0xff, cmd&0xff);
					if (zs_read_rpt_cmd(zd, class, cmd) != 0) {
						zwave_device_state_change(zd, DS_READ_RPT_VALUE_FAILED);
						schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
					} else {
						zwave_device_state_change(zd, DS_READ_RPT_VALUEING);
						schedue_add(&zd->task, 3000, zs_read_rpt_cmd_timeout, zd);
					}
				}
			}
#else
			zd->trycnt = 0;
			zwave_device_state_change(zd, DS_WORKED);
			ds_add_device(zd);
			schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			uproto_rpt_register_dusun(device_get_extaddr(zd));
#endif
			break;
		case DS_READ_RPT_VALUEING:
			break;
		case DS_READ_RPT_VALUE_SUCCESS:
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			break;
		case DS_READ_RPT_VALUE_FAILED:
			if (zd->trycnt < 3) {
				zd->trycnt++;
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			} else {
				zd->trycnt = 0;
				device_set_read_rpt_cmd(zd);
				zwave_device_state_change(zd, DS_READ_RPT_VALUE);
				schedue_add(&zd->task, 10, zwave_device_state_task_func, zd);
			}
	
			break;

		case DS_WORKED:
			{
				int delt = schedue_current() - zd->last;
				if (delt > 45 * 60 * 1000 && delt < 60 * 60 * 1000) {
					uproto_rpt_status_dusun(device_get_extaddr(zd));
				}

				schedue_add(&zd->task, 15 * 60 * 1000, zwave_device_state_task_func, zd);
			}
			break;
	}
}

void zwave_device_state_init() {
	int i = 0;
	extern stZWaveCache_t zc;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);
	for (i = 0; i < cnt; i++) {
		stZWaveDevice_t *zd = &zc.devs[i];
		if (zd->used == 0) {
			continue;
		}
		if (zd->state != DS_WORKED) {
			continue;
		}
	
		zd->last = schedue_current();
		schedue_add(&zd->task, 15 * 60 * 1000, zwave_device_state_task_func, zd);
	}
}
