#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "MyLog.h"
#include "mupdf-stream-open.h"
#include "mupdf_utility.h"

namespace mupdf
{

static bool fileExists(const std::string &path)
{
    struct stat fileInfo = {0};
    if(stat(path.c_str(), &fileInfo) != 0) {
        return false;
    }

    // XXX: != S_IFREG
    return !((fileInfo.st_mode & S_IFMT) == S_IFDIR);
}

static int mupdf_context_init(fz_context *&ctx)
{
    ctx = NULL;
    ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx)
    {
        LOG_ERROR("Cannot create mupdf context.");
        return -1;
    }

    fz_try(ctx)
    {
        fz_register_document_handlers(ctx);
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Cannot initialize mupdf error msg: {}.",  fz_caught_message(ctx));
        fz_drop_context(ctx);
        ctx = NULL;
        return -1;
    }

    return 0;
}

static int mupdf_init(const std::string &docInfo,
                      fz_context *&ctx,
                      fz_stream *&file_stream,
                      fz_buffer *&file_buffer)
{
    if (docInfo.empty())
    {
        LOG_ERROR("Empty doc info.");
        return -1;
    }

    int iRet = mupdf_context_init(ctx);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to init mupdf context.");
        return iRet;
    }

    /**
     * 加载文件流和缓冲
     */
    file_stream = NULL;
    file_buffer = NULL;
    fz_try(ctx)
    {
        if (!docInfo.empty() && 256 >= docInfo.size())
        {
            if (!fileExists(docInfo)) {
                LOG_ERROR("Doc Path: [{}] is not exist.", docInfo);
                fz_drop_context(ctx);
                ctx = NULL;
                return -1;
            }

            /* 读文件 */
            file_stream = fz_open_file(ctx, docInfo.c_str());
            file_buffer = fz_read_all(ctx, file_stream, 1024 * 512);
        }
        else if (256 < docInfo.size())
        {
            /* 读内存 */
            file_stream = fz_open_memory(ctx, (unsigned char*)&docInfo[0], docInfo.size());
            file_buffer = fz_new_buffer_from_copied_data(ctx, (unsigned char*)&docInfo[0], docInfo.size());
        }
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Cannot open document size: {}, error msg: {}.", docInfo.size(), fz_caught_message(ctx));
        fz_drop_context(ctx);
        ctx = NULL;
        return -1;
    }

    if (NULL == file_stream)
    {
        LOG_ERROR("Failed to open file stream.");
        fz_drop_context(ctx);
        ctx = NULL;
        return -1;
    }

    if (NULL == file_buffer)
    {
        LOG_ERROR("Failed to new file buffer.");
        fz_drop_context(ctx);
        ctx = NULL;
        return -1;
    }

    return 0;
}

static int mupdf_load_pdf_document(fz_context *ctx,
                                   fz_stream *file_stream,
                                   pdf_document *&pdfDoc)
{
    pdfDoc = NULL;
    fz_try(ctx)
    {
        pdfDoc = pdf_open_document_with_stream(ctx, file_stream);
    }
    fz_catch(ctx)
    {
         LOG_ERROR("Cannot open pdf document error msg: {}.", fz_caught_message(ctx));
         return -1;
    }

    if (NULL == pdfDoc)
    {
        LOG_ERROR("Failed to open pdf document.");
        return -1;
    }

    return 0;
}

static int mupdf_load_document(fz_context *ctx,
                               fz_stream *file_stream,
                               const char *docType,
                               fz_document *&doc)
{
    doc = NULL;
    fz_try(ctx)
    {
        std::string ext = std::string(".") + docType;
        doc = fz_open_document_with_stream(ctx, ext.c_str(), file_stream);
    }
    fz_catch(ctx)
    {
         LOG_ERROR("Cannot open {} document error msg: {}.", docType, fz_caught_message(ctx));
         return -1;
    }

    if (NULL == doc)
    {
        LOG_ERROR("Failed to open {} document.", docType);
        return -1;
    }

    return 0;
}

