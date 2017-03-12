
#include "request.h"
#ifdef GEM5
#include "m5op.h"
#endif

#include "encrypt.h"
#include "myhash.h"

void sendRequest(struct request* request) {

  //Send out all requests (only one unless multiget
 //// printf("entered sendRequest\n");
  struct request* sendRequest = request;
  if(request->connection->protocol == TCP_MODE){
    tcpSendRequest(sendRequest);
  } else if(request->connection->protocol == UDP_MODE){ 
    printf("UDP not working\n"); 
    exit(-1);
    udpSendRequest(sendRequest);
  } else {
    printf("Undefined protocol\n");
    exit(-1);
  }
  //struct request* sendRequest = request;
  //while(sendRequest != NULL) {
  ////  printf("request op %d\n", sendRequest->header.opcode);
  //  if(request->connection->protocol == TCP_MODE){
  //    tcpSendRequest(sendRequest);
  //  } else if(request->connection->protocol == UDP_MODE){
  //    udpSendRequest(sendRequest);
  //  } else {
  //    printf("Undefined protocol\n");
  //    exit(-1);
  //  }
  //  sendRequest = sendRequest->next_request;
  //}//End while

}//End sendRequest()
  
void tcpSendRequest(struct request* request) {
  
  //printf("tcpSendRequest\n");
  struct request* sendRequest = request;

#ifdef GEM5
      m5_work_begin(sendRequest->header.opcode, sendRequest->header.opaque); 
#endif

      gettimeofday(&request->send_time, NULL);
  if(request->bad_multiget) {

    while(sendRequest != NULL) {
      int totalSize = sendRequest->value_size + sendRequest->key_size + sendRequest->header.extras_length + sizeof(struct request_header);
      
      char* oneBigPacket = malloc(sizeof(char) * totalSize);
      char* ptr = oneBigPacket;

      memcpy(ptr, (char *) (& sendRequest->header), sizeof(struct request_header));
      ptr += sizeof(struct request_header);

      memcpy(ptr, sendRequest->extras, sendRequest->header.extras_length);
      ptr += sendRequest->header.extras_length;

      memcpy(ptr, sendRequest->key, sendRequest->key_size);
      ptr += sendRequest->key_size;

/* SeKV code begin */
      unsigned char *p_mac, *p_enc;
      int len_dst;
//      printf("begin multiget!\n");
      p_enc = (unsigned char *)malloc(sizeof(unsigned char)*sendRequest->key_size);
//      printf("1 begin encrypt!\n");
      p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
      my_aes_gcm_encrypt(sendRequest->key, sendRequest->key_size, p_enc, &len_dst, p_mac);
//      printf("1 encrypt success!\n");
      item *it_new, *it;
      int hv;
      it_new = (item *)malloc(sizeof(item));
      it_new->key = (char *)malloc(sizeof(char)*strlen(p_mac));
      memcpy(it_new->key, p_mac, strlen(p_mac));
      it_new->nkey = strlen(p_mac);
      hv = MurmurHash3_x86_32(it_new->key, it_new->nkey);
      it = assoc_find(it_new->key, it_new->nkey, hv);
      if(it){
        it_new->vn = it->vn;
        free(it_new);
      }
      else{
        it_new->vn = 0;
        assoc_insert(it_new,hv);
      }
      free(p_enc);
/*SeKV code end */

      memcpy(ptr, sendRequest->value, sendRequest->value_size);
/* SeKV code begin */
      unsigned char *p_evalue;
      if(sendRequest->value){
      p_evalue = (unsigned char *)malloc(sizeof(unsigned char)*sendRequest->value_size);
      my_aes_gcm_encrypt(sendRequest->value, sendRequest->value_size, p_evalue, &len_dst, p_mac);
      free(p_evalue);
      }
/*SeKV code end */

//      gettimeofday(&request->send_time, NULL);
      writeBlock(request->connection->sock, oneBigPacket, totalSize);

      free(oneBigPacket);

      sendRequest = sendRequest->next_request;
    }

  } else { 
    
    int totalSize = 0;
    while(sendRequest != NULL) {
      totalSize += sendRequest->value_size + sendRequest->key_size + sendRequest->header.extras_length + sizeof(struct request_header);
      sendRequest = sendRequest->next_request;
    }
    
    char* oneBigPacket = malloc(sizeof(char) * totalSize);
    char* ptr = oneBigPacket;

    gettimeofday(&request->send_time, NULL);
    sendRequest = request;
    while(sendRequest != NULL) {
      memcpy(ptr, (char *) (& sendRequest->header), sizeof(struct request_header));
      ptr += sizeof(struct request_header);

      memcpy(ptr, sendRequest->extras, sendRequest->header.extras_length);
      ptr += sendRequest->header.extras_length;

      memcpy(ptr, sendRequest->key, sendRequest->key_size);
      ptr += sendRequest->key_size;
/*begin SeKV code*/
      unsigned char *p_mac, *p_enc;
      int len_dst;
//      printf("2 begin encrypt!\n");
      p_enc = (unsigned char *)malloc(sizeof(unsigned char)*sendRequest->key_size);
//      printf("sendRequest->key:%s\n",sendRequest->key);
//      printf("key_size:%d\n",sendRequest->key_size);
//      printf("strlen(key):%d\n",strlen(sendRequest->key));
      p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
      my_aes_gcm_encrypt(sendRequest->key, sendRequest->key_size, p_enc, &len_dst, p_mac);
//      printf("2 encrypt success!\n");
      item *it_new, *it;
      int hv;
      it_new = (item *)malloc(sizeof(item));
//      printf("item malloc\n");
//      printf("strlen(p_mac):%d\n",strlen(p_mac));
      it_new->key = (char *)malloc(sizeof(char)*strlen(p_mac));
      memcpy(it_new->key, p_mac, strlen(p_mac));
//      printf("memcpy p_mac:%s\n",p_mac);
      it_new->nkey = strlen(p_mac);
      hv = MurmurHash3_x86_32(it_new->key, it_new->nkey);
//      printf("MurmurHash success!\n");
      it = assoc_find(it_new->key, it_new->nkey, hv);
//      printf("assoc_find\n");
      if(it){
        it_new->vn = it->vn;
        free(it_new);
//        printf("assoc_find it\n");
      }
      else{
        it_new->vn = 0;
        assoc_insert(it_new,hv);
//        printf("assoc_insert\n");
      }
      free(p_enc);
/*end SeKV code*/
      memcpy(ptr, sendRequest->value, sendRequest->value_size);
/*begin SeKV code*/
      unsigned char *p_evalue;
//      printf("encrypt value\n");
      if(sendRequest->value){
      p_evalue = (unsigned char *)malloc(sizeof(unsigned char)*sendRequest->value_size);
      my_aes_gcm_encrypt(sendRequest->value, sendRequest->value_size, p_evalue, &len_dst, p_mac);
      free(p_evalue);
      }
/*end SeKV code*/
      sendRequest = sendRequest->next_request;
    }

//    gettimeofday(&request->send_time, NULL);
    writeBlock(request->connection->sock, oneBigPacket, totalSize);

    free(oneBigPacket);
  }

}//End tcpSendRequest

