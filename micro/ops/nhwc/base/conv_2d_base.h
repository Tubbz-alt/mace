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

#ifndef MICRO_OPS_NHWC_BASE_CONV_2D_BASE_H_
#define MICRO_OPS_NHWC_BASE_CONV_2D_BASE_H_

#include "micro/ops/nhwc/base/filter_op_base.h"
#include "micro/ops/utils/activation.h"

namespace micro {
namespace ops {

class Conv2dBase : public FilterOpBase {
 public:
  virtual MaceStatus OnInit();
  virtual MaceStatus Run();

 protected:
  virtual MaceStatus Compute(int32_t (&output_dims)[4]);

 protected:
  const mifloat *input_;
  const int32_t *input_dims_;
  uint32_t input_dim_size_;

  const mifloat *filter_;
  const int32_t *filter_dims_;
  uint32_t filter_dim_size_;

  const mifloat *bias_;
  const int32_t *bias_dims_;
  uint32_t bias_dim_size_;

  mifloat *output_;
  Activation activation_;

  MACE_OP_INPUT_TAGS(INPUT, FILTER, BIAS);
  MACE_OP_OUTPUT_TAGS(OUTPUT);
};
}  // namespace ops
}  // namespace micro


#endif  // MICRO_OPS_NHWC_BASE_CONV_2D_BASE_H_
