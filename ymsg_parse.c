#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "ymsg_hdr.h"

void usage(void)
{
	printf("ymsg_parse username msgfile\n");
}

int read_next_msg(int fd, struct ymsg *pmsg)
{
	memset(pmsg, 0, sizeof(*pmsg));

	if ( sizeof(uint32_t) != read(fd, &(pmsg->hdr.unixdate), sizeof(uint32_t)) ) {
		printf("Failed to read timestamp @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	if ( sizeof(uint32_t) != read(fd, &(pmsg->hdr.msgtype), sizeof(uint32_t)) ) {
		printf("Failed to read msgtype @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	if ( sizeof(uint32_t) != read(fd, &(pmsg->hdr.recvd), sizeof(uint32_t)) ) {
		printf("Failed to read \"recvd\" @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	if ( sizeof(uint32_t) != read(fd, &(pmsg->hdr.len), sizeof(uint32_t)) ) {
		printf("Failed to read msg len @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	pmsg->msgoffset = lseek(fd, 0, SEEK_CUR);

	pmsg->pmsg = calloc(pmsg->hdr.len + 1, 1);
	if (!pmsg->pmsg) {
		printf("Failed to allocate memory for message of len %ld\n", pmsg->hdr.len);
		return -1;
	}

	if (pmsg->hdr.len != read(fd, pmsg->pmsg, pmsg->hdr.len)) {
		printf("Failed to read msg @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	if ( sizeof(uint32_t) != read(fd, &(pmsg->msgend), sizeof(uint32_t)) ) {
		printf("Failed to read msg terminator @ offset %llu\n", lseek(fd, 0, SEEK_CUR));
		return -1;
	}

	return 0;
}

int free_msg(struct ymsg *pmsg)
{
	free(pmsg->pmsg);
}

void print_msg(char *msg)
{
	while(*msg) {
		printf("%c", isprint(*msg) ? *msg : '^');
       		++msg;	
	}
}

void decode_message(char *user, struct ymsg *pmsg)
{
	int	 pos;

	for (pos = 0; pos < pmsg->hdr.len; ++pos) {
		pmsg->pmsg[pos] = pmsg->pmsg[pos] ^ user[pos%strlen(user)];	
	}

}


void dump_msg(FILE *fd, struct ymsg *pmsg, char *user)
{
	char tbuff[50];
	time_t ts = pmsg->hdr.unixdate;


	ctime_r((const time_t*)(&ts), tbuff);
	strchr(tbuff, '\n') && (*(strchr(tbuff, '\n')) = '\0');
	fprintf(fd, "%26s:", tbuff);
	//fprintf(fd, " %4x ", pmsg->hdr.msgtype);
	//fprintf(fd, " %20d ", pmsg->hdr.len);
	fprintf(fd, "%-30s:", pmsg->hdr.recvd ? "==>":user);
	if (user && (pmsg->hdr.msgtype == 0x6)) {
		decode_message(user, pmsg);
		print_msg(pmsg->pmsg);
	}
	fprintf(fd, "\n");
}

int main(int argc, char **argv)
{
	int	 	fd = -1;
	char 		*user = NULL;
	char		*fname = NULL;
	struct ymsg	 msg;

	if (argc != 3) {
		usage();
		exit(-1);
	};

	user=argv[1];
	fname=argv[2];

	fd = open(fname, O_RDONLY);

	if ( fd < 0 ) {
		fprintf(stderr, "Failed to open file %s : %s\n", fname, strerror(errno));
		return -2;
	}

	while (!read_next_msg(fd, &msg)) {
		dump_msg(stdout, &msg, user);
		free_msg(&msg);
	}


	return 0;
}

