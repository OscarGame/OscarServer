#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game_protocol.h"

int
command_login_pack(int cmd_type, struct user_login_req* req, 
                   unsigned char* dst) {
	unsigned char* walk = dst;
	int len = 0;
	*((int*)walk) = cmd_type;
	walk += 4;
	
	sprintf(walk, "%s", req->uname);
	walk += (strlen(walk) + 1);
	
	sprintf(walk, "%s", req->upsd);
	walk += (strlen(walk) + 1);

	*((int*)walk) = req->channel;
	walk += 4;

	len = (walk - dst);
	return len;
}

void
command_login_unpack(unsigned char* data, int len,
                     struct user_login_req* out) {
	char* walk = (char*)data;
	// ×Ö·û´®
	out->uname = strdup(walk);
	walk += (strlen(walk) + 1);
	// end

	// ×Ö·û´®
	out->upsd = strdup(walk);
	walk += (strlen(walk) + 1);
	// end

	out->channel = *(int*)walk;
	// end
}

int
login_respons_pack(int cmd_type, struct user_login_respons* respons,
                   unsigned char* out)  {
	unsigned char* walk = out;
	*(int*)walk = cmd_type;
	walk += 4;

	*(int*)walk = respons->status;;
	walk += 4;
	
	sprintf("%s", walk);
	walk += (strlen(walk) + 1);

	*(int*)walk = respons->level;
	walk += 4;

	return (walk - out);
}

void
login_respons_unpack(unsigned char* data, int len,
                     struct user_login_respons* out) {
	unsigned char* walk = data;
	out->status = *(int*)walk;
	walk += 4;

	out->name = strdup(walk);
	walk += (strlen(walk) + 1);

	out->level = *(int*)walk;
	walk += 4;

}