#include "epoch_timer.h"
#define BILLION 1000000000L

//compile with gcc executionName.c -lrt 	
	
	/*ARGSUSED */
int setinterrupt(){
	struct sigaction act;
	
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = sigepoch_hdl;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1))
		return 0;
}

int setperiodic(double sec){
	//timer_t timerid;
	struct itimerspec value;
	
	if(timer_create(CLOCK_REALTIME, NULL, &timerid) == -1)
		return -1;
	
	value.it_interval.tv_sec = (long)sec;
	value.it_interval.tv_nsec = (sec - value.it_interval.tv_sec)*BILLION;
	if(value.it_interval.tv_nsec >= BILLION){
		value.it_interval.tv_sec++;
		value.it_interval.tv_nsec -= BILLION;
	}
	
	value.it_value = value.it_interval;
	return timer_settime(timerid, 0, &value, NULL);
}

int changeperiodic(double sec){
	struct itimerspec value;
	
	value.it_interval.tv_sec = (long)sec;
	value.it_interval.tv_nsec = (sec - value.it_interval.tv_sec)*BILLION;
	if(value.it_interval.tv_nsec >= BILLION){
		value.it_interval.tv_sec++;
		value.it_interval.tv_nsec -= BILLION;
	}
	
	value.it_value = value.it_interval;
	return timer_settime(timerid, 0, &value, NULL);
}
