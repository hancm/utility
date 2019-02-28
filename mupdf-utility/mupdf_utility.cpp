#include <string>
#include <iostream>
#include <sstream>

#include "MyLog.h"
#include "mupdf-stream-open.h"
#include "mupdf_utility.h"

static bool fileExists(const std::string &path)
{
    struct stat fileInfo = {0};
    if(stat(path.c_str(), &fileInfo) != 0) {
        return false;
    }

    // XXX: != S_IFREG
    return !((fileInfo.st_mode & S_IFMT) == S_IFDIR);
}

MupdfUtil::MupdfUtil(const std::string &docInfo, const char *docType)
{
    /**
     * 判断是否文件路径
     */
    if (docInfo.empty())
    {
        LOG_ERROR("Empty doc info.");
        return;
    }

    // 可能是路径
    if (256 >= docInfo.size())
    {
        if (!fileExists(docInfo)) {
            LOG_ERROR("Invalid file path: {}", docInfo);
            return;
        }
        _isFilePath = true;
    }

    /**
     * 初始化文档信息
     */
    int iRet = init(docInfo, docType);
    if (0 != iRet) {
        return;
    }

    if ("pdf" == std::string(docType) && NULL == _pdfDoc) {
        return;
    }

    _isSucceedInit = true;
}

MupdfUtil::~MupdfUtil()
{
    destroy();
}

MupdfUtil::operator bool()
{
    return _isSucceedInit;
}

int MupdfUtil::init(const std::string &docInfo, const char *docType)
{
    _ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!_ctx)
    {
        LOG_ERROR("Cannot create mupdf context.");
        return -1;
    }

    fz_try(_ctx)
    {
        fz_register_document_handlers(_ctx);
    }
    fz_catch(_ctx)
    {
        LOG_ERROR("Cannot initialize mupdf error msg: {}.",  fz_caught_message(_ctx));
        return -1;
    }

    /**
     * 加载文件流和缓冲
     */
    fz_try(_ctx)
    {
        if (_isFilePath)
        {
            /* 读文件 */
            _file_stream = fz_open_file(_ctx, docInfo.c_str());
            _file_buffer = fz_read_all(_ctx, _file_stream, 1024 * 512);
        }
        else
        {
            /* 读内存 */
            _file_stream = fz_open_memory(_ctx, (unsigned char*)&docInfo[0], docInfo.size());
            _file_buffer = fz_new_buffer_from_shared_data(_ctx, (unsigned char*)&docInfo[0], docInfo.size());
        }
    }
    fz_catch(_ctx)
    {
        LOG_ERROR("Cannot open document size: {}, error msg: {}.", docInfo.size(), fz_caught_message(_ctx));
        return -1;
    }

    /**
     * 加载文档信息
     */
    if ("pdf" == std::string(docType) ||
        "xps" == std::string(docType) ||
        "cbz" == std::string(docType))
    {
        fz_try(_ctx)
        {
            std::string ext = std::string(".") + docType;
            _doc = fz_open_document_with_stream(_ctx, ext.c_str(), _file_stream);

            if ("pdf" == std::string(docType)) {
                _pdfDoc = pdf_document_from_fz_document(_ctx, _doc);
            }
        }
        fz_catch(_ctx)
        {
             LOG_ERROR("Cannot open document size: {}, error msg: {}.", docInfo.size(), fz_caught_message(_ctx));
             return -1;
        }
    }
    /**
     * 读取图片信息
     */
    else if ("image" == std::string(docType))
    {
        int iRet = loadImage(docInfo);
        if (0 != iRet)
        {
            LOG_ERROR("Failed to load image.");
            return iRet;
        }
    }
    else
    {
        LOG_ERROR("Invalid doc type: {}", docType);
        return -1;
    }

    return 0;
}

fz_context *MupdfUtil::getContext()
{
    return _ctx;
}

fz_document *MupdfUtil::getDocument()
{
    return _doc;
}

