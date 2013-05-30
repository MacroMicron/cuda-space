

#ifndef _BMP_
#define _BMP_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
	int w,h;
	float *red, *green, *blue;
	//red[w][h], green[w][h], blue[w][h]
	//every element of this array is a float number from 0 to 1	
} Bitmap;


Bitmap *CreateBMP(int width, int height);
void SaveBMPtoFile(Bitmap *bitmap, char *file);
void DestroyBMP(Bitmap *bitmap);


#ifdef __cplusplus
}
#endif



#endif