static int mupdf_pdf_incremental_save(fz_context *ctx, pdf_document *pdfDoc,
                                      fz_buffer *buffer, std::string &pdfOutputInfo)
{
    // 增量保存
    pdf_write_options opts  = {0};
    opts.do_incremental     = 1;        // 增量保存
    opts.do_compress        = 1;        // 压缩流
    opts.do_compress_images = 1;        // 压缩图片流
    opts.do_compress_fonts  = 1;        // 压缩字体流

#if 0
    /**
     * XRefStm为HybridXref(混合xref)，xref和xref stream同时存在，
     * /XRefStm 89267后面数字就是xref stream地址。
     * 这种情况应该作为老的xref处理不能作为xref stream处理，
     * 否则会导致adobe验证报未预期字节范围
     */
    if (1 == pdfDoc->has_xref_streams)
    {
        unsigned char *datap = NULL;
        size_t len = fz_buffer_storage(ctx, buffer, &datap);
        if (NULL == datap || 0 >= len)
        {
            LOG_ERROR("Null file buffer.");
            return -1;
        }

        std::string pdfFile((const char*)datap, len);
        if (std::string::npos != pdfFile.rfind("/XRefStm"))
        {
            // 混合xref, xref和xref stream同时存在
            // 此时设置为xref
            LOG_DEBUG("xref sections num: {}.", pdfDoc->num_xref_sections);
            for (int i = 0; i < pdfDoc->num_xref_sections; ++i)
            {
                pdf_xref *xref = &pdfDoc->xref_sections[i];
                if (NULL != xref->trailer)
                {
                    pdf_obj *xrefstm = pdf_dict_get(ctx, xref->trailer, PDF_NAME(XRefStm));
                    if (NULL != xrefstm)
                    {
                        int64_t xrefstmofs = pdf_to_int64(ctx, xrefstm);
                        if (0 < xrefstmofs)
                        {
                            pdfDoc->has_xref_streams = 0;
                            LOG_DEBUG("xrefstm offset: {}, set xref stream 0.", xrefstmofs);
                            break;
                        }
                    }
                }
            }
        }
    }
#endif

    /**
     * 写内存PDF
     */
    if (pdfOutputInfo.empty())
    {
        unsigned char *datap = NULL;
        size_t len = fz_buffer_storage(ctx, buffer, &datap);
        std::stringstream stream(std::string((char*)datap, len));
        fz_output *out = NULL;
        fz_try(ctx)
        {
            // 内存流进行读写
            out = fz_new_output_with_stream(ctx, &stream);
            pdf_write_document(ctx, pdfDoc, out, &opts);
            fz_close_output(ctx, out);
        }
        fz_always(ctx)
        {
            fz_drop_output(ctx, out);
            out = NULL;
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to increment save pdf size: {}, error msg: {}.", len, fz_caught_message(ctx));
            return -1;
        }

        pdfOutputInfo = stream.str();
        if (pdfOutputInfo.empty())
        {
            LOG_ERROR("Empty dst pdf buffer info.");
            return -1;
        }
        LOG_DEBUG("Pdf out info memory size: {}.", pdfOutputInfo.size());
    }
    /**
     * 写文件PDF
     */
    else if (256 >= pdfOutputInfo.size())
    {
        LOG_DEBUG("Pdf out is path: {}.", pdfOutputInfo);
        fz_try(ctx)
        {
            fz_save_buffer(ctx, buffer, pdfOutputInfo.c_str());
            pdf_save_document(ctx, pdfDoc, pdfOutputInfo.c_str(), &opts);
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to increment save pdf: {}, error msg: {}.", pdfOutputInfo, fz_caught_message(ctx));
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Failed to pdf out info size: {}, must empty for write buffer, <=256 for write path.", pdfOutputInfo.size());
        return -1;
    }

    return 0;
}

static void mupdf_destroy(fz_context *ctx,
                          fz_document *doc,
                          pdf_document *pdf_doc,
                          fz_stream *file_stream,
                          fz_buffer *file_buffer,
                          fz_image *docImage)
{
    if (NULL != docImage)
    {
        fz_drop_image(ctx, docImage);
        docImage = NULL;
    }

    if (NULL != file_buffer)
    {
        fz_drop_buffer(ctx, file_buffer);
        file_buffer = NULL;
    }

    if (NULL != file_stream)
    {
        fz_drop_stream(ctx, file_stream);
        file_stream = NULL;
    }

    if (NULL != pdf_doc)
    {
        pdf_drop_document(ctx, pdf_doc);
        pdf_doc = NULL;
    }

    if (NULL != doc)
    {
        fz_drop_document(ctx, doc);
        doc = NULL;
    }

    if (NULL != ctx)
    {
        fz_drop_context(ctx);
        ctx = NULL;
    }

    return;
}

static int mupdf_load_image(fz_context *ctx, fz_buffer *file_buffer, fz_image *&image)
{
    /**
     * 加载图片
     */
    if (NULL != file_buffer)
    {
        image = NULL;
        fz_try(ctx)
        {
            image = fz_new_image_from_buffer(ctx, file_buffer);
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to new image err msg: {}.", fz_caught_message(ctx));
            return -1;
        }
    }

    if (NULL == image)
    {
        LOG_ERROR("Failed to new image.");
        return -1;
    }

    /**
     * 设置透明通道
     */
    fz_try(ctx)
    {
        // 设置图片mask，alpha背景透明
        fz_pixmap *pix = fz_get_pixmap_from_image(ctx, image, NULL, NULL, 0, 0);
        if (NULL == pix)
        {
            LOG_ERROR("Failed to get pixmap form imag.");
            fz_drop_image(ctx, image);
            image = NULL;
            return -1;
        }

        if (NULL != pix && pix->alpha == 1)
        {
            // have alpha, therefore create a mask
            fz_pixmap *pm = fz_convert_pixmap(ctx, pix, NULL, NULL, NULL, NULL, 1);
            pm->alpha = 0;
            pm->colorspace = fz_keep_colorspace(ctx, fz_device_gray(ctx));

            fz_image *mask = fz_new_image_from_pixmap(ctx, pm, NULL);
            fz_image *zimg = fz_new_image_from_pixmap(ctx, pix, mask);
            fz_drop_image(ctx, mask);
            fz_drop_pixmap(ctx, pm);
            fz_drop_image(ctx, image);
            image = zimg;
        }
        fz_drop_pixmap(ctx, pix);
        pix = NULL;
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Failed to new image err msg: {}.", fz_caught_message(ctx));
        fz_drop_image(ctx, image);
        image = NULL;
        return -1;
    }

    return 0;
}

static int mupdf_load_image(fz_context *ctx, const std::string &imageBuffer, fz_image *&image)
{
    fz_buffer *buffer = NULL;
    fz_try(ctx)
    {
        buffer = fz_new_buffer_from_copied_data(ctx, (unsigned char*)&imageBuffer[0], imageBuffer.size());
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Failed to fz_new_buffer_from_data, error msg: {}", fz_caught_message(ctx));
        return -1;
    }

    int iRet = mupdf_load_image(ctx, buffer, image);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf load image.");
        fz_drop_buffer(ctx, buffer);
        buffer = NULL;
        return iRet;
    }

    fz_drop_buffer(ctx, buffer);
    buffer = NULL;
    return 0;
}

