#define _LARGEFILE_SOURCE
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "mupdf/fitz.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <fstream>
#include <sstream>

#include "MyLog.h"

struct fz_buffer_s
{
    int refs;
    unsigned char *data;
    size_t cap, len;
    int unused_bits;
    int shared;
};

typedef struct fz_filestream_stream_s
{
    std::iostream *stream;
    unsigned char buffer[4096];
} fz_filestream_stream;

static int next_stream(fz_context *ctx, fz_stream *stm, size_t n)
{
    fz_filestream_stream *state = (fz_filestream_stream*)stm->state;

    state->stream->read((char*)state->buffer, sizeof(state->buffer));
    n = state->stream->gcount();

    if (n < sizeof(state->buffer) && !state->stream->eof()) {
        LOG_ERROR("Failed to read.");
        fz_throw(ctx, FZ_ERROR_GENERIC, "read error: %s", strerror(errno));
    }
    stm->rp = state->buffer;
    stm->wp = state->buffer + n;
    stm->pos += (int64_t)n;

    if (n == 0) {
        return EOF;
    }
    return *stm->rp++;
}

static void seek_stream(fz_context *ctx, fz_stream *stm, int64_t offset, int whence)
{
    fz_filestream_stream *state = (fz_filestream_stream*)stm->state;

    std::ios_base::seekdir way = std::ios_base::beg;
    if (SEEK_SET == whence)
    {
        way = std::ios_base::beg;
    } else if (SEEK_CUR == whence)
    {
        way = std::ios_base::cur;
    } else if (SEEK_END == whence)
    {
        way = std::ios_base::end;
    }

    state->stream->seekg(offset, way);
    if (!state->stream->good()) {
        LOG_ERROR("Failed to seekg.");
        fz_throw(ctx, FZ_ERROR_GENERIC, "cannot seek: %s", strerror(errno));
    }

    stm->pos = state->stream->tellg();
    stm->rp = state->buffer;
    stm->wp = state->buffer;
}

static void drop_stream(fz_context *ctx, void *state_)
{
    fz_filestream_stream *state = (fz_filestream_stream*)state_;
    fz_free(ctx, state);
    state = NULL;
}

static fz_stream *fz_open_stream_ptr(fz_context *ctx, std::iostream *stream)
{
    fz_stream *stm;
    fz_filestream_stream *state = fz_malloc_struct(ctx, fz_filestream_stream);
    state->stream = stream;

    stm = fz_new_stream(ctx, state, next_stream, drop_stream);
    stm->seek = seek_stream;

    return stm;
}

static fz_stream *fz_open_stream_ptr_no_close(fz_context *ctx, std::iostream *stream)
{
    fz_stream *stm = fz_open_stream_ptr(ctx, stream);
    /* We don't own the file ptr. Ensure we don't close it */
    stm->drop = fz_free;
    return stm;
}

/**
 * 创建基于内存的output
 */

static void stream_write(fz_context *ctx, void *opaque, const void *buffer, size_t count)
{
    std::iostream *stream = (std::iostream*)opaque;

    if (count == 0) {
        return;
    }

    if (count == 1)
    {
        stream->put(((unsigned char*)buffer)[0]);
        if (!stream->good()) {
            LOG_ERROR("Failed to put.");
            fz_throw(ctx, FZ_ERROR_GENERIC, "cannot put: %s", strerror(errno));
        }
        return;
    }

    stream->write((char*)buffer, count);
    if (!stream->good()) {
        LOG_ERROR("Failed to write: {}.", count);
        fz_throw(ctx, FZ_ERROR_GENERIC, "cannot write: %s", strerror(errno));
    }
}

static void stream_drop(fz_context *ctx, void *opaque)
{
//  std::iostream *stream = (std::iostream*)opaque;

//  delete stream;
//  stream = NULL;
}

static void stream_seek(fz_context *ctx, void *opaque, int64_t off, int whence)
{
    std::iostream *stream = (std::iostream*)opaque;
    stream->clear();

    std::ios_base::seekdir way = std::ios_base::beg;
    if (SEEK_SET == whence)
    {
        way = std::ios_base::beg;
    } else if (SEEK_CUR == whence)
    {
        way = std::ios_base::cur;
    } else if (SEEK_END == whence)
    {
        way = std::ios_base::end;
    }

    stream->seekp(off, way);
    if (!stream->good()) {
        LOG_ERROR("Failed to seekp.");
        fz_throw(ctx, FZ_ERROR_GENERIC, "cannot seekp: %s", strerror(errno));
    }
}

static int64_t stream_tell(fz_context *ctx, void *opaque)
{
    std::iostream *stream = (std::iostream*)opaque;

    int64_t off = stream->tellp();
    if (off == -1) {
        LOG_ERROR("Failed to tellp.");
        fz_throw(ctx, FZ_ERROR_GENERIC, "cannot tell: %s", strerror(errno));
    }
    return off;
}

static fz_stream *stream_as_fz_stream(fz_context *ctx, void *opaque)
{
    std::iostream *stream = (std::iostream*)opaque;
    stream->flush();
    return fz_open_stream_ptr_no_close(ctx, stream);
};

fz_output *fz_new_output_with_stream(fz_context *ctx, std::iostream *stream)
{
    fz_output *out = NULL;
    out = fz_new_output(ctx, 0, stream, stream_write, NULL, stream_drop);
    out->seek = stream_seek;
    out->tell = stream_tell;
    out->as_stream = stream_as_fz_stream;

    return out;
}