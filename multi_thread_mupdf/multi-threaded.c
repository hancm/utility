// Multi-threaded rendering of all pages in a document to PNG images.

// First look at doc/example.c and make sure you understand it.
// Then read the multi-threading section in doc/overview.txt,
// before coming back here to see an example of multi-threading.

// This example will create one main thread for reading pages from the
// document, and one thread per page for rendering. After rendering
// the main thread will wait for each rendering thread to complete before
// writing that thread's rendered image to a PNG image. There is
// nothing in MuPDF requiring a rendering thread to only render a
// single page, this is just a design decision taken for this example.

// Compile a debug build of mupdf, then compile and run this example:
//
// gcc -g -o build/debug/example-mt -Iinclude docs/multi-threaded.c \
//	build/debug/libmupdf.a \
//	build/debug/libmupdfthird.a \
//	-lpthread -lcrypto -lm
//
// build/debug/example-mt /path/to/document.pdf
//
// Caution! As all pages are rendered simultaneously, please choose a
// file with just a few pages to avoid stressing your machine too
// much. Also you may run in to a limitation on the number of threads
// depending on your environment.

// Include the MuPDF header file, and pthread's header file.
#include <mupdf/fitz.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <unistd.h>

// 像素信息
static std::list<void*> g_pixMapList;
static pthread_mutex_t g_pixMapListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_pixMapListCond  = PTHREAD_COND_INITIALIZER;

// 是否完成
static int g_finishPagesNum = 0;
static bool g_isFinishStatus = false;
static pthread_mutex_t g_finishMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_finishCond  = PTHREAD_COND_INITIALIZER;
// A convenience function for dying abruptly on pthread errors.

void
fail(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

// The data structure passed between the requesting main thread and
// each rendering thread.

struct data {
	// A pointer to the original context in the main thread sent
	// from main to rendering thread. It will be used to create
	// each rendering thread's context clone.
	fz_context *ctx;

	// Page number sent from main to rendering thread for printing
	int pagenumber;

    int total_pagecnt;

	// The display list as obtained by the main thread and sent
	// from main to rendering thread. This contains the drawing
	// commands (text, images, etc.) for the page that should be
	// rendered.
	fz_display_list *list;

	// The area of the page to render as obtained by the main
	// thread and sent from main to rendering thread.
	fz_rect bbox;

	// This is the result, a pixmap containing the rendered page.
	// It is passed first from main thread to the rendering
	// thread, then its samples are changed by the rendering
	// thread, and then back from the rendering thread to the main
	// thread.
	fz_pixmap *pix;
};

// This is the function run by each rendering function. It takes
// pointer to an instance of the data structure described above and
// renders the display list into the pixmap before exiting.

void *
renderer(void *data)
{
	int pagenumber = ((struct data *) data)->pagenumber;
	fz_context *ctx = ((struct data *) data)->ctx;
	fz_display_list *list = ((struct data *) data)->list;
	fz_rect bbox = ((struct data *) data)->bbox;
	fz_pixmap *pix = ((struct data *) data)->pix;
	fz_device *dev;

	fprintf(stderr, "thread at page %d loading!\n", pagenumber);

	// The context pointer is pointing to the main thread's
	// context, so here we create a new context based on it for
	// use in this thread.

	ctx = fz_clone_context(ctx);

	// Next we run the display list through the draw device which
	// will render the request area of the page to the pixmap.

	fprintf(stderr, "thread at page %d rendering!\n", pagenumber);
	dev = fz_new_draw_device(ctx, &fz_identity, pix);
	fz_run_display_list(ctx, list, dev, &fz_identity, &bbox, NULL);
	fz_close_device(ctx, dev);
	fz_drop_device(ctx, dev);

	// This threads context is freed.

	fz_drop_context(ctx);

	fprintf(stderr, "thread at page %d done!\n", pagenumber);

    pthread_mutex_lock(&g_pixMapListMutex);
    g_pixMapList.push_back(data);

    printf("sizeof g_pixMapList: %d.\n", g_pixMapList.size());
     //广播条件变量，唤醒正在等待的线程
    pthread_cond_signal(&g_pixMapListCond);
    printf("renderer thread singal cond\n");
    pthread_mutex_unlock(&g_pixMapListMutex);
    printf("renderer thread unlocked\n");

	return data;
}

// These are the two locking functions required by MuPDF when
// operating in a multi-threaded environment. They each take a user
// argument that can be used to transfer some state, in this case a
// pointer to the array of mutexes.

void lock_mutex(void *user, int lock)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *) user;

	if (pthread_mutex_lock(&mutex[lock]) != 0)
		fail("pthread_mutex_lock()");
}

