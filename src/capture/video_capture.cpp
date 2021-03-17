/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#include "capture/capture_params.hpp"
#include "capture/video_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/video_capture.hpp"
#include "capture/video_recorder.hpp"
#include "utils/sugar/algostring.hpp"

#include "core/RDP/RDPDrawable.hpp"

#include "gdi/capture_api.hpp"

#include "utils/log.hpp"
#include "utils/strutils.hpp"

#include <cerrno>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>


using namespace std::chrono_literals;

namespace
{
    inline time_t to_time_t(
        MonotonicTimePoint t,
        MonotonicTimeToRealTime monotonic_to_real)
    {
        auto duration = monotonic_to_real.to_real_time_duration(t);
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }
}


using WaitingTimeBeforeNextSnapshot = gdi::CaptureApi::WaitingTimeBeforeNextSnapshot;

// VideoCaptureCtx
//@{

VideoCaptureCtx::VideoCaptureCtx(
    MonotonicTimePoint now,
    RealTimePoint real_now,
    TraceTimestamp trace_timestamp,
    unsigned frame_rate,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame
)
: drawable(drawable)
, monotonic_last_time_capture(now)
, monotonic_start_capture(now)
, monotonic_to_real(now, real_now)
// `frame_interval % frame_rate` should be equal to 0
, frame_interval(MonotonicTimePoint::duration(std::chrono::seconds(1)) / frame_rate)
, update_trace_timer(bool(trace_timestamp) ? now : MonotonicTimePoint::max())
, trace_timestamp(trace_timestamp)
, image_frame_api(image_frame)
, timestamp_tracer(image_frame.get_writable_image_view())
{}

void VideoCaptureCtx::preparing_video_frame(video_recorder & recorder)
{
    this->drawable.trace_mouse();
    this->image_frame_api.prepare_image_frame();
    if (TraceTimestamp::Yes == this->trace_timestamp) {
        tm tm_result;
        time_t rawtime = to_time_t(this->monotonic_last_time_capture, this->monotonic_to_real);
        localtime_r(&rawtime, &tm_result);
        this->timestamp_tracer.trace(tm_result);
    }
    recorder.preparing_video_frame();

    if (TraceTimestamp::Yes == this->trace_timestamp) {
        this->timestamp_tracer.clear();
    }
    this->drawable.clear_mouse();
}

void VideoCaptureCtx::frame_marker_event(video_recorder & recorder)
{
    this->preparing_video_frame(recorder);
    this->has_frame_marker = true;
}

void VideoCaptureCtx::encoding_video_frame(video_recorder & recorder)
{
    ++this->frame_index;
    recorder.encoding_video_frame(this->frame_index);
}

void VideoCaptureCtx::encoding_end_frame(video_recorder & recorder)
{
    this->preparing_video_frame(recorder);
    recorder.encoding_video_frame(this->frame_index+2);
}

void VideoCaptureCtx::next_video()
{
    this->frame_index = 0;
}

uint16_t VideoCaptureCtx::width() const noexcept
{
    return this->drawable.width();
}

uint16_t VideoCaptureCtx::height() const noexcept
{
    return this->drawable.height();
}

size_t VideoCaptureCtx::pix_len() const noexcept
{
    return this->drawable.pix_len();
}

const uint8_t * VideoCaptureCtx::data() const noexcept
{
    return this->drawable.data();
}

void VideoCaptureCtx::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
}

