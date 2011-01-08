#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <stdio.h>

#define BLOCK_SIZE 16 

int main()
{
	IplImage *pImage = cvLoadImage("./frame.jpg", 0);
	if (pImage == NULL)
		return -1;	
	
	int th = 6;	
	int width = pImage->width;
	int height = pImage->height;
	int w     = width/BLOCK_SIZE;
	int h     = height/BLOCK_SIZE;
	unsigned char *mb   = new unsigned char[w*h];
	memset(mb, 0, w*h);
	IplImage *pDiff  = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage *pColor = cvCreateImage(cvSize(width, height), 8, 3);
	// 计算图像的x-梯度和y-梯度值
	// dx = f(x+1, y) - f(x, y)
	// dy = f(x, y+1) - f(x, y)
	for (int i = 0; i < height-1; i++){
		for (int j = 0; j < width-1; j++){
			int dx = abs(cvGetReal2D(pImage, i+1, j) - cvGetReal2D(pImage, i, j));
			int dy = abs(cvGetReal2D(pImage, i, j+1) - cvGetReal2D(pImage, i, j));
			cvSetReal2D(pDiff, i, j, max(dx, dy));
		}
	}

	cvShowImage("image", pImage);
	cvShowImage("diff", pDiff);
	cvWaitKey(0);

	cvCvtColor(pDiff, pColor, CV_GRAY2BGR);
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			int top = i? (i*BLOCK_SIZE-1) : 0;        // 宏块顶边
			int left = j? (j*BLOCK_SIZE-1): 0;       // 宏块左边
			int vtop = 0;            // 宏块顶边的边缘点之和
			int vleft = 0;           // 宏块左边的边缘点之和
			for (int k = 0; k < BLOCK_SIZE; k++){
				double tmp_top = cvGetReal2D(pDiff, top, left+k);
				double tmp_left = cvGetReal2D(pDiff, top+k, left);
				vtop += ( tmp_top > th);
				vleft += (tmp_left > th);
			}

			if (vtop >= 15){
				cvLine(pColor, cvPoint(left, top), cvPoint(left+15, top), CV_RGB(255, 0, 0), 1, 4);
				mb[i*w+j] += 1;
				if (i > 0){
					mb[(i-1)*w+j] += 1;
				}
			}
			if (vleft >= 15){
				cvLine(pColor, cvPoint(left, top), cvPoint(left, top+15), CV_RGB(255, 0, 0), 1, 4);
				mb[i*w+j] += 1;
				if (j > 0){
					mb[i*w+j-1] += 1;
				}
			}
		}
	}

	cvShowImage("color", pColor);
	cvWaitKey(0);
	int mosaic_num = 0;
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			if (mb[i*w+j] >= 3){
				mosaic_num ++;
			}
		}
	}
	printf("mosaic number = %d\n" , mosaic_num);
	cvReleaseImage(&pImage);
	cvReleaseImage(&pDiff);	
	return 0;
}
