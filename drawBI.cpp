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

// g++ drawBI.cpp -o drawBI `Magick++-config --cppflags --cxxflags --ldflags --libs`

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

void drawBinaryImages(Image src_image){
	std::time_t t_start = std::time(0);
	string path = "out/" + to_string(t_start);
	const char *path_ = path.c_str();
	mkdir(path_, 0777);

	Pixels src_view(src_image);
	int width = src_image.size().width();
	int height = src_image.size().height();
	unsigned char r = 0;

	// Construct drawing list 
	std::list<Magick::Drawable> drawList;	   	   

	typedef std::list<Image>::iterator MyListIterator;
	typedef Image *current_;
	typedef std::list<Image> Images; //4*3 = 12 (4 for each channel)
	Images BImages;
	Image* currentBIpointer;

	for (int ii=0; ii<24; ii++){
		BImages.push_back(Image (Geometry(width, height), Color(MaxRGB, MaxRGB, MaxRGB, 0)));
	}
	MyListIterator iterator = BImages.begin();  //Берём первый элемент

	int comporator = 128;

	dump("Making binary images now. Slowly. Four seniour bits for all channels (total = 12)");
	//red channel
	for (int i=0; i<8; i++){
		currentBIpointer=&*iterator;	//используем указатель currentBIpointer для ссылки на объект
		currentBIpointer->type(TrueColorType);
		currentBIpointer->modifyImage();	
		currentBIpointer->strokeColor("white"); // Outline color 
		currentBIpointer->fillColor("black"); // Fill color 
		currentBIpointer->strokeWidth(1);
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				Color pixel_color = *(src_view.get(column,row,1,1));
				ColorRGB pixel_rgb(pixel_color);
				r = (unsigned char) 255*pixel_rgb.red();
				if (comporator&r){
					drawList.push_back(DrawablePoint(column,row));	
				}
			}
		}
		comporator/=2;
		currentBIpointer->draw(drawList);
		drawList.clear();
		iterator++;
		dump(to_string(i+1) + "/24 done");		
	}
	comporator = 128;
	//green channel
	for (int i=0; i<8; i++)
		{
		currentBIpointer=&*iterator;	//используем указатель currentBIpointer для ссылки на объект
		currentBIpointer->type(TrueColorType);
		currentBIpointer->modifyImage();	
		currentBIpointer->strokeColor("white"); // Outline color 
		currentBIpointer->fillColor("black"); // Fill color 
		currentBIpointer->strokeWidth(1);
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				Color pixel_color = *(src_view.get(column,row,1,1));
				ColorRGB pixel_rgb(pixel_color);
				r = (unsigned char) 255*pixel_rgb.green();
				if (comporator&r){
					drawList.push_back(DrawablePoint(column,row));	
				}
			}
		}
		comporator/=2;
		currentBIpointer->draw(drawList);
		drawList.clear();
		iterator++;
		dump(to_string(i+9) + "/24 done");
	}
	comporator = 128;
	//blue channel
	for (int i=0; i<8; i++)
	{
		currentBIpointer=&*iterator;	//используем указатель currentBIpointer для ссылки на объект
		currentBIpointer->type(TrueColorType);
		currentBIpointer->modifyImage();	
		currentBIpointer->strokeColor("white"); // Outline color 
		currentBIpointer->fillColor("black"); // Fill color 
		currentBIpointer->strokeWidth(1);
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				Color pixel_color = *(src_view.get(column,row,1,1));
				ColorRGB pixel_rgb(pixel_color);
				r = (unsigned char) 255*pixel_rgb.blue();
				if (comporator&r){
					drawList.push_back(DrawablePoint(column,row));	
				}
			}
		}
		comporator/=2;
		currentBIpointer->draw(drawList);
		drawList.clear();
		iterator++;
		dump(to_string(i+17) + "/24 done");		
	}

	//запись бинарных изображений
	int i=0;
	iterator = BImages.begin();  //Берём первый элемент:
	while(iterator != BImages.end()){
		currentBIpointer=&*iterator;	//используем указатель currentBIpointer для ссылки на объект
		currentBIpointer->write(path + "/bi"+to_string(++i)+".jpg");	//Выполняем метод myAction объекта Object
		iterator++;		//Берём следующий элемент
	}
}

int main(int argc,char **argv){
	dump("Started.");
	std::time_t t_start = std::time(0);
	string path = "out/" + to_string(t_start);
	const char *path_ = path.c_str();
	mkdir(path_, 0777);
	
	InitializeMagick(*argv);
	// Construct the image object. Seperating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	Image src_image;

	string imagepath = "in/Text0,5_0,95.bmp"; 

	try{
		// Read a file into image object
		src_image.read(imagepath);

		// нарисовать изображения (!)
		drawBinaryImages(src_image); //(!)		

		dump("Done"); 		
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
	}    

	std::time_t t_finish = std::time(0);
	dump(to_string(t_finish - t_start) + " seconds past.");
	return 0; 
}