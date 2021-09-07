#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zwave_device_storage.h"

static stDsHeader_t *g_dh = NULL;
static char *g_path = "/etc/config/dusun/zwdev/zwdev.db";


static int ds_create_dir(const char *path) {
#if 0
	return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
#else
	char cmd[256];
	sprintf(cmd, "mkdir -p %s", path);
	int ret = system(cmd);
	ret = ret;
#endif
	return 0;
}

static int ds_write_header(const char *path, stDsHeader_t *dh) {
	FILE *fp = fopen(path, "r+");
	if (fp == NULL) {
		char tpath[256];
		strcpy(tpath, path);
		char *xx = tpath + strlen(tpath);
		while (*xx != '/') xx--;
		*xx = '\0';

		ds_create_dir(tpath);

		fp = fopen(path, "w");
		if (fp == NULL) {
			return -1;
		}
	} 

	fseek(fp, 0, SEEK_SET);
	fwrite(&dh->ecnt, 4, 1, fp);
	int i = 0;
	for (i = 0; i < dh->ecnt; i++) {
		fwrite(&dh->elems[i].tag, 4, 1, fp);
		fwrite(&dh->elems[i].maxnum, 4, 1, fp);
		fwrite(&dh->elems[i].num, 4, 1, fp);
		fwrite(&dh->elems[i].size, 4, 1, fp);
		fwrite(dh->elems[i].map, BIT_BYPE_NUM(dh->elems[i].maxnum), 1, fp);
	}

	fclose(fp);
	return 0;
}

static int ds_free_header(stDsHeader_t *dh) {
	if (dh == NULL) {
		return -1;
	}

	int i = 0; 
	if (dh->elems != NULL) {
		for (i = 0; i < dh->ecnt; i++) {
			if (dh->elems[i].map != NULL) {
				free(dh->elems[i].map);
				dh->elems[i].map = NULL;
			}
		}
		free(dh->elems);
		dh->elems = NULL;
	}

	free(dh);

	return 0;
}


static stDsHeader_t *ds_read_header(const char *path) {
	stDsHeader_t *dh = (stDsHeader_t*)malloc(sizeof(stDsHeader_t));
	if (dh == NULL) {
		return  NULL;
	}
	memset(dh, 0, sizeof(stDsHeader_t));

	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		ds_free_header(dh);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);

	int ret = fread(&dh->ecnt, 4, 1, fp);
	ret = ret;
	int i = 0;
	if (dh->ecnt > 0) {
		dh->elems = (stDsElem_t*)malloc(sizeof(stDsElem_t)*dh->ecnt);
	}
	for (i = 0; i < dh->ecnt; i++) {
		ret = fread(&dh->elems[i].tag, 4, 1, fp);
		ret = fread(&dh->elems[i].maxnum, 4, 1, fp);
		ret = fread(&dh->elems[i].num, 4, 1, fp);
		ret = fread(&dh->elems[i].size, 4, 1, fp);
		dh->elems[i].map = malloc(BIT_BYPE_NUM(dh->elems[i].maxnum));
		ret = fread(dh->elems[i].map, BIT_BYPE_NUM(dh->elems[i].maxnum), 1, fp);
		ret = ret;
	}

	fclose(fp); 

	return dh;
}

static void ds_view_header() {
	stDsHeader_t *dh = g_dh;
	if (dh == NULL) {
		return;
	}

	printf("ecnt : %d\n", dh->ecnt);
	int i = 0;
	for (i = 0; i < dh->ecnt; i++) {
		printf("tag: %c, maxnum: %d, num: %d, size : %d, map:\n",
			dh->elems[i].tag, 
			dh->elems[i].maxnum, 
			dh->elems[i].num, 
			dh->elems[i].size);
		int j = 0;
		for (j = 0; j < BIT_BYPE_NUM(dh->elems[i].maxnum); j++) {
			printf("[%02X] ", dh->elems[i].map[j]&0xff);
			if ( (j + 1) % 20 == 0) {
				printf("\n");
			}
		}
		printf("\n");
	}
}


