#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <stdio.h>

#define BLOCK_SIZE 16 

#define TOP 0
#define RIGHT 1
#define BOTTOM 2
#define LEFT 3

struct microblock {
	double val[4];
	double operator[](int i) const {
		return val[i];
	}

	double & operator[](int i){
		return val[i];
	}
};

IplImage *pImage;
microblock *mb;
int h16,w16;
void on_tracker(int thresh){
	int mosaic_num = 0;
	IplImage *pColor = cvCreateImage(cvSize(pImage->width, pImage->height), 8, 3);
	IplImage *pDiff = cvCreateImage(cvSize(pImage->width, pImage->height), 8, 3);
	cvCvtColor(pImage, pColor, CV_GRAY2BGR);
	for (int i = 0; i < h16; i++){
		for (int j = 0; j < w16; j++){
			int n_edges = 0;
			for (int k = 0; k < 4; k++){
				if (mb[i*w16+j][k] >= thresh){
					n_edges ++;
				}
			}


			int top = i*16;
			int left = j*16;
			int bottom = top+15;
			int right = left+15;
			cvLine(pDiff, cvPoint(left, top), cvPoint(right, top),   CV_RGB(0, mb[i*w16+j][TOP]*10, 0), 1, CV_AA, 0);
			cvLine(pDiff, cvPoint(left, top), cvPoint(left, bottom), CV_RGB(0, mb[i*w16+j][LEFT]*10, 0), 1, CV_AA, 0);
			cvLine(pDiff, cvPoint(right, top), cvPoint(right, bottom), CV_RGB(0, mb[i*w16+j][RIGHT]*10, 0), 1, CV_AA, 0);
			cvLine(pDiff, cvPoint(left, bottom), cvPoint(right, bottom), CV_RGB(0, mb[i*w16+j][BOTTOM]*10, 0), 1, CV_AA, 0);

			if (n_edges >= 2){
				mosaic_num ++;
				cvRectangle(pColor, cvPoint(left, top), cvPoint(left+15, top+15), CV_RGB(255, 0, 0), 1, CV_AA, 0);
			}else{
				cvSet2D(pColor, top, left, CV_RGB(255, 0, 0));
			}

		}
	}

	printf("mosaic number = %d\n" , mosaic_num);
	fflush(stdin);
	cvShowImage("image", pColor);
	cvShowImage("diff", pDiff);
	cvReleaseImage(&pColor);
	cvReleaseImage(&pDiff);


}

int main()
{
	pImage = cvLoadImage("./frame.jpg", 0);
	if (pImage == NULL)
		return -1;	
	
	int width = pImage->width;
	int height = pImage->height;
	 w16     = width/BLOCK_SIZE;
	 h16     = height/BLOCK_SIZE;
	mb   = new microblock[w16*h16];
	// 计算图像的x-梯度和y-梯度值
	// fx'(x,y) = abs(f(x+1, y) - f(x, y))
	// fx"(x,y) = max{fx'(x,y)-f'(x-1,y), fx'(x,y)-f'(x+1,y)}
	//          = max{abs(f(x+1,y) - f(x,y)) - abs(f(x,y) - f(x-1,y)),
	//                abs(f(x+1,y) - f(x,y)) - abs(f(x+2,y) - f(x+1,y))}
	//              
	// fy'(x,y) = abs(f(x, y+1) - f(x, y))
	// fy"(x,y) = max(fy'(x, y)-fy'(x, y-1), fy'(x,y)-fy'(x,y-1)
	//          = max{abs(f(x,y+1) - f(x,y)) - abs(f(x,y) - f(x,y-1)),
	//                abs(f(x,y+1) - f(x,y)) - abs(f(x,y+2) - f(x,y+1))}
	


	for (int i = 0; i < h16; i++){
		for (int j = 0; j < w16; j++){
			int top = i ? (i*16-1) : 0;
			int left = j ? (j*16-1) : 0;

			double ddy = 0;
			for (int k = 0; i > 0 && k < 16; k++){
				double y[4];
				y[0] = cvGetReal2D(pImage, top-1, left+k);	
				y[1] = cvGetReal2D(pImage, top, left+k);
				y[2] = cvGetReal2D(pImage, top+1, left+k);
				y[3] = cvGetReal2D(pImage, top+2, left+k);
				ddy += min(abs(y[2] - y[1]) - abs(y[1] - y[0]),
						   abs(y[2] - y[1]) - abs(y[3] - y[2]));
			}
			ddy /= 16;
			mb[i*w16+j][TOP] = ddy; 
			if (i > 0){
				mb[(i-1)*w16+j][BOTTOM] = ddy;
			}

			double ddx = 0;
			for (int k = 0; j > 0 && k < 16; k++){
				double x[4];
				x[0] = cvGetReal2D(pImage, top+k, left-1);	
				x[1] = cvGetReal2D(pImage, top+k, left);
				x[2] = cvGetReal2D(pImage, top+k, left+1);
				x[3] = cvGetReal2D(pImage, top+k, left+2);
				ddx += min(abs(x[2] - x[1]) - abs(x[1] - x[0]),
						   abs(x[2] - x[1]) - abs(x[3] - x[2]));
			}
			ddx /= 16;
			mb[i*w16+j][LEFT] = ddx;
			if (j > 0){
				mb[i*w16+j-1][RIGHT] = ddx;
			}
		}	
	}

	cvNamedWindow("image");
	int val = 10;
	cvCreateTrackbar("tracker", "image", &val, 50, on_tracker);
	on_tracker(val);
	cvWaitKey(0);
	cvReleaseImage(&pImage);
	return 0;
}
