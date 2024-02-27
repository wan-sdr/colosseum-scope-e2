#ifndef __SRS_CONNECTOR_H__
#define __SRS_CONNECTOR_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// max number of slices in a slice-based config file
#define SLICE_NUM 10
// max line length in a IMSI-based config file
#define IMSI_LINE_LENGTH 100
// max length of a policy string
#define POLICY_LEN 256
// max length of config file name
#define CONFIG_FILENAME_LEN 1000

#define CONFIG_PATH "/root/radio_code/scope_config/slicing/"
//#define CONFIG_PATH "/home/orantestbed/ntia_demo/srs_connector/scope_config/slicing/"
#define SCHEDULING_POLICY_FILENAME "slice_scheduling_policy.txt"
#define ALLOCATION_POLICY_FILENAME_BASE "slice_allocation_mask_tenant_"
#define SLICE_POLICY_FILENAME "ue_imsi_slice.txt"
#define MCS_POLICY_FILENAME "ue_imsi_modulation_dl.txt"
#define GAIN_POLICY_FILENAME "tx_gain_dl.txt"





extern char last_policy[256];




void write_scheduling_policy(char* new_scheduler);
void write_allocation_policy(char* new_allocation);
void write_slice_policy(char* new_assignment);
void write_mcs_policy(char* new_mcs);
void write_gain_policy(char* new_gain);
int write_imsi_line (FILE *fp, char *imsi, char *new_policy);
void printf_neat(char* msg, char* dbg_str);
void write_control_policies(char* control_msg);

#endif
