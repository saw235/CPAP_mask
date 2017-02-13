
#ifndef MAFILTER_H
#define MAFILTER_H

#include <string.h>

void clearBuf(int* arr, const int BUF_LENGTH);
int average(int* arr, const int BUF_LENGTH);
void pushback(int* arr, int newsample, const int BUF_LENGTH);
int MA_Filter_current(int* arr, int newsample, const int BUF_LENGTH) ;

void clearBuf(int* arr, const int BUF_LENGTH) {
	memset(arr, 0, BUF_LENGTH);
}

void pushback(int* arr, int newsample, const int BUF_LENGTH) {
	memmove(arr, arr+1, sizeof(int)*BUF_LENGTH - 1);
	arr[BUF_LENGTH-1] = newsample;
}

int average(int* arr, const int BUF_LENGTH){
	
	long sum = 0;
	for (int i = 0; i < BUF_LENGTH; i++){
		sum += arr[i];
	}

	return sum / BUF_LENGTH;
}

int MA_Filter_current(int* arr, int newsample, const int BUF_LENGTH) {
	pushback(arr, newsample, BUF_LENGTH);
	
	return average(arr, BUF_LENGTH);
}
#endif
