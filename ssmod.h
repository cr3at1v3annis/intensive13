#ifndef _UAPI_KEYVALUE_H
#define _UAPI_KEYVALUE_H

#include <linux/types.h>


struct keyvalue_get {
    __u64 key;
    __u64 *size;
    void *data;
};
struct keyvalue_set {
    __u64 key;
    __u64 size;
    void *data;
};
struct keyvalue_delete {
    __u64 key;
};

#define KEYVALUE_IOCTL_GET     _IOWR('N', 0x43, struct keyvalue_get)
#define KEYVALUE_IOCTL_SET  _IOWR('N', 0x44, struct keyvalue_set)
#define KEYVALUE_IOCTL_DELETE  _IOWR('N', 0x45, struct keyvalue_delete)

#endif
