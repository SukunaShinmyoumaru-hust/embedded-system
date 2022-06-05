#include <sys/mman.h>
#include <linux/fb.h>
#include <stdio.h>

#include "../common/common.h"

#define RED		FB_COLOR(255,0,0)
#define ORANGE	FB_COLOR(255,165,0)
#define YELLOW	FB_COLOR(255,255,0)
#define GREEN	FB_COLOR(0,255,0)
#define CYAN	FB_COLOR(0,127,255)
#define BLUE	FB_COLOR(0,0,255)
#define PURPLE	FB_COLOR(139,0,255)
#define WHITE   FB_COLOR(255,255,255)
#define BLACK   FB_COLOR(0,0,0)

int color[9] = {RED,ORANGE,YELLOW,GREEN,CYAN,BLUE,PURPLE,WHITE,BLACK};

int main(int argc, char* argv[])
{
	int row,column,i;
	int32_t start, end;

	fb_init("/dev/fb0");
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,BLACK);
	fb_update();
	printf("\n========== Start Test ===========\n");

	sleep(1);
	start = task_get_time();
	for(row=-5; row<SCREEN_HEIGHT; row+=2)
	{
		for(column=-5; column<SCREEN_WIDTH; column+=2)
		{
			fb_draw_pixel(column,row,YELLOW);
		}
	}
	end = task_get_time();
	fb_update();
	printf("draw pixel: %d ms\n",end - start);

	sleep(1);
	start = task_get_time();
	for(row=-35,i=0; row<SCREEN_HEIGHT; row+=35)
	{
		for(column=-25; column<SCREEN_WIDTH; column+=25)
		{
			fb_draw_rect(column,row,20,20,color[++i%9]);
		}
	}
	end = task_get_time();
	fb_update();
	printf("draw rect: %d ms\n",end - start);

	sleep(1);
	fb_draw_rect(150,200,400,200,BLACK);
	fb_update();
	start = task_get_time();
	for(row=0;row<=400;row+=20){
		fb_draw_line(SCREEN_WIDTH/2-300,SCREEN_HEIGHT/2-200+row,SCREEN_WIDTH/2+300,SCREEN_HEIGHT/2+200-row,color[1]);
	}
	for(column=0;column<=400;column+=20){
		fb_draw_line(SCREEN_WIDTH/2-200+column,SCREEN_HEIGHT/2-200,SCREEN_WIDTH/2+200-column,SCREEN_HEIGHT/2+200,color[2]);
	}
	end = task_get_time();
	fb_update();
	printf("draw line: %d ms\n", end - start);
	return 0;
}