static int mupdf_save_image_to_png(fz_context *ctx, fz_image *image, std::string &pngBuffer)
{
    fz_buffer *buffer = NULL;
    fz_try(ctx)
    {
        buffer = fz_new_buffer_from_image_as_png(ctx, image, NULL);

        unsigned char *datap = NULL;
        ssize_t pngSize = fz_buffer_storage(ctx, buffer, &datap);
        pngBuffer.assign((char*)datap, (char*)datap + pngSize);
    }
    fz_always(ctx)
    {
        fz_drop_buffer(ctx, buffer);
        buffer = NULL;
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Failed to fz_new_buffer_from_image_as_png, error msg: {}", fz_caught_message(ctx));
        return -1;
    }

    return 0;
}

static int mupdf_save_pnm_to_png(fz_context *ctx, fz_buffer *file_buffer, std::string &pngBuffer)
{
    fz_pixmap *pix = NULL;
    fz_buffer *buffer = NULL;
    fz_try(ctx)
    {
        unsigned char *datap = NULL;
        ssize_t size = 0;
        size = fz_buffer_storage(ctx, file_buffer, &datap);

        pix = fz_load_pnm(ctx, datap, size);
        buffer = fz_new_buffer_from_pixmap_as_png(ctx, pix, NULL);

        size = fz_buffer_storage(ctx, buffer, &datap);
        pngBuffer.assign((char*)datap, (char*)datap + size);
    }
    fz_always(ctx)
    {
        fz_drop_buffer(ctx, buffer);
        buffer = NULL;
        fz_drop_pixmap(ctx, pix);
        pix = NULL;
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Failed to save pnm to png, error msg: {}", fz_caught_message(ctx));
        return -1;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////C++外部包装类/////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

MupdfUtil::MupdfUtil(const std::string &docInfo, const char *docType):
    _docType(docType)
{
    _isSucceedInit = false;

    if (docInfo.empty())
    {
        LOG_ERROR("Empty doc info.");
        return;
    }

    if (256 >= docInfo.size() && !fileExists(docInfo))
    {
        LOG_ERROR("Invalid file path: {}", docInfo);
        return;
    }

    int iRet = mupdf_init(docInfo, _ctx, _file_stream, _file_buffer);
    if (0 != iRet) {
        LOG_ERROR("Failed to init mupdf.");
        return;
    }

    if (std::string("pdf") == std::string(docType))
    {
        if (NULL == getPdfDocument()) {
            LOG_ERROR("Failed to get pdf doc.");
            return;
        }
    }
    else if (std::string("image") == std::string(docType))
    {
        if (NULL == getDocImage()) {
            LOG_ERROR("Failed to get doc image.");
            return;
        }
    }
    else if (std::string("xps") == std::string(docType) ||
             std::string("cbz") == std::string(docType))
    {
        if (NULL == getDocument()) {
            LOG_ERROR("Failed to get [{}] doc.", docType);
            return;
        }
    }
    else
    {
        LOG_ERROR("Invalid doc type: {}", docType);
        return;
    }

    _isSucceedInit = true;
}

MupdfUtil::~MupdfUtil()
{
    for (const auto &pages_pair: _pages_cache)
    {
        pdf_page *page = pages_pair.second.first;
        fz_stext_page *stext = pages_pair.second.second;
        if (NULL != page && NULL != stext)
        {
            fz_drop_stext_page(_ctx, stext);
        }

        if (NULL != page)
        {
            fz_drop_page(_ctx, &(page->super));
        }
    }

    for (auto &image: _imageList)
    {
        if (NULL != image)
        {
            fz_drop_image(_ctx, image);
            image = NULL;
        }
    }

    mupdf_destroy(_ctx, _doc, _pdfDoc, _file_stream, _file_buffer, _docImage);
}

MupdfUtil::operator bool()
{
    return _isSucceedInit;
}

fz_context *MupdfUtil::getContext()
{
    return _ctx;
}

fz_document *MupdfUtil::getDocument()
{
    if (NULL != _doc) {
        return _doc;
    }

    fz_document *doc = NULL;
    int iRet = mupdf_load_document(_ctx, _file_stream, _docType.c_str(), doc);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf load {} document.", _docType);
        return NULL;
    }

    _doc = doc;
    return _doc;
}

pdf_document *MupdfUtil::getPdfDocument()
{
    if (NULL != _pdfDoc) {
        return _pdfDoc;
    }

    pdf_document *pdfDoc = NULL;
    int iRet = mupdf_load_pdf_document(_ctx, _file_stream, pdfDoc);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf load pdf document.");
        return NULL;
    }

    _pdfDoc = pdfDoc;
    return _pdfDoc;
}