WaitingTimeBeforeNextSnapshot VideoCaptureCtx::snapshot(
    video_recorder & recorder, MonotonicTimePoint now
)
{
    auto tick { now - this->monotonic_last_time_capture };
    auto const frame_interval = this->frame_interval;
    if (tick >= frame_interval) {
        if (!this->has_frame_marker) {
            this->preparing_video_frame(recorder);
            if (this->monotonic_last_time_capture >= this->update_trace_timer) {
                this->update_trace_timer += std::chrono::seconds(1);
            }
        }

        do {
            if (this->monotonic_last_time_capture >= this->update_trace_timer) {
                this->preparing_video_frame(recorder);
                this->update_trace_timer += std::chrono::seconds(1);
            }
            this->encoding_video_frame(recorder);
            this->monotonic_last_time_capture += frame_interval;
            tick -= frame_interval;
        }
        while (tick >= frame_interval);
    }

    return WaitingTimeBeforeNextSnapshot(frame_interval - tick);
}

//@}


static void log_video_params(VideoParams const& video_params)
{
    if (video_params.verbosity) {
        LOG(LOG_INFO, "Video recording: codec: %s, frame_rate: %u, options: %s",
            video_params.codec, video_params.frame_rate, video_params.codec_options);
    }
}

using TraceTimestamp = VideoCaptureCtx::TraceTimestamp;

// FullVideoCaptureImpl
//@{

FullVideoCaptureImpl::FullVideoCaptureImpl(
    CaptureParams const & capture_params,
    RDPDrawable & drawable, gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params, FullVideoParams const & full_video_params)
: video_cap_ctx(capture_params.now, capture_params.real_now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    video_params.frame_rate, drawable, image_frame)
, recorder(
    str_concat(
        std::string_view{capture_params.record_path},
        std::string_view{capture_params.basename},
        '.',
        video_params.codec
    ).c_str(),
    capture_params.groupid, capture_params.session_log,
    image_frame.get_image_view(),
    checked_int{video_params.frame_rate},
    video_params.codec.c_str(),
    video_params.codec_options.c_str(),
    checked_int{video_params.verbosity})
{
    log_video_params(video_params);
}

FullVideoCaptureImpl::~FullVideoCaptureImpl()
{
    this->video_cap_ctx.encoding_end_frame(this->recorder);
}


void FullVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint /*now*/, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->video_cap_ctx.frame_marker_event(this->recorder);
}

WaitingTimeBeforeNextSnapshot FullVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    return this->video_cap_ctx.snapshot(this->recorder, now);
}

void FullVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

//@}


// SequencedVideoCaptureImpl
//@{

SequencedVideoCaptureImpl::FilenameGenerator::FilenameGenerator(
    std::string_view prefix,
    std::string_view filename,
    std::string_view extension)
: filename(str_concat(prefix, filename, "-000000."_av, extension))
, num_pos(int(this->filename.size() - (extension.size() + 1)))
{}

void SequencedVideoCaptureImpl::FilenameGenerator::next()
{
    ++this->num;
    auto chars = int_to_decimal_chars(this->num);
    memcpy(this->filename.data() + this->num_pos - chars.size(), chars.data(), chars.size());
}

char const* SequencedVideoCaptureImpl::FilenameGenerator::current() const
{
    return this->filename.c_str();
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->video_cap_ctx.snapshot(*this->recorder, now);
    if (!this->ic_has_first_img) {
        return this->first_periodic_snapshot(now);
    }
    return this->video_sequencer_periodic_snapshot(now);
}

void SequencedVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->video_cap_ctx.frame_marker_event(*this->recorder);
    if (!this->ic_has_first_img) {
        this->first_periodic_snapshot(now);
    }
    else {
        this->video_sequencer_periodic_snapshot(now);
    }
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::first_periodic_snapshot(MonotonicTimePoint now)
{
    WaitingTimeBeforeNextSnapshot ret;

    auto constexpr interval = std::chrono::microseconds(3s) / 2;
    auto const duration = now - this->monotonic_start_capture;
    if (duration >= interval) {
        auto video_interval = this->break_interval;
        if (this->ic_drawable.logical_frame_ended()
         || duration > 2s
         || duration >= video_interval
        ) {
            tm ptm;
            time_t t = to_time_t(now, this->monotonic_to_real);
            localtime_r(&t, &ptm);
            this->ic_flush(ptm);
            this->ic_has_first_img = true;
            ret = WaitingTimeBeforeNextSnapshot(video_interval);
        }
        else {
            ret = WaitingTimeBeforeNextSnapshot(interval / 3);
        }
    }
    else {
        ret = WaitingTimeBeforeNextSnapshot(interval - duration);
    }

    return std::min(ret.duration(), this->video_sequencer_periodic_snapshot(now).duration());
}