static stDsHeader_t *ds_create_header(int ecnt, stDsSize_t *esizes) {
	stDsHeader_t *dh = (stDsHeader_t*)malloc(sizeof(stDsHeader_t));
	if (dh == NULL) {
		return  NULL;
	}

	dh->ecnt	= ecnt;
	dh->elems	= (stDsElem_t*)malloc(sizeof(stDsElem_t) * ecnt);
	int i = 0;
	for (i = 0; i < ecnt; i++) {
		dh->elems[i].tag		= esizes[i].tag;
		dh->elems[i].maxnum = esizes[i].maxnum;
		dh->elems[i].num		= 0;
		dh->elems[i].size		= esizes[i].size;
		
		int	bit_byte_num		= BIT_BYPE_NUM(dh->elems[i].maxnum);
		dh->elems[i].map		= malloc(bit_byte_num);
		memset(dh->elems[i].map, 0, bit_byte_num);
	}

	return dh;
}


static int ds_create_file(const char *path, int ecnt, stDsSize_t *esizes) {
	stDsHeader_t *dh = ds_create_header(ecnt, esizes);
	if (dh == NULL) {
		return -1;
	}

	if (ds_write_header(path, dh) < 0) {
		ds_free_header(dh);
		dh = NULL;
		return -2;
	}

	g_dh = dh;

	return 0;
}

static int ds_file_exsit(const char *path) {
	if (access(path, F_OK) != 0) {
		return 0;		// not exsit
	}

	return 1;
}

static stDsElem_t *ds_match_elem(int tag) {
	stDsHeader_t *dh = g_dh;
	int i = 0;
	for (i = 0; i < dh->ecnt; i++) {
		if (dh->elems[i].tag == tag) {
			return &dh->elems[i];
		}
	}
	return NULL;
}

static void ds_map_set_bit(char *map, int nbit) {
	map[nbit/8] |= 1 << (nbit%8);
}
static void ds_map_clr_bit(char *map, int nbit) {
	map[nbit/8] &= ~(1 << (nbit%8));
}

static int ds_map_get_bit(char *map, int nbit) {
	return (map[nbit/8]>>(nbit%8))&0x1;
}

static int ds_map_find_first_free_bit(char *map, int maxbitnum) {
	int i = 0; 
	for (i = 0; i < maxbitnum; i++) {
		int bitval = ds_map_get_bit(map, i);
		//int bitval = (map[i/8] >> (i%8))&0x1;
		if (bitval == 0) {
			return i;
		}
	}
	return -1;
}

static int ds_tag_size(int tag) {
	stDsElem_t *de = ds_match_elem(tag);
	if (de == NULL) {
		return 0;
	}
	return de->size;
}

static int ds_header_size() {
	stDsHeader_t *dh = g_dh;
	int size = 0;

	size += sizeof(dh->ecnt);
	int i = 0;
	for (i = 0;i  < dh->ecnt; i++) {
		size += sizeof(dh->elems[i].tag); 
		size += sizeof(dh->elems[i].maxnum); 
		size += sizeof(dh->elems[i].num); 
		size += sizeof(dh->elems[i].size); 
		size += BIT_BYPE_NUM(dh->elems[i].maxnum);
	}

	return size;
}

static int ds_tag_start(int tag) {
	/* header + D +  E +  C +  A */
	int off = 0;
	int i = 0;
	stDsHeader_t *dh = g_dh;
	for (i = 0, off = ds_header_size(); i < dh->ecnt; i++) {
		if (dh->elems[i].tag == tag) {
			break;
		}
		off += dh->elems[i].size * dh->elems[i].maxnum;
	}
	return off;
}

static int ds_offset(int tag, int nbit) {
	int ret = ds_tag_start(tag) + nbit * ds_tag_size(tag);
	//printf("offset : %d\n", ret);
	return ret;
}


static int ds_malloc(int tag) {
	stDsElem_t *de = ds_match_elem(tag);
	if (de == NULL) {
		return -1;
	}
	int free_bit = ds_map_find_first_free_bit(de->map, de->maxnum);
	if (free_bit < 0) {
		return -2;
	}
	ds_map_set_bit(de->map, free_bit);
	return free_bit;
}

static int ds_free(int tag, int nbit) {
	stDsElem_t *de = ds_match_elem(tag);
	if (de == NULL) {
		return -1;
	}
	ds_map_clr_bit(de->map, nbit);
	return 0;
}
/////////////////////////////////////////////////////////////////////////
// device 
static int ds_free_command(stZWaveCommand_t *command) {
	ds_free('M', command->nbit);
	command->nbit = -1;
	command->nbit_next = -1;
	return 0;
}

static int ds_free_class(stZWaveClass_t *class) {
	ds_free('C', class->nbit);
	class->nbit = -1;
	class->nbit_next = -1;
	class->nbit_cmds_head = -1;

	if (class->cmdcnt > 0) {
		int i = 0;
		for (i = 0; i < class->cmdcnt; i++) {
			ds_free_command(&class->cmds[i]);
		}
	}
	return 0;
}