//Each UDP datagram contains a simple frame header, followed by data in the
//same format as the TCP protocol described above. In the current
//implementation, requests must be contained in a single UDP datagram, but
//responses may span several datagrams. (The only common requests that would
//span multiple datagrams are huge multi-key "get" requests and "set"
//requests, both of which are more suitable to TCP transport for reliability
//reasons anyway.)
//
//The frame header is 8 bytes long, as follows (all values are 16-bit integers
//in network byte order, high byte first):
//
//0-1 Request ID
//2-3 Sequence number
//4-5 Total number of datagrams in this message
//6-7 Reserved for future use; must be 0
void udpSendRequest(struct request* request) {

 int totalSize = request->value_size + request->key_size + request->header.extras_length + sizeof(struct request_header) + 8;

 char* oneBigPacket = malloc(totalSize);
 char* ptr = oneBigPacket;

 int requestId = request->worker->current_request_id;
 request->id = requestId;
 request->worker->current_request_id = (requestId + 1) % 0xFFFF;

 oneBigPacket[0] = (char)(requestId & 0xFFFF) >> 16;
 oneBigPacket[1] = (char)(requestId & 0xFF);
 oneBigPacket[2] = 0x00;
 oneBigPacket[3] = 0x00;
 oneBigPacket[4] = 0x00;
 oneBigPacket[5] = 0x01;
 oneBigPacket[6] = 0x00;
 oneBigPacket[7] = 0x00;

 ptr += 8;

 memcpy(ptr, (char *) (& request->header), sizeof(struct request_header));
 ptr += sizeof(struct request_header);

 memcpy(ptr, request->extras, request->header.extras_length);
 ptr += request->header.extras_length;

 memcpy(ptr, request->key, request->key_size);
 ptr += request->key_size;

 memcpy(ptr, request->value, request->value_size);

 int fd = request->connection->sock;
 gettimeofday(&request->send_time, NULL);
 writeBlock(fd, oneBigPacket, totalSize);
 free(oneBigPacket);

}//End udpSendRequest()

