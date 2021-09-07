#ifndef __PACKET_H_ 
#define __PACKET_H_



typedef struct stPacket {
	int		headlen;
	void	*head;
	int		datalen;
	void	*data;
	int		sumlen;
	void	*sum;
	int		taillen;
	void	*tail;
}stPacket_t;


typedef struct stPacketEnv {
	int						fd;
	stPacket_t		packet;
}stPacketEnv_t;

stPacketEnv *packet_init(int fd);
int packet_getfd(stPacketEnv_t *env);
int packet_setfd(stPacketEnv_t *env);
int packet_send(stPacket_t *env, stPacket_t *pkt);
stPacket_t *packet_recv(stPacketEnv_t *env);











#endif
