#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>
#include <mosquitto.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "mfcc.cc"
#include "svm.h"

struct svm_node *x;
struct svm_model* model;
int max_nr_attr = 128*128;
int predict_probability=1;

int channel = 1;
unsigned int rate = 44100;
int frames = 44100*5;
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
char hw_card[] = "hw:1";
MFCC mfccComputer(44100, 128, 25, 10, 40, 50, 44100/2);

snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;

struct mosquitto *mosq = NULL;
char topic[] = "cry-detection";

unsigned long long getms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1.0e+3 + tv.tv_usec/1000;
}


/*
 *   Copy from https://gist.github.com/albanpeignier/104902
 */

void asound_init() {

  int err;

  if ((err = snd_pcm_open(&capture_handle, hw_card, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             hw_card,
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface opened\n");
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params allocated\n");
				 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params initialized\n");
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params access setted\n");
	
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params format setted\n");
	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    exit (1);
  }
	
  fprintf(stdout, "hw_params rate setted\n");

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params channels setted\n");
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params setted\n");
	
  snd_pcm_hw_params_free (hw_params);

  fprintf(stdout, "hw_params freed\n");
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface prepared\n");

}

void asound_deinit() {
  snd_pcm_close(capture_handle);
  fprintf(stdout, "audio interface closed\n");
}

/*
 *  https://gist.github.com/piccaso/f463f6ad134a8a2479e62e6e2349e7c0
 */ 

void mqtt_setup(){

  char host[] = "localhost";
  int port = 1883;
  int keepalive = 60;
  bool clean_session = true;
  
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, clean_session, NULL);
  if(!mosq){
	fprintf(stderr, "Error: Out of memory.\n");
	exit(1);
  }
  
  //mosquitto_log_callback_set(mosq, mosq_log_callback);
  
  if(mosquitto_connect(mosq, host, port, keepalive)){
	fprintf(stderr, "Unable to connect.\n");
	exit(1);
  }

  /*
  int loop = mosquitto_loop_start(mosq);
  if(loop != MOSQ_ERR_SUCCESS){
    fprintf(stderr, "Unable to start loop: %i\n", loop);
    exit(1);
  }
  */
}

int mqtt_send(const char *msg){
  return mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 0, 0);
}


bool predict() {

  int j;
        svm_get_svm_type(model);
        int nr_class=svm_get_nr_class(model);
        double *prob_estimates=NULL;

        prob_estimates = (double *)malloc(nr_class*sizeof(double));
  svm_predict_probability(model, x, prob_estimates);
  for(j=0;j<nr_class;j++)
        printf(" %g",prob_estimates[j]);
  printf("\n");


        if(predict_probability)
                free(prob_estimates);

  if(prob_estimates[0] > 0.5)
    return true;
  else
    return false;
}



void* recognize(void *data) {
  int16_t *buffer = (int16_t*)data;

  printf("Start to recognize...\n");
  long long start = getms();
  mfccComputer.processBuffer(buffer, 44100*3);

  printf("mel spectrum time = %lld\n", getms() - start);
  start = getms();
  for(int i = 0; i < max_nr_attr; i++) {
    x[i].index = (1+1);
    x[i].value = 1.0;
  }
  printf("result = %d\n", predict());
  printf("SVM time = %lld\n", getms() - start);
  /*
  for(int j = 0; j < 10; j++) {
    printf("%d ", buffer[j]);
  }
  */
  mqtt_send("detect crying...");
  printf("......................\n");
  pthread_exit(NULL); // 離開子執行緒
}
	      

void svm_init() {

        if((model=svm_load_model("model.txt"))==0)
        {
                fprintf(stderr,"can't open model file \n");
                exit(1);
        }
        x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
}


void svm_deinit() {
        svm_free_and_destroy_model(&model);
        free(x);
}





int main (int argc, char *argv[]) {
  int err;
  uint8_t *buffer;

  svm_init();

  asound_init();

  mqtt_setup();

  pthread_t t; 

  //printf("%d\n", snd_pcm_format_width(format));

  uint8_t *buffer1, *buffer2;
  buffer1 = (uint8_t*)malloc(frames * snd_pcm_format_width(format) / 8 * 1);
  buffer2 = (uint8_t*)malloc(frames * snd_pcm_format_width(format) / 8 * 1);

  uint8_t *recv_buffer, *recg_buffer, *tmp;

  recv_buffer = buffer1;
  recg_buffer = buffer2;

  //fprintf(stdout, "buffer allocated\n");


  while(1) {

    long long start = getms();
    if ((err = snd_pcm_readi (capture_handle, recv_buffer, frames)) != frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               err, snd_strerror (err));
      exit (1);
    }
    printf("%lld\n", getms() - start);
/*   
    printf("read buffer........\n"); 
    for(int j = 0; j < 10; j++) {
      printf("%d ", recv_buffer[j]);
    }
    printf("............\n");
*/
    //swap buffer
    tmp = recg_buffer;
    recg_buffer = recv_buffer;
    recv_buffer = tmp;


    pthread_create(&t, NULL, recognize, recg_buffer);
    //fprintf(stdout, "read %d done\n", i);
  }

  pthread_join(t, NULL);
  free(buffer1);
  free(buffer2);

  fprintf(stdout, "buffer freed\n");
	
  asound_deinit();
  svm_deinit();
  return 0;
}
