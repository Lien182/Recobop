
#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_math.h"

#define KP -0.0020
#define KD -0.0020

#define SIGMAINV 0.1
#define Q 4.0

#define ROUNDING AP_SAT_SYM

#define N 24
#define M 12


void kalman(ap_fixed<N,M,AP_RND_CONV, ROUNDING>* u, ap_fixed<N,M,AP_RND_CONV, ROUNDING>* y, ap_fixed<N,M,AP_RND_CONV, ROUNDING>* x, ap_fixed<N,M,AP_RND_CONV, ROUNDING> * P)
{
	int i;

	//Constants
	ap_fixed<10,2,AP_RND_CONV, ROUNDING> sigmainv = SIGMAINV;
	ap_fixed<10,5,AP_RND_CONV, ROUNDING> q  = Q;
	ap_fixed<16,8,AP_RND_CONV, ROUNDING> B  = 210.21;
	ap_fixed<10,2,AP_RND_CONV, ROUNDING> Ta = 0.03;


	ap_fixed<N, M, AP_RND_CONV, ROUNDING>  tmp;
	ap_fixed<N, M, AP_RND_CONV, ROUNDING>  Kk[4];
	ap_fixed<N, M, AP_RND_CONV, ROUNDING>  xk[4];
	ap_fixed<N, M, AP_RND_CONV, ROUNDING>  Pk[16];
	ap_fixed<N, M, AP_RND_CONV, ROUNDING>* Pp = P;
	ap_fixed<N, M, AP_RND_CONV, ROUNDING>* xp = x;

	//if(hls::fabsf(xp[0]) < 200 && hls::fabsf(xp[2]) < 200  )
	{
		//prediction
		xk[0] = xp[0] + Ta * xp[1];
		xk[1] = xp[1] - B  * u[0];
		xk[2] = xp[2] + Ta * xp[3];
		xk[3] = xp[3] - B  * u[1];
	}


	Pk[0]  = Pp[0] 	+ Ta * Pp[4]  + (Pp[1] + Pp[5] * Ta) * Ta + q;
	Pk[1]  = Pp[4] 	+ Pp[5] * Ta;
	Pk[2]  = Pp[8] 	+ Ta * Pp[12] + (Pp[9] + Pp[13] * Ta) * Ta;
	Pk[3]  = Pp[12] + Ta * Pp[13];
	Pk[4]  = Pp[1]  * Ta * Pp[5];
	Pk[5]  = Pp[5]  + q;
	Pk[6]  = Pp[9] 	+ Pp[13] * Ta;
	Pk[7]  = Pp[13];

	Pk[8]  = Pp[2]  + Ta * Pp[6] + (Pp[3] + Ta * Pp[7]) * Ta;
	Pk[9]  = Pp[6]  + Pp[7] * Ta;
	Pk[10] = Pp[10] + Ta * Pp[14] + (Pp[11] + Pp[15] * Ta) * Ta + q;
	Pk[11] = Pp[14] + Ta * Pp[15];
	Pk[12] = Pp[3]  * Ta * Pp[7];
	Pk[13] = Pp[7];
	Pk[14] = Pp[11] + Pp[15] * Ta;
	Pk[15] = Pp[15] + q;

	//Korrection
	for(int j = 0; j < 4; j++)
		xp[j] = xk[j];

	//Windowing of the measurement y
	
	//if ((fabsf(y[0] - x[0]) + fabsf(y[1] - x[2])) < 20)
	{
		for (i = 0; i < 2; i++)
		{

			for(int j = 0; j < 4;j++)
			{
				Kk[j] = Pk[(j<<2) +  2*i] * sigmainv;
			}
			tmp = (y[i] - xp[i * 2]);


			for(int j = 0; j < 4;j++)
			{
				xp[j] += Kk[j] * tmp;
			}

			for(int j = 0; j < 16; j++)
			{
				Pp[j] = Pk[j];
			}

			for(int j = 0; j < 16; j++)
			{
				Pp[j] -= Kk[j>>2] * Pk[(j%4) + 8 * i];
			}

			for(int j = 0; j < 16; j++)
			{
				Pk[j] = Pp[j];
			}
		}
	}
}



