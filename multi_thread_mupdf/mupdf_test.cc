/*
 * muconvert -- command line tool for converting documents
 */

#include "mupdf/fitz.h"

#include <stdlib.h>
#include <stdio.h>

/* input options */
static const char *password = "";
static int alphabits = 8;
static float layout_w = 450;
static float layout_h = 600;
static float layout_em = 12;
static char *layout_css = NULL;
static int layout_use_doc_css = 1;

/* output options */
static const char *output = NULL;
static const char *format = NULL;
static const char *options = "";

static fz_context *ctx;
static fz_document *doc;
static fz_document_writer *out;
static int count;

#define PRINTF printf

#include <sys/time.h>
inline size_t get_usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void runpage(int number)
{
	fz_rect mediabox;
	fz_page *page;
	fz_device *dev = NULL;

    static size_t loadPageTotalTime = 0;
    size_t startLoadPageTime = get_usec();
    page = fz_load_page(ctx, doc, number - 1);
    size_t endLoadPageTime = get_usec();
    size_t loadPageTime = endLoadPageTime - startLoadPageTime;
    loadPageTotalTime += loadPageTime;
    PRINTF("111###Load page: %d, time: %d, total time: %d.\n", number - 1, loadPageTime, loadPageTotalTime);

	fz_var(dev);

	fz_try(ctx)
	{
        static size_t totalBoundTime = 0;
        size_t startBoundPageTime = get_usec();
        fz_bound_page(ctx, page, &mediabox);
        size_t endBoundPageTime = get_usec();
        size_t boundPageTime = endBoundPageTime - startBoundPageTime;
        totalBoundTime += boundPageTime;
        PRINTF("*****fz_bound_page time: %d, total time: %d.\n", boundPageTime, totalBoundTime);

        static size_t totalBeginPageTime = 0;
        size_t startBeginPageTime = get_usec();
        dev = fz_begin_page(ctx, out, &mediabox);
        size_t endBeginPageTime = get_usec();
        size_t beginPageTime = endBeginPageTime - startBeginPageTime;
        totalBeginPageTime += beginPageTime;
        PRINTF("!!!fz_begin_page time: %d, total time: %d.\n", beginPageTime, totalBeginPageTime);

        static size_t totalRunRangeTime = 0;
        size_t startRunRangeTime = get_usec();
        fz_run_page(ctx, page, dev, &fz_identity, NULL);                                // 总时间30%
        size_t endRunRangeTime = get_usec();
        size_t runRangeTime = endRunRangeTime - startRunRangeTime;
        totalRunRangeTime += runRangeTime;
        PRINTF("@@@fz_run_page time: %d, total time: %d.\n", runRangeTime, totalRunRangeTime);
    }
	fz_always(ctx)
	{
        static size_t totalEndPageTime = 0;
        size_t startEndPageTime = get_usec();
        if (dev)
            fz_end_page(ctx, out);                                                      // 总时间70%
        size_t endEndPageTime = get_usec();
        size_t endPageTime = endEndPageTime - startEndPageTime;
        totalEndPageTime += endPageTime;
        PRINTF("###fz_end_page time: %d, total Time: %d.\n", endPageTime, totalEndPageTime);

        static size_t totalDropPageTime = 0;
        size_t startDropPageTime = get_usec();
        fz_drop_page(ctx, page);
        size_t endDropPageTime = get_usec();
        size_t dropPageTime = endDropPageTime - startDropPageTime;
        totalDropPageTime += dropPageTime;
        PRINTF("$$$fz_drop_page time: %d, total time: %d.\n", dropPageTime, totalDropPageTime);
	}
	fz_catch(ctx)
		fz_rethrow(ctx);
}

static void runrange(const char *range)
{
	int start, end, i;

    PRINTF("###Range: %s.\n", range);
	while ((range = fz_parse_page_range(ctx, range, &start, &end, count)))
	{
        PRINTF("###Parse page range: %s, start: %d, end: %d.\n", range, start, end);

        if (start < end) {
            for (i = start; i <= end; ++i) {
				runpage(i);
            }
        } else {
            for (i = start; i >= end; --i) {
				runpage(i);
            }
        }
	}
}