static int ds_free_ep(stZWaveEndPoint_t *ep) {
	ds_free('E', ep->nbit);
	ep->nbit = -1;
	ep->nbit_next = -1;
	ep->nbit_classes_head = -1;

	if (ep->classcnt > 0) {
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_free_class(&ep->classes[i]);
		}
	}

	return 0;
}

static int ds_free_device(stZWaveDevice_t *dev) {
	ds_free('D', dev->nbit);
	dev->nbit = -1;
	dev->nbit_subeps_head = -1;

	if (dev->root.classcnt > 0) {
		stZWaveEndPoint_t *ep = &dev->root;
		ep->nbit_classes_head = -1;
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_free_class(&ep->classes[i]);
		}
	} else {
		dev->root.nbit_classes_head = -1;
	}

	if (dev->subepcnt > 0) {
		int i = 0; 
		for (i = 0; i < dev->subepcnt; i++) {
			ds_free_ep(&dev->subeps[i]);
		}
	} 

	return 0;
}

static int ds_malloc_command(stZWaveCommand_t *command) {
	command->nbit = ds_malloc('M');
	return 0;
}

static int ds_malloc_class(stZWaveClass_t *class) {
	class->nbit = ds_malloc('C');

	if (class->cmdcnt > 0) {
		int i = 0;
		for (i = 0; i < class->cmdcnt; i++) {
			ds_malloc_command(&class->cmds[i]);

			if (i == 0) {
				class->nbit_cmds_head = class->cmds[i].nbit;
			} else {
				class->cmds[i-1].nbit_next = class->cmds[i].nbit;
			}

			if (i == class->cmdcnt-1) {
				class->cmds[i].nbit_next = -1;
			}
			//printf("cmd %02X, nbit:%d, nbit_next:%d\n", class->cmds[i].cmdid&0xff, class->cmds[i].nbit, class->cmds[i].nbit_next);
		}
	} else {
		class->nbit_cmds_head = -1;
	}

	return 0;
}

static int ds_malloc_ep(stZWaveEndPoint_t *ep) {
	ep->nbit = ds_malloc('E');

	if (ep->classcnt > 0) {
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_malloc_class(&ep->classes[i]);
			if (i == 0) {
				ep->nbit_classes_head = ep->classes[i].nbit;
			} else {
				ep->classes[i-1].nbit_next = ep->classes[i].nbit;
			}

			if (i == ep->classcnt-1) {
				ep->classes[i].nbit_next = -1;
			} 
		}
	} else {
		ep->nbit_classes_head = -1;
	}

	return 0;
}

static int ds_malloc_device(stZWaveDevice_t *dev) {
	dev->nbit = ds_malloc('D');


	if (dev->root.classcnt > 0) {
		stZWaveEndPoint_t *ep = &dev->root;
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_malloc_class(&ep->classes[i]);
			if (i == 0) {
				ep->nbit_classes_head = ep->classes[i].nbit;
			} else {
				ep->classes[i-1].nbit_next = ep->classes[i].nbit;
			}

			if (i == ep->classcnt-1) {
				ep->classes[i].nbit_next = -1;
			} 
		}
	} else {
		dev->root.nbit_classes_head = -1;
	}

	if (dev->subepcnt > 0) {
		int i = 0; 
		for (i = 0; i < dev->subepcnt; i++) {
			ds_malloc_ep(&dev->subeps[i]);

			if (i == 0) {
				dev->nbit_subeps_head = dev->subeps[i].nbit;
			} else {
				dev->subeps[i-1].nbit_next = dev->subeps[i].nbit;
			}

			if (i == dev->subepcnt-1) {
				dev->subeps[i].nbit_next = -1;
			}
		}
	} else {
		dev->nbit_subeps_head = -1;
	}

	return 0;
}

static int ds_save_command(FILE *fp, stZWaveCommand_t *command) {
	fseek(fp, ds_offset('M', command->nbit), SEEK_SET);
	fwrite(command, ds_tag_size('M'), 1, fp);
	//printf("------- cmd nbit:%d, , nbit_next:%d\n", command->nbit, command->nbit_next);
	return 0;
}

static int ds_save_command_data(FILE *fp, stZWaveCommand_t *command) {
	fseek(fp, ds_offset('M', command->nbit) , SEEK_SET);
	fwrite(command, ds_tag_size('M'), 1, fp);
	return 0;
}


