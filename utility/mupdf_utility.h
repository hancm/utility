#pragma once

#include <string>
#include <map>

#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

// __MUPDF_VERBOSE_LESS_THAN_114__: mupdf版本小于1.14(1.14版本api改变)

namespace mupdf
{

typedef struct page_rect_info
{
    // 页面号: 从1开始
    int page;
    // 矩阵左上角坐标
    double x0, y0;
    // 矩阵右下角坐标
    double x1, y1;
} TAG_PAGE_RECT_INFO_S;

class MupdfUtil
{
public:
    /**
     * @brief 读取pdf、image、xps、cbz文档
     * @param [IN] docInfo      文档缓存或者路径信息
     * @param [IN] docType      文档类型: pdf、image、xps、cbz
     */
    MupdfUtil(const std::string &docInfo, const char *docType = "pdf");
    ~MupdfUtil();
    operator bool();

    fz_context *getContext();
    fz_document *getDocument();
    pdf_document *getPdfDocument();
    fz_image *getDocImage();

    fz_buffer *getFileBuffer();

    int getPdfPageCount();
    int pdfIncrementalSave(std::string &pdfOutputInfo);

    fz_image *getBufferImage(const std::string &imageBuffer);
    int savePnm2Png(std::string &pngBuffer);
    int saveImage2Png(std::string &pngOutBuffer);

    int hasXRefStream();

    int getKeywordPosition(const std::string &keyword, const std::vector<int> &pageList, std::vector<TAG_PAGE_RECT_INFO_S> &pageRectList);
    std::map <int, std::pair<pdf_page*, fz_stext_page*> > &getKeywordPositionPagesCache();

    /**
     * @brief PNG图片横向或者纵向分隔
     * @param [IN] splitCount               分隔图片数目
     * @param [OUT] splitPngImageList       分隔后的png图片列表
     * @param [IN] splitStyle               0: 横向分隔; 1: 纵向分隔
     * @return int
     * 成功: 0
     * 失败: -1
     * @note
     */
    int splitPngImage(int splitCount, std::vector<std::string> &splitPngImageList, int splitStyle = 0);
private:
    bool _isSucceedInit = false;

    // pdf/image/xps/cbz
    std::string _docType;

    fz_context *_ctx = NULL;

    // 文档二进制缓存信息
    fz_stream *_file_stream = NULL;
    fz_buffer *_file_buffer = NULL;

    // xps、cbz文档
    fz_document *_doc = NULL;

    pdf_document *_pdfDoc = NULL;

    // docType="image"加载的image
    fz_image *_docImage = NULL;

    // 图片列表
    std::vector<fz_image*> _imageList;

    // 查询关键字缓存
    // <页面号(1开始), <页面, 页面文本>>
    std::map <int, std::pair<pdf_page*, fz_stext_page*> > _pages_cache;
};

int pdf_has_xref_stream(const std::string &pdfBuffer, bool &first_xref_style, bool &has_xref_stream, bool &has_old_style_xref);

} /* namespace mupdf */
