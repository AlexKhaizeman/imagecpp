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
// #include <boost/filesystem.hpp>

// g++ stupidcompare.cpp -o stupidcompare `Magick++-config --cppflags --cxxflags --ldflags --libs`

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

void channelCompare(string path, string info, unsigned short **channel_R, unsigned short **channel_R_S, unsigned short **channel_R_C, int width, int height){
	ofstream f;
	f.open(path + "/output.txt", ios::app);	
	// dump(info);
	int square = width * height, index = 0;
	double seg_error=0;

    //раскраска
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			if (channel_R_S[row][column] != 255){
				channel_R_S[row][column] = 0;
			}
			if (channel_R[row][column] == channel_R_S[row][column])
				channel_R_C[row][column] = 255;
			else{
				seg_error++;
				channel_R_C[row][column] = 0;
			}				
		}
	}

	f<<info<<endl;

	// seg_error = square - seg_error;
	seg_error /= square/100;

    dump(info + " segmentation error " + to_string(seg_error) + "%");
    f<<"segmentation error " + to_string(seg_error) + "%"<<endl;
    // f<<"ошибка сегментации " + to_string(seg_error) + "%"<<endl;

}

void compareImages(string path, Image etalon_image, Image secondary_image, string info){
	ofstream f;
	f.open(path + "/output.txt", ios::app);	
	Pixels etalon(etalon_image);
	Pixels secondary(secondary_image);
	if (etalon_image.size() == secondary_image.size())
	{
		try{
			int width = etalon_image.size().width();
			int height = etalon_image.size().height();
			int square = width * height;

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

			//Declare a model of a comporation.
			unsigned short **channel_R_C = new unsigned short*[height];
			unsigned short **channel_G_C = new unsigned short*[height];
			unsigned short **channel_B_C = new unsigned short*[height];
			for(int i = 0; i < height; i++) {
				channel_R_C[i] = new unsigned short[width];
				channel_G_C[i] = new unsigned short[width];
				channel_B_C[i] = new unsigned short[width];
			}

			Color pixel_color = *(etalon.get(1,1,1,1));
			ColorRGB pixel_rgb(pixel_color);

			Color pixel_color_ = *(secondary.get(1,1,1,1));
			ColorRGB pixel_rgb_(pixel_color);

			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					pixel_color = *(etalon.get(column,row,1,1)); //вот это нельяз ли упростить
					ColorRGB pixel_rgb(pixel_color);					
					channel_R[row][column] = (unsigned short) 255*pixel_rgb.red();
					channel_G[row][column] = (unsigned short) 255*pixel_rgb.green();
					channel_B[row][column] = (unsigned short) 255*pixel_rgb.blue();		

					pixel_color_ = *(secondary.get(column,row,1,1)); //вот это нельяз ли упростить
 	 				ColorRGB pixel_rgb_(pixel_color_);
					channel_R_S[row][column] = (unsigned short) 255*pixel_rgb_.red();
					channel_G_S[row][column] = (unsigned short) 255*pixel_rgb_.green();
					channel_B_S[row][column] = (unsigned short) 255*pixel_rgb_.blue();								

					channel_R_C[row][column] = 0;
					channel_G_C[row][column] = 0;
					channel_B_C[row][column] = 0;
				}
			}

			channelCompare(path, "comparing test RED", channel_R, channel_R_S, channel_R_C, width, height);
			channelCompare(path, "comparing test GREEN", channel_G, channel_G_S, channel_G_C, width, height);
			channelCompare(path, "comparing test BLUE", channel_B, channel_B_S, channel_B_C, width, height);

			//рисуем итоговую разницу карт
			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					pixel_color = ColorRGB((double) channel_R_C[row][column]/255, (double) channel_G_C[row][column]/255, (double) channel_B_C[row][column]/255);
					*(dst_view.get(column,row,1,1)) = pixel_color;
				}
			}		

			dst_view.sync();
			dst_image.write(path + "/comporation " + info + ".bmp");

			//Destructing the model of an image
			for(int i = 0; i < height; ++i) {
				delete [] channel_R[i];
				delete [] channel_G[i];
				delete [] channel_B[i];
				delete [] channel_R_S[i];
				delete [] channel_G_S[i];
				delete [] channel_B_S[i];
				delete [] channel_R_C[i];
				delete [] channel_G_C[i];
				delete [] channel_B_C[i];
			}
			delete [] channel_R;
			delete [] channel_G;
			delete [] channel_B;
			delete [] channel_R_S;
			delete [] channel_G_S;
			delete [] channel_B_S;
			delete [] channel_R_C;
			delete [] channel_G_C;
			delete [] channel_B_C;

		}
		catch( Exception &error_ ){       
			cout << "Caught exception: " << error_.what() << endl;
			f << "Caught exception: " << error_.what() << endl;
		}    		
	}
	else{
		dump("Image sizes don't match.");
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

	Image seg_etalon;
	Image seg_etalon_1;

	//etalon path
/*	string imagepath_etalon = "in/Razm28_1000.bmp";
	string imagepath_etalon_1 = "in/Text0,5_0,95.bmpresult.bmp"; //bulk
*/
/*	string imagepath_etalon = "in/output initial.bmp";
	string imagepath_etalon_1 = "in/output filtr.bmp"; //bulk
*/
	string imagepath_etalon = "in/etalon.bmp";
	string imagepath_etalon_1 = "in/other.bmp"; //bulk

	// string imagepath_etalon = "in/output0.bmp";
	// string imagepath_etalon = "in/output.bmp";
	// string imagepath_etalon_1 = "in/output-6.bmp"; //bulk

	// string imagepath_etalon = "in/output.bmp";
	// string imagepath_etalon_1 = "in/output_-3.bmp";
	// string imagepath_etalon_1 = "in/output_-6.bmp"; //bulk

	//test 2
	// string imagepath_etalon = "in/Razm6_1000.bmp";
	// string imagepath_etalon = "in/output.bmp";
	// string imagepath_etalon_1 = "in/output.bmp";
	// string imagepath_etalon = "in/output0.bmp";
	// string imagepath_etalon_1 = "in/output0.bmp";
	// string imagepath_etalon = "in/output-3.bmp";
	// string imagepath_etalon_1 = "in/output-3.bmp";
	// string imagepath_etalon_1 = "in/output-6.bmp";

	//test 3
	// string imagepath_etalon = "in/Razm2_1000.bmp";
	// string imagepath_etalon = "in/Razm5_1000.bmp";
	// string imagepath_etalon = "in/Razm6_1000.bmp";
	// string imagepath_etalon = "in/Razm25_1000.bmp";
	// string imagepath_etalon = "in/Razm50_1000.bmp";	
	// string imagepath_etalon = "in/Razm95_1000.bmp";
	// string imagepath_etalon = "in/Razm28_1000.bmp";
	// string imagepath_etalon = "in/Razm70_1000.bmp";
	// string imagepath_etalon_1 = "in/output.bmp";	
	// string imagepath_etalon_1 = "in/output0.bmp";	
	// string imagepath_etalon_1 = "in/output-3.bmp";
	// string imagepath_etalon_1 = "in/output-6.bmp";

	try{
		seg_etalon.read(imagepath_etalon);
		seg_etalon_1.read(imagepath_etalon_1);
		compareImages(path, seg_etalon, seg_etalon_1, "etalon vs etalon");
		seg_etalon.write(path + "/etalon.bmp");
		seg_etalon_1.write(path + "/other.bmp");

		f.close();

		dump("Done"); 				
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}  

	std::time_t t_finish = std::time(0);
	f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	f<<"It was STUPID compare. Thanks."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	return 0; 
}