#ifndef EPOCH_TIMER_H
#define EPOCH_TIMER_H
#pragma once

#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void interrupt(int signo, siginfo_t *info, void *context);
int setinterrupt();
int setperiodic(double sec);
int changeperiodic(double sec);
timer_t timerid;

#endif
