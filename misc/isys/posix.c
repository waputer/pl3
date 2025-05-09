#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

#include "posix.h"
#include "isys.h"
#include "../mdbg/mdbg.h"


/*
 * local declarations
 */
static inline void chk(int err, const char *msg);


/**
 * Retrieve the time in seconds.
 *   &returns: The number of seconds since 1970.
 */
int64_t isys_time(void)
{
	return time(NULL);
}

/**
 * Retrieve the time in microseconds.
 *   &returns: The number of microseconds since 1970.
 */
int64_t isys_utime(void)
{
	struct timeval tv;

	if(gettimeofday(&tv, NULL) < 0)
		fatal("Failed to get current time (%d). %s.", errno, strerror(errno));

	return 1000000 * (int64_t)tv.tv_sec + (int64_t)tv.tv_usec;
}


/**
 * Sleep an interval given in seocnds.
 *   @sec: The number of seconds to sleep.
 */
void isys_sleep(uint64_t sec)
{
	sleep(sec);
}

/**
 * Sleep an interval given in microseocnds.
 *   @usec: The number of microseconds to sleep.
 */
void isys_usleep(uint64_t usec)
{
	struct timespec rem, req = { usec / 1000000, (usec % 1000000) * 1000 };

	while(nanosleep(&req, &rem) < 0) {
		if(errno != EINTR)
			fatal("Failed to sleep (%d). %s.", errno, strerror(errno));

		req = rem;
	}
}


/**
 * Thread structure.
 *   @pthrd: The POSIX thread.
 */
struct isys_thread_t {
	pthread_t pthrd;
};

/**
 * Create a thread.
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: The mutex.
 */
struct isys_thread_t *isys_thread_new(void *(*func)(void *), void *arg)
{
	int err;
	struct isys_thread_t *thread;
	
	thread = malloc(sizeof(struct isys_thread_t));

	err = pthread_create(&thread->pthrd, NULL, func, arg);
	if(err != 0)
		fatal("Failed to create thread. %s.", strerror(errno));

	return thread;
}

/**
 * Detach from a thread.
 *   @thread: The thread.
 */
void isys_thread_detach(struct isys_thread_t *thread)
{
	int err;

	err = pthread_detach(thread->pthrd);
	if(err != 0)
		fatal("Failed to join thread. %s.", strerror(errno));

	free(thread);
}

/**
 * Detach join a thread.
 *   @thread: The thread.
 *   &returns: The thread exit value.
 */
void *isys_thread_join(struct isys_thread_t *thread)
{
	int err;
	void *ptr;

	err = pthread_join(thread->pthrd, &ptr);
	if(err != 0)
		fatal("Failed to join thread. %s.", strerror(errno));

	free(thread);

	return ptr;
}


/**
 * Mutex structure.
 *   @pthrd: The pthread mutex.
 */
struct isys_mutex_t {
	pthread_mutex_t pthrd;
};


/**
 * Create a mutex.
 *   &returns: The mutex.
 */
struct isys_mutex_t *isys_mutex_new(void)
{
	struct isys_mutex_t *mutex;
	
	mutex = malloc(sizeof(struct isys_mutex_t));
	chk(pthread_mutex_init(&mutex->pthrd, NULL), "Failed to create mutex.");

	return mutex;
}

/**
 * Delete a mutex.
 *   @mutex: The mutex.
 */
void isys_mutex_delete(struct isys_mutex_t *mutex)
{
	chk(pthread_mutex_destroy(&mutex->pthrd), "Failed to destroy mutex.");
	free(mutex);
}


/**
 * Lock a mutex.
 *   @mutex: The mutex.
 */
void isys_mutex_lock(struct isys_mutex_t *mutex)
{
	chk(pthread_mutex_lock(&mutex->pthrd), "Failed to lock mutex.");
}

/**
 * Try to lock a mutex.
 *   @mutex: The mutex.
 *   &returns: True if successful, false if unable to lock.
 */
bool isys_mutex_trylock(struct isys_mutex_t *mutex)
{
	int err;

	err = pthread_mutex_trylock(&mutex->pthrd);
	if(err == EBUSY)
		return false;

	chk(err, "Failed attempt to lock mutex.");

	return true;
}

/**
 * Unlock a mutex.
 *   @mutex: The mutex.
 */
void isys_mutex_unlock(struct isys_mutex_t *mutex)
{
	chk(pthread_mutex_unlock(&mutex->pthrd), "Failed to unlock mutex.");
}


/**
 * Event structure.
 *   @fd: The file descriptor.
 */
struct isys_event_t {
	int fd;
};

/**
 * Create an event.
 *   &returns: The event.
 */
struct isys_event_t *isys_event_new(void)
{
	struct isys_event_t *event;

	event = malloc(sizeof(struct isys_event_t));
	event->fd = eventfd(EFD_NONBLOCK, 0);
	if(event->fd < 0)
		fatal("Cannot create event file descriptor.");

	return event;
}

/**
 * Delete the event.
 *   @event: The event.
 */
void isys_event_delete(struct isys_event_t *event)
{
	close(event->fd);
	free(event);
}


/**
 * Retrieve the file descriptor for the event.
 *   @event: The event.
 *   &returns: THe file descriptor.
 */
isys_fd_t isys_event_fd(struct isys_event_t *event)
{
	return event->fd;
}

/**
 * Signal an event.
 *   @event: The event.
 */
void isys_event_signal(struct isys_event_t *event)
{
	uint64_t val = 1;

	if(write(event->fd, &val, 8) < 0)
		fatal("Cannot write to event file descriptor. %s.", strerror(errno));
}

/**
 * Reset an event.
 *   @event: The event.
 */
void isys_event_reset(struct isys_event_t *event)
{
	uint64_t val;

	if(read(event->fd, &val, 8) < 0)
		fatal("Cannot read from event file descriptor. %s.", strerror(errno));
}


/**
 * Asynchronously poll files.
 *   @fds: The file descriptor set.
 *   @cnt: The size of the set.
 *   @timeout: The timeout in milliseconds. Negative waits forever.
 *   &returns: True if on file wakeup, false on timeout.
 */
bool isys_poll(struct isys_poll_t *fds, unsigned int cnt, int timeout)
{
	int err;
	unsigned int i;
	struct pollfd set[cnt];

	for(i = 0; i < cnt; i++) {
		set[i].fd = fds[i].fd;
		set[i].events = 0;
		set[i].events |= (fds[i].mask & ISYS_READ) ? POLLIN : 0;
		set[i].events |= (fds[i].mask & ISYS_WRITE) ? POLLOUT : 0;
		set[i].events |= (fds[i].mask & ISYS_ERROR) ? POLLERR : 0;
	}

	do
		err = poll(set, cnt, timeout);
	while((err < 0) && (errno == EINTR));

	for(i = 0; i < cnt; i++) {
		fds[i].got = 0;
		fds[i].got |= (set[i].revents & POLLIN) ? ISYS_READ : 0;
		fds[i].got |= (set[i].revents & POLLOUT) ? ISYS_WRITE : 0;
		fds[i].got |= (set[i].revents & POLLERR) ? ISYS_ERROR : 0;
	}

	return err > 0;
}


/**
 * Check an error, printing a message and terminating on failure.
 */
static inline void chk(int err, const char *msg)
{
	if(err != 0) {
		fprintf(stderr, "%s. %s.\n", msg, strerror(err));
		abort();
	}
}
