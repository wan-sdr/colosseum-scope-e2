#include "srs_connector.h"


char last_scheduling_policy[POLICY_LEN] = { '\0' };
char last_allocation_policy[POLICY_LEN] = { '\0' };
char last_slice_policy[POLICY_LEN] = { '\0' };
char last_mcs_policy[POLICY_LEN] = { '\0' };
char last_gain_policy[POLICY_LEN] = { '\0' };

void write_scheduling_policy(char* new_scheduler) {
  FILE *fp;
  char filename[1000];
  char* file_header = "# slice::scheduling policy\n"
                      "# 0 = default srsLTE round-robin\n"
                      "# 1 = waterfilling\n"
                      "# 2 = proportional";
  const char policies_delimiter[2] = ",";
  const char config_delimiter[3] = "::";
  const char default_policy[2] = "0";
  char* slice_policy;

  // form filename
  strcpy(filename, CONFIG_PATH);
  strcat(filename, SCHEDULING_POLICY_FILENAME);

  // copy new_scheduler so that it can be modified
  char* scheduler_policies = strdup(new_scheduler);

  printf("Writing scheduler policy: %s\n", new_scheduler);

  fp = fopen(filename, "w+");

  if (fp == NULL) {
    printf("ERROR: fp is NULL\n");
    return;
  }

  // write header
  fprintf(fp, "%s\n", file_header);

  // get first policy
  slice_policy = strtok(scheduler_policies, policies_delimiter);

  for (int i = 0; i < SLICE_NUM; ++i) {
    if (slice_policy != NULL) {
      fprintf(fp, "%d%s%s\n", i, config_delimiter, slice_policy);

      // get next policy
      slice_policy = strtok(NULL, policies_delimiter);
    }
    else {
      fprintf(fp, "%d%s%s\n", i, config_delimiter, default_policy);
    }
  }

  // save policy
  strcpy(last_scheduling_policy, new_scheduler);

  fclose(fp);
  printf("Scheduler policy written to file: %s\n", filename);
}

void write_allocation_policy(char* new_allocation){
  FILE *fp;
  char base_filename[CONFIG_FILENAME_LEN];
  const int rbg_num = 25;
  const char policies_delimiter[2] = ",";

  // form filename
  strcpy(base_filename, CONFIG_PATH);
  strcat(base_filename, ALLOCATION_POLICY_FILENAME_BASE);

  // copy new_allocation so that it can be modified
  char* allocation_policies = strdup(new_allocation);

  printf("Writing allocation policy: %s\n", new_allocation);

  // count how many policies were received
  int p_num = 0;
  for (int i = 0; i < strlen(new_allocation); ++i) {
    if (new_allocation[i] == policies_delimiter[0]) {
      p_num++;
    }
  }

  // increase to account for last policy
  p_num++;

  // write scheduling files
  int m_ptr = 0;
  for (int s_idx = 0; s_idx < p_num; ++s_idx) {
    // get slice number and form slicing filename
    char slice_num[2];
    sprintf(slice_num, "%d", s_idx);

    char filename[CONFIG_FILENAME_LEN];
    strcpy(filename, base_filename);
    strcat(filename, slice_num);
    strcat(filename, ".txt");

    // get current slicing policy
    char* rbg_policy_str = NULL;
    if (s_idx == 0) {
      rbg_policy_str = strtok(allocation_policies, policies_delimiter);
    }
    else {
      rbg_policy_str = strtok(NULL, policies_delimiter);
    }

    int rbg_policy = atoi(rbg_policy_str);
    char slicing_mask[rbg_num];

    // initialize slicing mask values to NULL
    for (int i = 0; i < rbg_num; ++i) {
      slicing_mask[i] = '\0';
    }

    int m_idx;
    for (m_idx = 0; m_idx < m_ptr; ++m_idx) {
      strcat(slicing_mask, "0");
    }

    for (int i = 0; i < rbg_policy && m_idx < rbg_num; ++i, ++m_idx) {
      strcat(slicing_mask, "1");
    }
    m_ptr = m_idx;

    for (; m_idx < rbg_num; ++m_idx) {
      strcat(slicing_mask, "0");
    }

    printf("%s\n", slicing_mask);

    // write mask on file
    fp = fopen(filename, "w");

    if (fp == NULL) {
      printf("ERROR: fp is NULL\n");
      return;
    }

    fprintf(fp, "%s", slicing_mask);
    fclose(fp);
    printf("Allocation policy written to file: %s\n", filename);
  }
  strcpy(last_allocation_policy, new_allocation);
}

