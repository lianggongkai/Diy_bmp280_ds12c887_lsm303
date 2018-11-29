/****************************************************************************************************
//=========================================电源接线================================================//
//   液晶屏模块            STM32单片机
//      VCC        接       DC5V/3.3V      //电源
//      GND        接          GND         //电源地
//=======================================液晶屏数据线接线==========================================//
//本模块默认数据总线类型为SPI总线
//   液晶屏模块            STM32单片机    
//      SDA        接          PB15        //SPI总线数据信号
//=======================================液晶屏控制线接线==========================================//
//   液晶屏模块 					 STM32单片机 
//      LED        接          PB9         //背光控制信号，如果不需要控制，接5V或3.3V
//      SCK        接          PB13        //SPI总线时钟信号
//    A0(RS/DC)    接          PB10        //数据/命令控制信号
//      RST        接          PB12        //复位控制信号
//      CS         接          PB11        //片选控制信号
//=========================================触摸屏触接线=========================================*/
//本模块不带触摸功能，不需要进行触摸屏接线

#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "font.h"


//******************************************************************
//函数名：  LCD_BGR2RGB
//功能：    将颜色值从GBR格式转换为RGB格式
//输入参数：c：GBR格式颜色值
//返回值：  RGB格式的颜色值
//修改记录：无
//******************************************************************  
u16 LCD_BGR2RGB(u16 c)
{
  u16  r,g,b,rgb;   
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;	 
  rgb=(b<<11)+(g<<5)+(r<<0);		 
  return(rgb);
}

//******************************************************************
//函数名：  _draw_circle_8
//功能：    8对称性画圆算法(内部调用)
//输入参数：(xc,yc) :圆中心坐标
// 			(x,y):光标相对于圆心的坐标
//         	c:填充的颜色
//返回值：  无
//修改记录：无
//******************************************************************  
static void _draw_circle_8(int xc, int yc, int x, int y, u16 c)
{
	Gui_DrawPoint(xc + x, yc + y, c);
	Gui_DrawPoint(xc - x, yc + y, c);
	Gui_DrawPoint(xc + x, yc - y, c);
	Gui_DrawPoint(xc - x, yc - y, c);
	Gui_DrawPoint(xc + y, yc + x, c);
	Gui_DrawPoint(xc - y, yc + x, c);
	Gui_DrawPoint(xc + y, yc - x, c);
	Gui_DrawPoint(xc - y, yc - x, c);
}

//******************************************************************
//函数名：  Gui_Circle
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    画圆
//输入参数：(X,Y):圆中心坐标
// 						R:光标相对于圆心的坐标
//         	  fc:颜色
//返回值：  无
//修改记录：无
//******************************************************************  
void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc) 
{
	//Bresenham算法 
    unsigned short  a,b; 
    int c; 
    a=0; 
    b=R; 
    c=3-2*R; 
    while (a<b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc);     //        7 
        Gui_DrawPoint(X-a,Y+b,fc);     //        6 
        Gui_DrawPoint(X+a,Y-b,fc);     //        2 
        Gui_DrawPoint(X-a,Y-b,fc);     //        3 
        Gui_DrawPoint(X+b,Y+a,fc);     //        8 
        Gui_DrawPoint(X-b,Y+a,fc);     //        5 
        Gui_DrawPoint(X+b,Y-a,fc);     //        1 
        Gui_DrawPoint(X-b,Y-a,fc);     //        4 

        if(c<0) c=c+4*a+6; 
        else 
        { 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    } 
    if (a==b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y-b,fc); 
        Gui_DrawPoint(X-a,Y-b,fc); 
        Gui_DrawPoint(X+b,Y+a,fc); 
        Gui_DrawPoint(X-b,Y+a,fc); 
        Gui_DrawPoint(X+b,Y-a,fc); 
        Gui_DrawPoint(X-b,Y-a,fc); 
    } 
} 

//******************************************************************
//函数名：  Fill_Circle
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    填充圆
//输入参数：(X,Y):圆中心坐标
// 						R:光标相对于圆心的坐标
//         	  fc:颜色
//返回值：  无
//修改记录：无
//******************************************************************  
void Fill_Circle(u16 X,u16 Y,u16 R,u16 fc)
{
	int x = 0, y = R, yi, d;
	d = 3 - 2 * R;
	while (x <= y) 
	{
		for (yi = x; yi <= y; yi++)
		{
			_draw_circle_8(X, Y, x, yi, fc);
		}
		if (d < 0) 
		{
				d = d + 4 * x + 6;
		}
		else 
		{
				d = d + 4 * (x - y) + 10;
				y--;
		}
		x++;
	}
}

