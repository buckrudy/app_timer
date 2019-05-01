#ifndef __APP_TIMER__H
#define __APP_TIMER__H

#ifdef __cplusplus
extern "C" {
#endif

#define APP_MAX_TIMER 10

typedef void PROC_FUNC(void *data);

/*
 * init timer feature
 * @return: zero success, others failed
 */
int app_timer_init(void);
/*
 * delete all timers
 */
int app_timer_exit(void);
/*
 * Add and Start a timer
 * @first_timeout: the timer first timeout seconds
 * @interval: the interval time seconds, zero means one time timer
 * @p: process functions when timer are expired
 * @user_data: will pass to `p'
 * @return: success return a non-Negative value
 */
int app_add_timer(int first_timeout, int interval, PROC_FUNC *p, void *user_data);
/*  
 * delete a timer
 * @t_fd: file descriptor of the timer
 */
int app_del_timer(int t_fd);

#ifdef __cplusplus
}
#endif
#endif /* __APP_TIMER__H */
