#include "myhash.h"

//#include <sys/stat.h>
//#include <sys/socket.h>
//#include <sys/signal.h>
//#include <sys/resource.h>
//#include <fcntl.h>
//#include <netinet/in.h>
//#include <errno.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include <sys/time.h>
//#include "murmur3_hash.h"
//#include "jenkins_hash.h"
//
//typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
//typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */
//
///* how many powers of 2's worth of buckets we use */
unsigned int hashpower = 16;
//
//#define hashsize(n) ((ub4)1<<(n))
//#define hashmask(n) (hashsize(n)-1)
//#define ITEM_key(item) (((char*)&((item)->data)) \
//         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))
//
//typedef struct _stritem {
//    /* Protected by LRU locks */
////    struct _stritem *next;
////    struct _stritem *prev;
//    /* Rest are protected by an item lock */
//    struct _stritem *h_next;    /* hash chain next */
////    rel_time_t      time;       /* least recent access */
////    rel_time_t      exptime;    /* expire time */
////    int             nbytes;     /* size of data */
////    unsigned short  refcount;
////    uint8_t         nsuffix;    /* length of flags-and-length string */
////    uint8_t         it_flags;   /* ITEM_* above */
////    uint8_t         slabs_clsid;/* which slab class we're in */
//    char key[256];
//    uint32_t         nkey;       /* key length, w/terminating null and padding */
//    uint32_t        vn;         /* version number */ 
//    /* this odd type prevents type-punning issues when we do
//     * the little shuffle to save space when not using CAS. */
////    union {
////        uint64_t cas;
////        char end;
////    } data[];
//    /* if it_flags & ITEM_CAS we have 8 bytes CAS */
//    /* then null-terminated key */
//    /* then " flags length\r\n" (no terminating null) */
//    /* then data with terminating \r\n (no terminating null; it's binary!) */
//} item;
//
static item** primary_hashtable = 0;
//
static unsigned int hash_items = 0;

void assoc_init(const int hashtable_init) {
    if (hashtable_init) {
        hashpower = hashtable_init;
    }
    primary_hashtable = calloc(hashsize(hashpower), sizeof(void *));
    if (! primary_hashtable) {
        fprintf(stderr, "Failed to init hashtable.\n");
        exit(EXIT_FAILURE);
    }
//    STATS_LOCK();
//    stats.hash_power_level = hashpower;
//    stats.hash_bytes = hashsize(hashpower) * sizeof(void *);
//    STATS_UNLOCK();
}

item *assoc_find(const char *key, const size_t nkey, const uint32_t hv) {
    item *it;
//    unsigned int oldbucket;

//    if (expanding &&
//        (oldbucket = (hv & hashmask(hashpower - 1))) >= expand_bucket)
//    {
//        it = old_hashtable[oldbucket];
//    } else {
        it = primary_hashtable[hv & hashmask(hashpower)];
//    }

    item *ret = NULL;
    int depth = 0;
    while (it) {
        if ((nkey == it->nkey) && (memcmp(key, it->key, nkey) == 0)) {
            ret = it;
            break;
        }
        it = it->h_next;
        ++depth;
    }
//    MEMCACHED_ASSOC_FIND(key, nkey, depth);
    return ret;
}

/* returns the address of the item pointer before the key.  if *item == 0,
   the item wasn't found */

static item** _hashitem_before (const char *key, const size_t nkey, const uint32_t hv) {
    item **pos;
//    unsigned int oldbucket;

//    if (expanding &&
//        (oldbucket = (hv & hashmask(hashpower - 1))) >= expand_bucket)
//    {
//        pos = &old_hashtable[oldbucket];
//    } else {
        pos = &primary_hashtable[hv & hashmask(hashpower)];
//    }

    while (*pos && ((nkey != (*pos)->nkey) || memcmp(key, (*pos)->key, nkey))) {
        pos = &(*pos)->h_next;
    }
    return pos;
}