int main(int argc, char **argv)
{
	int i, c;

    format = "png";
//    options = "resolution=96";

	/* Create a context to hold the exception stack and various caches. */
    size_t startNewContextTime = get_usec();
    ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);                           //******************2ms固定的
	if (!ctx)
	{
		fprintf(stderr, "cannot create mupdf context\n");
		return EXIT_FAILURE;
	}
    size_t endNewContextTime = get_usec();
    size_t newContextTime = endNewContextTime -startNewContextTime;
    PRINTF("New context time: %d\n", newContextTime);

	/* Register the default file types to handle. */
    size_t startRegisterDocumentTime = get_usec();
	fz_try(ctx)
		fz_register_document_handlers(ctx);
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot register document handlers: %s\n", fz_caught_message(ctx));
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}
    size_t endRegisterDocumentTime = get_usec();
    size_t registerDocumentTime = endRegisterDocumentTime - startRegisterDocumentTime;
    PRINTF("fz_register_document_handlers time: %d.\n", registerDocumentTime);

    size_t startSetLevelTime = get_usec();
    fz_set_aa_level(ctx, alphabits);
    size_t endSetLevelTime = get_usec();
    size_t setLevelTime = endSetLevelTime - startSetLevelTime;
    PRINTF("fz_set_aa_level time: %d.\n", setLevelTime);

    size_t startUseDocTime = get_usec();
	fz_set_use_document_css(ctx, layout_use_doc_css);
    size_t endUseDocTime = get_usec();
    size_t useDocTime = endUseDocTime - startUseDocTime;
    PRINTF("fz_set_use_document_css time: %d.\n", useDocTime);

	/* Open the output document. */
    size_t startNewDocTime = get_usec();
    fz_try(ctx)
    {
		out = fz_new_document_writer(ctx, output, format, options);
    }
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot create document: %s\n", fz_caught_message(ctx));
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}
    size_t endNewDocTime = get_usec();
    size_t newDocTime = endNewDocTime - startNewDocTime;
    PRINTF("fz_new_document_writer time: %d.\n", newDocTime);

    size_t startOpenocTime = get_usec();
    doc = fz_open_document(ctx, argv[1]);                                       // 读取时间随文件大小变化
    size_t endOpenocTime = get_usec();
    size_t openDocTime = endOpenocTime - startOpenocTime;
    PRINTF("fz_open_document time: %d.\n", openDocTime);

    size_t startLayoutDocTime = get_usec();
    fz_layout_document(ctx, doc, layout_w, layout_h, layout_em);
    size_t endLayoutDocTime = get_usec();
    size_t layoutDocTime = endLayoutDocTime - startLayoutDocTime;
    PRINTF("fz_layout_document time: %d.\n", layoutDocTime);

    size_t startCountPagesTime = get_usec();
    count = fz_count_pages(ctx, doc);
    size_t endCountPagesTime = get_usec();
    size_t countPagesTime = endCountPagesTime - startCountPagesTime;
    PRINTF("page num: %d, fz_count_pages time: %d.\n", count, countPagesTime);

    size_t startRunRangesTime = get_usec();
    runrange("1-N");                                                                // 最耗时部分,主要耗时地方
    size_t endRunRangesTime = get_usec();
    size_t runRangesTime = endRunRangesTime - startRunRangesTime;
    PRINTF("runrange time: %d.\n", runRangesTime);

    size_t startDropDocTime = get_usec();
    fz_drop_document(ctx, doc);                                                     // 也随文件大小变化，不明显
    size_t endDropDocTime = get_usec();
    size_t dropDocTime = endDropDocTime - startDropDocTime;
    PRINTF("fz_drop_document time: %d.\n", dropDocTime);

    size_t startCloseDocTime = get_usec();
	fz_close_document_writer(ctx, out);
    size_t endCloseDocTime = get_usec();
    size_t closeDocTime = endCloseDocTime - startCloseDocTime;
    PRINTF("fz_close_document_writer time: %d.\n", closeDocTime);

    size_t startdropDocWriteTime = get_usec();
	fz_drop_document_writer(ctx, out);
    size_t enddropDocWriteTime = get_usec();
    size_t dropDocWrite = enddropDocWriteTime - startdropDocWriteTime;
    PRINTF("fz_drop_document_writer time: %d.\n", dropDocWrite);

    size_t startDropContextTime = get_usec();
	fz_drop_context(ctx);
    size_t endDropContextTime = get_usec();
    size_t dropContext = endDropContextTime - startDropContextTime;
    PRINTF("fz_drop_context time: %d.\n", dropContext);

	return EXIT_SUCCESS;
}