fz_image *MupdfUtil::getDocImage()
{
    if (NULL != _docImage)
    {
        return _docImage;
    }

    int iRet = 0;
    fz_image *image = NULL;
    iRet = mupdf_load_image(_ctx, _file_buffer, image);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf load doc image.");
        return NULL;
    }

    _docImage = image;
    return _docImage;
}

fz_image *MupdfUtil::getBufferImage(const std::string &imageBuffer)
{
    int iRet = 0;
    fz_image *image = NULL;
    LOG_DEBUG("Load image from image buffer size: {}", imageBuffer.size());
    iRet = mupdf_load_image(_ctx, imageBuffer, image);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf load image.");
        return NULL;
    }

    _imageList.push_back(image);
    return image;
}

fz_buffer *MupdfUtil::getFileBuffer()
{
    if (NULL != _file_buffer)
    {
        return _file_buffer;
    }

    return NULL;
}

int MupdfUtil::getPdfPageCount()
{
    if (NULL == _pdfDoc)
    {
        LOG_ERROR("Null pdf doc.");
        return -1;
    }
    return pdf_count_pages(_ctx, _pdfDoc);
}

int MupdfUtil::pdfIncrementalSave(std::string &pdfOutputInfo)
{
    if (NULL == _pdfDoc)
    {
        LOG_ERROR("Null pdf doc");
        return -1;
    }
    return mupdf_pdf_incremental_save(_ctx, _pdfDoc, _file_buffer, pdfOutputInfo);
}

