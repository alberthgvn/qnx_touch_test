/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen/screen.h>

/* RBG color, picked by other tool */
#define TOUCH_TEST_RGB_BACKGROUND       0xFFFFFF
/* back ground color */
#define TOUCH_TEST_BACKGROUND_COLOR     0xFF000000|TOUCH_TEST_RGB_BACKGROUND 
/* RGB color for brush line */     
#define TOUCH_TEST_RBG_BRUSH            0xA53211
/* parameter for brush color */
#define TOUCH_TEST_BRUSH_COLOR          0xFF000000|TOUCH_TEST_RBG_BRUSH
/* brush size */
#define TOUCH_TEST_BRUSH_WIDTH          8
#define TOUCH_TEST_BRUSH_HEIGHT         8
#define NUM_CONTINUOUS_LOOP				1

struct area {
	int x;
	int y;
	int w;
	int h;
};
/* Define 4 rectangles to be detected for erase screen buffer */
#define DETECT_RECT_SIZE_W	30
#define DETECT_RECT_SIZE_H	30

int CROSS[] = {
	SCREEN_BLIT_SOURCE_WIDTH, DETECT_RECT_SIZE_W,
	SCREEN_BLIT_SOURCE_HEIGHT, DETECT_RECT_SIZE_H,
	SCREEN_BLIT_DESTINATION_X, 0,
	SCREEN_BLIT_DESTINATION_Y, 0,
	SCREEN_BLIT_DESTINATION_WIDTH, DETECT_RECT_SIZE_W,
	SCREEN_BLIT_DESTINATION_HEIGHT, DETECT_RECT_SIZE_H,
	SCREEN_BLIT_TRANSPARENCY, SCREEN_TRANSPARENCY_SOURCE_OVER,
	SCREEN_BLIT_END
};

/* top-left rectangle */
struct area DETECT_TOP_LEFT 	= 	{0, 0, 
									DETECT_RECT_SIZE_W, DETECT_RECT_SIZE_H};
/* top-right rectangle */									
struct area DETECT_TOP_RIGHT	= 	{800-DETECT_RECT_SIZE_W, 0, 
									DETECT_RECT_SIZE_W, DETECT_RECT_SIZE_H};
/* bottom-left rectangle */									
struct area DETECT_BOT_LEFT 	= 	{0, 480-DETECT_RECT_SIZE_H, 
									DETECT_RECT_SIZE_W, DETECT_RECT_SIZE_H};
/* bottom-right rectangle */									
struct area DETECT_BOT_RIGHT 	= 	{800-DETECT_RECT_SIZE_W, 480-DETECT_RECT_SIZE_H, 
									DETECT_RECT_SIZE_W, DETECT_RECT_SIZE_H};

struct detect_condition {
	struct area *rect;
	int got_touch;
};

struct detect_condition CONDITION_LIST [] =
{
	{&DETECT_TOP_LEFT, 0},
//	{&DETECT_TOP_RIGHT, 0},
//	{&DETECT_BOT_LEFT, 0},
//	{&DETECT_BOT_RIGHT, 0},
};

int display_idx = 0;

/* check all conditions with AND relation */
int is_condition_matched (int x, int y)
{
	int ret = 1;
	int i;
	struct detect_condition *cond = CONDITION_LIST;
	
	int num_condition = sizeof (CONDITION_LIST)/sizeof (*cond);
	for (i=0;i<num_condition;i++)
	{
		if ( x >= cond[i].rect->x && x <= (cond[i].rect->x + cond[i].rect->w) &&
			 y >= cond[i].rect->y && y <= (cond[i].rect->y + cond[i].rect->h) )
			cond[i].got_touch = 1;
		ret = ret * cond[i].got_touch;
	}
	if (ret)
	{
		for (i=0;i<num_condition;i++)
			cond[i].got_touch = 0;
	}
	return ret;
}

