/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "stats.h"

#include <algorithm>  // min_element, max_element
#include <cassert>
#include <cstdio>

#include "util.h"

namespace webrtc {
namespace test {

Stats::Stats() {
}

Stats::~Stats() {
}

bool LessForEncodeTime(const FrameStatistic& s1, const FrameStatistic& s2) {
    return s1.encode_time_in_us < s2.encode_time_in_us;
}

bool LessForDecodeTime(const FrameStatistic& s1, const FrameStatistic& s2) {
    return s1.decode_time_in_us < s2.decode_time_in_us;
}

bool LessForEncodedSize(const FrameStatistic& s1, const FrameStatistic& s2) {
    return s1.encoded_frame_length_in_bytes < s2.encoded_frame_length_in_bytes;
}

bool LessForBitRate(const FrameStatistic& s1, const FrameStatistic& s2) {
    return s1.bit_rate_in_kbps < s2.bit_rate_in_kbps;
}


FrameStatistic& Stats::NewFrame(int frame_number) {
  assert(frame_number >= 0);
  FrameStatistic stat;
  stat.frame_number = frame_number;
  stats_.push_back(stat);
  return stats_[frame_number];
}

void Stats::PrintSummary() {
  log("Processing summary:\n");
  if (stats_.size() == 0) {
    log("No frame statistics have been logged yet.\n");
    return;
  }

  // Calculate min, max, average and total encoding time
  int total_encoding_time_in_us = 0;
  int total_decoding_time_in_us = 0;
  int total_encoded_frames_lengths = 0;
  int total_encoded_key_frames_lengths = 0;
  int total_encoded_nonkey_frames_lengths = 0;
  int nbr_keyframes = 0;
  int nbr_nonkeyframes = 0;

  for (FrameStatisticsIterator it = stats_.begin();
      it != stats_.end(); ++it) {
    total_encoding_time_in_us += it->encode_time_in_us;
    total_decoding_time_in_us += it->decode_time_in_us;
    total_encoded_frames_lengths += it->encoded_frame_length_in_bytes;
    if (it->frame_type == webrtc::kKeyFrame) {
      total_encoded_key_frames_lengths += it->encoded_frame_length_in_bytes;
      nbr_keyframes++;
    } else {
      total_encoded_nonkey_frames_lengths += it->encoded_frame_length_in_bytes;
      nbr_nonkeyframes++;
    }
  }

  FrameStatisticsIterator frame;

  // ENCODING
  log("Encoding time:\n");
  frame = min_element(stats_.begin(),
                      stats_.end(), LessForEncodeTime);
  log("  Min     : %7d us (frame %d)\n",
         frame->encode_time_in_us, frame->frame_number);

  frame = max_element(stats_.begin(),
                      stats_.end(), LessForEncodeTime);
  log("  Max     : %7d us (frame %d)\n",
         frame->encode_time_in_us, frame->frame_number);

  log("  Average : %7d us\n",
         total_encoding_time_in_us / stats_.size());

  // DECODING
  log("Decoding time:\n");
  // only consider frames that were successfully decoded (packet loss may cause
  // failures)
  std::vector<FrameStatistic> decoded_frames;
  for (std::vector<FrameStatistic>::iterator it = stats_.begin();
      it != stats_.end(); ++it) {
    if (it->decoding_successful) {
      decoded_frames.push_back(*it);
    }
  }
  if (decoded_frames.size() == 0) {
    printf("No successfully decoded frames exist in this statistics.");
  } else {
    frame = min_element(decoded_frames.begin(),
                        decoded_frames.end(), LessForDecodeTime);
    log("  Min     : %7d us (frame %d)\n",
           frame->decode_time_in_us, frame->frame_number);

    frame = max_element(decoded_frames.begin(),
                        decoded_frames.end(), LessForDecodeTime);
    log("  Max     : %7d us (frame %d)\n",
           frame->decode_time_in_us, frame->frame_number);

    log("  Average : %7d us\n",
           total_decoding_time_in_us / decoded_frames.size());
    log("  Failures: %d frames failed to decode.\n",
        (stats_.size() - decoded_frames.size()));
  }

  // SIZE
  log("Frame sizes:\n");
  frame = min_element(stats_.begin(),
                      stats_.end(), LessForEncodedSize);
  log("  Min     : %7d bytes (frame %d)\n",
         frame->encoded_frame_length_in_bytes, frame->frame_number);

  frame = max_element(stats_.begin(),
                      stats_.end(), LessForEncodedSize);
  log("  Max     : %7d bytes (frame %d)\n",
         frame->encoded_frame_length_in_bytes, frame->frame_number);

  log("  Average : %7d bytes\n",
         total_encoded_frames_lengths / stats_.size());
  if (nbr_keyframes > 0) {
    log("  Average key frame size    : %7d bytes (%d keyframes)\n",
           total_encoded_key_frames_lengths / nbr_keyframes,
           nbr_keyframes);
  }
  if (nbr_nonkeyframes > 0) {
    log("  Average non-key frame size: %7d bytes (%d frames)\n",
             total_encoded_nonkey_frames_lengths / nbr_nonkeyframes,
             nbr_nonkeyframes);
  }

  // BIT RATE
  log("Bit rates:\n");
  frame = min_element(stats_.begin(),
                      stats_.end(), LessForBitRate);
  log("  Min bit rate: %7d kbps (frame %d)\n",
           frame->bit_rate_in_kbps, frame->frame_number);

  frame = max_element(stats_.begin(),
                      stats_.end(), LessForBitRate);
  log("  Max bit rate: %7d kbps (frame %d)\n",
             frame->bit_rate_in_kbps, frame->frame_number);

  log("\n");
  log("Total encoding time  : %7d ms.\n",
         total_encoding_time_in_us / 1000);
  log("Total decoding time  : %7d ms.\n",
         total_decoding_time_in_us / 1000);
  log("Total processing time: %7d ms.\n",
         (total_encoding_time_in_us + total_decoding_time_in_us) / 1000);
}

}  // namespace test
}  // namespace webrtc