//******************************************************************
//函数名：  Fill_Circle
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    使用Bresenham算法画线
//输入参数：(x0,y0):起点坐标
//          (x1,y1):终点坐标
//           color:颜色值
//返回值：  无
//修改记录：无
//******************************************************************  
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color)   
{
	int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	


	Lcd_SetXY(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			Gui_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			Gui_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}

//******************************************************************
//函数名：  LCD_DrawRectangle
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    GUI画矩形(非填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//          color:颜色
//返回值：  无
//修改记录：无
//******************************************************************  
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	Gui_DrawLine(x1,y1,x2,y1,color);
	Gui_DrawLine(x1,y1,x1,y2,color);
	Gui_DrawLine(x1,y2,x2,y2,color);
	Gui_DrawLine(x2,y1,x2,y2,color);
}  

//******************************************************************
//函数名：  LCD_DrawFillRectangle
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    GUI画矩形(填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//           color：颜色
//返回值：  无
//修改记录：无
//******************************************************************   
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	u16 i,j;			
	u16 width=x2-x1+1; 		//得到填充的宽度
	u16 height=y2-y1+1;		//高度
	Lcd_SetRegion(x1,y1,x2,y2);
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			Lcd_WriteData_16Bit(color);
		}   
	}
}

//******************************************************************
//函数名：  Draw_Triangel
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    画三角形
//输入参数：(x0,y0):第一点坐标
//          (x1,y1):第二点坐标
//          (x2,y2):第三点坐标
//           color:颜色值
//返回值：  无
//修改记录：无
//******************************************************************  
void Draw_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	Gui_DrawLine(x0,y0,x1,y1,color);
	Gui_DrawLine(x1,y1,x2,y2,color);
	Gui_DrawLine(x2,y2,x0,y0,color);
}

static void _swap(u16 *a, u16 *b)
{
	u16 tmp;
  tmp = *a;
	*a = *b;
	*b = tmp;
}

//******************************************************************
//函数名：  Fill_Triangel
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    填充三角形
//输入参数：(x0,y0):第一点坐标
//          (x1,y1):第二点坐标
//          (x2,y2):第三点坐标
//           color:颜色值
//返回值：  无
//修改记录：无
//******************************************************************  
void Fill_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 a, b, y, last;
	int dx01, dy01, dx02, dy02, dx12, dy12;
	long sa = 0;
	long sb = 0;
 	if (y0 > y1) 
	{
    _swap(&y0,&y1); 
		_swap(&x0,&x1);
 	}
 	if (y1 > y2) 
	{
    _swap(&y2,&y1); 
		_swap(&x2,&x1);
 	}
  if (y0 > y1) 
	{
    _swap(&y0,&y1); 
		_swap(&x0,&x1);
  }
	if(y0 == y2) 
	{ 
		a = b = x0;
		if(x1 < a)
    {
			a = x1;
    }
    else if(x1 > b)
    {
			b = x1;
    }
    if(x2 < a)
    {
			a = x2;
    }
		else if(x2 > b)
    {
			b = x2;
    }
		LCD_DrawFillRectangle(a,y0,b,y0,color);
    return;
	}
	dx01 = x1 - x0;
	dy01 = y1 - y0;
	dx02 = x2 - x0;
	dy02 = y2 - y0;
	dx12 = x2 - x1;
	dy12 = y2 - y1;
	
	if(y1 == y2)
	{
		last = y1; 
	}
  else
	{
		last = y1-1; 
	}
	for(y=y0; y<=last; y++) 
	{
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
    sb += dx02;
    if(a > b)
    {
			_swap(&a,&b);
		}
		LCD_DrawFillRectangle(a,y,b,y,color);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) 
	{
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b)
		{
			_swap(&a,&b);
		}
		LCD_DrawFillRectangle(a,y,b,y,color);
	}
}

//******************************************************************
//函数名：  Gui_box
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    在屏幕显示灰色矩形
//输入参数：(x,y):矩形左上角坐标
//           w:矩形宽
//           h:矩形高
//           bc：背景颜色
//返回值：  无
//修改记录：无
//****************************************************************** 
void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc)
{
	Gui_DrawLine(x,y,x+w,y,0xEF7D);
	Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
	Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
	Gui_DrawLine(x,y,x,y+h,0xEF7D);
    Gui_DrawLine(x+1,y+1,x+1+w-2,y+1+h-2,bc);
}

