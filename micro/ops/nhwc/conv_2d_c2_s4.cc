// Copyright 2020 The MACE Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "micro/ops/nhwc/conv_2d_c2_s4.h"

#include "micro/base/logging.h"

namespace micro {
namespace ops {

MaceStatus Conv2dC2S4Op::Compute(int32_t (&output_dims)[4]) {
  const int32_t batch = output_dims[0];
  const int32_t height = output_dims[1];
  const int32_t width = output_dims[2];
  const int32_t channel = output_dims[3];
  MACE_ASSERT(channel == 2);
  const int32_t k_height = filter_dims_[1];
  const int32_t k_width = filter_dims_[2];
  const int32_t k_channel = filter_dims_[3];
  MACE_ASSERT(filter_dims_[0] == channel && input_dims_[3] == k_channel);
  const int32_t in_height = input_dims_[1];
  const int32_t in_width = input_dims_[2];
  const int32_t in_channel = input_dims_[3];

  const int32_t pad_top = padding_sizes_[0] >> 1;
  const int32_t pad_left = padding_sizes_[1] >> 1;

  const int32_t size = batch * height * width;
  const int32_t size_end = size - 4;

  for (int32_t s = 0; s < size; s += 4) {
    if (s > size_end) {
      s = size_end;
    }
    int32_t h0 = s / width % height;
    int32_t h1 = (s + 1) / width % height;
    int32_t h2 = (s + 2) / width % height;
    int32_t h3 = (s + 3) / width % height;
    const int32_t in_h0 = h0 * strides_[0] - pad_top;
    const int32_t in_h1 = h1 * strides_[0] - pad_top;
    const int32_t in_h2 = h2 * strides_[0] - pad_top;
    const int32_t in_h3 = h3 * strides_[0] - pad_top;
    int32_t w0 = s % width;
    int32_t w1 = (s + 1) % width;
    int32_t w2 = (s + 2) % width;
    int32_t w3 = (s + 3) % width;

    int32_t width_base[4] = {s * channel};
    width_base[1] = width_base[0] + channel;
    width_base[2] = width_base[1] + channel;
    width_base[3] = width_base[2] + channel;
    const int32_t in_w0 = w0 * strides_[1] - pad_left;
    const int32_t in_w1 = w1 * strides_[1] - pad_left;
    const int32_t in_w2 = w2 * strides_[1] - pad_left;
    const int32_t in_w3 = w3 * strides_[1] - pad_left;
    for (int32_t kb = 0; kb < channel; kb += 2) {
      const int32_t k_batch_base0 = kb * k_height;
      const int32_t k_batch_base1 = k_batch_base0 + k_height;
      float output[2 * 4] = {0};
      for (int32_t kh = 0; kh < k_height; ++kh) {
        const int32_t in_h_idx0 = in_h0 + kh * dilations_[0];
        const int32_t in_h_idx1 = in_h1 + kh * dilations_[0];
        const int32_t in_h_idx2 = in_h2 + kh * dilations_[0];
        const int32_t in_h_idx3 = in_h3 + kh * dilations_[0];

        bool h_valid[4] = {true, true, true, true};
        if (in_h_idx0 < 0 || in_h_idx0 >= in_height) {
          h_valid[0] = false;
        }
        if (in_h_idx1 < 0 || in_h_idx1 >= in_height) {
          h_valid[1] = false;
        }
        if (in_h_idx2 < 0 || in_h_idx2 >= in_height) {
          h_valid[2] = false;
        }
        if (in_h_idx3 < 0 || in_h_idx3 >= in_height) {
          h_valid[3] = false;
        }
        const int32_t k_height_base0 = (k_batch_base0 + kh) * k_width;
        const int32_t k_height_base1 = (k_batch_base1 + kh) * k_width;
        const int32_t in_h_base0 = in_h_idx0 * in_width;
        const int32_t in_h_base1 = in_h_idx1 * in_width;
        const int32_t in_h_base2 = in_h_idx2 * in_width;
        const int32_t in_h_base3 = in_h_idx3 * in_width;
        for (int32_t kw = 0; kw < k_width; ++kw) {
          const int32_t kw_dilations = kw * dilations_[1];
          const int32_t in_w_idx0 = in_w0 + kw_dilations;
          const int32_t in_w_idx1 = in_w1 + kw_dilations;
          const int32_t in_w_idx2 = in_w2 + kw_dilations;
          const int32_t in_w_idx3 = in_w3 + kw_dilations;

          bool valid[4] = {
              h_valid[0], h_valid[1], h_valid[2], h_valid[3]
          };
          if (in_w_idx0 < 0 || in_w_idx0 >= in_width) {
            valid[0] = false;
          }
          if (in_w_idx1 < 0 || in_w_idx1 >= in_width) {
            valid[1] = false;
          }
          if (in_w_idx2 < 0 || in_w_idx2 >= in_width) {
            valid[2] = false;
          }
          if (in_w_idx3 < 0 || in_w_idx3 >= in_width) {
            valid[3] = false;
          }

          const int32_t k_width_base0 = (k_height_base0 + kw) * k_channel;
          const int32_t k_width_base1 = (k_height_base1 + kw) * k_channel;
          const int32_t in_w_base[4] = {
              (in_h_base0 + in_w_idx0) * in_channel,
              (in_h_base1 + in_w_idx1) * in_channel,
              (in_h_base2 + in_w_idx2) * in_channel,
              (in_h_base3 + in_w_idx3) * in_channel
          };
          for (int32_t kc = 0; kc < k_channel; ++kc) {
            float filter0 = filter_[k_width_base0 + kc];
            float filter1 = filter_[k_width_base1 + kc];
            if (valid[0]) {
              float input0 = input_[in_w_base[0] + kc];
              output[0] += input0 * filter0;
              output[1] += input0 * filter1;
            }
            if (valid[1]) {
              float input1 = input_[in_w_base[1] + kc];
              output[2] += input1 * filter0;
              output[3] += input1 * filter1;
            }
            if (valid[2]) {
              float input2 = input_[in_w_base[2] + kc];
              output[4] += input2 * filter0;
              output[5] += input2 * filter1;
            }
            if (valid[3]) {
              float input3 = input_[in_w_base[3] + kc];
              output[6] += input3 * filter0;
              output[7] += input3 * filter1;
            }
          }  // filter channel
        }  // filter width
      }  // filter height
      for (int32_t i = 0; i < 4; ++i) {
        int32_t out_idx = width_base[i] + kb;
        int32_t buf_idx = i * 2;
        output_[out_idx] = output[buf_idx];
        output_[out_idx + 1] = output[buf_idx + 1];
      }
    }  // filter batch, output channel
  }  // output size

  return MACE_SUCCESS;
}

}  // namespace ops
}  // namespace micro