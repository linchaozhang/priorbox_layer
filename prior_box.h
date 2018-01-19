#ifndef PRIOR_BOX_HPP
#define PRIOR_BOX_HPP

#include <vector>
#include "common.hpp"
#include <cmath>
#include "type.hpp"
//const float EPS = 0.000001f;

struct Param_prior_box {
  int num_priors;
  float step, offset;
  bool flip, clip;
  std::vector<float> min_sizes, max_sizes, aspect_ratios, variance, top_data;
  int height, width, img_height,
      img_width; // height = bottom.shape(2), weight = bottom.shape(3)
  Param_prior_box() {
    num_priors = 1;
    flip = true;
    clip = false;
    step = 0;
  }
};

class PriorBox {
public:

  void Setup(Param_prior_box param);
  int ComptutePriorBox();
  const float *PriorData();
  int GetOutputSize();
  void Release();

private:
  int num_priors_, height_, width_, img_height_, img_width_, total_out_size_;
  ;
  float step_, offset_;
  bool flip_, clip_;
  std::vector<float> min_sizes_, max_sizes_, aspect_ratios_, variance_;
  float *prior_box_;
};

#endif // PRIOR_BOX_HPP