void SequencedVideoCaptureImpl::init_recorder()
{
    this->recorder = std::make_unique<video_recorder>(
        this->vc_name_generator.current(),
        this->groupid,
        this->acl_report,
        this->image_frame_api.get_image_view(),
        this->video_params.frame_rate,
        this->video_params.codec.c_str(),
        this->video_params.codec_options.c_str(),
        this->video_params.verbosity
    );
    this->recorder->preparing_video_frame();
}

void SequencedVideoCaptureImpl::ic_flush(const tm & now)
{
    this->image_frame_api.prepare_image_frame();
    this->video_cap_ctx.timestamp_tracer.trace(now);
    this->ic_scaled_png.dump_png24(this->ic_name_generator.current(), this->image_frame_api, true);
    this->video_cap_ctx.timestamp_tracer.clear();
    this->ic_name_generator.next();
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::video_sequencer_periodic_snapshot(
    MonotonicTimePoint now)
{
    assert(this->break_interval.count());
    auto const interval = now - this->start_break;
    if (interval >= this->break_interval) {
        this->next_video_impl(now, NotifyNextVideo::Reason::sequenced);
    }
    return WaitingTimeBeforeNextSnapshot(this->break_interval);
}


SequencedVideoCaptureImpl::SequencedVideoCaptureImpl(
    CaptureParams const & capture_params,
    unsigned png_width, unsigned png_height,
    /* const */RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params,
    NotifyNextVideo & next_video_notifier)
: monotonic_start_capture(capture_params.now)
, monotonic_to_real(capture_params.now, capture_params.real_now)
, video_cap_ctx(capture_params.now, capture_params.real_now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    video_params.frame_rate, drawable, image_frame)
, vc_name_generator(capture_params.record_path, capture_params.basename, video_params.codec)
, groupid(capture_params.groupid)
, acl_report(capture_params.session_log)
, video_params(video_params)
, ic_name_generator(capture_params.record_path, capture_params.basename, "png")
, ic_drawable(drawable)
, image_frame_api(image_frame)
, ic_scaled_png(png_width, png_height)
, start_break(capture_params.now)
, break_interval((video_params.video_interval > std::chrono::microseconds::zero())
    ? video_params.video_interval
    : std::chrono::microseconds::max())
, next_video_notifier(next_video_notifier)
{
    log_video_params(video_params);
    this->init_recorder();
}

SequencedVideoCaptureImpl::~SequencedVideoCaptureImpl()
{
    if (this->recorder) {
        this->video_cap_ctx.encoding_end_frame(*this->recorder);
    }
}

void SequencedVideoCaptureImpl::next_video_impl(MonotonicTimePoint now, NotifyNextVideo::Reason reason)
{
    this->start_break = now;

    time_t t = to_time_t(now, this->monotonic_to_real);

    tm ptm;
    localtime_r(&t, &ptm);

    if (!this->ic_has_first_img) {
        this->ic_has_first_img = true;
        this->ic_flush(ptm);
    }

    if (this->recorder) {
        this->video_cap_ctx.encoding_end_frame(*this->recorder);
        this->recorder.reset();
        this->vc_name_generator.next();
    }

    this->init_recorder();

    this->ic_flush(ptm);

    this->next_video_notifier.notify_next_video(now, reason);
}

void SequencedVideoCaptureImpl::next_video(MonotonicTimePoint now)
{
    this->next_video_impl(now, NotifyNextVideo::Reason::external);
}

void SequencedVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

//@}
