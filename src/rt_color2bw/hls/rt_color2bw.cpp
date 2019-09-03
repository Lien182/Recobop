/********************************************************************          
* rt_color2bw.cpp      -- hardware rgb2gr implementation            *
*                         transforms a color image to a black white * 
*						  by averaging								*
*																	* 
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_math.h"

#define INPUT_WIDTH 640
#define INPUT_HEIGHT 480
#define INPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4 * 4)
#define INPUT_PREFETCH_SIZE	  (INPUT_WIDTH * 4 * 2)
#define INPUT_LINESIZE (INPUT_WIDTH * 4)
#define OUTPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4)
#define OUTPUT_WIDTH 1680
#define OUTPUT_LINE_SIZE (OUTPUT_WIDTH * 4)



THREAD_ENTRY()
{
	#pragma HLS INTERFACE ap_none port=debug_port
	*debug_port |= 0;	

	int32 input_linebuffer[INPUT_LINEBUFFER_SIZE];
	int32 output_linebuffer[OUTPUT_LINEBUFFER_SIZE];;
	int32 i,k,j, ii, jj;
	int16 tmp;
	uint8 filter_pointer;
	uint32 rc_flag[1];
	uint32 fb[1];
	uint32 initdata;

	{
		#pragma HLS PROTOCOL fixed
		THREAD_INIT();
		initdata = GET_INIT_DATA();
		MEM_READ( initdata+4 ,  fb , 4);
	}

	
	while (1)
	{
		uint32 address;
		uint32 status = 0;

		{
			#pragma HLS PROTOCOL fixed
			MEM_READ( initdata,  rc_flag , 4);
			if(rc_flag[0] == 1)
			{	
				break;
			}
			ap_wait();
			address = MBOX_GET(video_cmd);
		}

		/*			
		if(address != 0)
		{
			#pragma HLS PROTOCOL fixed
			*debug_port |= (1<<2);
			ap_wait();
			*debug_port &= ~(1<<2);
			ap_wait();

		}
		*/

		MEM_READ( address, input_linebuffer, INPUT_PREFETCH_SIZE);

		address += (INPUT_WIDTH<<3); // <<3 = *2*4

		for(i = 0; i < (INPUT_HEIGHT); i++)
		{
			MEM_READ( address , &(input_linebuffer[INPUT_WIDTH* ((i+1)&3)]) , INPUT_LINESIZE );
			address += (INPUT_WIDTH<<2);

			for(j = 0; j < (INPUT_WIDTH); j++)
			{

				tmp= 0; 
				
				uint32_t buffer_pointer = ((INPUT_WIDTH*((i)&3)+(j)));
				uint32_t actindata  = 	input_linebuffer[buffer_pointer];	
				for(k = 0; k < 3; k++)
				{
					#pragma HLS unroll factor=3
					int32_t data = ((actindata >> 8*k) & 0x000000ff);
	
					tmp += data;

				}
				tmp /= 3;


				output_linebuffer[j] = (input_linebuffer[((INPUT_WIDTH*((i)&3)+(j)))] & 0xff000000) | (tmp & 0xff)   | ((tmp & 0xff) << 8) | ((tmp & 0xff) << 16);
			}
			
			MEM_WRITE( output_linebuffer , (fb[0] + i*OUTPUT_LINE_SIZE), INPUT_LINESIZE );

		}
		
		{
			#pragma HLS PROTOCOL fixed
			*debug_port |= (1<<3);
			ap_wait();
			*debug_port &= ~(1<<3);
			ap_wait();
		}
	}

	while(MBOX_TRYPUT(reconfiguration_v_request, 0xAAAAAAAA) != 1);
	stream_write(osif_hw2sw, OSIF_CMD_THREAD_EXIT);
	while(1);
}