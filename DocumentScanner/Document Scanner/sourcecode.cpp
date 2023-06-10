#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>


using namespace cv;
using namespace std;

Mat imgOriginal, imggray, imgcanny, imgblur, imgdil, imgerode, imgWarp, imgCrop;
vector<Point> initialPoints; vector<Point> docPoints;

float w = 420, h = 596;

Mat preProcessing(Mat img)
{
	cvtColor(img, imggray, COLOR_BGR2GRAY);
	GaussianBlur(imggray, imgblur, Size(5, 5), 7, 0);
	Canny(imgblur, imgcanny, 25, 75);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgcanny, imgdil, kernel);
	/*erode(imgdil, imgerode, kernel);*/

	return imgdil;
}

vector<Point> getContours(Mat image)
{

	vector < vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	/*drawContours(img, contours, -1, Scalar(255, 0, 255), 5);*/

	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;

	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size() == 4)
			{
				/*drawContours(imgOriginal, contours, i, Scalar(255, 0, 255), 5);*/
				biggest = { conPoly[i][0],conPoly[i][1], conPoly[i][2], conPoly[i][3] };
				maxArea = area;
			}

			/*drawContours(imgOriginal, contours, i, Scalar(255, 0, 255), 5);*/
			/*rectangle(image, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);*/

		}
	}
	return biggest;
}

void drawPoints(vector<Point> points, Scalar color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(imgOriginal, points[i], 5, color, FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 5, color, 5);
	}
}

vector<Point> reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}
	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);//0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);//1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);//2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);//3

	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{

	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

void main()
{

	string path = "Resources/paper.jpg";
	imgOriginal = imread(path);

	resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);



	//Image PreProcessing//

	Mat thres = preProcessing(imgOriginal);



	//Get Contours-Biggest//

	initialPoints = getContours(thres);
	/*drawPoints(initialPoints, Scalar(0, 0, 255));*/
	docPoints = reorder(initialPoints);
	/*drawPoints(docPoints, Scalar(0, 255, 0));*/



	//Warp//

	imgWarp = getWarp(imgOriginal, docPoints, w, h);


	//crop//
	Rect roi(5, 5, w - (5 * 2), h - (5 * 2));
	imgCrop = imgWarp(roi);

	imshow("IMAGE", imgOriginal);
	imshow("PreProcessed Image", imgdil);
	imshow("Warp Image", imgWarp);
	imshow("Crop Image", imgCrop);
	waitKey(0);
}