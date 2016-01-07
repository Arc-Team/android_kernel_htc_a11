#ifndef _PIL_Q6V5_MSS_DEBUG_H_
#define _PIL_Q6V5_MSS_DEBUG_H_

#include "pil-q6v5.h"

extern void modem_read_spmi_setting(int ramdump_enable);
extern void modem_crash_msa_dump_rmb_regs(struct q6v5_data *drv,int ramdump_enable);

#endif 
