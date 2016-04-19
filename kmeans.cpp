#include <ImageMagick-6/Magick++.h>
#include <iostream>
using namespace std;
using namespace Magick;
#include <fstream>
#include <iomanip>
#include <list>     // подключаем заголовок списка
#include <iterator> // заголовок итераторов
#include <ctime>
#include <sys/stat.h>
#include <tiffio.h>
#include <vector>
#include <utility>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

// #include "cv.h"
// #include "cxcore.h"
// #include "highgui.h"

#include "cluster.hpp"
#include "cluster.cpp"

#define MAX_CLUSTERS 5

// using namespace cv;
// using namespace std;

using namespace Clustering;

// g++ kmeans.cpp -o kmeans `Magick++-config --cppflags --cxxflags --ldflags --libs`

void dump(string variable){
	cout << variable << endl;
}

void dump(int variable){
	cout << to_string(variable) << endl;
}

void dump(double variable){
	cout << to_string(variable) << endl;
}

double Round(double number, double accuracy){
	int quotient = number / accuracy;
	if ((quotient+1)*accuracy - number < number - quotient*accuracy)
		quotient++;
	return quotient*accuracy;
}

/*#include <iostream>
#include "cluster.hpp"

using namespace Clustering;

int main(int argc, char* argv[])
{
  ClusterId num_clusters = 30;
  PointId num_points = 3000;
  Dimensions num_dimensions = 10;

  PointsSpace ps(num_points, num_dimensions);
  //std::cout << "PointSpace" << ps;

  Clusters clusters(num_clusters, ps);

  clusters.k_means();
  
  std::cout << clusters;

}*/


int main(int argc,char **argv){
	dump("Started.");
	std::time_t t_start = std::time(0);
	string path = "out/" + to_string(t_start);
	const char *path_ = path.c_str();
	mkdir(path_, 0777);
	
	//описывает поток для записи данных в файл
	ofstream f;
	//открываем файл в режиме записи, режим ios::out устанавливается по умолчанию
	f.open(path + "/output.txt", ios::app);

	InitializeMagick(*argv);
	// Construct the image object. Seperating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	Image src_image;
	Image seg_test;

	//misc	
	// string imagepath = "in/";
	string imagepath = "in/12 small.jpg";

	bool colourfull = false;

	try{
		// Read a file into image object
		src_image.read(imagepath);

		Pixels src_view(src_image);

		int width = src_image.size().width();
		int height = src_image.size().height();
		int square = width * height;

		//dump end

		//@TODO: научиться создавать картинки нормально
		Image dst_image(Geometry(width, height), ColorRGB(MaxRGB, MaxRGB, MaxRGB));
		//dst_image.type(TrueColorType);
		dst_image.modifyImage();
		Pixels dst_view(dst_image);

		//Declare a model of an image.
		unsigned short **channel_R = new unsigned short*[height];
		unsigned short **channel_G = new unsigned short*[height];
		unsigned short **channel_B = new unsigned short*[height];
		unsigned short **channel_Grey = new unsigned short*[height];
		for(int i = 0; i < height; i++) {
			channel_R[i] = new unsigned short[width];
			channel_G[i] = new unsigned short[width];
			channel_B[i] = new unsigned short[width];
			channel_Grey[i] = new unsigned short[width];
		}

		//Declare a model of a result image.
		unsigned short **channel_R_S = new unsigned short*[height];
		unsigned short **channel_G_S = new unsigned short*[height];
		unsigned short **channel_B_S = new unsigned short*[height];
		unsigned short **channel_Grey_S = new unsigned short*[height];
		for(int i = 0; i < height; i++) {
			channel_R_S[i] = new unsigned short[width];
			channel_G_S[i] = new unsigned short[width];
			channel_B_S[i] = new unsigned short[width];
			channel_Grey_S[i] = new unsigned short[width];
		}

		Color pixel_color = *(src_view.get(1,1,1,1));
		ColorRGB pixel_rgb(pixel_color);

		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				pixel_color = *(src_view.get(column,row,1,1)); //вот это нельяз ли упростить
				ColorRGB pixel_rgb(pixel_color);

				channel_R[row][column] = (unsigned short) 255*pixel_rgb.red();
				channel_G[row][column] = (unsigned short) 255*pixel_rgb.green();
				channel_B[row][column] = (unsigned short) 255*pixel_rgb.blue();
					
				channel_Grey[row][column] = (unsigned short) (channel_R[row][column]+channel_G[row][column]+channel_B[row][column])/3;
			}
		}
