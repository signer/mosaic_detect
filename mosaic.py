#!/usr/bin/python
# -*- coding: utf-8 -*-
#


import cv

h1 = 8
h2 = 14
BLOCK_SIZE = 16

COLOR1 = cv.CV_RGB(0,0,255)

img = cv.LoadImage("./frame_20110108_154215.jpg", 0)
size = (img.width, img.height)
edge = cv.CreateImage(size, 8, 1)
xcolor = cv.CreateImage(size, 8, 3)
ycolor = cv.CreateImage(size, 8, 3)

w = int(img.width/BLOCK_SIZE)
h = int(img.height/BLOCK_SIZE)

print 'width=', img.width
print 'height=', img.height
print 'w=', w
print 'h=', h
print img.width%BLOCK_SIZE, img.height%BLOCK_SIZE

# draw grid lines
# vertical-lines

def ComputeDiff(image):
	w = image.width
	h = image.height
	xDiff = cv.CreateImage((w, h), 8, 1)
	yDiff = cv.CloneImage(xDiff)
	for i in range(0,h-1):
		for j in range(0,w-1):
			vx = abs(image[i+1, j] - image[i, j])
			vy = abs(image[i, j+1] - image[i, j])	
			xDiff[i, j] = vx
			yDiff[i, j] = vy
	return (xDiff, yDiff)	


#	tmp = [0]*16
#	# x-gradient
#	for i in range(h-1):
#		# horizontal lines
#		for k in range(w/16):
#			for l in range(16):
#				tmp[l] = abs(image[i+1, 16*k+l] - image[i, 16*k+l])
#			v = sum(tmp)/16
#			for l in range(16):
#				iDiff[i, 16*k+l] = v
#	for j in range(w-1):
#		# vertical lines
#		for k in range(h/16):
#			for l in range(16):
#				tmp[l] = abs(image[16*k+l, j+1] - image[16*k+l, j])
#			v = sum(tmp)/16
#			for l in range(16):
#				iDiff[16*k+l, j] += v
#	return iDiff

iDiff = ComputeDiff(img)
xDiff = iDiff[0]
yDiff = iDiff[1]
cv.ShowImage("xdiff", iDiff[0])
cv.ShowImage("ydiff", iDiff[1])
font = cv.InitFont(cv.CV_FONT_HERSHEY_PLAIN, 0.7, 0.7)
#for x in range(1,w):
#	cv.Line(color, (x*BLOCK_SIZE-1, 0), (x*BLOCK_SIZE-1, img.height-1), COLOR1, 1, 4)
#for y in range(1,h):
#	cv.Line(color, (0, y*BLOCK_SIZE-1), (img.width-1, y*BLOCK_SIZE-1), COLOR1, 1, 4)

def ont(th):
	cv.CvtColor(iDiff[0], xcolor, cv.CV_GRAY2BGR)
	mb = [0]*(w*h)
	# h*w macroblocks
	# compute the average gradient of the 4 edges

	for i in range(h):
		for j in range(w):
			top = i*16-1
			left = j*16-1
			vtop = 0
			vleft = 0
			if (top > 1):
				for k in range(16):
					a = xDiff[top, left+k] - xDiff[top-1, left+k]
					b = xDiff[top, left+k] - xDiff[top+1, left+k]
					vtop += min(a, b)
			if (left > 1):
				for k in range(16):	
					a = yDiff[top+k, left] - yDiff[top+k, left-1]
					b = yDiff[top+k, left] - yDiff[top+k, left+1]
					vleft += min(a, b)
		
			vtop /= 16
			vleft /= 16
			if (vtop >= th):
				cv.Line(xcolor, (left, top), (left+15, top), cv.CV_RGB(255, 0, 0), 
						1, 4)
				cv.Line(img, (left, top), (left+15, top), cv.CV_RGB(255, 0, 0), 
						1, 4)
				mb[i*w+j] += 1
				if (i>0): mb[(i-1)*w+j] += 1
			if (vleft >= th):
				cv.Line(xcolor, (left, top), (left, top+15), cv.CV_RGB(255, 0, 0), 
						1, 4)
				cv.Line(img, (left, top), (left+15, top), cv.CV_RGB(255, 0, 0), 
						1, 4)
				mb[i*w+j] += 1
				if (j>0): mb[i*w+j-1] += 1
			cv.Line(xcolor, (left, top), (left, top), cv.CV_RGB(255, 0, 0), 1, 4)
			cv.Line(img, (left, top), (left, top), cv.CV_RGB(255, 0, 0), 1, 4)
	
	mosaic_number = 0
	for i in range(h):
		for j in range(w):
			if (mb[i*w+j] >= 3):
				mosaic_number += 1
				cv.PutText(xcolor, 'M', (left+1, top+12), font, 
						cv.CV_RGB(0, 255, 0))
	print "mosaic=", mosaic_number
	cv.ShowImage("diff", xcolor)
	cv.ShowImage("image", img)
	return 

# show images
cv.NamedWindow('diff')
cv.ShowImage('image', img)
cv.CreateTrackbar('edge-threshold1', 'diff', 14, 60, ont)
#cv.CreateTrackbar('edge-threshold2', 'edge', 1, 300, ontracker2)
ont(14)

cv.WaitKey(0)
