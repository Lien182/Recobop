/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Application library
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Auto-generated application specific header file
 *                 including definitions of all resources and functions
 *                 to instantiate resources and threads automatically.
 *
 * ======================================================================
 */



#include "reconos_app.h"

#include "reconos.h"
#include "utils.h"

/* == Application resources ============================================ */

/*
 * @see header
 */
struct mbox servo_cmd_s;
struct mbox *servo_cmd = &servo_cmd_s;

struct mbox touch_pos_s;
struct mbox *touch_pos = &touch_pos_s;

struct mbox inverse_cmd_s;
struct mbox *inverse_cmd = &inverse_cmd_s;

struct mbox performance_perf_s;
struct mbox *performance_perf = &performance_perf_s;









struct reconos_resource servo_cmd_res = {
	.ptr = &servo_cmd_s,
	.type = RECONOS_RESOURCE_TYPE_MBOX
};

struct reconos_resource touch_pos_res = {
	.ptr = &touch_pos_s,
	.type = RECONOS_RESOURCE_TYPE_MBOX
};

struct reconos_resource inverse_cmd_res = {
	.ptr = &inverse_cmd_s,
	.type = RECONOS_RESOURCE_TYPE_MBOX
};

struct reconos_resource performance_perf_res = {
	.ptr = &performance_perf_s,
	.type = RECONOS_RESOURCE_TYPE_MBOX
};




/* == Application functions ============================================ */

/*
 * @see header
 */
void reconos_app_init() {
		mbox_init(servo_cmd, 8);
		mbox_init(touch_pos, 8);
		mbox_init(inverse_cmd, 8);
		mbox_init(performance_perf, 8);
	

	

	

	
}

/*
 * @see header
 */
void reconos_app_cleanup() {
		mbox_destroy(servo_cmd);
		mbox_destroy(touch_pos);
		mbox_destroy(inverse_cmd);
		mbox_destroy(performance_perf);
	

	

	

	
}

/*
 * Empty software thread if no software specified
 *
 *   data - pointer to ReconOS thread
 */
void *swt_idle(void *data) {
	pthread_exit(0);
}

struct reconos_resource *resources_servo[] = {&servo_cmd_res,&performance_perf_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_servo() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {0};
	reconos_thread_init(rt, "servo", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_servo, 2);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}




/*
 * @see header
 */
void reconos_thread_destroy_servo(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_touch[] = {&touch_pos_res,&performance_perf_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_touch() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {1};
	reconos_thread_init(rt, "touch", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_touch, 2);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}




/*
 * @see header
 */
void reconos_thread_destroy_touch(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_vga[] = {&touch_pos_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_vga() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {2};
	reconos_thread_init(rt, "vga", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_vga, 1);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}




/*
 * @see header
 */
void reconos_thread_destroy_vga(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_control[] = {&touch_pos_res,&inverse_cmd_res,&performance_perf_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_control() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {3};
	reconos_thread_init(rt, "control", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_control, 3);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}


extern void *rt_control(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_control() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {3};
	reconos_thread_init(rt, "control", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_control, 3);
	reconos_thread_setswentry(rt, rt_control);
	reconos_thread_create_auto(rt, RECONOS_THREAD_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_control(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_inverse[] = {&servo_cmd_res,&inverse_cmd_res,&performance_perf_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_inverse() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {5,6,7,8,9};
	reconos_thread_init(rt, "inverse", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 5);
	reconos_thread_setresourcepointers(rt, resources_inverse, 3);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}


extern void *rt_inverse(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_inverse() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {5,6,7,8,9};
	reconos_thread_init(rt, "inverse", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 5);
	reconos_thread_setresourcepointers(rt, resources_inverse, 3);
	reconos_thread_setswentry(rt, rt_inverse);
	reconos_thread_create_auto(rt, RECONOS_THREAD_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_inverse(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_performance[] = {&performance_perf_res};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_performance() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {4};
	reconos_thread_init(rt, "performance", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 1);
	reconos_thread_setresourcepointers(rt, resources_performance, 1);
	reconos_thread_create_auto(rt, RECONOS_THREAD_HW);

	return rt;
}




/*
 * @see header
 */
void reconos_thread_destroy_performance(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_power[] = {};



extern void *rt_power(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_power() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {};
	reconos_thread_init(rt, "power", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 0);
	reconos_thread_setresourcepointers(rt, resources_power, 0);
	reconos_thread_setswentry(rt, rt_power);
	reconos_thread_create_auto(rt, RECONOS_THREAD_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_power(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_saw[] = {};



extern void *rt_saw(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_saw() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {};
	reconos_thread_init(rt, "saw", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 0);
	reconos_thread_setresourcepointers(rt, resources_saw, 0);
	reconos_thread_setswentry(rt, rt_saw);
	reconos_thread_create_auto(rt, RECONOS_THREAD_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_saw(struct reconos_thread *rt) {
	// not implemented yet
}

struct reconos_resource *resources_web[] = {};



extern void *rt_web(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_web() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	int slots[] = {};
	reconos_thread_init(rt, "web", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setallowedslots(rt, slots, 0);
	reconos_thread_setresourcepointers(rt, resources_web, 0);
	reconos_thread_setswentry(rt, rt_web);
	reconos_thread_create_auto(rt, RECONOS_THREAD_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_web(struct reconos_thread *rt) {
	// not implemented yet
}



/*
 * @see header
 */
int reconos_clock_system_set(int f)
{
	return reconos_clock_set(0, 8, f);
}


