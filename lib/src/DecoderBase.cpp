#include <utility>
#include <cassert>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#include <gpc/_av/checked_calls.hpp>

#include <gpc/_av/internal/Decoder.hpp>
#include <gpc/_av/internal/Decoder_Impl.hpp>

GPC_AV_NAMESPACE_START

using namespace std;

// PUBLIC INTERFACE -------------------------------------------------

DecoderBase::DecoderBase(DecoderBase::Impl *impl) :
    _p(impl)
{
}

DecoderBase::DecoderBase(DecoderBase&& from)
{
    _p.swap(from._p);
}

DecoderBase & DecoderBase::operator = (DecoderBase&& from)
{
    _p.swap(from._p);

    return *this;
}

auto DecoderBase::time_base() const -> duration_t
{
    return _p->time_base();
}

// FRIEND INTERFACES ------------------------------------------------

// IMPLEMENTATION (PIMPL) -------------------------------------------

DecoderBase::Impl::Impl() = default;

DecoderBase::Impl::Impl(AVCodecContext *context_, AVCodec *codec_) :
    context(context_), codec(codec_), frame(AV(av_frame_alloc))
{
}

auto DecoderBase::Impl::time_base() const -> duration_t
{
    assert(context);

    return context->time_base;
}

GPC_AV_NAMESPACE_END