void write_slice_policy(char* new_slice) {
  FILE *fp;
  char filename[CONFIG_FILENAME_LEN];
  const char policies_delimiter[2] = ",";
  const char config_delimiter[3] = "::";
  const char default_policy[2] = "0";

  // form filename
  strcpy(filename, CONFIG_PATH);
  strcat(filename, SLICE_POLICY_FILENAME);

  // copy new_slice so that it can be modified
  char* slice_policies = strdup(new_slice);



  printf("Writing slice policy: %s\n", new_slice);

  // open config file for reading and writing
  fp = fopen(filename, "r+");

  if (fp == NULL) {
    printf("ERROR: file pointer is NULL\n");
    return;
  }

  // TODO: Loop through multiple slice policies, replace strdup with strtok to tokenize each slice policy
  char *slice_policy = strdup(slice_policies);

  // Tokenize UE IMSI and UE slice
  char *ue_imsi = strtok(slice_policy, config_delimiter);
  char *ue_slice = strtok(NULL, config_delimiter);

  char *imsi = strdup(ue_imsi);
  char *new_policy = strdup(ue_slice);

  int success = write_imsi_line(fp, ue_imsi, ue_slice);
  if (success == 1) {
    // save slice policy
    strcpy(last_slice_policy, new_slice);
    printf("Slice policy written to file: %s\n", filename);
  } else if (success == 0) {
    printf("Failed to write slice policy to file: UE IMSI requested not found in the file.\n");
  } else {
    printf("Failed to write slice policy to file: an error occurred.\n");
  }
}

void write_mcs_policy(char* new_mcs) {
  FILE *fp;
  char filename[CONFIG_FILENAME_LEN];
  const char policies_delimiter[2] = ",";
  const char config_delimiter[3] = "::";
  const char default_policy[2] = "0";

  // form filename
  strcpy(filename, CONFIG_PATH);
  strcat(filename, MCS_POLICY_FILENAME);

  // copy new_mcs so that it can be modified
  char* mcs_policies = strdup(new_mcs);



  printf("Writing MCS policy: %s\n", new_mcs);

  // open config file for reading and writing
  fp = fopen(filename, "r+");

  if (fp == NULL) {
    printf("ERROR: file pointer is NULL\n");
    return;
  }

  // TODO: Loop through multiple MCS policies, replace strdup with strtok to tokenize each MCS policy
  char *mcs_policy = strdup(mcs_policies);

  // Tokenize UE IMSI and UE MCS
  char *ue_imsi = strtok(mcs_policy, config_delimiter);
  char *ue_mcs = strtok(NULL, config_delimiter);

  char *imsi = strdup(ue_imsi);
  char *new_policy = strdup(ue_mcs);

  int success = write_imsi_line(fp, ue_imsi, ue_mcs);
  if (success == 1) {
    // save mcs policy
    strcpy(last_mcs_policy, new_mcs);
    printf("MCS policy written to file: %s\n", filename);
  } else if (success == 0) {
    printf("Failed to write MCS policy to file: UE IMSI requested not found in the file.\n");
  } else {
    printf("Failed to write MCS policy to file: an error occurred.\n");
  }
}

void write_gain_policy(char* new_gain) {
  FILE *fp;
  char filename[CONFIG_FILENAME_LEN];

  // form filename
  strcpy(filename, CONFIG_PATH);
  strcat(filename, GAIN_POLICY_FILENAME);

  // copy new_gain so that it can be modified
  char* gain = strdup(new_gain);

  printf("Writing gain policy: %s\n", new_gain);
  // open gain config file for writing
  fp = fopen(filename, "w+");

  // check for fopen success
  if (fp == NULL) {
    printf("ERROR: could not open file (%s)\n", filename);
    return;
  }

  // write gain policy to file
  if (fprintf(fp, "%s\n", gain) < 0){
    printf("ERROR: could not write gain policy (%s)\n", gain);
    return;
  }

  // save gain policy
  strcpy(last_gain_policy, gain);

  // close file
  fclose(fp);
  printf("Gain policy written to file: %s\n", filename);
}