void deleteRequest(struct request* request) {

  struct request* currentRequest = request;
  while(currentRequest != NULL) {

    if(currentRequest->value != NULL){
      free(currentRequest->value);
    }

    if(currentRequest->extras != NULL){
      free(currentRequest->extras);
    } 


    struct request* nextRequest;
    nextRequest = currentRequest->next_request;
    free(currentRequest);
    currentRequest = nextRequest;

  }//End while

}//End deleteRequest()

int generateUID(struct worker* worker) {

  struct config* config = worker->config;

  uint32_t uid = __sync_fetch_and_add(&(config->current_request_uid), 1);
  
  return uid;

}


struct request* createRequest(int requestType, struct conn* conn, struct worker* worker, char* key, char* value, int type) {

  struct request* request = malloc(sizeof(struct request));
  request->worker = worker;
  request->bad_multiget = 0;

  if(conn == NULL){
    printf("Tried to give request a null connection\n");
    exit(-1);
  }
  request->connection = conn;

  int keyLength = 0;
  if(key != NULL) {
    keyLength = strlen(key);
  }

  if(keyLength > MAX_KEY_LENGTH) {
    printf("The key is too long!\nkey: %s\nlength: %d\n", key, keyLength);
    exit(-1);
  }

  int valueLength = 0;
  if(value != NULL) {
    valueLength = strlen(value);
  } 

  if(valueLength > MAX_VALUE_LENGTH) {
    printf("The value is too long!\nvalue: %s\nlength: %d\n", value, valueLength);
  }
  request->request_type = type;

  struct request_header* request_header = &(request->header);
  request_header->magic = MAGIC_REQUEST;
  request_header->data_type = 0;

  request_header->reserved[0] = 0;
  request_header->reserved[1] = 0;

  memset(&request_header->CAS, 0, 8);
  // We are using the opaque field for UIDs that will be sent back to use
  // in the response packet
  request_header->opaque = generateUID(worker);

