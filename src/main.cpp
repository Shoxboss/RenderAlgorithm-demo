#include <string>
#include <stdio.h>
#include <optional>
#include <iostream>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;
#define SCALE 1.0
#define SQUEEZE 0.5
#define UNCLENCH 2.0
namespace colors
{
    Scalar GREEN = Scalar(0, 255, 0);
    Scalar WHITE = Scalar(255, 255, 255);
    Scalar BLACK = Scalar(0, 0, 0);
}

namespace locals
{

    void writeImg(string filename, Mat img)
    {
        imwrite(filename, img);
    }

    Mat readImage(string filename)
    {

        Mat src = imread(samples::findFile(filename), IMREAD_COLOR);

        if (src.empty())
        {

            printf(" Error opening image\n");
            printf(" Program Arguments: [image_name -- default %s] \n", filename.c_str());
            exit(0);
        }

        return src;
    }

    Mat Scale(Mat img, float scale)
    {
        Mat scaled;
        Size newSize = Size(img.size().width * scale, img.size().height * scale);
        resize(img, scaled, newSize, 0, 0, INTER_LINEAR);
        return scaled;
    }

    Mat Resize(Mat img, Size newSize)
    {
        Mat resized;
        resize(img, resized, newSize, 0, 0, INTER_LINEAR);
        return resized;
    }

    Mat Disguise(Mat img)
    {

        Mat mask(img.size(), img.type(), colors::WHITE);

        Mat mask_for_mask(mask.size(), mask.type(), colors::BLACK);

        Point center = Point(
            mask_for_mask.size().width / 2, // x
            mask_for_mask.size().height / 2 // y
        );

        int radus = mask_for_mask.size().height / 2 < mask_for_mask.size().width / 2
                        ? mask_for_mask.size().height / 2
                        : mask_for_mask.size().width / 2;

        circle(mask_for_mask, center, radus+radus/4, colors::WHITE, FILLED);
        Resize(mask_for_mask, img.size());

        img.copyTo(mask, mask_for_mask);

        return mask;
    }
}

void show(string title, Mat img)
{
    imshow(title, img);
    waitKey();
}

int dist(int ax, int ay, int bx, int by)
{
    return sqrt((bx - ax) ^ 2 + (by - ay) ^ 2);
}

Point getCenter(Point2f a, Point2f b) {
    return Point((a.x + b.x)/2, (a.y+b.y)/2);
}

double getLength(Point a, Point b){

    double y = (a.y-b.y);
    double x = (a.x-b.x);
    return sqrt(x*x+y*y);
}


Mat findCounters(Mat img) {
    
    int thresh = 100;
    Mat canny_output;
    
    Canny(img, canny_output, thresh, thresh);

    vector<vector<Point>> contours;
    vector<Point> bigger;
    vector<Vec4i> hierarchy;

    findContours(canny_output, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);

    int largest_area = 0;
    int largest_contour_index = 0;

    for (int i = 0; i < contours.size(); i++)
    {        

        drawContours(drawing, contours, i, Scalar(255, 255, 255), 0, 8, hierarchy);

        // double a = contourArea(contours[i], false);
        // if (a >= largest_area)
        // {
        //     largest_area = a;
        //     largest_contour_index = i;                        
        // }
    }

    // drawContours(drawing, contours, largest_contour_index, Scalar(255, 255, 255), 0, 8, hierarchy);

    // RotatedRect elp = fitEllipse( contours[largest_contour_index] );
    // Point2f* points= new Point2f[4];
    // elp.points(points);

    // cout<< getLength( points[0],  points[1] ) << endl;
    // cout<< getLength( points[1],  points[2] ) << endl;

    // ellipse( drawing, elp, Scalar(0, 255, 0, 0.5));
    // line( drawing, getCenter( points[0],  points[3]), getCenter(points[2],  points[1]) ,Scalar(0, 255, 255), 1);
    // line( drawing, getCenter( points[0],  points[1]), getCenter(points[2],  points[3]) ,Scalar(0, 255, 255), 1);
    


    // putText( 
    //     drawing, 
    //     to_string(getLength( points[1],  points[2] )), 
    //     getCenter( points[0],  points[3]), 
    //     FONT_HERSHEY_COMPLEX_SMALL, 
    //     1, 
    //     Scalar(0, 255, 255)
    // );


    return drawing;
}



Mat thresh_callback(Mat img)
{

    Mat res = findCounters( img );
    // equalizeHist(res, res);
    // show("some",res );

    // for (size_t i = 0; i < 3; i++)
    // {
    //     blur(res, res, Size(3, 3));
    //     res = findCounters( res );
    //     show("some",res );
    // }
    

    // ellipse( drawing, fitEllipse( contours[largest_contour_index] ), Scalar(0, 255, 0), 2 );

    // rectangle(drawing, bounding_rect, Scalar(0, 255, 0), 2, 8, 0);

    
    // show("Contours", );

    return locals::Resize(res, img.size());
}

void imgSave( Mat result, string filename ){

    string path = "./result/"+filename;
    imwrite(path, result);
}


int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Вы забыли указать обязательный параметр, имя файла");
        return 0;
    }

    string filename = argv[1];

    //------- read image -------------------------
    Mat src = locals::readImage("./src/"+filename);
    Mat image = src;

    int height = image.size().height;
    int width = image.size().width;


    // -------------------------------------------
    //*      squeeze --> resize
    // -------------------------------------------


    Mat squeezed = locals::Scale(image, 0.2);

    // imshow("сжатый", squeezed);


    //*------------[  disguise ]------------------

    Mat masked = locals::Disguise(squeezed);

    // imshow("masked", masked);




    //*-----------[ to grayscale ]----------------
    Mat grizzle;
    cvtColor(masked, grizzle, COLOR_BGR2GRAY);

    show("grizzle", grizzle);

    //! ----------- [ Blured ]--------------------

    GaussianBlur(grizzle, grizzle, Size(15, 15), 0, 0, 4);
    equalizeHist(grizzle, grizzle);
    medianBlur(grizzle, grizzle, 3);
    grizzle.convertTo(grizzle, -2, 4, 0);
    medianBlur(grizzle, grizzle, 9);


    Mat area;
    threshold(grizzle, area, 200, 255, THRESH_BINARY);
    // threshold( gray, area, 254, 255, 0);

    area = locals::Resize(area, Size(src.cols, src.rows));
    area = thresh_callback(area);
    // show("grizzle", area);

    Mat result(src.size(), src.type(), colors::WHITE);
    Mat tmp = locals::Resize( masked, src.size());

    src.copyTo( result, tmp);
    Mat line(src.size(), src.type(), colors::GREEN);
    line.copyTo(tmp, area);

    imgSave(tmp, filename);

    // show("grizzle", result);

    // disguise
    // clean
    // findContours
    // sortContours
    // findMean
    // drawOval
    // calculateArea

    return 0;
}
