#include <stdio.h>
#include "../common/common.h"
#define R 5
#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)
static int touch_fd;
static int colors[5] = {FB_COLOR(255,0,0),FB_COLOR(0,255,0),FB_COLOR(0,0,255),0x66ccff,0x0};
static int B = 0;
static int lastx[5];
static int lasty[5];
static void a_better_move(int x1, int y1, int x2, int y2,int color)
{
	printf("x:%d y:%d to x:%d y : %d",x1 * SCREEN_WIDTH / 4000,y1 * SCREEN_HEIGHT / 4000,x2 * SCREEN_WIDTH / 4000,y2 * SCREEN_HEIGHT / 4000);
	if(x1 == x2){
		int i = y1 * SCREEN_HEIGHT / 4000;
		for(;i < y2 * SCREEN_HEIGHT / 4000;i++){
			fb_draw_circle(x1 * SCREEN_WIDTH / 4000,i,R,color);
		}
		i = y2 * SCREEN_HEIGHT / 4000;
		for(;i < y1 * SCREEN_HEIGHT / 4000;i++){
			fb_draw_circle(x1 * SCREEN_WIDTH / 4000,i,R,color);
		}
	}
	else{
		double i = x1;
		double stepx = ((double)(x2 - x1))/10;
		double stepy = ((double)(y2 - y1))/10;
		if((y2 - y1  > 10 || y1 - y2 > 10) && (x2 - x1 < 10 || x1 - x2 < 10) ){
			printf("G");
			int i = y1 * SCREEN_HEIGHT / 4000;
			for(;i < y2 * SCREEN_HEIGHT / 4000;i++){
				fb_draw_circle(x1 * SCREEN_WIDTH / 4000,i,R,color);
			}
			i = y2 * SCREEN_HEIGHT / 4000;
			for(;i < y1 * SCREEN_HEIGHT / 4000;i++){
				fb_draw_circle(x1 * SCREEN_WIDTH / 4000,i,R,color);
			}
		}
		else{
			printf("H");
			for(int i = 0;i < 10000;i++){
				double x = ((double)x1 + stepx/1000*i);
				double y = ((double)y1 + stepy/1000*i);
				fb_draw_circle(x * SCREEN_WIDTH / 4000,y * SCREEN_HEIGHT / 4000,R,color);
			}
		}
		
	}
	return;
}
static void touch_event_cb(int fd)
{
	int type,x,y,finger;
	int tempx,tempy;
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		tempx = x * SCREEN_WIDTH / 4000;
		tempy = y * SCREEN_HEIGHT / 4000;
		if(tempx<200 && tempy<100){
			printf("clear!");
			fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
			fb_draw_rect(0,0,200,100,FB_COLOR(0,255,0));
			fb_draw_text(50,50,"清除",64,FB_COLOR(255,255,0));
			fb_update();
		}
		else{
			lastx[finger] = x;
			lasty[finger] = y;
			fb_draw_circle(tempx,tempy,R,colors[(B + finger) % 5] );
			fb_update();
		}
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_MOVE:
		a_better_move(x,y,lastx[finger],lasty[finger],colors[(B + finger) % 5] );
		lastx[finger] = x;
		lasty[finger] = y;
		fb_update();
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_RELEASE:
		B = (B+1) % 5;
		printf("TOUCH_RELEASE：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		close(fd);
		task_delete_file(fd);
		break;
	default:
		return;
	}
	fb_update();
	return;
}

int main(int argc, char *argv[])
{
	font_init("/home/pi/font.ttc");
	fb_init("/dev/fb0");
	
	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event0");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_draw_rect(0,0,200,100,FB_COLOR(0,255,0));
	fb_draw_text(50,50,"清除",64,FB_COLOR(255,255,0));
	fb_update();
	task_loop(); //进入任务循环
	return 0;
}
