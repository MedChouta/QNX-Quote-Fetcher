//Based on the QNX timer_create() example


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <curl/curl.h>
#include <string.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

#define handle_error(msg) \
	do {perror(msg); exit(EXIT_FAILURE);} while(0)


typedef union {
    struct _pulse   pulse;
    /* your other message structures would go here too */
} my_message_t;


typedef struct {
	char *data;
	size_t size;
} RESPONSE;


static size_t WriteMemoryCallback(void *content, size_t size, size_t nmemb, void *userp){

	//Heavily inspired by the following example: https://curl.se/libcurl/c/getinmemory.html

	size_t realsize = size * nmemb;
	RESPONSE *response = (RESPONSE *)userp;

	char *ptr = realloc(response->data, response->size + realsize + 1);
	 if(!ptr) {
	   /* out of memory! */
	   printf("not enough memory (realloc returned NULL)\n");
	   return 0;
	 }

	 response->data = ptr;
	 memcpy(&(response->data [response->size]), content, realsize);
	 response->size += realsize;
	 response->data[response->size] = '\0';

	 return realsize;
}


char* extract_quote(const char *json_string) {
    // Find the start of the "quote" field
    const char *quote_start = strstr(json_string, "\"quote\":\"");
    if (!quote_start) {
        printf("'quote' field not found.\n");
        return NULL;
    }

    // Move the pointer to the beginning of the actual quote text
    quote_start += strlen("\"quote\":\"");

    // Find the end of the quote (closing double quote)
    const char *quote_end = strchr(quote_start, '"');
    if (!quote_end) {
        printf("End of 'quote' field not found.\n");
        return NULL;
    }

    // Calculate the length of the quote
    size_t quote_length = quote_end - quote_start;

    // Allocate memory for the quote (plus null terminator)
    char *quote = malloc(quote_length + 1);
    if (!quote) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Copy the quote into the allocated memory
    strncpy(quote, quote_start, quote_length);
    quote[quote_length] = '\0'; // Null-terminate the string

    return quote;
}

void fetch_quote(){

	CURL *curl = curl_easy_init();

	if(curl){
		CURLcode res;

		RESPONSE response;

		response.data = malloc(1);
		response.size = 0;

		curl_easy_setopt(curl, CURLOPT_URL, "https://api.kanye.rest/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		res = curl_easy_perform(curl);

		if(res != CURLE_OK) {
		    fprintf(stderr, "curl_easy_perform() failed: %s\n",
		    curl_easy_strerror(res));
		 }else{
			 char *quote = extract_quote(response.data);
			 printf("Kanye said: \"%s\"\n", quote);
		 }

		curl_easy_cleanup(curl);
		free(response.data);
	}else{
		handle_error("error setting up curl");
	}

}


int main()
{

    struct sigevent event;
    struct itimerspec itime;
    timer_t timer_id;
    int chid;
    rcvid_t rcvid;
    my_message_t msg;
    struct sched_param scheduling_params;
    int prio;

    chid = ChannelCreate(0);

    /* Get our priority. */
    if (SchedGet( 0, 0, &scheduling_params) != -1)
    {
        prio = scheduling_params.sched_priority;
    }
    else
    {
        prio = 10;
    }

    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    SIGEV_PULSE_INIT(&event, coid, prio, MY_PULSE_CODE, 0);

    timer_create(CLOCK_MONOTONIC, &event, &timer_id);

    itime.it_value.tv_sec = 1;
    /* 500 million nsecs = .5 secs */
    itime.it_value.tv_nsec = 500000000;
    itime.it_interval.tv_sec = 1;
    /* 500 million nsecs = .5 secs */
    itime.it_interval.tv_nsec = 500000000;
    timer_settime(timer_id, 0, &itime, NULL);

    /*
     * As of the timer_settime(), we will receive our pulse in 1.5 seconds
     * (the itime.it_value) and every 1.5 seconds thereafter
     * (the itime.it_interval)
     */
    for (;;) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0) { /* we got a pulse */
            if (msg.pulse.code == MY_PULSE_CODE) {
            	fetch_quote();
            } /* else other pulses ... */
        } /* else other messages ... */
    }
    return(EXIT_SUCCESS);
}
