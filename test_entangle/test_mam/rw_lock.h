/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 * 
 * Modifications copyright (c) Zan-Jun Wang
 */

#ifndef __UTILS_HANDLES_RW_LOCK_H__
#define __UTILS_HANDLES_RW_LOCK_H__

/**
 * We define a type rw_lock_handle_t mapping to a system available read/write
 * lock primitive and its associated functions, some of them might have no
 * effect if not needed by the underlying API
 */
#if !defined(_WIN32) && defined(__unix__) || defined(__unix) || \
    (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

#ifdef _POSIX_THREADS

#include <pthread.h>

typedef pthread_rwlock_t rw_lock_handle_t;

static inline int rw_lock_handle_init(rw_lock_handle_t* const lock) {
  return pthread_rwlock_init(lock, NULL);
}

static inline int rw_lock_handle_rdlock(rw_lock_handle_t* const lock) {
  return pthread_rwlock_rdlock(lock);
}

static inline int rw_lock_handle_wrlock(rw_lock_handle_t* const lock) {
  return pthread_rwlock_wrlock(lock);
}

static inline int rw_lock_handle_unlock(rw_lock_handle_t* const lock) {
  return pthread_rwlock_unlock(lock);
}

static inline int rw_lock_handle_destroy(rw_lock_handle_t* const lock) {
  return pthread_rwlock_destroy(lock);
}

#elif defined(_WIN32)

typedef SRWLOCK rw_lock_handle_t;

static inline int rw_lock_handle_init(rw_lock_handle_t* const lock) {
  InitializeSRWLock(lock);
  return 0;
}

static inline int rw_lock_handle_rdlock(rw_lock_handle_t* const lock) {
  AcquireSRWLockShared(lock);
  return 0;
}

static inline int rw_lock_handle_wrlock(rw_lock_handle_t* const lock) {
  AcquireSRWLockExclusive(lock);
  return 0;
}

static inline int rw_lock_handle_unlock(rw_lock_handle_t* const lock) {
  void* state = *(void**)lock;

  if (state == (void*)1) {
    ReleaseSRWLockExclusive(lock);
  } else {
    ReleaseSRWLockShared(lock);
  }
  return 0;
}

static inline int rw_lock_handle_destroy(rw_lock_handle_t* const lock) {
  return 0;
}

#else

//#error "No read/write lock primitive found"primitive

typedef uint32_t rw_lock_handle_t;
//int readers;
//rw_lock_handle_t mutex;
//rw_lock_handle_t roomEmpty;

static inline int rw_lock_handle_init(rw_lock_handle_t* const lock) {
  /*readers = 0;
  mutex = os_semaphore_create(1);
  roomEmpty = os_semaphore_create(1);*/
  return 0;
}

static inline int rw_lock_handle_rdlock(rw_lock_handle_t* const lock) {
  /*os_semaphore_wait(mutex, 0xFFFFFFFF);
  readers += 1;
  if (readers == 1) {
    os_semaphore_wait(roomEmpty, 0xFFFFFFFF);
  }
  os_semaphore_release(mutex);*/
  return 0;
}

static inline int rw_lock_handle_wrlock(rw_lock_handle_t* const lock) {
  //os_semaphore_wait(roomEmpty, 0xFFFFFFFF);
  return 0;
}

static inline int rw_lock_handle_unlock(rw_lock_handle_t* const lock) {
  /*void* state = *(void**)lock;

  if (state == (void*)1) {
    //writer
    os_semaphore_release(roomEmpty);
  } else {
    //reader
    os_semaphore_wait(mutex, 0xFFFFFFFF);
    readers -= 1;
    if (readers == 0) {
      os_semaphore_release(roomEmpty);
    }    
    os_semaphore_release(mutex);
  }*/
  return 0;
}

static inline int rw_lock_handle_destroy(rw_lock_handle_t* const lock) {
  return 0;
}

#endif  // _POSIX_THREADS

/**
 * Initializes a read/write lock
 *
 * @param lock The lock
 *
 * @return exit status
 */
static inline int rw_lock_handle_init(rw_lock_handle_t* const lock);

/**
 * Acquires reading ownership of the given read/write lock
 *
 * @param lock The lock
 *
 * @return exit status
 */
static inline int rw_lock_handle_rdlock(rw_lock_handle_t* const lock);

/**
 * Acquires writing ownership of the given read/write lock
 *
 * @param lock The lock
 *
 * @return exit status
 */
static inline int rw_lock_handle_wrlock(rw_lock_handle_t* const lock);

/**
 * Releases ownership of the given read/write lock
 *
 * @param lock The lock
 *
 * @return exit status
 */
static inline int rw_lock_handle_unlock(rw_lock_handle_t* const lock);

/**
 * Destroys the read/write lock
 *
 * @param lock The lock
 *
 * @return exit status
 */
static inline int rw_lock_handle_destroy(rw_lock_handle_t* const lock);

#endif  // __UTILS_HANDLES_RW_LOCK_H__
