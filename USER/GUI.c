/****************************************************************************************************
//=========================================��Դ����================================================//
//   Һ����ģ��            STM32��Ƭ��
//      VCC        ��       DC5V/3.3V      //��Դ
//      GND        ��          GND         //��Դ��
//=======================================Һ���������߽���==========================================//
//��ģ��Ĭ��������������ΪSPI����
//   Һ����ģ��            STM32��Ƭ��    
//      SDA        ��          PB15        //SPI���������ź�
//=======================================Һ���������߽���==========================================//
//   Һ����ģ�� 					 STM32��Ƭ�� 
//      LED        ��          PB9         //��������źţ��������Ҫ���ƣ���5V��3.3V
//      SCK        ��          PB13        //SPI����ʱ���ź�
//    A0(RS/DC)    ��          PB10        //����/��������ź�
//      RST        ��          PB12        //��λ�����ź�
//      CS         ��          PB11        //Ƭѡ�����ź�
//=========================================������������=========================================*/
//��ģ�鲻���������ܣ�����Ҫ���д���������

#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "font.h"


//******************************************************************
//��������  LCD_BGR2RGB
//���ܣ�    ����ɫֵ��GBR��ʽת��ΪRGB��ʽ
//���������c��GBR��ʽ��ɫֵ
//����ֵ��  RGB��ʽ����ɫֵ
//�޸ļ�¼����
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
//��������  _draw_circle_8
//���ܣ�    8�Գ��Ի�Բ�㷨(�ڲ�����)
//���������(xc,yc) :Բ��������
// 			(x,y):��������Բ�ĵ�����
//         	c:������ɫ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_Circle
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��Բ
//���������(X,Y):Բ��������
// 						R:��������Բ�ĵ�����
//         	  fc:��ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc) 
{
	//Bresenham�㷨 
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
//��������  Fill_Circle
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ���Բ
//���������(X,Y):Բ��������
// 						R:��������Բ�ĵ�����
//         	  fc:��ɫ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Fill_Circle
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ʹ��Bresenham�㷨����
//���������(x0,y0):�������
//          (x1,y1):�յ�����
//           color:��ɫֵ
//����ֵ��  ��
//�޸ļ�¼����
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
	dx = x1-x0;//����x����
	dy = y1-y0;//����y����

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

	if (dx > dy)//x�������y���룬��ôÿ��x����ֻ��һ���㣬ÿ��y���������ɸ���
	{//���ߵĵ�������x���룬��x���������
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//Ҫ���ĵ������ᳬ��x����
		{
			//����
			Gui_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //�Ƿ���Ҫ����y����ֵ
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//����y����ֵ
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x����ֵÿ�λ���󶼵���1
		} // end for
	} // end if |slope| <= 1
	else//y�����x�ᣬ��ÿ��y����ֻ��һ���㣬x�����ɸ���
	{//��y��Ϊ��������
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
//��������  LCD_DrawRectangle
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    GUI������(�����)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//          color:��ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	Gui_DrawLine(x1,y1,x2,y1,color);
	Gui_DrawLine(x1,y1,x1,y2,color);
	Gui_DrawLine(x1,y2,x2,y2,color);
	Gui_DrawLine(x2,y1,x2,y2,color);
}  

//******************************************************************
//��������  LCD_DrawFillRectangle
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    GUI������(���)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//           color����ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************   
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	u16 i,j;			
	u16 width=x2-x1+1; 		//�õ����Ŀ��
	u16 height=y2-y1+1;		//�߶�
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
//��������  Draw_Triangel
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��������
//���������(x0,y0):��һ������
//          (x1,y1):�ڶ�������
//          (x2,y2):����������
//           color:��ɫֵ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Fill_Triangel
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ���������
//���������(x0,y0):��һ������
//          (x1,y1):�ڶ�������
//          (x2,y2):����������
//           color:��ɫֵ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_box
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ����Ļ��ʾ��ɫ����
//���������(x,y):�������Ͻ�����
//           w:���ο�
//           h:���θ�
//           bc��������ɫ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_box2
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ����Ļ��ʾ��ɫ����
//���������(x,y):�������Ͻ�����
//           w:���ο�
//           h:���θ�
//           bc��������ɫ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  DisplayButtonDown
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ����Ļ��ʾһ͹��İ�ť��
//���������(x1,y1):��ť�����Ͻ�����
//          (x2,y2):��ť�����½�����
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  DisplayButtonUp
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ����Ļ��ʾһ���µİ�ť��
//���������(x1,y1):��ť�����Ͻ�����
//          (x2,y2):��ť�����½�����
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_DrawFont_GBK8
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��ʾ6*8�ַ�������Ӣ��
//���������(x,y):��������
//          fc:������ɫ
//          bc:������ɫ
//          s:����ʾ�ַ���ָ��
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_DrawFont_GBK16
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��ʾ16*16�ַ�������Ӣ��
//���������(x,y):��������
//          fc:������ɫ
//          bc:������ɫ
//          s:����ʾ�ַ���ָ��
//����ֵ��  ��
//�޸ļ�¼����
//****************************************************************** 
void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128)//ASIC�ַ���� 
		{
			k=*s;
			if (k==13)//�س����� 
			{
				x=x0;
				y+=16;
			}
			else 
			{
				if (k>32) k-=32; else k=0;//��ȥ32��������ʾ�ַ��ɵó��ַ�������k
				for(i=0;i<16;i++)//ÿ���ַ�һ��16��
				{
					for(j=0;j<8;j++)//ÿ���ַ�ÿ��8�����ص� 
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
			
		else //�������
		{
			for (k=0;k<hz16_num;k++)//��ѯ�������ڽṹ�������е�λ�á� 
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
				s+=2;x+=16;//�����ַ�ƫ��2��������ƫ��16
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
//��������  Gui_DrawFont_GBK24
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��ʾ24*24�ַ�������Ӣ��
//���������(x,y):��������
//          fc:������ɫ
//          bc:������ɫ
//          s:����ʾ�ַ���ָ��
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  Gui_DrawFont_Num32
//���ߣ�    xiao��@ȫ������
//���ڣ�    2018-07-28
//���ܣ�    ��ʾ32*32����
//���������(x,y):��������
//          fc:������ɫ
//          bc:������ɫ
//          num:���ֱ��
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num)
{
	unsigned char i,j,k,c;
	for(i=0;i<32;i++)//32��
	{
		for(j=0;j<4;j++)//ÿ��4��8bit���������� 
		{
			c=*(sz32+num*32*4+i*4+j);//��ȡ��ǰ������
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