int MupdfUtil::saveImage2Png(std::string &pngOutBuffer)
{
    if (NULL == _docImage)
    {
        LOG_ERROR("Null doc image.");
        return -1;
    }

    int iRet = mupdf_save_image_to_png(_ctx, _docImage, pngOutBuffer);
    if (0 != iRet)
    {
        LOG_ERROR("Failed to mupdf save image to png.");
        return iRet;
    }

    return 0;
}

int MupdfUtil::savePnm2Png(std::string &pngBuffer)
{
    if ("image" != _docType)
    {
        LOG_ERROR("Doc type is not image: {}", _docType);
        return -1;
    }
    return mupdf_save_pnm_to_png(_ctx, _file_buffer, pngBuffer);
}

int MupdfUtil::hasXRefStream()
{
    if (NULL == _pdfDoc)
    {
        LOG_ERROR("Null pdf doc");
        return -1;
    }

    LOG_DEBUG("Xref stream: {}", _pdfDoc->has_xref_streams);
    return _pdfDoc->has_xref_streams;
}

int pdf_has_xref_stream(const std::string &pdfBuffer, bool &first_xref_style,
                        bool &has_old_style_xref, bool &has_xref_stream)
{
    // true: xref
    // false: xref stream
    first_xref_style = false;

    // xref
    has_old_style_xref = false;

    // xref stream
    has_xref_stream = false;

    /**
     * 第一个xref风格
     */
    size_t first_eof_pos = pdfBuffer.find("%%EOF");
    if (std::string::npos == first_eof_pos)
    {
        LOG_ERROR("Invalid pdf format no %%EOF.");
        return -1;
    }

    if (std::string::npos != pdfBuffer.rfind("trailer", first_eof_pos))
    {
        // xref
        first_xref_style = true;
    }

    /**
     * 旧风格: xref trailer
     */
    if (std::string::npos != pdfBuffer.rfind("trailer"))
    {
        has_old_style_xref = true;
    }

    /**
     * 新风格XRefStream: "/XRef" "XRefStm"
     */
    if (std::string::npos != pdfBuffer.rfind("/XRef"))
    {
        has_xref_stream = true;
    }

    if (std::string::npos != pdfBuffer.rfind("/XRefStm"))
    {
        // 混合xref, xref和xref stream同时存在
        // 此时设置为xref
        has_old_style_xref = true;
        has_xref_stream = false;
    }

    LOG_DEBUG("First xref style: {}.", first_xref_style);
    LOG_DEBUG("Old style xref: {}",  has_old_style_xref);
    LOG_DEBUG("Xref stream: {}", has_xref_stream);
    return 0;
}

