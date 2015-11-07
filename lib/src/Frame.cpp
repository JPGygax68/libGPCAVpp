#include <cassert>

extern "C" {
#include "libavutil/frame.h"
}

#include "checked_calls.hpp"

#include <gpc/_av/Frame.hpp>

GPC_AV_NAMESPACE_START

// TODO: check if actually reference counted: https://ffmpeg.org/doxygen/2.7/structAVFrame.html#details

FrameBase::FrameBase()
{
    frame = _av(av_frame_alloc);
}

FrameBase::FrameBase(FrameBase&& from)
{
    frame = from.frame;
    from.frame = nullptr;
}

FrameBase::FrameBase(const FrameBase& from)
{
    frame = _av(av_frame_clone, from.frame);
}

void FrameBase::assign(const FrameBase &from)
{
    frame = _av(av_frame_clone, from.frame);
}

void FrameBase::assign(FrameBase &&from)
{
    frame = from.frame;
    from.frame = nullptr;
}

FrameBase::FrameBase(AVFrame *frame_): 
    frame(frame_)
{
}

auto FrameBase::presentation_timestamp() -> int64_t
{
    return frame->pts;
}

FrameBase::~FrameBase()
{
    //av_frame_free(&frame);
    av_frame_unref(frame);
    //assert(!frame);
}

GPC_AV_NAMESPACE_END