void draw_cross_button (screen_context_t ctx, screen_buffer_t *pbuf)
{
	int i,j;
	screen_pixmap_t pix;
	
	/* create pixmap */
	screen_create_pixmap(&pix, ctx);
	
	/* set screen pxmap format */
	int format = SCREEN_FORMAT_RGBA8888;
	screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_FORMAT, &format);
	/* set screen pixmap properties */
	int usage = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE;
	screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_USAGE, &usage);

	int size[2] = { DETECT_RECT_SIZE_W, DETECT_RECT_SIZE_H };
	screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_BUFFER_SIZE, size);
	screen_create_pixmap_buffer(pix);
	screen_get_pixmap_property_pv(pix, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)pbuf);

	unsigned char *ptr = NULL;
	screen_get_buffer_property_pv(*pbuf, SCREEN_PROPERTY_POINTER, (void **)&ptr);
	int stride = 0;
	screen_get_buffer_property_iv(*pbuf, SCREEN_PROPERTY_STRIDE, &stride);
	/* draw rectangle */ 
	int draw_condition = 0;
	for (i = 0; i < size[1]; i++, ptr += stride) {
			for (j = 0; j < size[0]; j++) {
				ptr[j*4] = 0x00;
				ptr[j*4+1] = 0x00;
				ptr[j*4+2] = 0xff;
				draw_condition = (i == 0) || 
								 (i == (DETECT_RECT_SIZE_H-1)) ||
								 (j == 0) ||
								 (j == (DETECT_RECT_SIZE_W - 1)) ||
								 (i == j) ||
								 ((DETECT_RECT_SIZE_W*i + DETECT_RECT_SIZE_H*j) == 
										(DETECT_RECT_SIZE_H*DETECT_RECT_SIZE_W));
				ptr[j*4+3] = draw_condition? 0xff : 0;
			}
	}
}

void print_eventsps (void)
{
	static struct timespec to;
	static uint64_t t, last_t, delta;
	static int events = 0;

	clock_gettime(CLOCK_REALTIME, &to);
	t = timespec2nsec(&to);

	if (events == 0) {
		last_t = t;
	} else {
		delta = t - last_t;
		if (delta >= 1000000000LL) {
			fprintf(stderr, "Screen %d: %d events in %6.3f seconds = %6.3f EPS\n",
				display_idx, events, 0.000000001f * delta, 1000000000.0f * events / delta);
			events = -1;
		}
	}

	events++;	
}