static int GetKeywordPosition(fz_context *ctx, pdf_document *doc,
                              const std::string &keyword, const std::vector<int> &pageList,
                              std::vector<TAG_PAGE_RECT_INFO_S> &pageRectList,
                              std::map <int, std::pair<pdf_page*, fz_stext_page*> > &pages_cache)
{
    int page_count = pdf_count_pages(ctx, doc);
    LOG_DEBUG("Page count: {}", page_count);

    /**
     * 参数校验
     */
    if (keyword.empty())
    {
        LOG_ERROR("Empty keyword.");
        return -1;
    }

    // 页面列表
    if (pageList.empty())
    {
        LOG_ERROR("Empty pages.");
        return -1;
    }
    else if (1 == pageList.size())
    {
        // 只有一个页面可取[0..页面数]中任意一个
        if (0 > pageList[0] || pageList[0] > page_count)
        {
            LOG_ERROR("Invalid page: {}, one page must be: [0..{}]", pageList[0], page_count);
            return -1;
        }
    }
    else
    {
        // 有多个页面不能包含0,可取[1..页面数]任意一个
        for (int page_num: pageList)
        {
            if (0 >= page_num || page_num > page_count)
            {
                LOG_ERROR("Invalid page: {}, mulit pages must be: [1..{}]", page_num, page_count);
                return -1;
            }
        }
    }

    /**
     * 0: 所有页面特殊处理
     */
    std::vector<int> pageListTmp = pageList;
    if (1 == pageListTmp.size() && 0 == pageListTmp[0])
    {
        LOG_DEBUG("Page is 0, page count: {}", page_count);
        pageListTmp.clear();
        // 页面从1开始
        for (int page_cnt = 1; page_cnt <= page_count; ++page_cnt)
        {
            pageListTmp.push_back(page_cnt);
        }
    }

    /**
     * 查询页面关键字坐标
     */
    for (int page_num: pageListTmp)
    {
        // 获取页面
        pdf_page *page = NULL;
        if (pages_cache.end() != pages_cache.find(page_num))
        {
            page = pages_cache[page_num].first;
        }
        else
        {
            fz_try(ctx)
            {
                page = pdf_load_page(ctx, doc, page_num - 1);
            }
            fz_catch(ctx)
            {
                LOG_ERROR("Failed to load page, error msg: {}.", fz_caught_message(ctx));
                return -1;
            }
            pages_cache[page_num].first = page;
            pages_cache[page_num].second = NULL;
        }

        // 获取页面文本
        fz_stext_page *text = NULL;
        if (pages_cache.end() != pages_cache.find(page_num) &&
            NULL != pages_cache[page_num].first &&
            NULL != pages_cache[page_num].second)
        {
            text = pages_cache[page_num].second;
        }
        else if (pages_cache.end() != pages_cache.find(page_num) &&
                 NULL != pages_cache[page_num].first &&
                 NULL == pages_cache[page_num].second)
        {
            fz_try(ctx)
            {
                text = fz_new_stext_page_from_page(ctx, &page->super, NULL);
            }
            fz_catch(ctx)
            {
                LOG_ERROR("Failed to new stext, error msg: {}.", fz_caught_message(ctx));
                return -1;
            }
            pages_cache[page_num].second = text;
        }
        else
        {
            LOG_ERROR("Invalid pages cache text.");
            return -1;
        }

        // 获取页面关键字
#ifndef __MUPDF_VERBOSE_LESS_THAN_114__
        fz_quad hit_bbox[1024] = {0};
#else
        fz_rect hit_bbox[1024] = {0};
#endif
        int count = 0;
        fz_try(ctx)
        {
#ifndef __MUPDF_VERBOSE_LESS_THAN_114__
            count = fz_search_stext_page(ctx, text, keyword.c_str(), hit_bbox, nelem(hit_bbox));
#else
            count = fz_search_stext_page(ctx, text, keyword.c_str(), hit_bbox, nelem(hit_bbox));
#endif
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to search stext, error msg: {}.", fz_caught_message(ctx));
            return -1;
        }
        LOG_DEBUG("Page number: {}, keyword: {}, count: {}", page_num, keyword, count);
        if (0 >= count)
        {
            LOG_DEBUG("No keyword: {} in page: {} continue.", keyword, page_num);
            continue;
        }

        // 插入页面位置信息
        for (int bbox_cnt = 0; bbox_cnt < count; ++bbox_cnt)
        {
#ifndef __MUPDF_VERBOSE_LESS_THAN_114__
            fz_rect rect = fz_rect_from_quad(hit_bbox[bbox_cnt]);
#else
            fz_rect rect = hit_bbox[bbox_cnt];
#endif
            LOG_DEBUG("Keyword: {}, count: {}, rect: {}:{}, {}:{}", keyword, bbox_cnt, rect.x0, rect.y0, rect.x1, rect.y1);

            // 跳过重复位置
            bool bFound = false;
            for (const TAG_PAGE_RECT_INFO_S &pageRect: pageRectList)
            {
                // 查找结果中可能会重复出现多个很相近的矩阵(实际只有一个)
                // 关键字左上角或者右下角近似的认为是重复的
                const int rect_diff_value = 5;
                if (pageRect.page == page_num) {
                    if ((rect_diff_value >= std::abs((int)pageRect.x0 - (int)rect.x0) && rect_diff_value >= std::abs((int)pageRect.y0 - (int)rect.y0)) ||
                        (rect_diff_value >= std::abs((int)pageRect.x1 - (int)rect.x1) && rect_diff_value >= std::abs((int)pageRect.y1 - (int)rect.y1)))
                    {
                        LOG_DEBUG("Keyword: {}, count: {}, rect: {}:{}, {}:{} is exists break.", keyword, bbox_cnt, rect.x0, rect.y0, rect.x1, rect.y1);
                        bFound = true;
                        break;
                    }
                }
            }

            if (!bFound)
            {
                pageRectList.push_back(TAG_PAGE_RECT_INFO_S{page_num, rect.x0, rect.y0, rect.x1, rect.y1});
            }
        }
    }

    if (pageRectList.empty())
    {
        std::string pageStr;
        for (int pageNum: pageList)
        {
            pageStr += std::to_string(pageNum) + " ";
        }
        LOG_ERROR("Failed to find keyword: {} in pages: {}", keyword, pageStr);
        return -1;
    }

    for (const TAG_PAGE_RECT_INFO_S &pageRect: pageRectList)
    {
        LOG_DEBUG("Keyword: {}, page: {}, rect: {}:{}, {}:{}.", keyword, pageRect.page, pageRect.x0, pageRect.y0, pageRect.x1, pageRect.y1);
    }

    return 0;
}

