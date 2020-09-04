#ifndef __MUPDF_STREAM__OPEN__H__
#define __MUPDF_STREAM__OPEN__H__

#include <sstream>
#include "mupdf/fitz.h"

namespace mupdf
{

fz_output *fz_new_output_with_stream(fz_context *ctx, std::iostream *stream);

} /* namespace mupdf */
#endif /* __MUPDF_STREAM__OPEN__H__ */