#include "hdmi_input.h"
#include <errno.h>
#include "reconos.h"
#include "reconos_calls.h"


#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void   *start;
	size_t  length;
};

struct buffer          *buffers;
static unsigned int     n_buffers;

static int xioctl(int fh, unsigned long int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static void init_mmap(t_hdmi_input * hdmi_input)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(hdmi_input->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "does not support "
				 "memory mapping\n");
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory\n");
		exit(EXIT_FAILURE);
	}

	buffers = calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(hdmi_input->fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap(NULL /* start anywhere */,
			      buf.length,
			      PROT_READ | PROT_WRITE /* required */,
			      MAP_SHARED /* recommended */,
			      hdmi_input->fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}



void hdmi_input_buffer_thread(t_hdmi_input * hdmi_input)
{

	struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        while(1)
        {
            LOOP:
            if (-1 == xioctl(hdmi_input->fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                case EAGAIN:
                        usleep(10);
                        goto LOOP;

                case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }


            MBOX_PUT(hdmi_input->mb, buffers[buf.index].start);
            
            if (-1 == xioctl(hdmi_input->fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
        }

}



uint32_t hdmi_input_init(t_hdmi_input * hdmi_input, char * device , struct mbox *buffer_pointer )
{

	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;

    hdmi_input->mb = buffer_pointer;

	hdmi_input->fd = open(device, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == hdmi_input->fd) 
    {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", device, errno, strerror(errno));
		return -1;
	}

	if (-1 == xioctl(hdmi_input->fd, VIDIOC_QUERYCAP, &cap)) 
    {
        fprintf(stderr, "%s is no V4L2 device\n", device);
        return -2;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",device);
		return -3;
	}

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf(stderr, "%s does not support streaming i/o\n",  device);
            return -3;
    }



	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(hdmi_input->fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(hdmi_input->fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	} else {
		/* Errors ignored. */
	}



	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
	fmt.fmt.pix.field       = V4L2_FIELD_TOP;//V4L2_FIELD_INTERLACED;

	if (-1 == xioctl(hdmi_input->fd, VIDIOC_S_FMT, &fmt))
		errno_exit("VIDIOC_S_FMT");


	printf("Input screen : width: %d , height: %d\n", fmt.fmt.pix.width, fmt.fmt.pix.height );

	init_mmap(hdmi_input);



	unsigned int i;
	enum v4l2_buf_type type;

	struct v4l2_bt_timings timing;


	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(hdmi_input->fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(hdmi_input->fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

	if (-1 == xioctl(hdmi_input->fd, VIDIOC_QUERY_DV_TIMINGS, &timing))
			errno_exit("VIDIOC_QUERY_DV_TIMINGS");


	printf("width: %d, height: %d \n", timing.width, timing.height);


    if(pthread_create(&(hdmi_input->thread), 0, &hdmi_input_buffer_thread, (void*)hdmi_input) != 0)
	{
		printf("[HDMI output] Error during check thread starting \n");
	}


    return 0;
}


