#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_math.h"

#define INPUT_WIDTH 640
#define INPUT_HEIGHT 480
#define INPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4 * 4)
#define INPUT_PREFETCH_SIZE	  (INPUT_WIDTH * 4 * 3)
#define INPUT_LINESIZE (INPUT_WIDTH * 4)
#define OUTPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4)
#define OUTPUT_WIDTH 1680
#define OUTPUT_LINE_SIZE (OUTPUT_WIDTH * 4)

const int filter_x[] = { 1,  2,  1,  0,  0,  0, -1, -2, -1};
const int filter_y[] = { 1,  0, -1,  2,  0, -2,  1,  0, -1};

THREAD_ENTRY()
{
	#pragma HLS INTERFACE ap_none port=debug_port
	*debug_port |= 1;
	THREAD_INIT();

	uint32 fb = GET_INIT_DATA();

	RAM(int8,INPUT_LINEBUFFER_SIZE, input_linebuffer);
	RAM(int8,OUTPUT_LINEBUFFER_SIZE,output_linebuffer);
	int32 i,k,j, ii, jj;
	int16 tmp_x, tmp_y;
	uint8 filter_pointer;

	while (1)
	{
		uint32 adress = MBOX_GET(video_cmd); //adress in BYTE
		adress <<=2;
		MEM_READ( adress, input_linebuffer, INPUT_PREFETCH_SIZE);
		adress += (INPUT_WIDTH<<3); // <<3 = *2*4

		for(i = 1; i < (INPUT_HEIGHT-1); i++)
		{
			MEM_READ( adress , &(input_linebuffer[INPUT_LINESIZE* ((i+1)&3)]) , INPUT_LINESIZE );
			adress += (INPUT_WIDTH<<2);

			for(j = 1; j < (INPUT_WIDTH-1); j++)
			{
				for(k = 0; k < 3; k++)
				{
					tmp_x = 0; tmp_y = 0;
					filter_pointer = 0;
					for(ii=-1; ii < 2; ii++)
					{
						for(jj=-1; jj < 2; jj++)
						{
							uint32_t buffer_pointer = ((INPUT_WIDTH*((i+ii)&3)+(j+jj))<<2)+k;
							tmp_x += input_linebuffer[buffer_pointer] * filter_x[filter_pointer];
							tmp_y += input_linebuffer[buffer_pointer] * filter_y[filter_pointer];
							filter_pointer++;
						}
					}
					output_linebuffer[(j<<2)+k] = (uint8_t)((abs(tmp_x) + abs(tmp_y)) >> 3);
				}
				output_linebuffer[(j<<2)+3] = input_linebuffer[(j<<2)+3];
			}

			MEM_WRITE( output_linebuffer , (fb +i* OUTPUT_LINE_SIZE), INPUT_LINESIZE );

		}
	}
}
