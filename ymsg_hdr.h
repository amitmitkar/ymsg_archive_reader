#ifndef __YMSG_HDR_H__

#define __YMSG_HDR_H__

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

struct ymsg_hdr {
	uint32_t unixdate;
	uint32_t msgtype;
	uint32_t recvd;
	uint32_t len;
};

struct ymsg {
	struct 	ymsg_hdr hdr;
	uint64_t 	 msgoffset;
	uint32_t	 msgend;
	char 		*pmsg;
};

int read_next_msg(int fd, struct ymsg *pmsg);
int free_msg(struct ymsg *pmsg);
void dump_msg(FILE *fd, struct ymsg *pmsg, char *user);
#endif //__YMSG_HDR_H__
