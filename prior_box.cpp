#include "prior_box.h"

void PriorBox::Setup(Param_prior_box param){
	min_sizes_ = param.min_sizes;
	//CHECK_GT(min_sizes_.size(), 0) << "must provide min_size.";
	flip_ = param.flip;
	const auto &aspect_ratios = param.aspect_ratios;
	aspect_ratios_.clear();
	aspect_ratios_.push_back(1.f);
	for (const auto &ratio : aspect_ratios) {
		bool already_exist = false;
		for (const auto &ar : aspect_ratios_) {
			if (std::abs(ratio - ar) < EPS) {
				already_exist = true;
				break;
			}
		}
		if (!already_exist) {
			aspect_ratios_.push_back(ratio);
			if (flip_) {
				aspect_ratios_.push_back(1.f / ratio);
			}
		}
	}
	num_priors_ = static_cast<int>(aspect_ratios_.size() * min_sizes_.size());
	max_sizes_ = param.max_sizes;
	if (!max_sizes_.empty()) {
		//CHECK_EQ(min_sizes_.size(), max_sizes_.size());
		for (int i = 0; i < max_sizes_.size(); ++i) {
			//CHECK_GT(max_sizes_[i], min_sizes_[i])
		//		<< "max_size must be greater than min_size.";
			num_priors_ += 1;
		}
	}
	clip_ = param.clip;
	variance_.clear();
	variance_ = param.variance;
	if (variance_.size() > 1) {
		//CHECK_EQ(variance_.size(), 4);
	}
	else if (variance_.empty()) {
		variance_.push_back(0.1);
	}
	step_ = param.step;
	offset_ = param.offset;
	height_ = param.height;
	width_ = param.width;
	img_height_ = param.img_height;
	img_width_ = param.img_width;
	total_out_size_ = height_ * width_ * num_priors_ * 4;
	prior_box_ = nullptr;
	prior_box_ = static_cast<float *>(Shadow::fast_malloc(2 * total_out_size_ * sizeof(float)));
}

int PriorBox::ComptutePriorBox() {
	float step_h, step_w;
	if (step_ == 0) {
		step_h = static_cast<float>(img_height_) / height_;
		step_w = static_cast<float>(img_width_) / width_;
	}
	else {
		step_h = step_w = step_;
	}
	int idx = 0;
	for (int h = 0; h < height_; ++h) {
		for (int w = 0; w < width_; ++w) {
			float center_h = (h + offset_) * step_h;
			float center_w = (w + offset_) * step_w;
			float box_width, box_height;
			for (int s = 0; s < min_sizes_.size(); ++s) {
				float min_size = min_sizes_[s];
				// first prior: aspect_ratio = 1, size = min_size
				box_width = box_height = min_size;
				prior_box_[idx++] = (center_w - box_width / 2.f) / img_width_;
				prior_box_[idx++] = (center_h - box_height / 2.f) / img_height_;
				prior_box_[idx++] = (center_w + box_width / 2.f) / img_width_;
				prior_box_[idx++] = (center_h + box_height / 2.f) / img_height_;

				if (!max_sizes_.empty()) {
					//CHECK_EQ(min_sizes_.size(), max_sizes_.size());
					float max_size = max_sizes_[s];
					// second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)
					box_width = box_height = std::sqrt(min_size * max_size);
					prior_box_[idx++] = (center_w - box_width / 2.f) / img_width_;
					prior_box_[idx++] = (center_h - box_height / 2.f) / img_height_;
					prior_box_[idx++] = (center_w + box_width / 2.f) / img_width_;
					prior_box_[idx++] = (center_h + box_height / 2.f) / img_height_;
				}

				// rest of priors
				for (const auto &ar : aspect_ratios_) {
					if (std::abs(ar - 1.f) < EPS) {
						continue;
					}
					box_width = min_size * std::sqrt(ar);
					box_height = min_size / std::sqrt(ar);
					prior_box_[idx++] = (center_w - box_width / 2.f) / img_width_;
					prior_box_[idx++] = (center_h - box_height / 2.f) / img_height_;
					prior_box_[idx++] = (center_w + box_width / 2.f) / img_width_;
					prior_box_[idx++] = (center_h + box_height / 2.f) / img_height_;
				}
			}
		}
	}
	if (clip_) {
		for (int i = 0; i < total_out_size_; ++i) {
			prior_box_[i] = std::min(std::max(prior_box_[i], 0.f), 1.f);
		}
	}
	int top_offset = total_out_size_;
	if (variance_.size() == 1) {
		for (int i = 0; i < total_out_size_; ++i) {
			prior_box_[top_offset + i] = variance_[0];
		}
	}
	else {
		int count = 0;
		for (int h = 0; h < height_; ++h) {
			for (int w = 0; w < width_; ++w) {
				for (int i = 0; i < num_priors_; ++i) {
					for (int j = 0; j < 4; ++j) {
						prior_box_[top_offset + (count++)] = variance_[j];
					}
				}
			}
		}
	}
	return 0;
}

const float *PriorBox::PriorData(){
	return prior_box_;
}

int PriorBox::GetOutputSize(){
	return total_out_size_;
}

void PriorBox::Release() {
	Shadow::fast_free(prior_box_);
}
