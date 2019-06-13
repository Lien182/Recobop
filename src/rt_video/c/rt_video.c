#include "recobop.h"
#include "reconos.h"

#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"
#include "hdmi_input.h"
#include "hdmi_output.h"

#include <math.h>
#include <stdio.h>

#define SOBEL_NEW
//#define SOBEL


#ifdef COPY
 
THREAD_ENTRY() {
	t_video_info * video_info;
	struct reconos_thread * rt;
	t_hdmi_output * hdmi_output;

	int i =0, j = 0;

	uint32_t * image_in;
	uint32_t * image_out; 
	uint32_t width, height;

	THREAD_INIT();
	rt = (struct reconos_thread *)GET_INIT_DATA();
	video_info = (t_video_info*)rt->init_data;
	
	hdmi_output = &(video_info->hdmi_output);


	

	width  = hdmi_output->width;
	height = hdmi_output->height;

	while (1) 
	{
		image_in = (uint32_t*)MBOX_GET(video_cmd);
		image_out = (uint32_t*)hdmi_output->image;

		a9timer_caputure(a9timer, &(log_sw_video.a9timer_capture), A9TIMER_CAPTURE_START); 
		
		for(i = 0; i < 480; i++)
        {
			for(j = 0; j < 640; j++)
			{

				*image_out = *image_in;
				
				image_out++;
				image_in++;

			}
			image_out+=(width-640);
		}

		a9timer_caputure(a9timer, &(log_sw_video.a9timer_capture), A9TIMER_CAPTURE_STOP);
	}
}


#endif


#ifdef SOBEL
 
THREAD_ENTRY() {
	t_video_info * video_info;
	struct reconos_thread * rt;
	t_hdmi_output * hdmi_output;

	int filter_x[] = { 1,  2,  1,  0,  0,  0, -1, -2, -1};
	int filter_y[] = { 1,  0, -1,  2,  0, -2,  1,  0, -1};

	int i =0, j = 0, ii = 0, jj = 0 ,k = 0;

	int y1 = 0;
	int y2 = 480;
	int x1 = 0;
	int x2 = 640;

	uint8_t * image_in;
	uint8_t * image_out; 
	uint32_t width, height;


	int tmp_x, tmp_y;


	THREAD_INIT();
	rt = (struct reconos_thread *)GET_INIT_DATA();
	video_info = (t_video_info*)rt->init_data;
	
	hdmi_output = &(video_info->hdmi_output);


	

	width  = hdmi_output->width;
	height = hdmi_output->height;

	while (1) 
	{
		image_in =  (uint8_t*)MBOX_GET(video_cmd);
		image_out = (uint8_t*)hdmi_output->image;
		a9timer_caputure(a9timer, &(log_sw_video.a9timer_capture), A9TIMER_CAPTURE_START); 
		
		for (i=y1; i<y2; i++) {
		for (j=x1; j<x2; j++) {
			if(i == y1 || i == (y2-1) || j == x1 || j== (x2-1))
			{
				((uint32_t*)image_out)[(width*i+j)] = 0;
			}
			else
			{
				image_out[((width*i+j)<<2)+3] = image_in[((x2*i+j)<<2)+3];// k==3 is the alpha channel
					
				/* code */
				for(k=0; k < 3; k++) 
				{
						tmp_x = 0;
						tmp_y = 0;
						for(ii=-1; ii < 2; ii++)
						{
							for(jj=-1; jj < 2; jj++)
							{
								tmp_x += image_in[((x2*(i+ii)+(j+jj))<<2)+k] * filter_x[(1+ii)*3+(1+jj)];
								tmp_y += image_in[((x2*(i+ii)+(j+jj))<<2)+k] * filter_y[(1+ii)*3+(1+jj)];
							}
						}
						
						image_out[((width*i+j)<<2)+k] = (abs(tmp_x) + abs(tmp_y)) >> 3;
				}
				
			}
			
			
		}
	}

		a9timer_caputure(a9timer, &(log_sw_video.a9timer_capture), A9TIMER_CAPTURE_STOP); 
	}
}

#endif

#ifdef SOBEL_NEW


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


const int filter_x[] = { 1,  2,  1,  0,  0,  0, -1, -2, -1};
const int filter_y[] = { 1,  0, -1,  2,  0, -2,  1,  0, -1};

//const int filter_x[] = { 0,  0,  0,  0,  4,  0,  0,  0,  0};
//const int filter_y[] = { 0,  0,  0,  0,  4,  0,  0,  0,  0};

THREAD_ENTRY()
{
	THREAD_INIT();

	uint32_t fb = (uint32_t)GET_INIT_DATA();

	int8_t input_linebuffer[INPUT_LINEBUFFER_SIZE];
	int8_t output_linebuffer[OUTPUT_LINEBUFFER_SIZE];
	int32_t i,k,j, ii, jj;
	int16_t tmp_x, tmp_y;
	uint8_t filter_pointer;

	while (1)
	{
		uint32_t adress = MBOX_GET(video_cmd);
		
		MEM_READ_L( adress, input_linebuffer, INPUT_PREFETCH_SIZE);
		adress += (INPUT_WIDTH<<3);

		for(i = 1; i < (INPUT_HEIGHT-1); i++)
		{
			MEM_READ_L( adress , &(input_linebuffer[INPUT_LINESIZE* ((i+1)&3)]) , INPUT_LINESIZE );
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
			
			MEM_WRITE_L( output_linebuffer , (fb + i* OUTPUT_LINE_SIZE), INPUT_LINESIZE );
			
		}
		


		/*small test 
		memset(output_linebuffer, 0xff , OUTPUT_LINEBUFFER_SIZE);

		for(j = 0; j < INPUT_HEIGHT; j++)
		{
			//memcpy(&(fb[j* OUTPUT_LINE_SIZE]), output_linebuffer,OUTPUT_LINEBUFFER_SIZE );
			MEM_WRITE_L(&(fb[j* OUTPUT_LINE_SIZE]),output_linebuffer,OUTPUT_LINEBUFFER_SIZE );
		}
		*/


	}
}

#endif

