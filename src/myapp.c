#include <stdio.h>
#include <unistd.h>

#define IOMEM_BASE 0x2FF
#define VALUE_REG  (IOMEM_BASE + 3)

// This must be a power of 2!
#define BUFFER_SIZE 8
#define MAX_ITEMS (BUFFER_SIZE-1)
static int my_filter[BUFFER_SIZE];
static int readIdx = 0;
static int writeIdx = 0;


int filter_len(){ return (BUFFER_SIZE + writeIdx - readIdx) % BUFFER_SIZE; }

void filter_add(int val) {
	my_filter[writeIdx] = val;
	writeIdx = (writeIdx+1) & (BUFFER_SIZE-1);
	if(writeIdx == readIdx) readIdx = (readIdx+1) & (BUFFER_SIZE-1);
}

#ifndef TESTING
int myapp_do_dangerous_io()
{
	// lets dereference an io mapped register
	// - on the target it is at address IOMEM_BASE + 3
	return *((int *)VALUE_REG);
}
#endif

int myapp_get_average(){
	int len = filter_len();
	if(0 == len) return 0;
	int sum = 0;
	for(int i = 0; i < len; i++){
		sum += my_filter[(i+readIdx)%BUFFER_SIZE];
	}
	return sum/len;
}

int myapp_task()
{
	// get value from register
	int nextval = myapp_do_dangerous_io();

	// add to filter line
	filter_add(nextval);

	// return the average value as the next delay
	return myapp_get_average();
}

int myapp_mainloop()
{
	for(;;){
		int nextloopdelay = myapp_task();
		sleep(nextloopdelay);
	}
}

#ifndef TESTING
int main() {
	printf("!!!Hello World!!!\n");
	return myapp_mainloop();
}
#endif
