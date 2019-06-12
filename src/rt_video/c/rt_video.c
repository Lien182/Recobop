#include "recobop.h"
#include "reconos.h"

#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"
#include "hdmi_input.h"
#include "hdmi_output.h"

#include <math.h>
#include <stdio.h>

/*
 
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
	}
}


*/


 
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
		
		for (i=y1; i<y2; i++) {
			for (j=x1; j<x2; j++) {
				for(k=0; k < 4; k++) // k==3 is the alpha channel
				{
					if(k == 3) 
					{
						image_out[(width*i+j)*4+k] = image_in[(x2*i+j)*4+k];
						break;
					}

					if(i == y1 || i == (y2-1) || j == x1 || j== (x2-1))
					{
						image_out[(width*i+j)*4+k] = 0;
					}
					else
					{
						tmp_x = 0;
						tmp_y = 0;
						for(ii=-1; ii < 2; ii++)
						{
							for(jj=-1; jj < 2; jj++)
							{
								tmp_x += image_in[(x2*(i+ii)+(j+jj))*4+k] * filter_x[(1+ii)*3+(1+jj)];
								tmp_y += image_in[(x2*(i+ii)+(j+jj))*4+k] * filter_y[(1+ii)*3+(1+jj)];
							}
						}
						
						image_out[(width*i+j)*4+k] = (abs(tmp_x) + abs(tmp_y)) >> 3;
					}
				}
			}
		}

		image_in =  (uint8_t*)MBOX_GET(video_cmd);

	}
}



