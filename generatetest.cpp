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


#include <string.h>
#include "stdio.h"
// #include <boost/filesystem.hpp>

// g++ generatetest.cpp -o generatetest `Magick++-config --cppflags --cxxflags --ldflags --libs`

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

void makeTest(string path, Image image_map, string info){
	ofstream f;
	f.open(path + "/output.txt", ios::app);	
	// dump(info);
	
	std::list<unsigned short> colours_;
	std::list<unsigned short>::iterator it_ = colours_.begin();	
	int index;

	bool thereis = false;

	Pixels map(image_map);

	try{
		int width = image_map.size().width();
		int height = image_map.size().height();
		// int square = width * height;

		Image dst_image(Geometry(width, height), ColorRGB(MaxRGB, MaxRGB, MaxRGB));
		dst_image.modifyImage();
		Pixels dst_view(dst_image);

		//Declare a model of an etalon image.
		unsigned short **channel_R = new unsigned short*[height];
		unsigned short **channel_G = new unsigned short*[height];
		unsigned short **channel_B = new unsigned short*[height];
		for(int i = 0; i < height; i++) {
			channel_R[i] = new unsigned short[width];
			channel_G[i] = new unsigned short[width];
			channel_B[i] = new unsigned short[width];
		}

		//Declare a model of a secondary image.
		unsigned short **channel_R_S = new unsigned short*[height];
		unsigned short **channel_G_S = new unsigned short*[height];
		unsigned short **channel_B_S = new unsigned short*[height];
		for(int i = 0; i < height; i++) {
			channel_R_S[i] = new unsigned short[width];
			channel_G_S[i] = new unsigned short[width];
			channel_B_S[i] = new unsigned short[width];
		}

		Color pixel_color_ = *(dst_view.get(1,1,1,1));
		ColorRGB pixel_rgb_(pixel_color_);

		Color pixel_color = *(map.get(1,1,1,1));
		ColorRGB pixel_rgb(pixel_color);

		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				pixel_color = *(map.get(column,row,1,1)); //вот это нельяз ли упростить
				ColorRGB pixel_rgb(pixel_color);					
				channel_R[row][column] = (unsigned short) 255*pixel_rgb.red();
				channel_G[row][column] = (unsigned short) 255*pixel_rgb.green();
				channel_B[row][column] = (unsigned short) 255*pixel_rgb.blue();		

				pixel_color_ = *(dst_view.get(column,row,1,1)); //вот это нельяз ли упростить
	 				ColorRGB pixel_rgb_(pixel_color_);
				channel_R_S[row][column] = 0;
				channel_G_S[row][column] = 0;
				channel_B_S[row][column] = 0;
			}
		}

		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){								
				thereis = false;
	  			for (std::list<unsigned short>::iterator it = colours_.begin(); it != colours_.end(); ++it)
	  				if (*it == channel_R[row][column])
	  					thereis = true;  		
	  			if (!thereis)
					colours_.push_back(channel_R[row][column]);
			}
		}

		int *colours_array = new int[colours_.size()];
		double *pi_array = new double[colours_.size()];
		index = 0;
		string temp;
		for (std::list<unsigned short>::iterator it = colours_.begin(); it != colours_.end(); ++it){
			colours_array[index] = *it;
			dump("Enter a " + to_string(index) + " probability (0-100)");
			cin>>temp;
			try{
				pi_array[index] = stod(temp, 0);
				pi_array[index] = (double) pi_array[index] / 100;
				if (pi_array[index] < 0)
					pi_array[index] = 0;
				if (pi_array[index] > 1)
					pi_array[index] = 1;
				dump ("The " + to_string(index) + " probability is " + to_string(pi_array[index]));
				f<<("The " + to_string(index) + " probability is " + to_string(pi_array[index]))<<endl;
				index++;
			}
			catch( Exception &error_ ){       
				cout << "Caught exception: " << error_.what() << endl;
				f << "Caught exception: " << error_.what() << endl;
			}  			
		}
		dump("Ok");

		//generation
		// cout << "RAND_MAX = " << RAND_MAX << endl;
		srand(time(0));
