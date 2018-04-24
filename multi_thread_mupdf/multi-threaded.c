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

static int alphabits = 8;
static int layout_use_doc_css = 1;
static float layout_w = 450;
static float layout_h = 600;
static float layout_em = 12;
static const int resolution = 72;


// 页面信息
//static std::list<void*> g_pagesInfoList;
//static pthread_mutex_t g_pagesInfoListMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t  g_pagesInfoListCond  = PTHREAD_COND_INITIALIZER;
//
//// 像素信息
//static std::list<void*> g_pixMapList;
//static pthread_mutex_t g_pixMapListMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t  g_pixMapListCond  = PTHREAD_COND_INITIALIZER;
//
//// 是否完成
//static int g_finishPagesNum = 0;
//static bool g_isFinishStatus = false;
//static pthread_mutex_t g_finishMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t  g_finishCond  = PTHREAD_COND_INITIALIZER;
// A convenience function for dying abruptly on pthread errors.

void
fail(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

typedef struct tag_pdf_context {
    fz_context *ctx;

    fz_document *doc;

    // 总页面数
    int total_pagecnt;

    // 页面信息转像素
    std::list<void*> pagesInfoList;
    pthread_mutex_t pagesInfoListMutex;
    pthread_cond_t  pagesInfoListCond;

    // 像素信息转图片
    std::list<void*> pixMapList;
    pthread_mutex_t pixMapListMutex;
    pthread_cond_t  pixMapListCond;

    // 是否完成
    int finishPagesNum;
    bool isFinishStatus;
    pthread_mutex_t finishMutex;
    pthread_cond_t  finishCond;
} TAG_PDF_CONTEXT_S;

// The data structure passed between the requesting main thread and
// each rendering thread.

typedef struct tag_pdf_data {
	// A pointer to the original context in the main thread sent
	// from main to rendering thread. It will be used to create
	// each rendering thread's context clone.
	fz_context *ctx;

    fz_document *doc;

	// Page number sent from main to rendering thread for printing
	int pagenumber;

    // 总页面数
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

    TAG_PDF_CONTEXT_S *pdfContext;
} TAG_PDF_DATA_S;

// This is the function run by each rendering function. It takes
// pointer to an instance of the data structure described above and
// renders the display list into the pixmap before exiting.

void *
Renderer(void *param)
{
    TAG_PDF_CONTEXT_S *pdfContext = (TAG_PDF_CONTEXT_S*)param;

    while (!pdfContext->isFinishStatus)
    {
        pthread_mutex_lock(&pdfContext->pagesInfoListMutex);
        while (pdfContext->pagesInfoList.empty()) {
            printf("###wait g_pagesInfoList.\n");
            pthread_cond_wait(&pdfContext->pagesInfoListCond, &pdfContext->pagesInfoListMutex);
        }
        TAG_PDF_DATA_S *data = (TAG_PDF_DATA_S*)pdfContext->pagesInfoList.front();
        pdfContext->pagesInfoList.pop_front();
        printf("#####Get data.\n");
        pthread_mutex_unlock(&pdfContext->pagesInfoListMutex);

        int pagenumber = data->pagenumber;
        fz_context *ctx = data->ctx;
        fz_display_list *list = data->list;
        fz_rect bbox = data->bbox;
        fz_pixmap *&pix = data->pix;
        fz_device *dev;

        fprintf(stderr, "thread at page %d loading!\n", pagenumber);

        // The context pointer is pointing to the main thread's
        // context, so here we create a new context based on it for
        // use in this thread.

        ctx = fz_clone_context(ctx);

        fz_irect rbox;
        pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(&rbox, &bbox), 0, 0);
        fz_clear_pixmap_with_value(ctx, pix, 0xff);
        fz_set_pixmap_resolution(ctx, pix, resolution, resolution);

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

        pthread_mutex_lock(&pdfContext->pixMapListMutex);
        pdfContext->pixMapList.push_back(data);

        printf("sizeof g_pixMapList: %d.\n", pdfContext->pixMapList.size());
        pthread_cond_signal(&pdfContext->pixMapListCond);
        printf("renderer thread singal cond\n");
        pthread_mutex_unlock(&pdfContext->pixMapListMutex);
        printf("renderer thread unlocked\n");
    }
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
    TAG_PDF_CONTEXT_S *pdfContext = (TAG_PDF_CONTEXT_S*)param;

    while (!pdfContext->isFinishStatus)
    {
        pthread_mutex_lock(&pdfContext->pixMapListMutex);
        while (pdfContext->pixMapList.empty()) {
            printf("###wait g_pixMapList.\n");
            pthread_cond_wait(&pdfContext->pixMapListCond, &pdfContext->pixMapListMutex);
        }

        TAG_PDF_DATA_S *data = (TAG_PDF_DATA_S*)pdfContext->pixMapList.front();
        pdfContext->pixMapList.pop_front();
        pthread_mutex_unlock(&pdfContext->pixMapListMutex);

        fz_context *ctx = data->ctx;
        ctx = fz_clone_context(ctx);
        int page_cnt = data->total_pagecnt;

        fprintf(stderr, "####66 Page: %d\n", data->pagenumber);

        char filename[42];

        sprintf(filename, "multi-out%04d.png", data->pagenumber);
        fprintf(stderr, "\tSaving %s...\n", filename);

        // Write the rendered image to a PNG file

        fz_set_pixmap_resolution(ctx, data->pix, resolution, resolution);
        fz_save_pixmap_as_png(ctx, data->pix, filename);

        fz_buffer *buffer = fz_new_buffer_from_pixmap_as_png(ctx, data->pix, NULL);
        unsigned char *datap = NULL;
        ssize_t png_size = fz_buffer_storage(ctx, buffer, &datap);
        printf("###########png size: %d.\n", png_size);
        fz_drop_buffer(ctx, buffer);

        // Free the thread's pixmap and display list since
        // they were allocated by the main thread above.

        fz_drop_pixmap(ctx, data->pix);
        fz_drop_display_list(ctx, data->list);

        // Free the data structured passed back and forth
        // between the main thread and rendering thread.

//      free(data);
        delete data;
        data = NULL;

        pthread_mutex_lock(&pdfContext->finishMutex);
        ++pdfContext->finishPagesNum;
        printf("finish page num: %d.\n", pdfContext->finishPagesNum);
        if (pdfContext->finishPagesNum == page_cnt)
        {
            printf("Finish cond signal.\n");
            pdfContext->isFinishStatus = true;
            pthread_cond_signal(&pdfContext->finishCond);
        }
        pthread_mutex_unlock(&pdfContext->finishMutex);
    }
}