static int ds_save_class(FILE *fp, stZWaveClass_t *class) {
	fseek(fp, ds_offset('C', class->nbit), SEEK_SET);
	fwrite(class, ds_tag_size('C'), 1, fp);

	if (class->cmdcnt > 0) {
		int i = 0;
		for (i = 0; i < class->cmdcnt; i++) {
			ds_save_command(fp, &class->cmds[i]);
		}
	}
	return 0;
}

static int ds_save_ep(FILE *fp, stZWaveEndPoint_t *ep) {
	fseek(fp, ds_offset('E', ep->nbit), SEEK_SET);
	fwrite(ep, ds_tag_size('E'), 1, fp);

	if (ep->classcnt > 0) {
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_save_class(fp, &ep->classes[i]);
		}
	} 
	return 0;
}

static int ds_save_device(FILE *fp, stZWaveDevice_t *dev) {

	fseek(fp, ds_offset('D', dev->nbit), SEEK_SET);
	fwrite(dev, ds_tag_size('D'), 1, fp);

	if (dev->root.classcnt > 0) {
		stZWaveEndPoint_t *ep = &dev->root;
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			ds_save_class(fp, &ep->classes[i]);
		}
	}


	if (dev->subepcnt > 0) {
		int i = 0;
		for (i = 0; i < dev->subepcnt; i++) {
			ds_save_ep(fp, &dev->subeps[i]);
		}
	}
	
	return 0;
}

static int ds_load_command(FILE *fp, int nbit, stZWaveCommand_t *cmd) {
	fseek(fp, ds_offset('M', cmd->nbit), SEEK_SET);
	int ret = fread(cmd, ds_tag_size('M'), 1, fp);
	ret = ret;
	//printf("------- cmd nbit:%d, , nbit_next:%d\n", cmd->nbit, cmd->nbit_next);
	return 0;
}

static int ds_load_class(FILE *fp, int nbit, stZWaveClass_t *class) {
	fseek(fp, ds_offset('C', class->nbit), SEEK_SET);
	int ret = fread(class, ds_tag_size('C'), 1, fp);
	ret = ret;

	if (class->cmdcnt > 0) {
		int i = 0;
		class->cmds = (stZWaveCommand_t*)malloc(class->cmdcnt * sizeof(stZWaveCommand_t));
		for (i = 0; i < class->cmdcnt; i++) {
			if (i == 0) {
				class->cmds[i].nbit = class->nbit_cmds_head;
			} else  {
				class->cmds[i].nbit = class->cmds[i-1].nbit_next;
			}
			ds_load_command(fp, class->cmds[i].nbit, &class->cmds[i]);
		}
	} 

	return 0;
}

static int ds_load_ep(FILE *fp, int nbit, stZWaveEndPoint_t *ep) {
	fseek(fp, ds_offset('E', ep->nbit), SEEK_SET);
	int ret = fread(ep, ds_tag_size('E'), 1, fp);
	ret = ret;

	if (ep->classcnt > 0) {
		int i = 0;
		ep->classes = (stZWaveClass_t*)malloc(ep->classcnt * sizeof(stZWaveClass_t));
		for (i = 0; i < ep->classcnt; i++) {
			if (i == 0) {
				ep->classes[i].nbit = ep->nbit_classes_head;
			} else  {
				ep->classes[i].nbit = ep->classes[i-1].nbit_next;
			}
			ds_load_class(fp, ep->classes[i].nbit, &ep->classes[i]);
		}
	} 

	return 0;
}
static int ds_load_device(FILE *fp, int nbit, stZWaveDevice_t *dev) {
	fseek(fp, ds_offset('D', nbit), SEEK_SET);
	int ret = fread(dev, ds_tag_size('D'), 1, fp);
	ret = ret;

	if (dev->root.classcnt > 0) {
		int i = 0;
		stZWaveEndPoint_t *ep = &dev->root;
		ep->classes = (stZWaveClass_t*)malloc(ep->classcnt * sizeof(stZWaveClass_t));
		for (i = 0; i < ep->classcnt; i++) {
			if (i == 0) {
				ep->classes[i].nbit = ep->nbit_classes_head;
			} else  {
				ep->classes[i].nbit = ep->classes[i-1].nbit_next;
			}
			ds_load_class(fp, ep->classes[i].nbit, &ep->classes[i]);
		}
	} else {
		dev->root.nbit_classes_head = -1;
	}

	if (dev->subepcnt > 0) {
		int i = 0;
		dev->used			= 1;

		if (dev->subepcnt > 0) {
			dev->subeps = (stZWaveEndPoint_t*)malloc(sizeof(stZWaveEndPoint_t) * dev->subepcnt);
			for (i = 0; i < dev->subepcnt; i++) {
				if (i == 0) {
					dev->subeps[i].nbit = dev->nbit_subeps_head;
				} else {
					dev->subeps[i].nbit = dev->subeps[i-1].nbit_next;
				}
				ds_load_ep(fp, dev->subeps[i].nbit, &dev->subeps[i]);
			}
		} else {
			dev->subeps = NULL;
		}
	}

	return 0;
}

