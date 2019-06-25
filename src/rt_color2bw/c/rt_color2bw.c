#include "recobop.h"
#include "reconos.h"

#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"
#include "hdmi_input.h"
#include "hdmi_output.h"

#include <math.h>
#include <stdio.h>


#define INPUT_WIDTH 640
#define INPUT_HEIGHT 480
#define INPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4 * 4)
#define INPUT_PREFETCH_SIZE	  (INPUT_WIDTH * 4 * 2)
#define INPUT_LINESIZE (INPUT_WIDTH * 4)
#define OUTPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4)
#define OUTPUT_WIDTH 1680
#define OUTPUT_LINE_SIZE (OUTPUT_WIDTH * 4)

#define MEM_READ_L( adr, dest, length ) { memcpy( dest, (void*)adr,  length); }
#define MEM_WRITE_L( src, adr, length ) { memcpy((void*)adr, src, length); }

THREAD_ENTRY()
{
	THREAD_INIT();

	
	struct reconos_thread * rt = GET_INIT_DATA();

	uint32_t fb = (uint32_t)rt->init_data;

	//uint32_t fb = (uint32_t)GET_INIT_DATA();

	int32_t input_linebuffer[INPUT_LINEBUFFER_SIZE];
	int32_t output_linebuffer[OUTPUT_LINEBUFFER_SIZE];
	int32_t i,k,j;
	int32_t tmp;

	while (1)
	{
		uint32_t address = MBOX_GET(video_cmd); //address in BYTE
		MEM_READ_L( address, input_linebuffer, INPUT_PREFETCH_SIZE);

		address += (INPUT_WIDTH<<3); // <<3 = *2*4

		for(i = 1; i < (INPUT_HEIGHT-1); i++)
		{
			MEM_READ_L( address , &(input_linebuffer[INPUT_WIDTH* ((i+1)&3)]) , INPUT_LINESIZE );
			address += (INPUT_WIDTH<<2);

			for(j = 1; j < (INPUT_WIDTH-1); j++)
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
			
			MEM_WRITE_L( output_linebuffer , (fb + i*OUTPUT_LINE_SIZE), INPUT_LINESIZE );
		}
	}
}
