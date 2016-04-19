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

		f.close();

		dump("Done");
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}    

	std::time_t t_finish = std::time(0);
	// f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	// f<<"Прошло " + to_string(t_finish - t_start) + " секунд."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	return 0; 
}