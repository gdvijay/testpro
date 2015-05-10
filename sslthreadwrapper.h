
#ifndef SSL_THREAD_H
#define SSL_THREAD_H


#include <pthread.h>
#include <openssl/ssl.h>

static pthread_mutex_t *mutex_buf = NULL;


struct CRYPTO_dynlock_value
{
  pthread_mutex_t mutex;
};


static void
locking_function (int mode, int n, const char *file, int line)
{
  if (mode & CRYPTO_LOCK)
    {
      pthread_mutex_lock (&(mutex_buf[n]));
    }
  else
    {
      pthread_mutex_unlock (&(mutex_buf[n]));
    }
}

static unsigned long
id_function (void)
{
  return ((unsigned long) pthread_self ());
}



static struct CRYPTO_dynlock_value *
dyn_create_function (const char *file, int line)
{

  struct CRYPTO_dynlock_value *value;
  value =
    (struct CRYPTO_dynlock_value *)
    malloc (sizeof (struct CRYPTO_dynlock_value));
  if (!value)
    {
      return NULL;
    }
  pthread_mutex_init (&(value->mutex), NULL);
  return value;
}


static void
dyn_lock_function (int mode, struct CRYPTO_dynlock_value *l,
		   const char *file, int line)
{
  if (mode & CRYPTO_LOCK)
    pthread_mutex_lock (&(l->mutex));

  else
    pthread_mutex_unlock (&(l->mutex));
}

static void
dyn_destroy_function (struct CRYPTO_dynlock_value *l, const char *file,
		      int line)
{
  pthread_mutex_destroy (&(l->mutex));
  free (l);
}


static int ssl_thread_setup (void)
{
  int i;
  mutex_buf =
    (pthread_mutex_t *) malloc (CRYPTO_num_locks () *
				sizeof (pthread_mutex_t));
  if (!mutex_buf)
    {
      return 0;
    }

  for (i = 0; i < CRYPTO_num_locks (); i++)
    {
      pthread_mutex_init (&(mutex_buf[i]), NULL);
    }
  CRYPTO_set_id_callback (id_function);
  CRYPTO_set_locking_callback (locking_function);

/* The following three CRYPTO_... functions are the OpenSSL functions
for registering the callbacks we implemented above */

  CRYPTO_set_dynlock_create_callback (dyn_create_function);
  CRYPTO_set_dynlock_lock_callback (dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback (dyn_destroy_function);
  return 1;
}



static int ssl_thread_cleanup (void)
{
  int i;
  if (!mutex_buf)
    {
      return 0;
    }
  CRYPTO_set_id_callback (NULL);
  CRYPTO_set_locking_callback (NULL);
  CRYPTO_set_dynlock_create_callback (NULL);
  CRYPTO_set_dynlock_lock_callback (NULL);
  CRYPTO_set_dynlock_destroy_callback (NULL);
  for (i = 0; i < CRYPTO_num_locks (); i++)
    {
      pthread_mutex_destroy (&(mutex_buf[i]));
      free (mutex_buf);
    }
  mutex_buf = NULL;
  return 1;
}


#endif // SSL_THREAD