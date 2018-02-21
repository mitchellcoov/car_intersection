#importing some useful packages
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import cv2
import os
import math

#uses Eddie's code, breaks in Yellow Lines

#reading in an image
image_1 = cv2.imread('test_images/solidWhiteCurve.png')
image_2 = cv2.imread('test_images/solidWhiteRight.png')
image_3 = cv2.imread('test_images/solidYellowCurve.png')
image_4 = cv2.imread('test_images/solidYellowCurve2.png')
image_5 = cv2.imread('test_images/solidYellowLeft.png')
image_6 = cv2.imread('test_images/whiteCarLaneSwich.png')

images = [image_1, image_2, image_3, image_4, image_5, image_6]

image_og = cv2.imread('test_images/og_lane.png')

#1 - convert image to HSL
def to_hsv(img):
    return cv2.cvtColor(img, cv2.COLOR_RGB2HSV)   
def to_hsl(img):
    return cv2.cvtColor(img, cv2.COLOR_RGB2HLS)

#2 - isolate yellow and white from HSL
# Image should have already been converted to HSL color space
def isolate_yellow_hsl(img):
    # Caution - OpenCV encodes the data in ****HLS*** format
    # Lower value equivalent pure HSL is (30, 45, 15)
    low_threshold = np.array([15, 38, 115], dtype=np.uint8)
    # Higher value equivalent pure HSL is (75, 100, 80)
    high_threshold = np.array([35, 204, 255], dtype=np.uint8)  
    yellow_mask = cv2.inRange(img, low_threshold, high_threshold)
    return yellow_mask
                        
# Image should have already been converted to HSL color space
def isolate_white_hsl(img):
    # Caution - OpenCV encodes the data in ***HLS*** format
    # Lower value equivalent pure HSL is (30, 45, 15)
    low_threshold = np.array([0, 200, 0], dtype=np.uint8)
    # Higher value equivalent pure HSL is (360, 100, 100)
    high_threshold = np.array([180, 255, 255], dtype=np.uint8)  
    yellow_mask = cv2.inRange(img, low_threshold, high_threshold)
    return yellow_mask

#3 - combine isloated HSL with original image 
def combine_hsl_isolated_with_original(img, hsl_yellow, hsl_white):
    hsl_mask = cv2.bitwise_or(hsl_yellow, hsl_white)
    return cv2.bitwise_and(img, img, mask=hsl_mask)

def filter_img_hsl(img):
    hsl_img = to_hsl(img)
    hsl_yellow = isolate_yellow_hsl(hsl_img)
    hsl_white = isolate_white_hsl(hsl_img)
    return combine_hsl_isolated_with_original(img, hsl_yellow, hsl_white)

#4 - Convert image to grayscale for easier manipulation
def grayscale(img):
    return cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)

#5 - Apply Gaussian Blur to smoothen edges
def gaussian_blur(grayscale_img, kernel_size=3):
    return cv2.GaussianBlur(grayscale_img, (kernel_size, kernel_size), 0)

#6 - Apply Canny Edge Detection on smoothed gray image
def canny_edge_detector(blurred_img, low_threshold, high_threshold):
    return cv2.Canny(blurred_img, low_threshold, high_threshold)

#7 - Region Of Interest
def get_vertices_for_img(img):
    img_shape = img.shape
    height = img_shape[0]
    width = img_shape[1]

    vert = None
    
    if (width, height) == (960, 540):
        region_bottom_left = (130 ,img_shape[0] - 1)
        region_top_left = (410, 330)
        region_top_right = (650, 350)
        region_bottom_right = (img_shape[1] - 30,img_shape[0] - 1)
        vert = np.array([[region_bottom_left , region_top_left, region_top_right, region_bottom_right]], dtype=np.int32)
    else:
        region_bottom_left = (200 , 680)
        region_top_left = (600, 450)
        region_top_right = (750, 450)
        region_bottom_right = (1100, 650)
        vert = np.array([[region_bottom_left , region_top_left, region_top_right, region_bottom_right]], dtype=np.int32)

    return vert

def region_of_interest(img):
    #defining a blank mask to start with
    mask = np.zeros_like(img)   
        
    #defining a 3 channel or 1 channel color to fill the mask with depending on the input image
    if len(img.shape) > 2:
        channel_count = img.shape[2]  # i.e. 3 or 4 depending on your image
        ignore_mask_color = (255,) * channel_count
    else:
        ignore_mask_color = 255
        
    vert = get_vertices_for_img(img)    
        
    #filling pixels inside the polygon defined by "vertices" with the fill color    
    cv2.fillPoly(mask, vert, ignore_mask_color)
    
    #returning the image only where mask pixels are nonzero
    masked_image = cv2.bitwise_and(img, mask)
    return masked_image


#8 -Hough Transform
rho = 1
# 1 degree
theta = (np.pi/180) * 1
threshold = 15
min_line_length = 20
max_line_gap = 10

#this function returns lines, not an IMAGE
def hough_transform(canny_img, rho, theta, threshold, min_line_length, max_line_gap):
    return cv2.HoughLinesP(canny_img, rho, theta, threshold, np.array([]), minLineLength=min_line_length, maxLineGap=max_line_gap)


#9 - Separate left and right lanes
def draw_lines(img, lines, color=[0, 0, 255], thickness=10, make_copy=True):
    # Copy the passed image
    img_copy = np.copy(img) if make_copy else img
    
    for line in lines:
        for x1,y1,x2,y2 in line:
            cv2.line(img_copy, (x1, y1), (x2, y2), color, thickness)
    
    return img_copy

