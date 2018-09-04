#ifndef __APP_TIMER__H
#define __APP_TIMER__H

#ifdef __cplusplus
extern "C" {
#endif
#include "list.h"
#define APP_MAX_TIMER 10

typedef void PROC_FUNC(void *data);

struct app_timer_data {
	struct list_head node;
	int is_repeat;
	int t_fd;
	void *user_data;
	void (*timer_process)(void *data);
};

int app_timer_init(void);
int app_timer_exit(void);
int app_add_timer(int first_timeout, int interval, PROC_FUNC *p, void *user_data);
int app_del_timer(int t_fd);

#ifdef __cplusplus
}
#endif
#endif /* __APP_TIMER__H */