void *LoadPage(void *param)
{
    TAG_PDF_CONTEXT_S *pdfContext = (TAG_PDF_CONTEXT_S*)param;
    int threads = pdfContext->total_pagecnt;
    fz_context *ctx = pdfContext->ctx;
    fz_document *doc = pdfContext->doc;
    ctx = fz_clone_context(ctx);

//  delete pdfData;
//  pdfData = NULL;

    printf("#################Total page num: %d.\n", threads);
    for (int i = 0; i < threads; i++)
    {
        fz_page *page;
        fz_rect bbox;
        fz_irect rbox;
        fz_display_list *list;
        fz_device *dev;
        fz_pixmap *pix;
//      struct data *data;

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

//      pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(&rbox, &bbox), 0, 0);
//      fz_clear_pixmap_with_value(ctx, pix, 0xff);
//      fz_set_pixmap_resolution(ctx, pix, resolution, resolution);

        // Populate the data structure to be sent to the
        // rendering thread for this page.

//      data = (struct data*)malloc(sizeof (struct data));
        TAG_PDF_DATA_S *data = new(TAG_PDF_DATA_S);
        data->pagenumber = i + 1;
        data->ctx = ctx;
        data->list = list;
        data->bbox = bbox;
        data->pix = NULL;       //pix;
        data->total_pagecnt = threads;

        pthread_mutex_lock(&pdfContext->pagesInfoListMutex);

        pdfContext->pagesInfoList.push_back(data);
        printf("sizeof g_pagesInfoList: %d.\n", pdfContext->pagesInfoList.size());

        pthread_cond_signal(&pdfContext->pagesInfoListCond);
        printf("g_pagesInfoList thread singal cond\n");
        pthread_mutex_unlock(&pdfContext->pagesInfoListMutex);
        printf("g_pagesInfoList thread unlocked\n");
    }
}

