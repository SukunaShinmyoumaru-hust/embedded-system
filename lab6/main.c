#include <stdio.h>
#include "../common/common.h"
#define R 5
#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)
#define NUM 100
static char name[20];
static char num[5];
static int touch_fd;
void int2str(int n, char *str)
{
        char buf[10] = "";
        int i = 0;
        int len = 0;
        int temp = n < 0 ? -n: n;  // temp为n的绝对值
  
       if (str == NULL)
       {
           return;
       }
       while(temp)
       {
           buf[i++] = (temp % 10) + '0';  //把temp的每一位上的数存入buf
           temp = temp / 10;
       }
  
       len = n < 0 ? ++i: i;  //如果n是负数，则多需要一位来存储负号
       str[i] = 0;            //末尾是结束符0
       while(1)
       {
           i--;
           if (buf[len-i-1] ==0)
           {
               break;
           }
           str[i] = buf[len-i-1];  //把buf数组里的字符拷到字符串
       }
       if (i == 0 )
       {
           str[i] = '-';          //如果是负数，添加一个负号
       }
}
static void display(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0);
	fb_update();
	system("omxplayer -o local 00001.mp3");
	for(int i = 0;i < NUM;i++){
		memset(name,0,sizeof(name));
		if(i==0) strcat(name,"./pic/test0000.jpg");
		else if(i<10){
			strcat(name,"./pic/test000");
			int2str(i,num);
			strcat(name,num);
			strcat(name,".jpg");
			
		}
		else if(i<100){
			strcat(name,"./pic/test00");
			int2str(i,num);
			strcat(name,num);
			strcat(name,".jpg");
		}
		else if(i<1000){
			strcat(name,"./pic/test0");
			int2str(i,num);
			strcat(name,num);
			strcat(name,".jpg");
		}
		else if(i<6000){
			strcat(name,"./pic/test");
			int2str(i,num);
			strcat(name,num);
			strcat(name,".jpg");
		}
		else{
			strcat(name,"./pic/test");
			int2str(i,num);
			strcat(name,num);
			strcat(name,".png");
			fb_image *img;
			img = fb_read_png_image(name);
			fb_draw_image(0,0,img,0);
			fb_update();
			fb_free_image(img);
			continue;
		}
		//printf("%s\n",name);
		fb_image *img;
		img = fb_read_jpeg_image(name);
		fb_draw_image(0,0,img,0);
		fb_update();
		for(int i=0;i<5000000;i++){

		}
		fb_free_image(img);
	}
}
static void touch_event_cb(int fd)
{
	int type,x,y,finger;
	int tempx,tempy;
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		if(x < 800 && y < 800) display();
		fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
		fb_draw_rect(0,0,200,100,FB_COLOR(0,255,0));
		fb_draw_text(50,50,"播放",64,FB_COLOR(255,255,0));
		fb_draw_text(150,300,"树莓派视频播放软件",64,FB_COLOR(255,255,0));
		fb_update();
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_MOVE:
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_RELEASE:

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
	fb_draw_text(50,50,"播放",64,FB_COLOR(255,255,0));
	fb_draw_text(150,300,"树莓派视频播放软件",64,FB_COLOR(255,255,0));
	fb_update();
	task_loop(); //进入任务循环
	return 0;
}