pdf_document *MupdfUtil::getPdfDocument()
{
    return _pdfDoc;
}

void MupdfUtil::destroy()
{
    if (NULL != _image)
    {
        fz_drop_image(_ctx, _image);
        _image = NULL;
    }

    if (NULL != _file_buffer)
    {
        fz_drop_buffer(_ctx, _file_buffer);
        _file_buffer = NULL;
    }

    if (NULL != _file_stream)
    {
        fz_drop_stream(_ctx, _file_stream);
        _file_stream = NULL;
    }

    if (NULL != _doc)
    {
        fz_drop_document(_ctx, _doc);
        _doc = NULL;
    }

    if (NULL != _ctx)
    {
        fz_drop_context(_ctx);
        _ctx = NULL;
    }

    return;
}

static int mupdf_incremental_save(fz_context *ctx, pdf_document *doc,
                                  fz_buffer *buffer, std::string &dst_pdf_info)
{
    // 增量保存
    pdf_write_options opts  = {0};
    opts.do_incremental     = 1;        // 增量保存
    opts.do_compress        = 1;        // 压缩流
    opts.do_compress_images = 1;        // 压缩图片流
    opts.do_compress_fonts  = 1;        // 压缩字体流

//#if 0
    /**
     * XRefStm为HybridXref(混合xref)，xref和xref stream同时存在，
     * /XRefStm 89267后面数字就是xref stream地址。
     * 这种情况应该作为老的xref处理不能作为xref stream处理，
     * 否则会导致adobe验证报未预期字节范围
     */
    if (1 == doc->has_xref_streams)
    {
        LOG_DEBUG("xref sections num: {}.", doc->num_xref_sections);
        for (int i = 0; i < doc->num_xref_sections; ++i)
        {
            pdf_xref *xref = &doc->xref_sections[i];
            if (NULL != xref->trailer)
            {
                pdf_obj *xrefstm = pdf_dict_get(ctx, xref->trailer, PDF_NAME(XRefStm));
                if (NULL != xrefstm)
                {
                    int64_t xrefstmofs = pdf_to_int64(ctx, xrefstm);
                    if (0 < xrefstmofs)
                    {
                        doc->has_xref_streams = 0;
                        LOG_DEBUG("xrefstm offset: {}, set xref stream 0.", xrefstmofs);
                        break;
                    }
                }
            }
        }
    }
//#endif

    /**
     * 写内存PDF
     */
    if (dst_pdf_info.empty())
    {
        unsigned char *datap = NULL;
        size_t len = fz_buffer_extract(ctx, buffer, &datap);
        std::stringstream stream(std::string((char*)datap, len));
        fz_output *out = NULL;
        fz_try(ctx)
        {
            // 内存流进行读写
            out = fz_new_output_with_stream(ctx, &stream);
            pdf_write_document(ctx, doc, out, &opts);
            fz_close_output(ctx, out);
        }
        fz_always(ctx)
        {
            fz_drop_output(ctx, out);
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to increment save pdf size: {}, error msg: {}.", len, fz_caught_message(ctx));
            return -1;
        }

        std::string dstPdfBuf = stream.str();
        dst_pdf_info.assign(dstPdfBuf.begin(), dstPdfBuf.end());
        LOG_DEBUG("Dst pdf is memory size: {}.", dst_pdf_info.size());
    }
    /**
     * 写文件PDF
     */
    else if (256 >= dst_pdf_info.size())
    {
        /* 目的文件存在，失败 */
        LOG_DEBUG("Dst pdf is path: {}.", dst_pdf_info);
        if (fileExists(dst_pdf_info))
        {
            LOG_ERROR("Dst pdf file is exist {}.", dst_pdf_info);
            return -1;
        }

        fz_try(ctx)
        {
            fz_save_buffer(ctx, buffer, dst_pdf_info.c_str());
            pdf_save_document(ctx, doc, dst_pdf_info.c_str(), &opts);
        }
        fz_catch(ctx)
        {
            LOG_ERROR("Failed to increment save pdf: {}, error msg: {}.", dst_pdf_info, fz_caught_message(ctx));
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Failed to dst pdf info size: {}.", dst_pdf_info.size());
        return -1;
    }

    return 0;
}

int MupdfUtil::pdfIncrementalSave(std::string &dstPdfInfo)
{
    return mupdf_incremental_save(_ctx, _pdfDoc, _file_buffer, dstPdfInfo);
}

static fz_image *fz_new_image_from_buf(fz_context *ctx, unsigned char *data, size_t size)
{
    fz_buffer *buffer = NULL;
    fz_image *image = NULL;

    buffer = fz_new_buffer_from_shared_data(ctx, data, size);
    fz_try(ctx)
    {
        image = fz_new_image_from_buffer(ctx, buffer);
    }
    fz_always(ctx)
    {
        fz_drop_buffer(ctx, buffer);
        buffer = NULL;
    }
    fz_catch(ctx)
    {
        LOG_ERROR("Failed to new image for buffer, err msg: {}.", fz_caught_message(ctx));
        fz_rethrow(ctx);
    }

    return image;
}

int MupdfUtil::loadImage(const std::string &picture)
{
    /**
     * 加载图片
     */
    fz_try(_ctx)
    {
        if (_isFilePath)
        {
            _image = fz_new_image_from_file(_ctx, picture.c_str());
        }
        else
        {
            _image = fz_new_image_from_buf(_ctx, (unsigned char*)&picture[0], picture.size());
        }
    }
    fz_catch(_ctx)
    {
        LOG_ERROR("Failed to new image err msg: {}.", fz_caught_message(_ctx));
        return -1;
    }

    if (NULL == _image)
    {
        LOG_ERROR("Empty image.");
        return -1;
    }

    /**
     * 设置透明通道
     */
    fz_try(_ctx)
    {
        // 设置图片mask，alpha背景透明
        fz_pixmap *pix = fz_get_pixmap_from_image(_ctx, _image, NULL, NULL, 0, 0);
        if (NULL == pix)
        {
            LOG_ERROR("Failed to get pixmap form imag.");
            return -1;
        }

        if (NULL != pix && pix->alpha == 1)
        {
            // have alpha, therefore create a mask
            fz_pixmap *pm = fz_convert_pixmap(_ctx, pix, NULL, NULL, NULL, NULL, 1);
            pm->alpha = 0;
            pm->colorspace = fz_keep_colorspace(_ctx, fz_device_gray(_ctx));

            fz_image *mask = fz_new_image_from_pixmap(_ctx, pm, NULL);
            fz_image *zimg = fz_new_image_from_pixmap(_ctx, pix, mask);
            fz_drop_image(_ctx, mask);
            fz_drop_pixmap(_ctx, pm);
            fz_drop_image(_ctx, _image);
            _image = zimg;
        }
        fz_drop_pixmap(_ctx, pix);
        pix = NULL;
    }
    fz_catch(_ctx)
    {
        LOG_ERROR("Failed to new image err msg: {}.", fz_caught_message(_ctx));
        return -1;
    }

    return 0;
}

int MupdfUtil::savePnm2Png(std::string &pngBuffer)
{
    fz_buffer *buffer = NULL;
    fz_try(_ctx)
    {
        buffer = fz_new_buffer_from_image_as_png(_ctx, _image, NULL);

        unsigned char *datap = NULL;
        ssize_t pngSize = fz_buffer_storage(_ctx, buffer, &datap);
        pngBuffer.assign((char*)datap, (char*)datap + pngSize);
    }
    fz_always(_ctx)
    {
        fz_drop_buffer(_ctx, buffer);
        buffer = NULL;
    }
    fz_catch(_ctx)
    {
        LOG_ERROR("Failed to fz_new_buffer_from_image_as_png, error msg: {}", fz_caught_message(_ctx));
        return -1;
    }

    return 0;
}

