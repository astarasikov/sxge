#ifndef __SXGE_SCENE_ANIMATION_HH__
#define __SXGE_SCENE_ANIMATION_HH__

#include <algorithm>
#include <vector>
#include <sxge/math/utils.hh>

namespace sxge {

typedef long long anitime_t;
extern anitime_t AnimationWallTime(void);

struct AnimationTarget {
	virtual void set(float value) = 0;
};

struct Interpolator {
	virtual float eval(float from, float to, float progress) = 0;
};

struct Animation {
	enum mode {
		Relative,
		Absolute,
	};
	anitime_t time_start;
	anitime_t time_end;

	float from;
	float to;

	AnimationTarget *target;
	Interpolator *interpolator;
};

struct Timeline {
	std::vector<Animation *> animations;
	anitime_t time_start;

	void tick(anitime_t time) {
		if (!time_start) {
			time_start = AnimationWallTime();
		}

		for (auto *a : animations) {
			if ((!a->interpolator) || (!a->target))
			{
				continue;
			}

			anitime_t position = (time - time_start) - (a->time_start);
			anitime_t duration = (a->time_end - a->time_start);

			float progress = (float)position / duration;

			if (progress < 0) {
				continue;
			}

			float from = a->from;
			float to = a->to;

			if ((int)floor(progress) & 1) {
				from = a->to;
				to = a->from;
			}

			a->target->set(a->interpolator->eval(from, to, fmodf(progress, 1.0f)));
		}
	}
};

struct VsyncEstimator {
	enum {
		vsync_buf_size = 3,
	};

	anitime_t buffer[vsync_buf_size];
	size_t idx;

	anitime_t predict(anitime_t now) {
		buffer[(idx++) % vsync_buf_size] = now;

		anitime_t tmp = 0;
		for (anitime_t t : buffer) {
			tmp += t;
		}
		return tmp / vsync_buf_size;
	};
};

/******************************************************************************
 * Some interpolators
 *****************************************************************************/
static struct LinearInterpolator : Interpolator {
	virtual float eval(float from, float to, float progress) {
		return from + ((to - from) * progress);
	}
} gLinearInterpolator = {};

static struct SineInterpolator : Interpolator {
	virtual float eval(float from, float to, float progress) {
		return from + ((to - from) * 0.5 * (1 + sin(progress * M_PI)));
	}
} gSineInterpolator = {};

static struct EaseOutBounceInterpolator : Interpolator {
	virtual float eval(float from, float to, float progress) {
		float c = (to - from);
		float b = from;
		float t = progress;

		if (t < (1/2.75)) {
			return c*(7.5625*t*t) + b;
		} else if (t < (2/2.75)) {
			return c*(7.5625*(t-=(1.5/2.75))*t + .75) + b;
		} else if (t < (2.5/2.75)) {
			return c*(7.5625*(t-=(2.25/2.75))*t + .9375) + b;
		} else {
			return c*(7.5625*(t-=(2.625/2.75))*t + .984375) + b;
		}
	}
} gEaseOutBounceInterpolator;

struct IntAnimationTarget : AnimationTarget {
	int &target;
	IntAnimationTarget(int &ptr) : target(ptr) {}

	virtual void set(float value) {
		target = value;
	}
};

struct FloatAnimationTarget : AnimationTarget {
	float &target;
	FloatAnimationTarget(float &ptr) : target(ptr) {}

	virtual void set(float value) {
		target = value;
	}
};

} //namespace sxge

#endif //__SXGE_SCENE_ANIMATION_HH__
