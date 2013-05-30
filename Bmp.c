#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //antiwarning for close
#include "Bmp.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

Bitmap *CreateBMP(int w, int h)
{
	Bitmap *bitmap;
	bitmap = (Bitmap*) malloc(sizeof(Bitmap));
	assert(bitmap);
	bitmap->w = w;
	bitmap->h = h;
	bitmap->red = (float *)calloc(w*h, sizeof(float));
	assert(bitmap->red);
	bitmap->green = (float *)calloc(w*h, sizeof(float));
	assert(bitmap->green);
	bitmap->blue = (float *)calloc(w*h, sizeof(float));	
	assert(bitmap->blue);	
	return bitmap;
}


void DestroyBMP(Bitmap *bitmap)
{
	if (bitmap)
	{
		if (bitmap->red) free(bitmap->red);
		if (bitmap->green) free(bitmap->green);
		if (bitmap->blue) free(bitmap->blue);
		if (bitmap) free(bitmap);
        	bitmap = NULL;
	}
}

void SaveBMPtoFile(Bitmap *bitmap, char *filename)
{
	if (!bitmap) return;
	if (!filename) return;
	if (!bitmap->red) return;
	if (!bitmap->green) return;
	if (!bitmap->blue) return;
	const int w=bitmap->w;
	const int h=bitmap->h;
	int r,g,b, i,j,x,y;
	float *red=bitmap->red, *green=bitmap->green, *blue=bitmap->blue;
	/*for (i=0; i<w; i++)
		for (j=0; j<h; j++)
		{
			if (j!=0) red[i][j]=green[i][j]=blue[i][j] = 1.0;
			else red[i][j]=green[i][j]=blue[i][j] = 0.5;
			if (i==0) red[i][j] = 1.0;
		}
	*/
	
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3*w*h;  //w is your image width, h is image height, both int
	img = (unsigned char *)malloc(3*w*h);
	memset(img,0,sizeof(img));
	

	for(i=0; i<w; i++)
	{
	    for(j=0; j<h; j++)
	{
	    x=i; y=j;//y=(yres-1)-j;
	    r = red[i*h+j]*255;
	    g = green[i*h+j]*255;
	    b = blue[i*h+j]*255;
	    //printf("%f\n", blue[i*h+j]);
	    if (r > 255) r=255;
	    if (g > 255) g=255;
	    if (b > 255) b=255;
	    img[(x+y*w)*3+2] = (unsigned char)(r);
	    img[(x+y*w)*3+1] = (unsigned char)(g);
	    img[(x+y*w)*3+0] = (unsigned char)(b);
	}
	}
	
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};
	
	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);
	
	bmpinfoheader[ 4] = (unsigned char)(       w    );
	bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       w>>16);
	bmpinfoheader[ 7] = (unsigned char)(       w>>24);
	bmpinfoheader[ 8] = (unsigned char)(       h    );
	bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
	bmpinfoheader[10] = (unsigned char)(       h>>16);
	bmpinfoheader[11] = (unsigned char)(       h>>24);
	
	f = fopen(filename,"wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	for(i=0; i<h; i++)
	{
	    fwrite(img+(w*(h-i-1)*3),3,w,f);
	    fwrite(bmppad,1,(4-(w*3)%4)%4,f);
	}
	fclose(f);	
	free( img );
}

#ifdef __cplusplus
}
#endif

