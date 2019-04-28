
#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_math.h"

#define KP -0.3
//#define KI -0.00004
#define KI 0
#define KD -28

#define MC 16
#define MCL 4

static ap_fixed<22,12> average(ap_fixed<22,12> *data, int mc) {
	ap_fixed<22 + MCL,15 + MCL> sum = 0;

	for (int i = 0; i < mc; i++) {
		sum += data[i];
	}

	return sum / mc;
}

THREAD_ENTRY() {
	ap_uint<32> rb_info;

	THREAD_INIT();
	rb_info.range(31,0)  = GET_INIT_DATA();

	ap_fixed<22,12> error_x, error_x_diff, error_x_sum = 0;
	ap_fixed<22,12> error_y, error_y_diff, error_y_sum = 0;

	ap_int<12> p_p_b_x_last = 0, p_p_b_y_last = 0;

	ap_fixed<22,12> error_x_diff_m[MC], error_y_diff_m[MC];


	ap_uint<32> tmp = 1;		
	MEM_WRITE(tmp, (uint32)(rb_info+36).range(31,0) , 4);


	while (1) {

		ap_uint<32> pos;
		pos.range(31,0) = MBOX_GET(touch_pos);
		ap_uint<32> wait;
		wait.range(31,0) =  100000;   //MBOX_GET(touch_pos);
		//MBOX_PUT(performance_perf, (ap_uint<8>("10", 16), ap_uint<24>(0)));

		ap_uint<12> pos_x = pos(23, 12);
		ap_uint<12> pos_y = pos(11, 0);

		ap_int<12> p_p_b_x, p_p_b_y;
		for (int i = 0; i < 12; i++) {
			p_p_b_x[i] = pos_x[i];
			p_p_b_y[i] = pos_y[i];
		}

		ap_fixed<22,12> delta = ap_fixed<42,32>(wait) / 100000;
		int mc = -(int)delta / 6 + 17;

		// calculate errrors
		error_x = p_p_b_x;
		error_y = p_p_b_y;


		// implement PID controller for x
		error_x_diff = p_p_b_x - p_p_b_x_last;
		error_x_diff /= delta;
		error_x_sum += error_x * delta;

		for (int i = MC - 1; i > 0; i--) {
			error_x_diff_m[i] = error_x_diff_m[i - 1];
		}
		error_x_diff_m[0] = error_x_diff;
		//error_x_diff = average(error_x_diff_m, mc);

		ap_fixed<22,12> ctrl_x_p = ap_fixed<22,12>(KP) * error_x;
		ap_fixed<22,12> ctrl_x_i = ap_fixed<22,12>(KI) * error_x_sum;
		ap_fixed<22,12> ctrl_x_d = ap_fixed<22,12>(KD) * error_x_diff;
		ap_fixed<22,12> ctrl_x = ctrl_x_p + ctrl_x_i + ctrl_x_d;


		// implement PID controller for y
		error_y_diff = p_p_b_y - p_p_b_y_last;
		error_y_diff /= delta;
		error_y_sum += error_y * delta;

		for (int i = MC - 1; i > 0; i--) {
			error_y_diff_m[i] = error_y_diff_m[i - 1];
		}
		error_y_diff_m[0] = error_y_diff;
		//error_y_diff = average(error_y_diff_m, mc);

		ap_fixed<22,12> ctrl_y_p = ap_fixed<22,12>(KP) * error_y;
		ap_fixed<22,12> ctrl_y_i = ap_fixed<22,12>(KI) * error_y_sum;
		ap_fixed<22,12> ctrl_y_d = ap_fixed<22,12>(KD) * error_y_diff;
		ap_fixed<22,12> ctrl_y = ctrl_y_p + ctrl_y_i + ctrl_y_d;


		// store last position
		p_p_b_x_last = p_p_b_x;
		p_p_b_y_last = p_p_b_y;


		// calculate plate rotation
		ap_ufixed<22,12> len = hls::sqrt(ap_ufixed<22,12>(ctrl_x * ctrl_x + ctrl_y * ctrl_y));

		ap_fixed<10,2> t_p2b_ra_x = -ctrl_y / len;
		ap_fixed<10,2> t_p2b_ra_y = ctrl_x / len;

		if (len > 100) {
			len = 100;
		}

		ap_uint<10> cmd_x, cmd_y;
		// ugly workaround for bit selection
		for (int i = 0; i < 10; i++) {
			cmd_x[i] = t_p2b_ra_x[i];
			cmd_y[i] = t_p2b_ra_y[i];
		}
		ap_uint<9> cmd_a = len;


		//MBOX_PUT(performance_perf, (ap_uint<8>("11", 16), ap_uint<24>(0)));
		for (int i = 0; i < 6; i++) {
			MBOX_PUT(inverse_cmd, (cmd_x, cmd_y, cmd_a, (ap_uint<3>)i));
		}

		// saw
		//ap_uint<32> saw[1] = { len };
		//uint32 tmp = saw[0].range(31,0);
		//rb_info += 36;


	}
}
