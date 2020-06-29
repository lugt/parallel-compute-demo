//
// Created by xc5 on 6/25/20.
//

#ifndef MEMORY_CLIENT_H
#define MEMORY_CLIENT_H

PROTO_OBJ *Create_request(ACTION_TYPE action, UINT32 mem_id, void *data, UINT32 length, PROTO_OBJ *request);
PROTO_OBJ *Request_by_socket(char *buf, UINT32 length, SPTR parentSpan);
void Http_saving_and_reading(SPTR parent);
void batch_send_data(SPTR parent);
void batch_recv_data(SPTR parent);


#endif //MEMORY_CLIENT_H
