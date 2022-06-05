#include "common.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>


static int LCD_FB_FD;
static int *LCD_FB_BUF = NULL;
static int *LCD_FB_FRONT, *LCD_FB_BACK;
struct fb_var_screeninfo LCD_FB_VAR;
static int DRAW_BUF[SCREEN_WIDTH*SCREEN_HEIGHT];

static struct area {
	int x1, x2, y1, y2;
} update_area = {0,0,0,0};

#define AREA_SET_EMPTY(pa) do {\
	(pa)->x1 = SCREEN_WIDTH;\
	(pa)->x2 = 0;\
	(pa)->y1 = SCREEN_HEIGHT;\
	(pa)->y2 = 0;\
} while(0)

void fb_init(char *dev)
{
	int fd;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;

	if(LCD_FB_BUF != NULL) return; /*already done*/

	//First: Open the device
	if((fd = open(dev, O_RDWR)) < 0){
		printf("Unable to open framebuffer %s, errno = %d\n", dev, errno);
		return;
	}
	if(ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0){
		printf("Unable to FBIOGET_FSCREENINFO %s\n", dev);
		return;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0){
		printf("Unable to FBIOGET_VSCREENINFO %s\n", dev);
		return;
	}

	printf("framebuffer info: bits_per_pixel=%u,size=(%d,%d),virtual_pos_size=(%d,%d)(%d,%d),line_length=%u,smem_len=%u\n",
		fb_var.bits_per_pixel, fb_var.xres, fb_var.yres, fb_var.xoffset, fb_var.yoffset,
		fb_var.xres_virtual, fb_var.yres_virtual, fb_fix.line_length, fb_fix.smem_len);

	//Second: mmap
	int *addr;
	addr = mmap(NULL, fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if((int)addr == -1){
		printf("failed to mmap memory for framebuffer.\n");
		return;
	}

	if((fb_var.xoffset != 0) ||(fb_var.yoffset != 0))
	{
		fb_var.xoffset = 0;
		fb_var.yoffset = 0;
		if(ioctl(fd, FBIOPAN_DISPLAY, &fb_var) < 0) {
			printf("FBIOPAN_DISPLAY framebuffer failed\n");
		}
	}


	LCD_FB_FD = fd;
	LCD_FB_BUF = addr;
	LCD_FB_FRONT = addr;
	LCD_FB_BACK = addr + fb_var.xres*fb_var.yres;
	LCD_FB_VAR = fb_var;

	//set empty
	AREA_SET_EMPTY(&update_area);
	return;
}

static void _copy_area(int *dst, int *src, struct area *pa)
{
	int x, y, w, h;
	x = pa->x1; w = pa->x2-x;
	y = pa->y1; h = pa->y2-y;
	src += y*SCREEN_WIDTH + x;
	dst += y*SCREEN_WIDTH + x;
	while(h-- > 0){
		memcpy(dst, src, w*4);
		src += SCREEN_WIDTH;
		dst += SCREEN_WIDTH;
	}
}

static int _check_area(struct area *pa)
{
	if(pa->x2 == 0) return 0; //is empty

	if(pa->x1 < 0) pa->x1 = 0;
	if(pa->x2 > SCREEN_WIDTH) pa->x2 = SCREEN_WIDTH;
	if(pa->y1 < 0) pa->y1 = 0;
	if(pa->y2 > SCREEN_HEIGHT) pa->y2 = SCREEN_HEIGHT;

	if((pa->x2 > pa->x1) && (pa->y2 > pa->y1))
		return 1; //no empty

	//set empty
	AREA_SET_EMPTY(pa);
	return 0;
}

void fb_update(void)
{
	if(_check_area(&update_area) == 0) return; //is empty
	_copy_area(LCD_FB_FRONT, DRAW_BUF, &update_area);
	AREA_SET_EMPTY(&update_area); //set empty
	return;
}

/*======================================================================*/

static void * _begin_draw(int x, int y, int w, int h)
{
	int x2 = x+w;
	int y2 = y+h;
	if(update_area.x1 > x) update_area.x1 = x;
	if(update_area.y1 > y) update_area.y1 = y;
	if(update_area.x2 < x2) update_area.x2 = x2;
	if(update_area.y2 < y2) update_area.y2 = y2;
	return DRAW_BUF;
}

void fb_draw_pixel(int x, int y, int color)
{
	if(x<0 || y<0 || x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT) return;
	int *buf = _begin_draw(x,y,1,1);
/*---------------------------------------------------*/
	*(buf + y*SCREEN_WIDTH + x) = color;
/*---------------------------------------------------*/
	return;
}
void fb_draw_rect(int x, int y, int w, int h, int color)
{
	if(x < 0) { w += x; x = 0;}
	if(x+w > SCREEN_WIDTH) { w = SCREEN_WIDTH-x;}
	if(y < 0) { h += y; y = 0;}
	if(y+h >SCREEN_HEIGHT) { h = SCREEN_HEIGHT-y;}
	if(w<=0 || h<=0) return;
	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------*/
	
	int currentx,currenty;
	for(currentx = x;currentx <= x+w; currentx++){
		for(currenty = y;currenty <= y+h; currenty++){
			fb_draw_pixel(currentx,currenty,color);
		}
	}
	// Add your code here
/*---------------------------------------------------*/
	return;
}
void fb_draw_circle(int x,int y,int r,int color){
	int* buf = _begin_draw(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	if(r<=0) return;
	int currentx,currenty;
	for(currentx = x-r;currentx <= x+r; currentx++){
		for(currenty = y-r;currenty <= y+r; currenty++){
			if(currentx > 0 && currentx < SCREEN_WIDTH && currenty > 0 && currenty <SCREEN_HEIGHT && (currentx-x)*(currentx-x)+(currenty-y)*(currenty-y)<r*r){
				*(buf +SCREEN_WIDTH * currenty + currentx) = color; 
			}
			
		}
	}
}
void fb_draw_line(int x1, int y1, int x2, int y2, int color)
{

	int* buf = _begin_draw(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	if(x1 == x2){
		for(int i = y1;i < y2;i++){
			*(buf +SCREEN_WIDTH * i + x1) = color; 
		}
	}
	else{
		double i = x1;
		double stepx = ((double)(x2 - x1));
		double stepy = ((double)(y2 - y1))*10;
		for(int i = 0;i < 10000;i++){
			int x = (int)((double)x1 + stepx/10000*i);
			int y = (int)((double)y1 + stepy/100000*i);
			*(buf +SCREEN_WIDTH * y + x) = color; 
		}
	}
	return;
}

void fb_draw_image(int x, int y, fb_image *image, int color)
{
	if(image == NULL) return;

	int ix = 0; //image x
	int iy = 0; //image y
	int w = image->pixel_w; //draw width
	int h = image->pixel_h; //draw height

	if(x<0) {w+=x; ix-=x; x=0;}
	if(y<0) {h+=y; iy-=y; y=0;}
	
	if(x+w > SCREEN_WIDTH) {
		w = SCREEN_WIDTH - x;
	}
	if(y+h > SCREEN_HEIGHT) {
		h = SCREEN_HEIGHT - y;
	}
	if((w <= 0)||(h <= 0)) return;

	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------------------*/
	int *dst = (buf + y*SCREEN_WIDTH + x);
	int *src = image->content + iy*image->line_byte + ix*4;
/*---------------------------------------------------------------*/

	char alpha;
	int ww;
	char* p;

	if (image->color_type == FB_COLOR_RGB_8880) {
        for(ww = 0;ww < h - 1;ww++){
			memcpy(dst,src,image->line_byte);
			dst+=SCREEN_WIDTH;
			src+=image->line_byte/4;
		}
		return;
    }

	if(image->color_type == FB_COLOR_RGBA_8888) /*lab3: png*/
	{
		for(int i = 0;i < h;i++){
			for(int j = 0;j < w;j++){
				if(x+j<0||x+j>SCREEN_WIDTH||y+i<0||y+j>SCREEN_HEIGHT) break;
				int* origin = src+j;
				int color = *(origin);
				alpha = color>>24;
				color = color & 0x00ffffff;
				int* dest = dst+j;
				switch(alpha) {
               	case 0: break;
               	case 255:
                    *dest = color;
					break;
               	default:
                    break;
         		}
			}
			dst+=SCREEN_WIDTH;
			src+=image->line_byte/4;
		}
		return;
	}

	if(image->color_type == FB_COLOR_ALPHA_8) /*lab3: font*/
	{
		int i=0,j=0;
        for(;i<image->pixel_h;i++){
            j=0;
            for(;j<image->pixel_w;j++){
                if(y+i>=SCREEN_HEIGHT||x+j>=SCREEN_WIDTH) break;

                int originprgb = *(buf + (y+i)*SCREEN_WIDTH + x+j);
                int preR=(originprgb>>16)&0x000000ff;
                int preG=(originprgb>>8)&0x000000ff;
                int preB=originprgb&0x000000ff;
                int alpha=*((char *)image->content+i*image->pixel_w+j);
                int r=(color>>16)&0x000000ff;
                int g=(color>>8)&0x000000ff;
                int b=color&0x000000ff;
                switch (alpha) {
                    case 0:break;
                    case 255:{
                        preR=r;
                        preG=g;
                        preB=b;
                    }
                    default:{
                        preR+=((r-preR)*alpha)>>8;
                        preG+=((g-preG)*alpha)>>8;
                        preB+=((b-preB)*alpha)>>8;
                    }
                }

                *(buf + (y+i)*SCREEN_WIDTH + x+j) = (originprgb&0xff000000)|(preR<<16)|(preG<<8)|(preB);
            }

        }

		return;
	}

/*---------------------------------------------------------------*/
	return;
}

void fb_draw_border(int x, int y, int w, int h, int color)
{
	if(w<=0 || h<=0) return;
	fb_draw_rect(x, y, w, 1, color);
	if(h > 1) {
		fb_draw_rect(x, y+h-1, w, 1, color);
		fb_draw_rect(x, y+1, 1, h-2, color);
		if(w > 1) fb_draw_rect(x+w-1, y+1, 1, h-2, color);
	}
}

/** draw a text string **/
void fb_draw_text(int x, int y, char *text, int font_size, int color)
{
	fb_image *img;
	fb_font_info info;
	int i=0;
	int len = strlen(text);
	while(i < len)
	{
		img = fb_read_font_image(text+i, font_size, &info);
		if(img == NULL) break;
		fb_draw_image(x+info.left, y-info.top, img, color);
		fb_free_image(img);
		x += info.advance_x;
		i += info.bytes;
	}
	return;
}

