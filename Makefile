sekv: libsekv_op.c encrypt.c encrypt.h hash_op.c hash_op.h murmur3_hash.c murmur3_hash.h
	gcc -o sekv libsekv_op.c encrypt.c encrypt.h hash_op.c hash_op.h murmur3_hash.c murmur3_hash.h -lcrypto -std=c99
