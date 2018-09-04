#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <pthread.h>
#include "app_timer.h"
#include "list.h"

static int app_ep_fd;
static pthread_t app_tid;
static int app_timer_is_init = 0;
static LIST_HEAD(app_timer_list);
static pthread_mutex_t app_timer_list_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t app_init_lock = PTHREAD_MUTEX_INITIALIZER;

static void *app_timer_process(void *arg)
{
	struct epoll_event revents[APP_MAX_TIMER];
	int rc;
	uint64_t count;

	pthread_detach(app_tid);
	for (;;) {
		rc = epoll_wait(app_ep_fd, revents, APP_MAX_TIMER, -1);
		if (rc) {
			int i;
			struct app_timer_data *atd_p;
			for (i=0; i<rc; i++) {
				if (revents[i].events & EPOLLIN) {
					atd_p = (struct app_timer_data *)revents[i].data.ptr;
					read(atd_p->t_fd, &count, sizeof(count));	//必须要此操作
					if (atd_p->timer_process)
						atd_p->timer_process(atd_p->user_data);
					if (atd_p->is_repeat == 0) {	//非重复定时器
						/*
						epoll_ctl(app_ep_fd, EPOLL_CTL_DEL, atd_p->t_fd, (struct epoll_event *)1);
						close(atd_p->t_fd);
						free(atd_p);
						*/
						app_del_timer(atd_p->t_fd);
					}
				}
			}
		}
	}
	return NULL;
}

/*
 * init timer feature
 * @return: zero success, others failed
 */
int app_timer_init(void)
{
	pthread_mutex_lock(&app_init_lock);
	if (app_timer_is_init == 0) {
		app_ep_fd = epoll_create(10);
		if (app_ep_fd < 0) {
			return -1;
		}

		if (0 != pthread_create(&app_tid, NULL, app_timer_process, NULL)) {
			close(app_ep_fd);
			return -2;
		}

		app_timer_is_init = 1;
	}
	pthread_mutex_unlock(&app_init_lock);
	return 0;
}

/*
 * delete all timers
 */
int app_timer_exit(void)
{
	pthread_mutex_lock(&app_init_lock);
	if (app_timer_is_init != 1) {
		return -1;
	}
	if (0 == pthread_cancel(app_tid)) {
		struct app_timer_data *p, *t;
		close(app_ep_fd);
		pthread_mutex_lock(&app_timer_list_lock);
		list_for_each_entry_safe(p, t, &app_timer_list, node) {
			list_del(&p->node);
			close(p->t_fd);
			free(p);
		}
		pthread_mutex_unlock(&app_timer_list_lock);
		app_timer_is_init = 0;
		return 0;
	}
	pthread_mutex_unlock(&app_init_lock);
	return -2;
}

/*
 * Add and Start a timer
 * @first_timeout: the timer first timeout seconds
 * @interval: the interval time seconds, zero means one time timer
 * @p: process functions when timer are expired
 * @user_data: will pass to `p'
 * @return: success return a non-Negative value
 */
int app_add_timer(int first_timeout, int interval, PROC_FUNC *p, void *user_data)
{
	int t_fd;
	struct epoll_event ev;
	struct app_timer_data *atd_p;
	struct itimerspec timeout = {
		.it_value = {
			.tv_sec = first_timeout,
			.tv_nsec = 0
		},
		.it_interval = {
			.tv_sec = interval,
			.tv_nsec = 0
		}
	};


	t_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (t_fd < 0)
		return -1;

	atd_p = malloc(sizeof(struct app_timer_data));
	if (atd_p == NULL) {
		close(t_fd);
		return -2;
	}

	atd_p->t_fd = t_fd;

	if (interval != 0)
		atd_p->is_repeat = 1;
	if (p)
		atd_p->timer_process = p;
	if (user_data)
		atd_p->user_data = user_data;

	if (0 != timerfd_settime(t_fd, 0, &timeout, NULL)) {
		free(atd_p);
		close(t_fd);
		return -3;
	}

	ev.events = EPOLLIN;
	ev.data.ptr = atd_p;
	if (0 != epoll_ctl(app_ep_fd, EPOLL_CTL_ADD, t_fd, &ev)) {
		free(atd_p);
		close(t_fd);
		return -4;
	}

	pthread_mutex_lock(&app_timer_list_lock);
	list_add_tail(&atd_p->node, &app_timer_list);
	pthread_mutex_unlock(&app_timer_list_lock);

	return t_fd;
}

/*  
 * delete a timer
 * @t_fd: file descriptor of the timer
 */
int app_del_timer(int t_fd)
{
	struct app_timer_data *p, *t;
	epoll_ctl(app_ep_fd, EPOLL_CTL_DEL, t_fd, (struct epoll_event*)1);
	close(t_fd);
	pthread_mutex_lock(&app_timer_list_lock);
	list_for_each_entry_safe(p, t, &app_timer_list, node) {
		if (p->t_fd == t_fd) {
			list_del(&p->node);
			free(p);
		}
	}
	pthread_mutex_unlock(&app_timer_list_lock);
	return 0;
}
