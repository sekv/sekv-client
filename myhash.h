//#include <sys/stat.h>
//#include <sys/socket.h>
//#include <sys/signal.h>
//#include <sys/resource.h>
//#include <fcntl.h>
//#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include "murmur3_hash.h"
//#include "jenkins_hash.h"

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

/* how many powers of 2's worth of buckets we use */
//unsigned int hashpower = 16;

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define ITEM_key(item) (((char*)&((item)->data)) \
         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))

typedef struct _stritem {
    /* Protected by LRU locks */
//    struct _stritem *next;
//    struct _stritem *prev;
    /* Rest are protected by an item lock */
    struct _stritem *h_next;    /* hash chain next */
//    rel_time_t      time;       /* least recent access */
//    rel_time_t      exptime;    /* expire time */
//    int             nbytes;     /* size of data */
//    unsigned short  refcount;
//    uint8_t         nsuffix;    /* length of flags-and-length string */
//    uint8_t         it_flags;   /* ITEM_* above */
//    uint8_t         slabs_clsid;/* which slab class we're in */
    char *key;
    uint32_t         nkey;       /* key length, w/terminating null and padding */
    uint32_t        vn;         /* version number */
    /* this odd type prevents type-punning issues when we do
     * the little shuffle to save space when not using CAS. */
//    union {
//        uint64_t cas;
//        char end;
//    } data[];
    /* if it_flags & ITEM_CAS we have 8 bytes CAS */
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
} item;

//static item** primary_hashtable = 0;

//static unsigned int hash_items = 0;


void assoc_init(const int hashtable_init);
item *assoc_find(const char *key, const size_t nkey, const uint32_t hv);
int assoc_insert(item *it, const uint32_t hv);
void assoc_delete(const char *key, const size_t nkey, const uint32_t hv);

