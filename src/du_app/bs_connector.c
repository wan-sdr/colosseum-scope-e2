#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "bs_connector.h"
#include "csv_reader.h"
//#include "du_e2ap_msg_hdl.h"

int report_data_nrt_ric = 1;


// handle timer got from RIC Subscription Request
// timer is in seconds
void handleTimer(float* timer, uint32_t* ric_req_id) {

  pthread_t thread;
  printf("Handle timer %f seconds, ricReqId %" PRIu32 "\n", timer[0], ric_req_id[0]);

  // populate thread arguments
  thread_args *t_args = (thread_args*) calloc(1, sizeof(thread_args));
  t_args->timer = timer;
  t_args->ric_req_id = ric_req_id;

  // start thread
  report_data_nrt_ric = 1;
  if (pthread_create(&thread, NULL, periodicDataReport, (void *) t_args) != 0) {
    printf("Error creating thread\n");
  }

  printf("periodicDataReport thread created successfully\n");
}


// function to periodically report data
void *periodicDataReport(void* arg) {

  thread_args *t_args = (thread_args*) arg;

  // retrieve timer
  float* timer = t_args->timer;
  float timer_deref = timer[0];
  printf("timer expired, timer_deref %f\n", timer_deref);

  // retrieve ricReqId
  uint32_t* ric_req_id = t_args->ric_req_id;
  uint32_t ric_req_id_deref = ric_req_id[0];
  printf("ricReqId %" PRIu32 "\n", ric_req_id_deref);
  
  if (DEBUG) {
    // debug
    printf("Debug mode\n");
    char *payload = "0,1,2,3,4,5\n1,6,7,8,9,10\n2,11,12,13,14,15";

    // to send timer as payload
    //char *payload = NULL;
    //asprintf(&payload, "%g", timer_deref);

    BuildAndSendRicIndicationReport(payload, strlen(payload), ric_req_id_deref);
  }
  else {
    sendMetricsXapp(ric_req_id_deref);
  }

  sleep(timer_deref);

  if (report_data_nrt_ric) {
    periodicDataReport((void*) t_args);
  }
}


// get and send metrics to xApp
void sendMetricsXapp(uint32_t ric_req_id) {

  char *payload = NULL;
  int lines_to_read = LINES_TO_READ;

  //printf("[mau][bs_connector] calling get_tx_string\n");
  get_tx_string(&payload, lines_to_read);

  if (payload) {
    int payload_len = strlen(payload);

    // split if more than maximum payload for ric indication report
    char *chunk = NULL;
    chunk = (char*) calloc(MAX_REPORT_PAYLOAD + 1, sizeof(char));

    // send in chunks, append
    for (int i = 0; i < payload_len; i += MAX_REPORT_PAYLOAD) {
      memset(chunk, 0, MAX_REPORT_PAYLOAD + 1 + 1);

      int offset = 0;
      // add 'm' at the beginning to indicate there are more chunks
      if (i + MAX_REPORT_PAYLOAD < payload_len) {
        strcpy(chunk, "m");
        offset = 1;
      }
      size_t numcpy_chars = 0;
      if (payload_len < MAX_REPORT_PAYLOAD){
        numcpy_chars = payload_len;
      }else{
        numcpy_chars = MAX_REPORT_PAYLOAD;
      }
      strncpy(chunk + offset, payload + i, numcpy_chars);

      BuildAndSendRicIndicationReport(chunk, strlen(chunk), ric_req_id);
    }

    printf("Sent RICIndicationReport\n");

    free(chunk);
    chunk = NULL;

    free(payload);
    payload = NULL;
  }
}


// log message on file
void log_message(char* message, char* message_type, int len) {

  FILE *fp;
  char filename[100] = "/logs/du_l2.log";

  char buffer[26];
  int millisec;
  struct tm* tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
  if (millisec>=1000) { // Allow for rounding up to nearest second
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);

  strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

  fp = fopen(filename, "a+");

  if (fp == NULL) {
    printf("ERROR: fp is NULL\n");
    return;
  }

  const int msg_len = len;
  char msg_copy[msg_len];
  strcpy(msg_copy, message);

  for (int i = 0; i < msg_len; i++)
  {
    if (message[i] == '\n') {
       msg_copy[i] = 'n';
    }
  }

  // print to console and log on file
  printf("%s,%03d\t%s\t%d\t%s\n", buffer, millisec, message_type, len, msg_copy);
  fprintf(fp, "%s,%03d\t%s\t%d\t%s\n", buffer, millisec, message_type, len, msg_copy);

  fclose(fp);
}


// terminate periodic thread that reports data to near real-time RIC
void stop_data_reporting_nrt_ric(void) {
  printf("Terminating data reporting to non real-time RIC\n");
  report_data_nrt_ric = 0;
}