inline int make_thread(pthread_t *pthreadList, int threadCnt, void *(fun)(void*), void *param = NULL)
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

int main(int argc, char **argv)
{
    const char *filename = argv[1];

    // 环境上下文
    TAG_PDF_CONTEXT_S *pdfContext = new(TAG_PDF_CONTEXT_S);
    pthread_mutex_init(&pdfContext->pagesInfoListMutex, NULL);
    pthread_mutex_init(&pdfContext->pixMapListMutex, NULL);
    pthread_mutex_init(&pdfContext->finishMutex, NULL);
    pdfContext->finishPagesNum = 0;
    pdfContext->isFinishStatus = false;

    // 页面转换像素消费者
    pthread_t rendererThreadList[2];
    make_thread(rendererThreadList, sizeof(rendererThreadList) / sizeof(*rendererThreadList), Renderer, pdfContext);

    // 像素转图片消费者
    pthread_t pthreadList[4];
    make_thread(pthreadList, sizeof(pthreadList) / sizeof(*pthreadList), PixMapMain, pdfContext);

    // Initialize FZ_LOCK_MAX number of non-recursive mutexes.
    pthread_mutex_t mutex[FZ_LOCK_MAX];
    for (int i = 0; i < FZ_LOCK_MAX; i++)
	{
        if (pthread_mutex_init(&mutex[i], NULL) != 0) {
			fail("pthread_mutex_init()");
        }
	}

	// Initialize the locking structure with function pointers to
	// the locking functions and to the user data. In this case
	// the user data is a pointer to the array of mutexes so the
	// locking functions can find the relevant lock to change when
	// they are called. This way we avoid global variables.
    fz_locks_context locks;
	locks.user = mutex;
	locks.lock = lock_mutex;
	locks.unlock = unlock_mutex;

	// This is the main threads context function, so supply the
	// locking structure. This context will be used to parse all
    // the pages from the document.
    fz_context *ctx = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);

    // Register default file types.
    fz_register_document_handlers(ctx);
    fz_set_aa_level(ctx, alphabits);
    fz_set_use_document_css(ctx, layout_use_doc_css);

	// Open the PDF, XPS or CBZ document. Note, this binds doc to ctx.
    // You must only ever use doc with ctx - never a clone of it!
    fz_document *doc = fz_open_document(ctx, filename);

	// Retrieve the number of pages, which translates to the
    // number of threads used for rendering pages.
    fz_layout_document(ctx, doc, layout_w, layout_h, layout_em);
    int pages_cnt = fz_count_pages(ctx, doc);

    pdfContext->ctx = ctx;
    pdfContext->doc = doc;
    pdfContext->total_pagecnt = pages_cnt;

    // 加载页面线程
    pthread_t threadId[1];
    make_thread(threadId, sizeof(threadId) / sizeof(*threadId), LoadPage, pdfContext);

    // 等待流程处理完成
    pthread_mutex_lock(&pdfContext->finishMutex);
    while (!pdfContext->isFinishStatus) {
        printf("@@@@Wait finish, page num: %d.\n", pdfContext->finishPagesNum);
        pthread_cond_wait(&pdfContext->finishCond, &pdfContext->finishMutex);
    }
    pthread_mutex_unlock(&pdfContext->finishMutex);

    fprintf(stderr, "finally!\n");
	fflush(NULL);

    delete(pdfContext);
    pdfContext = NULL;

	// Finally the document is closed and the main thread's
    // context is freed.
	fz_drop_document(ctx, doc);
	fz_drop_context(ctx);

	return 0;
}
