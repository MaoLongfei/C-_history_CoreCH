#ifndef TIMER_H
#define TIMER_H

/*
#include <sys/time.h>

//! Returns the number of microseconds since the epoch, i.e., since 1st Jan 1970 00:00 UTC.
inline
long long get_micro_time(){
	timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec*1000000ll+t.tv_usec;
}
*/

#include <chrono>

//! Returns the number of microseconds since the epoch, i.e., since 1st Jan 1970 00:00 UTC.
long long get_micro_time(){
	return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}

#endif