void unlock_mutex(void *user, int lock)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *) user;

	if (pthread_mutex_unlock(&mutex[lock]) != 0)
		fail("pthread_mutex_unlock()");
}

void *PixMapMain(void *param)
{
    while (!g_isFinishStatus)
    {
        pthread_mutex_lock(&g_pixMapListMutex);
        while (g_pixMapList.empty()) {
            printf("###wait g_pixMapList.\n");
            pthread_cond_wait(&g_pixMapListCond, &g_pixMapListMutex);
        }

        struct data* data = (struct data*)g_pixMapList.front();
        g_pixMapList.pop_front();
        pthread_mutex_unlock(&g_pixMapListMutex);

        fz_context *ctx = data->ctx;
        ctx = fz_clone_context(ctx);
        int page_cnt = data->total_pagecnt;

        fprintf(stderr, "####66 Page: %d\n", data->pagenumber);

        char filename[42];

        sprintf(filename, "out%04d.png", data->pagenumber);
        fprintf(stderr, "\tSaving %s...\n", filename);

        // Write the rendered image to a PNG file

        fz_save_pixmap_as_png(ctx, data->pix, filename);

        // Free the thread's pixmap and display list since
        // they were allocated by the main thread above.

        fz_drop_pixmap(ctx, data->pix);
        fz_drop_display_list(ctx, data->list);

        // Free the data structured passed back and forth
        // between the main thread and rendering thread.

        free(data);
        data = NULL;

        pthread_mutex_lock(&g_finishMutex);
        ++g_finishPagesNum;
        printf("finish page num: %d.\n", g_finishPagesNum);
        if (g_finishPagesNum == page_cnt)
        {
            printf("Finish cond signal.\n");
            g_isFinishStatus = true;
            pthread_cond_signal(&g_finishCond);
        }
        pthread_mutex_unlock(&g_finishMutex);
    }
}

int make_thread(pthread_t *pthreadList, int threadCnt, void *(fun)(void*), void *param = NULL)
{
    for (int i = 0; i < threadCnt; ++i)
    {
        if (pthread_create(&pthreadList[i], NULL, fun, param) != 0) {
            fail("pthread_create()");
        }
        pthread_detach(pthreadList[i]);
    }

    return 0;
}

//pthread_t pthreadList[1];
//int pixMapThread = make_thread(pthreadList, sizeof(pthreadList) / sizeof(*pthreadList), PixMapMain);

