#pragma once

/*
 * common headers
 */
#include <stdbool.h>
#include <stdint.h>

/*
 * poll mask definitions
 */
#define ISYS_READ  (0x01)
#define ISYS_WRITE (0x02)
#define ISYS_ERROR (0x04)
#define ISYS_ALL   (0x07)

/**
 * Poll structure.
 *   @fd: The file descriptor.
 *   @mask, got: The mask and gotten events.
 */
struct isys_poll_t {
	isys_fd_t fd;
	uint16_t mask, got;
};
#define isys_pollfd(fd, mask) ((struct isys_poll_t){ fd, mask, 0 })


/*
 * time declarations
 */
int64_t isys_time(void);
int64_t isys_utime(void);

void isys_sleep(uint64_t sec);
void isys_usleep(uint64_t usec);

/*
 * thread declarations
 */
typedef struct isys_thread_t *isys_thread_t;

isys_thread_t isys_thread_new(void *(*func)(void *), void *arg);
void isys_thread_detach(isys_thread_t thread);
void *isys_thread_join(isys_thread_t thread);

/*
 * mutex declarations
 */
typedef struct isys_mutex_t *isys_mutex_t;

isys_mutex_t isys_mutex_new(void);
void isys_mutex_delete(isys_mutex_t mutex);

void isys_mutex_lock(isys_mutex_t mutex);
bool isys_mutex_trylock(isys_mutex_t mutex);
void isys_mutex_unlock(isys_mutex_t mutex);

/*
 * event declarations
 */
struct isys_event_t *isys_event_new(void);
void isys_event_delete(struct isys_event_t *event);

isys_fd_t isys_event_fd(struct isys_event_t *event);
void isys_event_signal(struct isys_event_t *event);
void isys_event_reset(struct isys_event_t *event);

/*
 * polling declarations
 */
bool isys_poll(struct isys_poll_t *fds, unsigned int cnt, int timeout);
uint16_t isys_poll1(isys_fd_t fd, uint16_t mask, int timeout);

/*
 * task declarations
 */
struct isys_task_t *isys_task_new(void (*func)(isys_fd_t, void *), void *arg);
void isys_task_delete(struct isys_task_t *task);