void fill_x_y (screen_context_t ctx, screen_buffer_t buf, int *rect, int x, int y)
{
	/* init brush parameters */
	int dot[] = 
	{
		SCREEN_BLIT_COLOR, TOUCH_TEST_BRUSH_COLOR,
		SCREEN_BLIT_DESTINATION_X, x,
		SCREEN_BLIT_DESTINATION_Y, y,
		SCREEN_BLIT_DESTINATION_HEIGHT,TOUCH_TEST_BRUSH_HEIGHT,
		SCREEN_BLIT_DESTINATION_WIDTH, TOUCH_TEST_BRUSH_WIDTH,
		SCREEN_BLIT_END
	};
	
	/* fill to screen */
	screen_fill(ctx, buf, dot);
}
/******************************************************************************
** Function Name: main(int argc, char **argv)
** Description  : The main function of the application
** Parameter    : argc - number of arguments
**                argv - argument pointer
** Return       : EXIT_SUCCESS - if application excute successful
**                EXIT_FAILURE - if application excute failure
** Note         : 
******************************************************************************/
int main(int argc, char **argv)
{	
	screen_display_t screen_disp;
	screen_display_t *screen_dlist;	
	int event_type;
	screen_buffer_t buffer;
	screen_event_t event;
	screen_context_t ctx;
	screen_window_t win;
	screen_buffer_t cross_pbuf;
	int ndisplays = 0;	
	char *display = "1";
	int i, j, window_update=0, dbg_eventsps = 0;
	int usage;
	int format = SCREEN_FORMAT_RGBA8888;
	int pos[2] = {0,0};
	int rect[4] = { 0, 0 };
	
	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-display=", strlen("-display=")) == 0) {
			display = argv[i] + strlen("-display=");
		} else if (strncmp(argv[i], "-eventsps", strlen("-eventsps")) == 0) {
			dbg_eventsps = 1;
		} else {
			break;
		}
	}
	
	/* create screen context */
	screen_create_context(&ctx, SCREEN_APPLICATION_CONTEXT);
	screen_get_context_property_iv(ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &ndisplays);
	screen_dlist = calloc(ndisplays, sizeof(*screen_dlist));
	screen_get_context_property_pv(ctx, SCREEN_PROPERTY_DISPLAYS, (void **)screen_dlist);
	if (isdigit(*display)) {
		int want_id = atoi(display);
		display_idx = want_id - 1;
		for (j = 0; j < ndisplays; ++j) {
			int actual_id = 0;  // invalid
			(void)screen_get_display_property_iv(screen_dlist[j],
					SCREEN_PROPERTY_ID, &actual_id);
			if (want_id == actual_id) {
				break;
			}
		}
	}
	else
	{
		printf ("Invalid display %s\n", display);
		free(screen_dlist);
		return -1;
	}
	if (j >= ndisplays) {
		fprintf(stderr, "couldn't find display %s\n", display);
		free(screen_dlist);
		return -1;
	}
	
	screen_disp = screen_dlist[j];
	free(screen_dlist);	
	
	/* create screen window */
	screen_create_window(&win, ctx);
	screen_set_window_property_pv(win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp);

	/* set usage for window */
	usage = SCREEN_USAGE_NATIVE;
	screen_set_window_property_iv(win, SCREEN_PROPERTY_USAGE, &usage);
  
	/* get max display rectangle */
	screen_create_window_buffers(win, 1);
	screen_get_window_property_iv(win, SCREEN_PROPERTY_BUFFER_SIZE, rect+2);
	
	/* create touch event */
	screen_create_event(&event);
	
	/* Set buffer as render buffer */
	screen_get_window_property_pv(win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&buffer);
	
	/* Draw cross button */
	draw_cross_button (ctx, &cross_pbuf);
	
	/* Create a full white background */
	int backgrnd[] = { SCREEN_BLIT_COLOR,TOUCH_TEST_BACKGROUND_COLOR, SCREEN_BLIT_END };
	screen_fill(ctx, buffer, backgrnd);
	screen_blit(ctx, buffer, cross_pbuf, CROSS);	
	screen_post_window(win, buffer, 1, rect, 0);
	
	/* look for events */
	while (1) 
	{	
		for (i = 0 ; i < NUM_CONTINUOUS_LOOP ; i++)
		{
			/* get event without wait */
			screen_get_event(ctx, event, 0);
			screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, (int*)&event_type);
			/* event touch or move occur */
			if ((event_type == SCREEN_EVENT_MTOUCH_TOUCH) ||
				(event_type == SCREEN_EVENT_MTOUCH_MOVE))
			{
				screen_get_event_property_iv(event, SCREEN_PROPERTY_POSITION, (int*)&pos);		
				//screen_flush_context(ctx, SCREEN_DONT_FLUSH);
				fill_x_y (ctx, buffer, rect, pos[0], pos[1]);
				window_update = 1;
				/* check if reset condition is satisfied */
				if (is_condition_matched (pos[0], pos[1]))
				{
					screen_fill(ctx, buffer, backgrnd);
					screen_blit(ctx, buffer, cross_pbuf, CROSS);
					screen_post_window(win, buffer, 1, rect, 0);
					/* already updated */ 
					window_update = 0;					
				}
				/* print events per second */
				if (dbg_eventsps)
					print_eventsps();
			}    			
		}
		/* require update to framebuffer if needed */
		if (window_update)
		{
			screen_post_window(win, buffer, 1, rect, 0);
			window_update = 0;
		}	
	}
	
	return EXIT_SUCCESS;
}