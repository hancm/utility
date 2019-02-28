#pragma once

#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

class MupdfUtil
{
public:
    /**
     * @brief 初始化mupdf
     * @param [IN] docInfo          文档信息: 路径或者二进制缓存信息
     * @param [IN] docType          文档类型: 加载pdf、xps、cbz、image时需要指定
     * @return
     * @note
     */
    MupdfUtil(const std::string &docInfo, const char *docType);
    ~MupdfUtil();
    operator bool();

    fz_context *getContext();
    fz_document *getDocument();
    pdf_document *getPdfDocument();

    int pdfIncrementalSave(std::string &dstPdfInfo);

    /**
     * @brief 转换pnm图片为png
     * @param [IN] imageBuffer      pnm缓存数据
     * @param [IN] pngBuffer        png缓存数据
     * @return int
     * @note
     */
    int savePnm2Png(std::string &pngBuffer);
private:
    int init(const std::string &docInfo, const char *docType);
    void destroy();

    /**
     * @brief 加载图片
     * @param [IN] picture      图片路径或者图片二进制数据
     * @return int
     * @note
     */
    int loadImage(const std::string &picture);
private:
    bool _isSucceedInit = false;

    // 数据信息是否路径，否则内存数据
    bool _isFilePath = false;

    fz_context *_ctx = NULL;

    // 文档二进制缓存信息
    fz_stream *_file_stream = NULL;
    fz_buffer *_file_buffer = NULL;

    // pdf、xps、cbz文档时需要
    fz_document *_doc = NULL;
    pdf_document *_pdfDoc = NULL;

    // 加载图片信息
    fz_image *_image = NULL;
};