  switch(requestType) {

    case STAT:{

      request_header->opcode = OP_STAT;
      printf("STAT\n");
      break;

    }//End case STAT
    case ADD:{
      printf("ADD\n");
      int body_length = 0;

      request_header->opcode = OP_ADD;

      request_header->key_length[0] = ((unsigned int)(strlen(key) & 0xff00))>>8;
      request_header->key_length[1] = (strlen(key) & 0xff);

      request->key = key;
      request->key_size = keyLength;
      request->value = value;
      request->value_size = valueLength;

      //Extra information
      request_header->extras_length = (char)8;
      request->extras = malloc(8);
      request->extras[0] = 0xde;
      request->extras[1] = 0xad;
      request->extras[2] = 0xbe;
      request->extras[3] = 0xef;
      request->extras[4] = 0;
      request->extras[5] = 0;
      request->extras[6] = 0;
      request->extras[7] = 0;

      body_length = 8 + keyLength + valueLength;
      #if DEBUG
      printf("body_length %d\n", body_length);
      #endif

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)(body_length & 0xff00))>>8;
      request_header->total_body_length[1] = ((unsigned int)(body_length & 0xff0000))>>16;
      request_header->total_body_length[0] = ((unsigned int)(body_length & 0xff000000))>>24;
      break;

    }//End case REP
    case REP:{
      printf("REP\n");
      int body_length = 0;

      request_header->opcode = OP_REP;

      request_header->key_length[0] = ((unsigned int)(strlen(key) & 0xff00))>>8;
      request_header->key_length[1] = (strlen(key) & 0xff);

      request->key = key;
      request->key_size = keyLength;
      request->value = value;
      request->value_size = valueLength;

      //Extra information
      request_header->extras_length = (char)8;
      request->extras = malloc(8);
      request->extras[0] = 0xde;
      request->extras[1] = 0xad;
      request->extras[2] = 0xbe;
      request->extras[3] = 0xef;
      request->extras[4] = 0;
      request->extras[5] = 0;
      request->extras[6] = 0;
      request->extras[7] = 0;

      body_length = 8 + keyLength + valueLength;
      #if DEBUG
      printf("body_length %d\n", body_length);
      #endif

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)(body_length & 0xff00))>>8;
      request_header->total_body_length[1] = ((unsigned int)(body_length & 0xff0000))>>16;
      request_header->total_body_length[0] = ((unsigned int)(body_length & 0xff000000))>>24;
      break;

    }//End case REP
    case DEL:{
      printf("DEL\n");
      request_header->opcode = OP_DEL;

      request_header->key_length[0] = ((unsigned int)(strlen(key) & 0xff00))>>8;
      request_header->key_length[1] = (strlen(key) & 0xff);

      request->key = key;
      request->key_size = keyLength;

      request_header->extras_length = 0;
      request->extras = NULL;

      request->value = NULL;
      request->value_size = 0;
      request->extras = NULL;

      int body_length = keyLength;

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)body_length & 0xff00)>>8;
      request_header->total_body_length[1] = ((unsigned int)body_length & 0xff0000)>>16;
      request_header->total_body_length[0] = ((unsigned int)body_length & 0xff000000)>>24;

      break;

    }//End case DEL
    case INCR:{
      printf("INCR\n");
      request_header->opcode = OP_INCR;

      request_header->key_length[0] = ((unsigned int)(strlen(key) & 0xff00))>>8;
      request_header->key_length[1] = (strlen(key) & 0xff);

      request->key = key;
      request->key_size = keyLength;

      //Extra information
      // 8 byte value to add / subtract
      // 8 byte initial value (unsigned)
      // 4 byte expiration time

      request_header->extras_length = (char)20;
      request->extras = malloc(20);
      //request->extras[12] = 1;//value++
      //request->extras[3] = 0xff;
      //request->extras[2] = 0xff;
      //request->extras[1] = 0xff;
      //request->extras[0] = 0xff;
      request->extras[7] = 1;//value++
      request->extras[16] = 0xff;
      request->extras[17] = 0xff;
      request->extras[18] = 0xff;
      request->extras[19] = 0xff;
      //Right now, the expiration is 0, so we'll have to follow up the incr with a set if it fails.

      request->value = NULL;
      request->value_size = 0;

      int body_length = keyLength + 20;

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)body_length & 0xff00)>>8;
      request_header->total_body_length[1] = ((unsigned int)body_length & 0xff0000)>>16;
      request_header->total_body_length[0] = ((unsigned int)body_length & 0xff000000)>>24;

      break;

    }//End case INCR
    case SET:{
//      printf("SET\n");
      int body_length = 0;

      request_header->opcode = OP_SET;

      request_header->key_length[0] = ((unsigned int)(strlen(key) & 0xff00))>>8;
      request_header->key_length[1] = (strlen(key) & 0xff);

      request->key = key;
      request->key_size = keyLength;
      request->value = value;
      request->value_size = valueLength;

      //Extra information
      request_header->extras_length = (char)8;
      request->extras = malloc(8);
      request->extras[0] = 0xde;
      request->extras[1] = 0xad;
      request->extras[2] = 0xbe;
      request->extras[3] = 0xef;
      request->extras[4] = 0;
      request->extras[5] = 0;
      request->extras[6] = 0;
      request->extras[7] = 0;

      body_length = 8 + keyLength + valueLength;
      #if DEBUG
      printf("body_length %d\n", body_length);
      #endif

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)(body_length & 0xff00))>>8;
      request_header->total_body_length[1] = ((unsigned int)(body_length & 0xff0000))>>16;
      request_header->total_body_length[0] = ((unsigned int)(body_length & 0xff000000))>>24;
      break;
    }
  case GET:{
//      printf("GET\n");
      int body_length = 0;

      request_header->opcode = OP_GET;
      request_header->key_length[0] = ((unsigned int)(keyLength & 0xff00))>>8;
      request_header->key_length[1] = (keyLength & 0xff);

      request->key = key;
      request->key_size = keyLength;
      request->header.extras_length = (char)0;

      request->value = NULL;
      request->value_size = 0;
      request->extras = NULL;

      body_length = keyLength;

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)(body_length & 0xff00))>>8;
      request_header->total_body_length[1] = ((unsigned int)(body_length & 0xff0000))>>16;
      request_header->total_body_length[0] = ((unsigned int)(body_length & 0xff000000))>>24;
      //printf("In get\n");
      //printf("body_length is %d\n", body_length);
      //int i;
      //for(i = 0; i < 4; i++){
      //  printf("%8x ", request_header->total_body_length[i]);
      //}
      //printf("\n");


      break;
    }
  case GETQ:{
//      printf("GETQ\n");
      int body_length = 0;

      request_header->opcode = OP_GETQ;
      request_header->key_length[0] = ((unsigned int)(keyLength & 0xff00))>>8;
      request_header->key_length[1] = (keyLength & 0xff);

      request->key = key;
      request->key_size = keyLength;
      request->header.extras_length = (char)0;

      request->value = NULL;
      request->value_size = 0;
      request->extras = NULL;

      body_length = keyLength;

      request_header->total_body_length[3] = (body_length & 0xff);
      request_header->total_body_length[2] = ((unsigned int)body_length & 0xff00)>>8;
      request_header->total_body_length[1] = ((unsigned int)body_length & 0xff0000)>>16;
      request_header->total_body_length[0] = ((unsigned int)body_length & 0xff000000)>>24;

      break;
    }

  }//End switch

  return request;

}//End generateRequest()