int main(int argc, char **argv)
{
    pthread_t pthreadList[8];
    make_thread(pthreadList, sizeof(pthreadList) / sizeof(*pthreadList), PixMapMain);

    const char *filename = argc >= 2 ? argv[1] : "";
	pthread_t *thread = NULL;
	fz_locks_context locks;
	pthread_mutex_t mutex[FZ_LOCK_MAX];
	fz_context *ctx;
	fz_document *doc;
	int threads;
	int i;

	// Initialize FZ_LOCK_MAX number of non-recursive mutexes.

	for (i = 0; i < FZ_LOCK_MAX; i++)
	{
		if (pthread_mutex_init(&mutex[i], NULL) != 0)
			fail("pthread_mutex_init()");
	}

	// Initialize the locking structure with function pointers to
	// the locking functions and to the user data. In this case
	// the user data is a pointer to the array of mutexes so the
	// locking functions can find the relevant lock to change when
	// they are called. This way we avoid global variables.

	locks.user = mutex;
	locks.lock = lock_mutex;
	locks.unlock = unlock_mutex;

	// This is the main threads context function, so supply the
	// locking structure. This context will be used to parse all
	// the pages from the document.

	ctx = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);

	// Register default file types.

	fz_register_document_handlers(ctx);

	// Open the PDF, XPS or CBZ document. Note, this binds doc to ctx.
	// You must only ever use doc with ctx - never a clone of it!

	doc = fz_open_document(ctx, filename);

	// Retrieve the number of pages, which translates to the
	// number of threads used for rendering pages.

	threads = fz_count_pages(ctx, doc);
	fprintf(stderr, "spawning %d threads, one per page...\n", threads);

    thread = (pthread_t*)malloc(threads * sizeof (pthread_t));

	for (i = 0; i < threads; i++)
	{
		fz_page *page;
		fz_rect bbox;
		fz_irect rbox;
		fz_display_list *list;
		fz_device *dev;
		fz_pixmap *pix;
		struct data *data;

		// Load the relevant page for each thread. Note, that this
		// cannot be done on the worker threads, as each use of doc
		// uses ctx, and only one thread can be using ctx at a time.

		page = fz_load_page(ctx, doc, i);

		// Compute the bounding box for each page.

		fz_bound_page(ctx, page, &bbox);

		// Create a display list that will hold the drawing
		// commands for the page. Once we have the display list
		// this can safely be used on any other thread as it is
		// not bound to a given context.

		list = fz_new_display_list(ctx, &bbox);

		// Run the loaded page through a display list device
		// to populate the page's display list.

		dev = fz_new_list_device(ctx, list);
		fz_run_page(ctx, page, dev, &fz_identity, NULL);
		fz_close_device(ctx, dev);
		fz_drop_device(ctx, dev);

		// The page is no longer needed, all drawing commands
		// are now in the display list.

		fz_drop_page(ctx, page);

		// Create a white pixmap using the correct dimensions.

        pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(&rbox, &bbox), 0, 0);
		fz_clear_pixmap_with_value(ctx, pix, 0xff);

		// Populate the data structure to be sent to the
		// rendering thread for this page.

        data = (struct data*)malloc(sizeof (struct data));

		data->pagenumber = i + 1;
		data->ctx = ctx;
		data->list = list;
		data->bbox = bbox;
		data->pix = pix;
        data->total_pagecnt = threads;

		// Create the thread and pass it the data structure.

        if (pthread_create(&thread[i], NULL, renderer, data) != 0) {
			fail("pthread_create()");
        }
//        pthread_detach(thread[i]);
	}

    for (int i = threads - 1; i >= 0; --i)
    {
        printf("pthread_join.\n");

        struct data *data;
        if (pthread_join(thread[i], (void **) &data) != 0){
          fail("pthread_join");
        }
    }

//  // Now each thread is rendering pages, so wait for each thread
//  // to complete its rendering.
//
//  fprintf(stderr, "joining %d threads...\n", threads);
//  for (i = threads - 1; i >= 0; i--)
//  {
//      char filename[42];
//      struct data *data;
//
//      if (pthread_join(thread[i], (void **) &data) != 0)
//          fail("pthread_join");
//
//      sprintf(filename, "out%04d.png", i);
//      fprintf(stderr, "\tSaving %s...\n", filename);
//
//      // Write the rendered image to a PNG file
//
//      fz_save_pixmap_as_png(ctx, data->pix, filename);
//
//      // Free the thread's pixmap and display list since
//      // they were allocated by the main thread above.
//
//      fz_drop_pixmap(ctx, data->pix);
//      fz_drop_display_list(ctx, data->list);
//
//      // Free the data structured passed back and forth
//      // between the main thread and rendering thread.
//
//      free(data);
//  }

    pthread_mutex_lock(&g_finishMutex);
    while (!g_isFinishStatus) {
        printf("@@@@Wait finish, page num: %d.\n", g_finishPagesNum);
        pthread_cond_wait(&g_finishCond, &g_finishMutex);
    }
    pthread_mutex_unlock(&g_finishMutex);

//    sleep(1);

//    while (g_finishPagesNum != threads)
//    {
//        printf("Sleep.\n");
//        sleep(1);
//    }

//    while (true)
//    {
//        sleep(1);
//    }
    fprintf(stderr, "finally!\n");
	fflush(NULL);

	free(thread);

	// Finally the document is closed and the main thread's
	// context is freed.

	fz_drop_document(ctx, doc);
	fz_drop_context(ctx);

	return 0;
}