int MupdfUtil::getKeywordPosition(const std::string &keyword, const std::vector<int> &pageList, std::vector<TAG_PAGE_RECT_INFO_S> &pageRectList)
{
    if (NULL == _pdfDoc)
    {
        LOG_ERROR("Null pdf doc");
        return -1;
    }

    return GetKeywordPosition(_ctx, _pdfDoc, keyword, pageList, pageRectList, _pages_cache);
}

std::map <int, std::pair<pdf_page*, fz_stext_page*> > &MupdfUtil::getKeywordPositionPagesCache()
{
    return _pages_cache;
}

static int SplitPngImage(fz_context *ctx, fz_image *image, int splitCount, std::vector<std::string> &splitPngImageList, int splitStyle)
{
    if (0 >= splitCount)
    {
        LOG_ERROR("Invalid split count: {}", splitCount);
        return -1;
    }

    int meanCount = 0;
    int freeCount = 0;
    // 横向
    if (0 == splitStyle)
    {
        if (splitCount > image->h)
        {
            LOG_ERROR("Split count: {} more than image height: {}", splitCount, image->h);
            return -1;
        }

        meanCount = image->h / splitCount;
        freeCount = image->h % splitCount;
    }
    // 纵向
    else if (1 == splitStyle)
    {
        if (splitCount > image->w)
        {
            LOG_ERROR("Split count: {} more than image width: {}", splitCount, image->w);
            return -1;
        }

        meanCount = image->w / splitCount;
        freeCount = image->w % splitCount;
    }
    else
    {
        LOG_ERROR("Invalid split style: {}, 0:row, 1:column.", splitStyle);
        return -1;
    }
    LOG_DEBUG("Split style: {}, split count: {}, image height: {}, width: {}, mean count: {}, free count: {}",
              splitStyle, splitCount, image->h, image->w, meanCount, freeCount);

    // 图片像素
    fz_pixmap *srcPixmap = NULL;

    // 起始、结束坐标位置
    int startPos = 0;
    int endPos = 0;
    for (int i = 0; i < splitCount; ++i)
    {
        /**
         * 设置待复制区域
         */
        if (i < freeCount) {
            // 前freeCount个大小meanCount + 1
            endPos = startPos + (meanCount + 1);
        } else {
            endPos = startPos + meanCount;
        }
        LOG_DEBUG("Start:end: {}:{}", startPos, endPos);

        // 设置感兴趣图片区域
        fz_rect rect = {0};
        if (0 == splitStyle)
        {
            if (endPos > image->h)
            {
                LOG_ERROR("Error end pos: {} more than image height: {}", endPos, image->h);
                return -1;
            }
            rect = {0, (float)startPos, (float)image->w, (float)endPos};
            if (i == splitCount - 1)
            {
                // 最后一个区域
                rect.y1 = image->h;
            }
        }
        else if (1 == splitStyle)
        {
            if (endPos > image->w)
            {
                LOG_ERROR("Error end pos: {} more than image width: {}", endPos, image->w);
                return -1;
            }
            rect = {(float)startPos, 0, (float)endPos, (float)image->h};
            if (i == splitCount - 1)
            {
                // 最后一个区域
                rect.x1 = image->w;
            }
        }
        startPos = endPos;
        fz_irect subarea = fz_irect_from_rect(rect);

        /**
         * 获取图片像素
         */
        if (NULL == srcPixmap)
        {
            fz_try(ctx)
            {
                srcPixmap = fz_get_pixmap_from_image(ctx, image, NULL, NULL, NULL, NULL);
            }
            fz_catch(ctx)
            {
                LOG_ERROR("Failed to get pixmap from image, error msg: {}.", fz_caught_message(ctx));
                return -1;
            }
        }

        /**
         * 复制目的像素
         */
        fz_pixmap *destPixmap = NULL;
        fz_buffer *buf = NULL;
        fz_output *outPut = NULL;
        fz_try(ctx)
        {
            destPixmap = fz_new_pixmap_with_bbox(ctx, srcPixmap->colorspace, subarea, srcPixmap->seps, srcPixmap->alpha);
            fz_clear_pixmap(ctx, destPixmap);
            fz_copy_pixmap_rect(ctx, destPixmap, srcPixmap, subarea, NULL);

            buf = fz_new_buffer(ctx, 10240);
            outPut = fz_new_output_with_buffer(ctx, buf);
            fz_write_pixmap_as_png(ctx, outPut, destPixmap);

            char *data = NULL;
            size_t len = fz_buffer_storage(ctx, buf, (unsigned char**)&data);
            splitPngImageList.push_back(std::string(data, len));
        }
        fz_always(ctx)
        {
            fz_drop_output(ctx, outPut);
            fz_drop_buffer(ctx, buf);
            fz_drop_pixmap(ctx, destPixmap);
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to copy pixmap, error msg: {}.", fz_caught_message(ctx));
            fz_drop_pixmap(ctx, srcPixmap);
            srcPixmap = NULL;
            return -1;
        }
    }

    fz_drop_pixmap(ctx, srcPixmap);
    srcPixmap = NULL;

    if (splitPngImageList.empty() ||
        (int)splitPngImageList.size() != splitCount)
    {
        LOG_ERROR("Error split png image list size: {}.", splitPngImageList.size());
        return -1;
    }

    return 0;
}

int MupdfUtil::splitPngImage(int splitCount, std::vector<std::string> &splitPngImageList, int splitStyle)
{
    if (NULL == _docImage)
    {
        LOG_ERROR("Null doc image.");
        return -1;
    }
    return SplitPngImage(_ctx, _docImage, splitCount, splitPngImageList, splitStyle);
}

} /* namespace mupdf */