def separate_lines(lines, img):
    img_shape = img.shape
    
    middle_x = img_shape[1] / 2
    
    left_lane_lines = []
    right_lane_lines = []

    for line in lines:
        for x1, y1, x2, y2 in line:
            dx = x2 - x1 
            if dx == 0:
                #Discarding line since we can't gradient is undefined at this dx
                continue
            dy = y2 - y1
            
            # Similarly, if the y value remains constant as x increases, discard line
            if dy == 0:
                continue
            
            slope = dy / dx
            
            # This is pure guess than anything... 
            # but get rid of lines with a small slope as they are likely to be horizontal one
            epsilon = 0.1
            if abs(slope) <= epsilon:
                continue
            
            if slope < 0 and x1 < middle_x and x2 < middle_x:
                # Lane should also be within the left hand side of region of interest
                left_lane_lines.append([[x1, y1, x2, y2]])
            elif x1 >= middle_x and x2 >= middle_x:
                # Lane should also be within the right hand side of region of interest
                right_lane_lines.append([[x1, y1, x2, y2]])
    
    return left_lane_lines, right_lane_lines

def color_lanes(img, left_lane_lines, right_lane_lines, left_lane_color=[255, 0, 0], right_lane_color=[0, 0, 255]):
    left_colored_img = draw_lines(img, left_lane_lines, color=left_lane_color, make_copy=True)
    right_colored_img = draw_lines(left_colored_img, right_lane_lines, color=right_lane_color, make_copy=False)
    
    return right_colored_img

#10 - Interpolate line gradients to create two smooth lines
from scipy import stats

def find_lane_lines_formula(lines):
    xs = []
    ys = []
    print(lines)
    for line in lines:
        for x1, y1, x2, y2 in line:
            xs.append(x1)
            xs.append(x2)
            ys.append(y1)
            ys.append(y2)
    slope, intercept, r_value, p_value, std_err = stats.linregress(xs, ys)
    
    # Remember, a straight line is expressed as f(x) = Ax + b. Slope is the A, while intercept is the b
    return (slope, intercept)


def trace_lane_line(img, lines, top_y, make_copy=True):

    A, b = find_lane_lines_formula(lines)
    vert = get_vertices_for_img(img)

    img_shape = img.shape
    bottom_y = img_shape[0] - 1
    # y = Ax + b, therefore x = (y - b) / A
    x_to_bottom_y = (bottom_y - b) / A
    
    top_x_to_y = (top_y - b) / A 
    
    new_lines = [[[int(x_to_bottom_y), int(bottom_y), int(top_x_to_y), int(top_y)]]]
    return draw_lines(img, new_lines, make_copy=make_copy)

def trace_both_lane_lines(img, left_lane_lines, right_lane_lines):
    vert = get_vertices_for_img(img)
    region_top_left = vert[0][1]

    full_left_lane_img = trace_lane_line(img, left_lane_lines, region_top_left[1], make_copy=True)
    full_left_right_lanes_img = trace_lane_line(full_left_lane_img, right_lane_lines, region_top_left[1], make_copy=False)
    
    # image1 * α + image2 * β + λ
    # image1 and image2 must be the same shape.
    img_with_lane_weight =  cv2.addWeighted(img, 0.7, full_left_right_lanes_img, 0.3, 0.0)
    
    return img_with_lane_weight




'''
filtered_image = filter_img_hsl(image_1)
gray_image = grayscale(filtered_image)
gauss_image = gaussian_blur(gray_image, kernel_size=5)
canny_image = canny_edge_detector(gauss_image,50,150)
roi_image = region_of_interest(canny_image)
hough_image_lines = hough_transform(roi_image, rho, theta, threshold, min_line_length, max_line_gap)
line_image = draw_lines(image_1, hough_image_lines, color=[0, 0, 255], thickness=10, make_copy=True)

left_lane, right_lane = separate_lines(hough_image_lines, image_1)
color_image = color_lanes(image_1, left_lane, right_lane, left_lane_color=[255, 0, 0], right_lane_color=[0, 0, 255])
full_lane_image = trace_both_lane_lines(image_1, left_lane, right_lane)
 '''                                

def pipeline(input_image):
    filtered_image = filter_img_hsl(input_image)
    gray_image = grayscale(filtered_image)
    gauss_image = gaussian_blur(gray_image, kernel_size=5)
    canny_image = canny_edge_detector(gauss_image,50,150)
    roi_image = region_of_interest(canny_image)
    hough_image_lines = hough_transform(roi_image, rho, theta, threshold, min_line_length, max_line_gap)
    line_image = draw_lines(input_image, hough_image_lines, color=[0, 0, 255], thickness=10, make_copy=True)

    left_lane, right_lane = separate_lines(hough_image_lines, input_image)
    color_image = color_lanes(input_image, left_lane, right_lane, left_lane_color=[255, 0, 0], right_lane_color=[0, 0, 255])
    full_lane_image = trace_both_lane_lines(input_image, left_lane, right_lane)
    return full_lane_image


#cv2.imwrite('output_images/Image'+str(0)+'.png', pipeline(images[0]))
#cv2.imwrite('output_images/Image'+str(1)+'.png', pipeline(images[1]))


#cv2.imwrite('output_images/Image'+str(2)+'.png', pipeline(images[2]))
#cv2.imwrite('output_images/Image'+str(3)+'.png', pipeline(images[3]))

#cv2.imwrite('output_images/Image'+str(4)+'.png', pipeline(images[4]))
#cv2.imwrite('output_images/Image'+str(5)+'.png', pipeline(images[5]))

cv2.imwrite('output_images/ImageOG.png', pipeline(image_og))