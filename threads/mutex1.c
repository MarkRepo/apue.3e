#include <stdlib.h>
#include <pthread.h>

struct foo {
	int             f_count;
	pthread_mutex_t f_lock;
	int             f_id;
	/* ... more stuff here ... */
};

struct foo *
foo_alloc(int id) /* allocate the object */
{
	struct foo *fp;

	if ((fp = malloc(sizeof(struct foo))) != NULL) {
		fp->f_count = 1;
		fp->f_id = id;
		if (pthread_mutex_init(&fp->f_lock, NULL) != 0) {
			free(fp);
			return(NULL);
		}
		/* ... continue initialization ... */
	}
	return(fp);
}

void
foo_hold(struct foo *fp) /* add a reference to the object */
{
	pthread_mutex_lock(&fp->f_lock);
	fp->f_count++;
	pthread_mutex_unlock(&fp->f_lock);
}

void
foo_rele(struct foo *fp) /* release a reference to the object */
{
	pthread_mutex_lock(&fp->f_lock);
	if (--fp->f_count == 0) { /* last reference */
		pthread_mutex_unlock(&fp->f_lock);
        /*
        * 如果有另一个线程在调用foo_hold时阻塞等待互斥锁，这时即使该对象的引用计数为0，foo_rele释放该对象的内存仍然是不对的。
        * 参考mutex2.c, mutex3.c中在释放对象内存前从列表中移除对象的做法，以确保对象在释放内存前不会被找到来避免上述问题。
        */
		pthread_mutex_destroy(&fp->f_lock);
		free(fp);
	} else {
		pthread_mutex_unlock(&fp->f_lock);
	}
}
