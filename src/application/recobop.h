#ifndef RECOBOP_H
#define RECOBOP_H

#include "reconos.h"
#include "log.h"
#include "a9timer.h"
#include "hdmi_input.h"
#include "hdmi_output.h"
#include "cycle_timer.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

t_cycle_timer cycle_timer;

struct recobop_info {
	volatile uint32_t* pTouch;			// +0
	volatile uint32_t* pServo;			// +4
	volatile uint32_t  demo_nr;			// +8
	volatile uint32_t* timerregister;	// +12
	volatile uint32_t* stackaddr;		// +16
	volatile int thread_count;
	volatile struct reconos_thread *thread_p[16];
	
};


typedef struct {
	t_hdmi_input	hdmi_input;
	t_hdmi_output 	hdmi_output;

	volatile struct reconos_thread *thread_p;

} t_video_info;


static inline int rbi_thread_count_m(struct recobop_info *rb_info,
                                     char *thread_name, int thread_mode) {
	int i, count = 0;
	struct reconos_thread *rt;

	for (i = 0; i < 16; i++) {
		rt = rb_info->thread_p[i];
		if (!rt) {
			continue;
		}

		if (strcmp(thread_name, rt->name) == 0)
			if (   (  rt->state == RECONOS_THREAD_STATE_RUNNING_HW
				    && thread_mode == RECONOS_THREAD_HW)
			    || (  rt->state == RECONOS_THREAD_STATE_RUNNING_SW
			    	&& thread_mode == RECONOS_THREAD_SW)) {
				count++;
			}
	}

	return count;
}

static inline int rbi_thread_count(struct recobop_info *rb_info,
                                   char *thread_name) {
	return   rbi_thread_count_m(rb_info, thread_name, RECONOS_THREAD_SW)
	       + rbi_thread_count_m(rb_info, thread_name, RECONOS_THREAD_HW);
}

static inline int rbi_thread_index(struct recobop_info *rb_info,
                                   char *thread_name, int thread_mode) {
	int i;
	struct reconos_thread *rt;

	for (i = 0; i < 16; i++) {
		rt = rb_info->thread_p[i];
		if (!rt) {
			continue;
		}

		if (   strcmp(thread_name, rt->name) == 0 
			&& rt->state == RECONOS_THREAD_STATE_RUNNING_HW
			&& thread_mode == RECONOS_THREAD_HW) {
			return i;
		}

		if (   strcmp(thread_name, rt->name) == 0 
			&& rt->state == RECONOS_THREAD_STATE_RUNNING_SW
			&& thread_mode == RECONOS_THREAD_SW) {
			return i;
		}
	}

	return -1;
}

static inline int rbi_thread_index_free(struct recobop_info *rb_info) {
	int i;

	for (i = 0; i < 16; i++) {
		if (!rb_info->thread_p[i]) {
			return i;
		}
	}

	return -1;
}

static inline uint32_t fltofi(float f, int n, int dn) {
	int i;
	int wn = n - dn;

	int d;
	float w;

	uint32_t df, wf;
	uint32_t m = 0xffffffff;

	if (f > 0) {
		d = floor(f);
		w = f - d;
	} else {
		d = floor(f);
		w = f - d;
	}

	df = 0 | (d << wn);

	wf = 0;
	for (i = -1; i >= -wn; i--) {
		if (w >= pow(2, i)) {
			wf |= 1 << (wn + i);
			w -= pow(2, i);
		}
	}

	return (df | wf) & (m >> (32 - n));
}

static inline float fitofl(uint32_t f, int n, int dn) {
	int i;
	int wn = n - dn;

	int d;
	float w;

	uint32_t m = 0xffffffff;

	if (((f >> (n - 1)) & 0x1) == 1) {
		d = (m << dn) | (f >> wn);
	} else {
		d = 0 | (f >> wn);
	}

	w = 0;
	for (i = -1; i >= -wn; i--) {
		if (((f >> (wn + i)) & 0x1) == 1) {
			w += pow(2, i);
		}
	}

	return d + w;
}


t_log log_sw_control;
t_log log_sw_inverse;
t_log log_sw_video;


t_a9timer * a9timer;

#else
extern t_log log_sw_control;
extern t_log log_sw_inverse;
extern t_log log_sw_video;

extern t_a9timer * a9timer;

extern t_cycletimer cycle_timer;
#endif /* RECOBOP_H */