// other

// util
static char * ds_dev_map() {
	stDsHeader_t *dh = g_dh;
	int i = 0;
	for (i = 0; i < dh->ecnt; i++) {
		if (dh->elems[i].tag == 'D') {
			return dh->elems[i].map;
		}
	}
	return NULL;
}
static int ds_dev_maxnum() {
	stDsHeader_t *dh = g_dh;
	int i = 0;
	for (i = 0; i < dh->ecnt; i++) {
		if (dh->elems[i].tag == 'D') {
			return dh->elems[i].maxnum;
		}
	}
	return 0;
}

//interface 
int ds_init(const char *path) {
	stDsSize_t dss[] = {
		{MAX_DEV_NUM,					sizeof(stZWaveDevice_t),								'D'},
		{MAX_DEV_NUM*5,				sizeof(stZWaveEndPoint_t),							'E'},
		{MAX_DEV_NUM*5*40,		sizeof(stZWaveClass_t),									'C'},
		{MAX_DEV_NUM*5*40*40, sizeof(stZWaveCommand_t),								'M'},
	};


	g_path = (char *)path;

	if (!ds_file_exsit(path)) {
		printf("create new db ...: %s\n", path);
		if (ds_create_file(path, sizeof(dss)/sizeof(dss[0]), dss) != 0) {
			return -1;
		}
		return 0;
	} else {
		printf("use old db : %s\n", path);
	}

	printf("read head from %s\n", path);
	g_dh = ds_read_header(path);
	if (g_dh == NULL) {
		return -2;
	}

	//ds_view_header();

	return 0;
}
int ds_load_alldevs(stZWaveDevice_t *devs) {
	char *map			= ds_dev_map();
	int  maxnum		= ds_dev_maxnum();

	printf("[%s] loading devices ...\n", __func__);

	FILE *fp = fopen(g_path, "r+");

	int i = 0;
	int j = 0;
	for (i = 0; i < maxnum; i++) {
		int bitval = ds_map_get_bit(map, i);
		if (bitval == 1) {
			ds_load_device(fp, i, &devs[j++]);
		}
	}

	fclose(fp);

	printf("[%s] load over!\n", __func__);

	return 0;
}
int ds_add_device(stZWaveDevice_t *dev) {
	if (dev == NULL) {
		return -1;
	}

	printf("[%s] add device: %s, nodeid:[%02X]\n", __func__,device_make_macstr(dev), dev->bNodeID);
	if (dev->nbit >= 0) {
		return -2;
	}

	ds_malloc_device(dev);

	FILE *fp = fopen(g_path, "r+");
	ds_save_device(fp, dev);
	fclose(fp);

	ds_write_header(g_path, g_dh);

	printf("[%s] add ok !\n", __func__);
	return 0;
}
int ds_del_device(stZWaveDevice_t *dev) {
	printf("[%s] del device: %s...\n", __func__, device_make_macstr(dev));

	ds_free_device(dev);

	ds_write_header(g_path, g_dh);

	printf("[%s] del ok !\n", __func__);
	return 0;
}
int ds_update_cmd_data(stZWaveCommand_t *cmd) {
	printf("[%s] update attr: [%04X] ...\n", __func__, cmd->cmdid);

	FILE *fp = fopen(g_path, "r+");
	ds_save_command_data(fp, cmd);
	fclose(fp);

	printf("[%s] update ok !\n", __func__);
	return 0;
}

int ds_update_dev_member(stZWaveDevice_t *dev, int offset, char *buf, int size) {
	printf("[%s] update dev %02X, offset:%08X ...\n", __func__, dev->bNodeID&0xff, offset);

	FILE *fp = fopen(g_path, "r+");
	fseek(fp, ds_offset('D', dev->nbit) + offset, SEEK_SET);
	fwrite(buf, size, 1, fp);
	fclose(fp);

	printf("[%s] update ok !\n", __func__);
	return 0;
}



