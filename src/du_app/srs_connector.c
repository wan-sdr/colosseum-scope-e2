#include "srs_connector.h"

char last_scheduling_policy[256] = { '\0' };
char last_slicing_policy[256] = { '\0' };
char last_slice_assignment[256] = { '\0' };

// write UE slice assignment on config file
void write_slice_assignment(char* new_assignment) {

  // form filename
  char filename[1000];
  strcpy(filename, CONFIG_PATH);
  strcat(filename, SLICE_ASSIGNMENT_FILENAME);
  printf("%s\n", filename);

  FILE *file = fopen(filename, "r");


  if (file == NULL) {
      perror("Error opening the file");
      exit(1);
  }

  FILE *tempFile = fopen("tempfile.txt", "w");
  if (tempFile == NULL) {
      perror("Error creating a temporary file");
      exit(1);
  }

  const char config_delimiter[3] = "::";
  const char default_ue_slice[2] = "0";

  printf("tokenize message: %s\n", new_assignment);
  // copy new_assignment so that it can be modified
  char *assignment = strdup(new_assignment);

  // get UE IMSI and new UE slice ID as integers from assignment string
//  int ue_imsi, ue_slice;

  char *ue_imsi = strtok(assignment, config_delimiter);
//  sscanf(token, "%d", &ue_imsi);

  char *ue_slice = strtok(NULL, config_delimiter);
//  sscanf(token, "%d", &ue_slice);
    printf("ue_imsi:%s\n", ue_imsi);
    printf("ue_slice:%s\n", ue_slice);
  // read file line by line to replace the line with the matching IMSI
  char buffer[MAX_LINE_LENGTH];
  int updated = 0;
  char line_ue_imsi[1000];
  char line_ue_slice[1000];


  while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
//      printf("buffer: %s\n", buffer);
      int scanf_code = sscanf(buffer, "%[^:]::%[^\n]", line_ue_imsi, line_ue_slice);
//      printf("scanf_code: %d\n", scanf_code);
//      printf("line_ue_imsi: %s\n", line_ue_imsi);
//      printf("line_ue_slice: %s\n", line_ue_slice);
      if (scanf_code == 2) {
          if (strcmp(line_ue_imsi, ue_imsi) == 0) {
              fprintf(tempFile, "%s::%s\n", ue_imsi, ue_slice);
              updated = 1;
          } else {
              fprintf(tempFile, "%s", buffer);
          }
      } else {
          // If the line doesn't match the expected format, copy it as is
          fprintf(tempFile, "%s", buffer);
      }
  }

  fclose(file);
  fclose(tempFile);

  if (!updated) {
      printf("UE IMSI requested not found in the file.\n");
      remove("tempfile.txt"); // Delete the temporary file if no update occurred
  } else {
      remove(filename);         // Delete the original file
      rename("tempfile.txt", filename); // Rename the temporary file to the original file
      printf("File updated successfully.\n");
  }
}


// write scheduling policies on config file
void write_scheduling_policy(char* new_policy) {

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
  strcat(filename, SCHEDULING_FILENAME);

  // copy new_policy so that it can be modified
  char *policies = strdup(new_policy);

  fp = fopen(filename, "w+");

  if (fp == NULL) {
    printf("ERROR: fp is NULL\n");
    return;
  }

  // write header
  fprintf(fp, "%s\n", file_header);

  // get first policy
  slice_policy = strtok(policies, policies_delimiter);

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
  strcpy(last_scheduling_policy, new_policy);

  fclose(fp);
}


// write slicing policy on config file
void write_slicing_policy(char* new_policy) {

  FILE *fp;
  char base_filename[1000];

  const int rbg_num = 25;
  const char policies_delimiter[2] = ",";

  // copy control message so it can be modified
  char *policies = strdup(new_policy);

  // form filename
  strcpy(base_filename, CONFIG_PATH);
  strcat(base_filename, SLICING_BASE_FILENAME);

  // count how many policies were received
  int p_num = 0;
  for (int i = 0; i < strlen(new_policy); ++i) {
    if (new_policy[i] == policies_delimiter[0]) {
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

    char filename[1000];
    strcpy(filename, base_filename);
    strcat(filename, slice_num);
    strcat(filename, ".txt");

    // get current slicing policy
    char* rbg_policy_str = NULL;
    if (s_idx == 0) {
      rbg_policy_str = strtok(policies, policies_delimiter);
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
  }
}


// receive agent control and write it on config files
// expected control looks like: '1,0,0\n5,10,3\n001010123456002::0' --> scheduling on first, slicing on second line, UE slice assignment on third line <imsi>::<slice ID>
void write_control_policies(char* control_msg) {

  // copy control message so it can be modified
  char *control = strdup(control_msg);
  
  char* scheduling_control = NULL;
  char* slicing_control = NULL;
  char* slice_assignment_control = NULL;

  // printf_neat("Received control message: ", control);
  printf_neat("Received control message: ", control);


  // divide scheduling and slicing control
  if (control[0] == '\n') {
    slicing_control = strtok(control, "\n");
  }
  else {
    scheduling_control = strtok(control, "\n");
    slicing_control = strtok(NULL, "\n");
    slice_assignment_control = strtok(NULL, "\n");
  }

  //write slice assignment control
  if (slice_assignment_control) {
    if (strcmp(slice_assignment_control, last_slice_assignment) == 0) {
      printf("Slice assignments are the same as last ones\n");
    }
    else {
      printf_neat("Writing new slice assignments on config file ", slice_assignment_control);
      write_slice_assignment(slice_assignment_control);

      // update last policy
      strcpy(last_slice_assignment, slice_assignment_control);
    }
  }
  else {
    printf("No slice assignment control received\n");
  }


  // write scheduling control
  if (scheduling_control) {
    if (strcmp(scheduling_control, last_scheduling_policy) == 0) {
      printf("Scheduling policies are the same as last ones\n");
    }
    else {
      printf_neat("Writing new scheduling policies on config file ", scheduling_control);
      write_scheduling_policy(scheduling_control);

      // update last policy
      strcpy(last_scheduling_policy, scheduling_control);
    }
  }
  else {
    printf("No scheduling control received\n");
  }

  // write slicing control
  if (slicing_control) {
    if (strcmp(slicing_control, last_slicing_policy) == 0) {
      printf("Slicing policies are the same as last ones\n");
    }
    else{
      printf_neat("Writing new slicing policies on config file ", slicing_control);
      write_slicing_policy(slicing_control);

      // update last policy
      strcpy(last_slicing_policy, slicing_control);
    }
  }
  else {
    printf("No slicing control received\n");
  }
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
int tester(void) {

  char *msg = "1,0,1\n4,5,7\n001010123456002::3";
//  uint8_t* msg2 = "\n10,5,4";

  // write_scheduling_policy((char*) msg);
  write_control_policies((char*) msg);

  return 0;
}