//******************************************************************
//函数名：  Gui_box2
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    在屏幕显示灰色矩形
//输入参数：(x,y):矩形左上角坐标
//           w:矩形宽
//           h:矩形高
//           bc：背景颜色
//返回值：  无
//修改记录：无
//****************************************************************** 
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode)
{
	if (mode==0)	{
		Gui_DrawLine(x,y,x+w,y,0xEF7D);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
		Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
		Gui_DrawLine(x,y,x,y+h,0xEF7D);
		}
	if (mode==1)	{
		Gui_DrawLine(x,y,x+w,y,0x2965);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xEF7D);
		Gui_DrawLine(x,y+h,x+w,y+h,0xEF7D);
		Gui_DrawLine(x,y,x,y+h,0x2965);
	}
	if (mode==2)	{
		Gui_DrawLine(x,y,x+w,y,0xffff);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xffff);
		Gui_DrawLine(x,y+h,x+w,y+h,0xffff);
		Gui_DrawLine(x,y,x,y+h,0xffff);
	}
}

//******************************************************************
//函数名：  DisplayButtonDown
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    在屏幕显示一凸起的按钮框
//输入参数：(x1,y1):按钮框左上角坐标
//          (x2,y2):按钮框右下角坐标
//返回值：  无
//修改记录：无
//****************************************************************** 
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, GRAY2);  //H
	Gui_DrawLine(x1+1,y1+1,x2,y1+1, GRAY1);  //H
	Gui_DrawLine(x1,  y1,  x1,y2, GRAY2);  //V
	Gui_DrawLine(x1+1,y1+1,x1+1,y2, GRAY1);  //V
	Gui_DrawLine(x1,  y2,  x2,y2, WHITE);  //H
	Gui_DrawLine(x2,  y1,  x2,y2, WHITE);  //V
}

//******************************************************************
//函数名：  DisplayButtonUp
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    在屏幕显示一凹下的按钮框
//输入参数：(x1,y1):按钮框左上角坐标
//          (x2,y2):按钮框右下角坐标
//返回值：  无
//修改记录：无
//****************************************************************** 
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, WHITE); //H
	Gui_DrawLine(x1,  y1,  x1,y2, WHITE); //V
	
	Gui_DrawLine(x1+1,y2-1,x2,y2-1, GRAY1);  //H
	Gui_DrawLine(x1,  y2,  x2,y2, GRAY2);  //H
	Gui_DrawLine(x2-1,y1+1,x2-1,y2, GRAY1);  //V
    Gui_DrawLine(x2  ,y1  ,x2,y2, GRAY2); //V
}

//******************************************************************
//函数名：  Gui_DrawFont_GBK8
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    显示6*8字符或者中英文
//输入参数：(x,y):数字坐标
//          fc:数字颜色
//          bc:背景颜色
//          s:待显示字符串指针
//返回值：  无
//修改记录：无
//****************************************************************** 
void Gui_DrawFont_GBK8(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128) 
		{
			k=*s;
			if (k==13) 
			{
				x=x0;
				y+=8;
			}
			else 
			{
				if (k>32) k-=32; else k=0;
	
			  for(i=0;i<8;i++)
					for(j=0;j<6;j++) 
					{
				    	if(asc8[k*8+i]&(0x80>>j))	Gui_DrawPoint(x+j,y+i,fc);
						else 
						{
							if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
						}
					}
				x+=6;
			}
			s++;
		}
		else
		{
			return;
		}
	}
}

//******************************************************************
//函数名：  Gui_DrawFont_GBK16
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    显示16*16字符或者中英文
//输入参数：(x,y):数字坐标
//          fc:数字颜色
//          bc:背景颜色
//          s:待显示字符串指针
//返回值：  无
//修改记录：无
//****************************************************************** 
void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128)//ASIC字符输出 
		{
			k=*s;
			if (k==13)//回车换行 
			{
				x=x0;
				y+=16;
			}
			else 
			{
				if (k>32) k-=32; else k=0;//减去32个不可显示字符可得出字符编码编号k
				for(i=0;i<16;i++)//每个字符一共16行
				{
					for(j=0;j<8;j++)//每个字符每行8个像素点 
					{
						if(asc16[k*16+i]&(0x80>>j))	Gui_DrawPoint(x+j,y+i,fc);
						else 
						{
							if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
						}
					}
				}
				x+=8;
			}
			s++;
		}
			
		else //汉字输出
		{
			for (k=0;k<hz16_num;k++)//查询汉字所在结构体数组中的位置。 
			{
			  if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<16;i++)
				    {
							for(j=0;j<8;j++) 
							{
									if(hz16[k].Msk[i*2]&(0x80>>j))	Gui_DrawPoint(x+j,y+i,fc);
								else {
									if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
								}
							}
							for(j=0;j<8;j++) 
							{
								if(hz16[k].Msk[i*2+1]&(0x80>>j))	Gui_DrawPoint(x+j+8,y+i,fc);
								else 
								{
									if (fc!=bc) Gui_DrawPoint(x+j+8,y+i,bc);
								}
							}
				    }
					}
			  }
				s+=2;x+=16;//汉字字符偏移2，横坐标偏移16
		} 
	}
}

