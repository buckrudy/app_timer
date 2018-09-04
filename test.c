#include <stdio.h>
#include <unistd.h>
#include "app_timer.h"

struct TT {
	char timer_name[16];
	int data;
};

void time1_2_handle(void *data)
{
	struct TT *p = (struct TT*)data;
	printf("%s: %d\n", p->timer_name, p->data++);
}

void time3_handle(void *data)
{
	printf("time3.\n");
}

int main(void)
{
	struct TT count1 = {"timer1", 0}, count2 = {"timer2", 0};
	int xxx = 0;
	int t1, t2;
	if (0 != app_timer_init()) {
		printf("app_timer_init() err\n");
		return 1;
	}

	t1 = app_add_timer(1, 5, time1_2_handle, (void *)&count1);

	t2 = app_add_timer(1, 1, time1_2_handle, (void *)&count2);

	app_add_timer(10, 0, time3_handle, NULL);
	while (1) {
		sleep(2);
		xxx++;
		if (xxx == 10) {
			printf("delete timer2\n");
			app_del_timer(t2);
		}
		if (xxx == 20) {
			printf("delete app_timer\n");
			app_timer_exit();
		}
	}
	return 0;
}