/* Note: this isn't an assoc_update.  The key must not already exist to call this */
int assoc_insert(item *it, const uint32_t hv) {
//    unsigned int oldbucket;

//    assert(assoc_find(ITEM_key(it), it->nkey) == 0);  /* shouldn't have duplicately named things defined */

//    if (expanding &&
//        (oldbucket = (hv & hashmask(hashpower - 1))) >= expand_bucket)
//    {
//        it->h_next = old_hashtable[oldbucket];
//        old_hashtable[oldbucket] = it;
//    } else {
        it->h_next = primary_hashtable[hv & hashmask(hashpower)];
        primary_hashtable[hv & hashmask(hashpower)] = it;
//    }

//    pthread_mutex_lock(&hash_items_counter_lock);
    hash_items++;
//    if (! expanding && hash_items > (hashsize(hashpower) * 3) / 2) {
//        assoc_start_expand();
//    }
//    pthread_mutex_unlock(&hash_items_counter_lock);

//    MEMCACHED_ASSOC_INSERT(ITEM_key(it), it->nkey, hash_items);
    return 1;
}

void assoc_delete(const char *key, const size_t nkey, const uint32_t hv) {
    item **before = _hashitem_before(key, nkey, hv);

    if (*before) {
        item *nxt;
//        pthread_mutex_lock(&hash_items_counter_lock);
        hash_items--;
//        pthread_mutex_unlock(&hash_items_counter_lock);
        /* The DTrace probe cannot be triggered as the last instruction
         * due to possible tail-optimization by the compiler
         */
//        MEMCACHED_ASSOC_DELETE(key, nkey, hash_items);
        nxt = (*before)->h_next;
        (*before)->h_next = 0;   /* probably pointless, but whatever. */
        *before = nxt;
        return;
    }
    /* Note:  we never actually get here.  the callers don't delete things
       they can't find. */
    assert(*before != 0);
}



//int main(){
//    int randkey,i;
//    int keys[10000];
//    char akeys[10000][256];
//    item *it_new, *it;
//    uint32_t hv;
//
//    struct timeval t_start,t_end;
////gettimeofday(&t_start, NULL);
//    assoc_init(16);
//    for(i=0;i<10000;i++){
//    randkey = rand();
////    printf("rand():%d\n",randkey);
//    keys[i] = randkey;
//    it_new = (item *)malloc(sizeof(item));
//    sprintf(akeys[i],"%d",randkey);
////    it_new->key = &akeys[i];
////    printf("akeys[%d]:%s\n",i,akeys[i]);
////    printf("sizeof():%d\n",sizeof(akeys[i]));
//    memcpy(it_new->key,akeys[i],sizeof(akeys[i]));
//    it_new->nkey = sizeof(akeys[i]);
//    it_new->vn = 0;
////    printf("item:%s,%d\n",it_new->key, it_new->nkey);
//    hv = MurmurHash3_x86_32(it_new->key, it_new->nkey);
//    it = assoc_find(it_new->key, it_new->nkey, hv);
//    if(it){
////        printf("Find the item key:%s\n", it->key);
//    }
//    else{
//        assoc_insert(it_new,hv);
////        printf("insert item: %s\n", it_new->key);
////        printf("hv:%d\n",hv);
//    }
//    }
//gettimeofday(&t_start, NULL);
//    for(i=0;i<10000;i++){
////        sprintf(akey,"%d",keys[i]);
//        hv = MurmurHash3_x86_32(akeys[i],sizeof(akeys[i]));
//        it=assoc_find(akeys[i], sizeof(akeys[i]),hv);
//        if(it==NULL){
////            printf("Failed: can not find key: %s\n",akeys[i]);
////            printf("hv:%d\n",hv);
//        printf("Not Find!\n");
//        }
//        else{
////            printf("Find!\n");
//        }
//    }
//printf("Very efficient!\n");
//gettimeofday(&t_end, NULL);
//printf("time cost is: %u s or %u us.\n", t_end.tv_sec-t_start.tv_sec, t_end.tv_usec-t_start.tv_usec);
//    printf("Success\n"); 
//    return 0;
//}