THREAD_ENTRY() {
	#pragma HLS INTERFACE ap_none port=debug_port

	ap_fixed<22,7,AP_RND_CONV, ROUNDING> rad2grad = 57.324840764;
	ap_fixed<12,1,AP_RND_CONV, ROUNDING> grad2rad = 0.017444444;
	ap_fixed<11,1,AP_RND_CONV, ROUNDING> inc2mm   = 116.67e-3;
	ap_fixed<10,5,AP_RND_CONV, ROUNDING> maxgrad  = 0.19199; //11.0; now rad
	ap_fixed<10,1,AP_RND_CONV, ROUNDING> delta    = 0.03;

	ap_fixed<N,M,AP_RND_CONV, ROUNDING> u[2] = {0.0f, 0.0f};
	ap_fixed<N,M,AP_RND_CONV, ROUNDING> y[2] = {0.0f, 0.0f};
	ap_fixed<N,M,AP_RND_CONV, ROUNDING> x[4]  = { 0.0f, 0.0f, 0.0f, 0.0f };

	ap_fixed<N,M,AP_RND_CONV, ROUNDING> P[16] = {  	1.0f, 0.0f, 0.0f, 0.0f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f,
													0.0f, 0.0f, 0.0f, 1.0f };
	
	ap_fixed<N,M,AP_RND_CONV, ROUNDING> error_x, error_x_diff, error_x_sum = 0, error_y, error_y_diff, error_y_sum = 0, p_p_b_x = 0, p_p_b_y = 0, error_x_last = 0, error_y_last = 0;


	uint32 thread_stack[20];

	uint32 rb_info;
	uint32 demonstrator_nr[1];
	uint32 stack_addr[1];
	{
		#pragma HLS PROTOCOL fixed
		THREAD_INIT();
		rb_info = GET_INIT_DATA();
		MEM_READ( rb_info + 8,  demonstrator_nr , 4);
		MEM_READ( rb_info + 16, stack_addr      , 4);
		*debug_port = 0;
	}

	while (1) {

		ap_uint<32> pos;
		switch(demonstrator_nr[0])
		{
			case 0: pos.range(31,0) = MBOX_GET(touch_0_pos); break;
			case 1: pos.range(31,0) = MBOX_GET(touch_1_pos); break;
			case 2: pos.range(31,0) = MBOX_GET(touch_2_pos); break;
			default: break;
		};
		if(demonstrator_nr[0] == 0)
		{
			#pragma HLS PROTOCOL fixed
			*debug_port |= (1<<2);
			ap_wait();
			*debug_port &= ~(1<<2);
			ap_wait();
		}	

		

		ap_int<12> pos_x = pos(23, 12);
		ap_int<12> pos_y = pos(11, 0);

		p_p_b_x = pos_x;
		p_p_b_y = pos_y;

		y[0] = p_p_b_x * inc2mm;
		y[1] = p_p_b_y * inc2mm;


		kalman(u,y,x,P);

		// calculate errors
		error_x = x[0];
		error_y = x[2];


		// implement PD controller for x
		error_x_diff = error_x - error_x_last;
		error_x_diff /= delta;


		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_x_p = ap_fixed<14,1>(KP) * error_x;
		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_x_d = ap_fixed<14,1>(KD) * error_x_diff;
		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_x = ctrl_x_p + ctrl_x_d;


		// implement PD controller for y
		error_y_diff = error_y - error_y_last;
		error_y_diff /= delta;


		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_y_p = ap_fixed<14,1>(KP) * error_y;
		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_y_d = ap_fixed<14,1>(KD) * error_y_diff;
		ap_fixed<N,M,AP_RND_CONV, ROUNDING> ctrl_y = ctrl_y_p + ctrl_y_d;


		// store last position
		error_x_last = error_x;
		error_y_last = error_y;


		
		u[0] = -ctrl_y;		
		u[1] =  ctrl_x;
		
		if (u[0] > maxgrad)
			u[0] = maxgrad;
		else if(u[0] < -maxgrad)
			u[0] = -maxgrad;

		
		if (u[1] > maxgrad)
			u[1] = maxgrad;
		else if(u[1] < -maxgrad)
			u[1] = -maxgrad;

		ap_int<14> cmd_x, cmd_y;


		cmd_x = (u[0] * rad2grad) << 6;
		cmd_y = (u[1] * rad2grad) << 6;


		if(demonstrator_nr[0] == 0)
		{
			#pragma HLS PROTOCOL fixed
			*debug_port |= (1<<3);
			ap_wait();
			*debug_port &= ~(1<<3);
			ap_wait();

		}
		switch(demonstrator_nr[0])
		{
			case 0: for (int i = 0; i < 6; i++) MBOX_PUT(inverse_0_cmd, (cmd_x, cmd_y, (ap_uint<3>)i)); break;
			case 1: for (int i = 0; i < 6; i++) MBOX_PUT(inverse_1_cmd, (cmd_x, cmd_y, (ap_uint<3>)i)); break;
			case 2: for (int i = 0; i < 6; i++) MBOX_PUT(inverse_2_cmd, (cmd_x, cmd_y, (ap_uint<3>)i)); break;
			default: break;
		}


		/*

		//marshalling
		for(int i = 0; i < 4; i++)
			thread_stack[i] =  x[i].range(19,0);
		for(int i = 0; i < 16; i++)
			thread_stack[i+4] = P[i].range(19,0);

		MEM_WRITE(thread_stack,stack_addr[0],20*4);

		*/

	
		if(demonstrator_nr[0] == 0)
		{
			#pragma HLS PROTOCOL fixed
			*debug_port |= (1<<4);
			ap_wait();
			*debug_port &= ~(1<<4);
			ap_wait();

		}
	}
}
