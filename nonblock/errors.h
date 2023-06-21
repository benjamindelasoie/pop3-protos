#ifndef _ERRORS_H_
#define _ERRORS_H_

typedef enum return_status {
    OK_STATUS,
    MEMORY_ALOC,
    STRING_COPY,
    RECV_ERR,
    SEND_ERR,
    FILE_ERR
} return_status;

#endif