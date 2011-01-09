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
	char filename[64];
	sprintf(filename, "mosaic-%d-%d.jpg", thresh, mosaic_num);
	cvSaveImage(filename, pColor);
	cvReleaseImage(&pColor);
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
	// 计算图像的x-梯度和y-梯度值(1阶, 2阶)
	// fx'(x,y) = abs(f(x+1, y) - f(x, y))
	// fx"(x,y) = min{fx'(x,y)-f'(x-1,y), fx'(x,y)-f'(x+1,y)}
	//          = min{abs(f(x+1,y) - f(x,y)) - abs(f(x,y) - f(x-1,y)),
	//                abs(f(x+1,y) - f(x,y)) - abs(f(x+2,y) - f(x+1,y))}
	//              
	// fy'(x,y) = abs(f(x, y+1) - f(x, y))
	// fy"(x,y) = min(fy'(x, y)-fy'(x, y-1), fy'(x,y)-fy'(x,y-1)
	//          = min{abs(f(x,y+1) - f(x,y)) - abs(f(x,y) - f(x,y-1)),
	//                abs(f(x,y+1) - f(x,y)) - abs(f(x,y+2) - f(x,y+1))}
	
	IplImage *pDiff = cvCreateImage(cvSize(pImage->width, pImage->height), 8, 3);
	IplImage *p2Diff = cvCreateImage(cvSize(pImage->width, pImage->height), 8, 3); 
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

				double dy[3];
				dy[0] = abs(y[1] - y[0]);
				dy[1] = abs(y[2] - y[1]);  
				dy[2] = abs(y[3] - y[2]);
				cvSet2D(pDiff, top-1,left+k,  CV_RGB(0, dy[0]*5, 0));
				cvSet2D(pDiff, top,left+k,  CV_RGB(0, dy[1]*5, 0));
				cvSet2D(pDiff,top+1, left+k,  CV_RGB(0, dy[2]*5, 0));

				ddy += min(dy[1] - dy[0], dy[1] - dy[2]);
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

				double dx[3];
				dx[0] = abs(x[1] - x[0]);
				dx[1] = abs(x[2] - x[1]);
				dx[2] = abs(x[3] - x[2]);
				
				cvSet2D(pDiff,  top+k,left-1, CV_RGB(0, dx[0]*5, 0));
				cvSet2D(pDiff,  top+k,left, CV_RGB(0, dx[0]*5, 0));
				cvSet2D(pDiff, top+k,left+1,  CV_RGB(0, dx[0]*5, 0));
				ddx += min(dx[1] - dx[0], dx[1] - dx[2]);
			}

			ddx /= 16;
			mb[i*w16+j][LEFT] = ddx;
			if (j > 0){
				mb[i*w16+j-1][RIGHT] = ddx;
			}

			cvLine(p2Diff, cvPoint(left, top), cvPoint(left+15, top),   CV_RGB(0, ddy*10, 0), 1, CV_AA, 0);
			cvLine(p2Diff, cvPoint(left, top), cvPoint(left, top+15), CV_RGB(0, ddx*10, 0), 1, CV_AA, 0);
		}
	}

	cvShowImage("1-degree gradient", pDiff);
	cvShowImage("2-degree gradient", p2Diff);

	cvSaveImage("1-degree-gradient.jpg", pDiff);
	cvSaveImage("2-degree-gradient.jpg", p2Diff);

	cvNamedWindow("image");
	int val = 10;
	cvCreateTrackbar("tracker", "image", &val, 50, on_tracker);
	on_tracker(val);
	cvWaitKey(0);
	cvReleaseImage(&pImage);
	cvReleaseImage(&pDiff);
	cvReleaseImage(&p2Diff);
	return 0;
}