/*		for ( ssize_t row = 0; row < 2 ; row++ ){
			// cout << "random number = " << (double) rand()/RAND_MAX << endl;
			for ( ssize_t column = 0; column < 2 ; column++ ){
				for (int i = 0; i < colours_.size(); i++)
					if (channel_R[row][column] == colours_array[i]){
						if (pi_array[i] < (double) rand()/RAND_MAX){
							channel_R_S[row][column] = 255;
						}
					}
				channel_B_S[row][column] = channel_R_S[row][column]; //
				channel_G_S[row][column] = channel_R_S[row][column]; //
			}
		}*/

		for ( ssize_t row = 1; row < height ; row++ ){
			for (int i = 0; i < colours_.size(); i++)
				if (channel_R[row][0] == colours_array[i]){
					if (pi_array[i] < (double) rand()/RAND_MAX){
						channel_R_S[row][0] = 255 - channel_R_S[row-1][0];
					}
					else{
						channel_R_S[row][0] = channel_R_S[row-1][0];
					}
				}
			channel_B_S[row][0] = channel_R_S[row][0]; //
			channel_G_S[row][0] = channel_R_S[row][0]; //
		}

		for ( ssize_t column = 1; column < width ; column++ ){
			for (int i = 0; i < colours_.size(); i++)
				if (channel_R[0][column] == colours_array[i]){
					if (pi_array[i] < (double) rand()/RAND_MAX){
						channel_R_S[0][column] = 255 - channel_R_S[0][column-1];
					}
					else{
						channel_R_S[0][column] = channel_R_S[0][column-1];
					}
				}
			channel_B_S[0][column] = channel_R_S[0][column]; //
			channel_G_S[0][column] = channel_R_S[0][column]; //
		}

		for ( ssize_t row = 1; row < height ; row++ ){
			for ( ssize_t column = 1; column < width ; column++ ){
				for (int i = 0; i < colours_.size(); i++)
					if (channel_R[row][column] == colours_array[i]){
						if (channel_R_S[row][column-1] != channel_R_S[row-1][column]){
							if (0.5 > (double) rand()/RAND_MAX){
								channel_R_S[row][column] = channel_R_S[row][column-1];
							}
							else
							{
								channel_R_S[row][column] = 255 - channel_R_S[row][column-1];
							}																				
						}
						else{
							if (pi_array[i] < (double) rand()/RAND_MAX){
								channel_R_S[row][column] = 255 - channel_R_S[row][column-1];
							}
							else
							{
								channel_R_S[row][column] = channel_R_S[row][column-1];
							}													
						}
					}
				channel_B_S[row][column] = channel_R_S[row][column]; //
				channel_G_S[row][column] = channel_R_S[row][column]; //
			}
		}

		Color pixel_color__ = *(map.get(1,1,1,1));
		// ColorRGB pixel_rgb__(pixel_color__);

		//рисуем итоговую разницу карт
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				pixel_color__ = ColorRGB((double) channel_R_S[row][column]/255, (double) channel_G_S[row][column]/255, (double) channel_B_S[row][column]/255);
				*(dst_view.get(column,row,1,1)) = pixel_color__;
			}
		}		

		dst_view.sync();
		dst_image.write(path + "/test" + info + ".bmp");

		//Destructing the model of an image
		for(int i = 0; i < height; ++i) {
			delete [] channel_R[i];
			delete [] channel_G[i];
			delete [] channel_B[i];
			delete [] channel_R_S[i];
			delete [] channel_G_S[i];
			delete [] channel_B_S[i];
		}
		delete [] channel_R;
		delete [] channel_G;
		delete [] channel_B;
		delete [] channel_R_S;
		delete [] channel_G_S;
		delete [] channel_B_S;

	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}    		
}

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

	Image image_map;	

	//path
	string imagepath_etalon = "in/map.bmp";

	try{
		image_map.read(imagepath_etalon);
		image_map.write(path + "/map.bmp");
		makeTest(path, image_map, "");

		f.close();

		dump("Done"); 				
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}  

	std::time_t t_finish = std::time(0);
	f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	return 0; 
}