#ifndef __CLIT_DEVICE_H
#define __CLIT_DEVICE_H

enum device_result_e {
    DEVICE_OK = 0x00,
    DEVICE_EUNAVAIL,
    DEVICE_ERROR,
};
typedef enum device_result_e device_result_t;

device_result_t device_init(void);
    
    

#endif