void Gui_DrawFont_GBK1632(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char LineNum = 0,i=0,offset = 0; 
	while(*s){
		if(*s>':')*s = ':'+1;
		offset = *s - 48;
		for(LineNum = 0; LineNum < 32;LineNum++)
		{
			for(i=0; i < 8; i++){
				if(lll[offset*64 + LineNum*2]&(0x80>>i)){
					Gui_DrawPoint(x+i,y+LineNum,fc);
				}else if(fc!=bc) Gui_DrawPoint(x+i,y+LineNum,bc);
			}
			
			for(i=0; i < 8; i++){
				if(lll[offset*64 + LineNum*2+1]&(0x80>>i)){
					Gui_DrawPoint(x+i+8,y+LineNum,fc);
				}else if(fc!=bc) Gui_DrawPoint(x+i+8,y+LineNum,bc);
			}
		}
		x+=16;
		s++;
	}
}

//******************************************************************
//函数名：  Gui_DrawFont_GBK24
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    显示24*24字符或者中英文
//输入参数：(x,y):数字坐标
//          fc:数字颜色
//          bc:背景颜色
//          s:待显示字符串指针
//返回值：  无
//修改记录：无
//******************************************************************  
void Gui_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k;

	while(*s) 
	{
		if( *s < 0x80 ) 
		{
			k=*s;
			if (k>32) k-=32; else k=0;

		  for(i=0;i<16;i++)
			for(j=0;j<8;j++) 
			{
				if(asc16[k*16+i]&(0x80>>j))	
				Gui_DrawPoint(x+j,y+i,fc);
				else 
				{
					if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
				}
			}
			s++;x+=8;
		}
		else 
		{

			for (k=0;k<hz24_num;k++) 
			{
			  if ((hz24[k].Index[0]==*(s))&&(hz24[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<24;i++)
				    {
						for(j=0;j<8;j++) 
						{
							if(hz24[k].Msk[i*3]&(0x80>>j))
							Gui_DrawPoint(x+j,y+i,fc);
							else 
							{
								if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
							}
						}
						for(j=0;j<8;j++) 
						{
							if(hz24[k].Msk[i*3+1]&(0x80>>j))	Gui_DrawPoint(x+j+8,y+i,fc);
							else {
								if (fc!=bc) Gui_DrawPoint(x+j+8,y+i,bc);
							}
						}
						for(j=0;j<8;j++) 
						{
							if(hz24[k].Msk[i*3+2]&(0x80>>j))	
							Gui_DrawPoint(x+j+16,y+i,fc);
							else 
							{
								if (fc!=bc) Gui_DrawPoint(x+j+16,y+i,bc);
							}
						}
				    }
			  }
			}
			s+=2;x+=24;
		}
	}
}

//******************************************************************
//函数名：  Gui_DrawFont_Num32
//作者：    xiao冯@全动电子
//日期：    2018-07-28
//功能：    显示32*32数字
//输入参数：(x,y):数字坐标
//          fc:数字颜色
//          bc:背景颜色
//          num:数字编号
//返回值：  无
//修改记录：无
//******************************************************************  
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num)
{
	unsigned char i,j,k,c;
	for(i=0;i<32;i++)//32行
	{
		for(j=0;j<4;j++)//每行4个8bit字码进行输出 
		{
			c=*(sz32+num*32*4+i*4+j);//获取当前的字码
			for (k=0;k<8;k++)	
			{
		    	if(c&(0x80>>k))	Gui_DrawPoint(x+j*8+k,y+i,fc);
				else {
					if (fc!=bc) Gui_DrawPoint(x+j*8+k,y+i,bc);
				}
			}
		}
	}
}