void write_control_policies(char* control_msg) {

    // Copy RIC control message so it can be modified
    char *control = strdup(control_msg);

    // Declare and initialize an array of functions
    void (*func_array[]) (char *) = {write_scheduling_policy,
                                      write_allocation_policy,
                                      write_slice_policy,
                                      write_mcs_policy,
                                      write_gain_policy };

    // Declare and initialize an array of last policies
    char *last_policy_array[] = {last_scheduling_policy,
                                 last_allocation_policy,
                                 last_slice_policy,
                                 last_mcs_policy,
                                 last_gain_policy };

    // Declare an array of new policies
    char *new_policy_array[5] = {NULL, NULL, NULL, NULL, NULL};

    // Print RIC control message
    printf_neat("\n==========Received RIC control message=========\n", control);

    // Tokenize RIC control message into policy strings
    char* policy = strtok(control, "\n");
    int index = 0;

    while (policy != NULL && index < 5) {
        new_policy_array[index] = policy;
        printf("Parsed policy %d: %s\n", index, policy);  // Debugging print to confirm policy is received
        policy = strtok(NULL, "\n");
        index++;
    }

    // Iterate through all policy functions or until no tokens left in control message
    for (int i = 0; i < 5; i++) {
        // If policy exists and is different from last policy, print debug statement
        if (new_policy_array[i] != NULL) {
            if (strcmp(new_policy_array[i], last_policy_array[i]) == 0) {
                printf("New policy is the same as last policy: %s\n", new_policy_array[i]);
            } else {
                // Print the new policy instead of writing to files
                printf("Debug: Would have written new policy %d: %s\n", i, new_policy_array[i]);
                // Comment out actual file-writing functions for now:
                // func_array[i](new_policy_array[i]);
            }
        } else {
            // Handle missing policy
            printf("No policy provided or empty line at index %d\n", i);
        }
    }
}

int write_imsi_line (FILE *fp, char *imsi, char *new_policy) {

  // read file line by line to replace the line with the matching IMSI
  char buffer[IMSI_LINE_LENGTH];
  int updated = 0;
  char line_imsi[IMSI_LINE_LENGTH];
  char line_policy[IMSI_LINE_LENGTH];

  while (fgets(buffer, IMSI_LINE_LENGTH , fp) != NULL) {

    // dynamically allocate memory for the actual string length in buffer
    int line_len = strlen(buffer);
    char *line = (char *)malloc((line_len+1) * sizeof(char));

    // Check if memory allocation was successful
    if (line == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      return 0;
    }

    // copy number of chars specified the dynamically determined length
    strncpy(line, buffer, line_len);
    // null terminate string
    line[line_len] = '\0';

    // Set file offset position for line write
    fseek(fp, -line_len, SEEK_CUR);
    int sscanf_code = sscanf(line, "%[^:]::%[^\n]", line_imsi, line_policy);
    if (sscanf_code == 2) {
      if (strcmp(line_imsi, imsi) == 0) {
        fprintf(fp, "%s::%s", imsi, new_policy);
        updated = 1;
      } else {
        fprintf(fp, "%s", line);
      }
    } else {
    // If the line doesn't match the expected format, copy it as is
      fprintf(fp, "%s", line);
    }
    free(line);
  }
  return updated;
}


// debug print: print \n literally
void printf_neat(char* msg, char* dbg_str) {

  printf("%s", msg);

  for (int i = 0; i < strlen(dbg_str); ++i) {
    if (dbg_str[i] == '\n') {
      printf("\\n");
    }
    else {
      printf("%c", dbg_str[i]);
    }
  }

  printf("\n");
}

// tester function
int tester(int argc, char *argv[]) {

  // Check if at least one command line argument is provided
  int test_num = 4;
  char *messages[] = {"1,0,1\n5,10,35\n001010123456001::2\n001010123456001::0\n80",
                     "1,2,0\n15,15,20\n001010123456001::0\n001010123456001::3\n70",
                     "1,2,0\n15,15,20\n001010123456001::0\n001010123456001::3\n70",
//                     "\n5,10,35\n001010123456001::2\n001010123456001::0\n80",
//                     "\n\n001010123456001::2\n001010123456001::0\n80",
//                     "\n\n\n\n",
//                     "1,0,1\n5,10,35\n001010123456001::2\n001010123456001::0\n",
//                     "1,0,1\n5,10,35\n001010123456001::2\n001010123456001::0",
                     "1,0,1\n5,10,35\n001010123456001::2\n001010123456001::0\n80"
                     };

  char msg[256];
  if (argc >= 2) {
    strcpy(msg, argv[1]);
  }

  for (int i = 0; i < 8; i++) {
    strcpy(msg, messages[i]);
    write_control_policies(msg);
  }


  return 0;
}