/*
        // изображение для показа точек 
        IplImage* img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );

        // таблица цветов кластеров
        CvScalar color_tab[MAX_CLUSTERS];
        color_tab[0] = CV_RGB(255,0,0);
        color_tab[1] = CV_RGB(0,255,0);
        color_tab[2] = CV_RGB(100,100,255);
        color_tab[3] = CV_RGB(255,0,255);
        color_tab[4] = CV_RGB(255,255,0);
        
        // инициализация состояния ГПСЧ
        CvRNG rng = cvRNG(0xffffffff);

        cvNamedWindow( "clusters", 1 );

        for(;;){
                int k, cluster_count = cvRandInt(&rng)%MAX_CLUSTERS + 1;
                int i, sample_count = cvRandInt(&rng)%1000 + 1;
                CvMat* points = cvCreateMat( sample_count, 1, CV_32FC2 );
                CvMat* clusters = cvCreateMat( sample_count, 1, CV_32SC1 );

                // генерация случайного гауссового распределения точек
                for( k = 0; k < cluster_count; k++ ){
                        CvPoint center;
                        CvMat point_chunk;
                        center.x = cvRandInt(&rng)%img->width;
                        center.y = cvRandInt(&rng)%img->height;
                        cvGetRows( points, &point_chunk, 
                                k*sample_count/cluster_count,
                                k == cluster_count - 1 ? sample_count :  
                                (k+1)*sample_count/cluster_count );
                        cvRandArr( &rng, &point_chunk, CV_RAND_NORMAL,
                                cvScalar(center.x,center.y,0,0),
                                cvScalar(img->width/6, img->height/6,0,0) );
                }

                // точки перемешиваются
                for( i = 0; i < sample_count/2; i++ ){
                        CvPoint2D32f* pt1 = (CvPoint2D32f*)points->data.fl +
                                cvRandInt(&rng)%sample_count;
                        CvPoint2D32f* pt2 = (CvPoint2D32f*)points->data.fl + 
                                cvRandInt(&rng)%sample_count;
                        CvPoint2D32f temp;
                        CV_SWAP( *pt1, *pt2, temp );
                }

                // определение кластеров
                cvKMeans2( points, cluster_count, clusters,
                        cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 
                        10, 1.0 ));
                cvZero( img );

                // показываем точки
                for( i = 0; i < sample_count; i++ ){
                        CvPoint2D32f pt = ((CvPoint2D32f*)points->data.fl)[i];
                        // индекс кластера
                        int cluster_idx = clusters->data.i[i];
                        cvCircle( img, cvPointFrom32f(pt), 2, color_tab[cluster_idx], CV_FILLED );
                }

                cvReleaseMat( &points );
                cvReleaseMat( &clusters );

                // показываем
                cvShowImage( "clusters", img );

                int key = cvWaitKey(330);
                if( key == 27 ){ // 'ESC'
                        break;
                }
        }
        
        // освобождаем ресурсы
        cvReleaseImage(&img);
        // удаляем окна
        cvDestroyAllWindows();
*/

		//SEGMENTATION
		if (colourfull){
		}
		else{
		}
/*		//рисуем итоговую карту
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				pixel_color = ColorRGB((double) channel_R_S[row][column]/255, (double) channel_G_S[row][column]/255, (double) channel_B_S[row][column]/255);
				pixel_color = ColorRGB((double) channel_Grey_S[row][column]/255, (double) channel_Grey_S[row][column]/255, (double) channel_Grey_S[row][column]/255);
				*(dst_view.get(column,row,1,1)) = pixel_color;
			}
		}*/

		//Destructing the model of an image
		for(int i = 0; i < height; ++i) {
			delete [] channel_R[i];
			delete [] channel_G[i];
			delete [] channel_B[i];
			delete [] channel_Grey[i];
			delete [] channel_R_S[i];
			delete [] channel_G_S[i];
			delete [] channel_B_S[i];
			delete [] channel_Grey_S[i];
		}
		delete [] channel_R;
		delete [] channel_G;
		delete [] channel_B;
		delete [] channel_Grey;
		delete [] channel_R_S;
		delete [] channel_G_S;
		delete [] channel_B_S;
		delete [] channel_Grey_S;

		dst_view.sync();
		//запись результата
		dst_image.write(path + "/output.bmp");
		src_image.write(path + "/initial.jpg");

		dump("Done");
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}

/*  
работает, но что с этим делать
  ClusterId num_clusters = 3;
  PointId num_points = 300;
  Dimensions num_dimensions = 2;

  PointsSpace ps(num_points, num_dimensions);
  // std::cout << "PointSpace" << ps;
  f << "PointSpace" << ps;
  f << endl;
  Clusters clusters(num_clusters, ps);

  clusters.k_means();
  
  // std::cout << clusters;
  f << clusters;
  f<<endl;	*/

	std::time_t t_finish = std::time(0);
	// f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	f<<"Прошло " + to_string(t_finish - t_start) + " секунд."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	f.close();
	return 0; 
}