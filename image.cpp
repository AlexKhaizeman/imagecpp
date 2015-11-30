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

// g++ image.cpp -o image `Magick++-config --cppflags --cxxflags --ldflags --libs`

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

void channelSegmentation(string path, string info, unsigned short **channel_R, unsigned short **channel_R_S, int width, int height, int bi, int points_number, int frameWidth, int frameHeight){
	ofstream f;
	f.open(path + "/output.txt", ios::app);
	f<<"	" +info<<endl;
	dump(info);
	f<<to_string(bi)+" binary image"<<endl;
	dump(to_string(bi)+" binary image");

	//0<bi<7 - number of a binary image
	int 
		comporator = 1<<bi,
		frameSquare = frameHeight * frameWidth,
		//points
		current_points_number = 0,
		frameHeightHalf = frameHeight / 2,
		frameWidthHalf = frameWidth / 2,
		element = 0,
		sequences = 0;

	double 
		accuracy = 1 / ((double) points_number),
		max_point = 0;

	//points
	double *points = new double[points_number];
	bool *minims = new bool[points_number];
	short 
		minims_counter = 0, 
		minims_flag = 0, //кто ты теперь?
		color_header = 0;		

	// double **pi1 = new double*[height];
	// double **pi2 = new double*[height];
	double **pi = new double*[height]; // (!) pi - для центра окна - среднее по окну (типа симметричные пи1-пи2)
	double **pi_average = new double*[height];
	double **PI = new double*[height];
	for(int i = 0; i < height; i++) {
		// pi1[i] = new double[width];
		// pi2[i] = new double[width];
		pi[i] = new double[width];
		PI[i] = new double[width];
		pi_average[i] = new double[width];
	}		

	/*for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			channel_S[row][column] = channel[row][column];
		}
	}*/
	
	// dump("Accuracy = " + to_string(accuracy));
	f<<"Accuracy = " + to_string(accuracy)<<endl;
	// dump("Number of points is " + to_string(points_number));
	f<<"Number of points is " + to_string(points_number)<<endl;

	//чтобы рисовать гистограммы
	int histHeight = 650, histWidth = 1400;
	Image hist_image(Geometry(histWidth, histHeight), Color(MaxRGB, MaxRGB, MaxRGB, 0));
	hist_image.strokeColor("olive"); // Outline color 
	hist_image.fillColor("green"); // Fill color 
	hist_image.strokeWidth(1);

	// Construct drawing list 
	std::list<Magick::Drawable> drawList;	   

	histHeight-=5;
	drawList.push_back(DrawableLine(5, histHeight, histWidth, histHeight)); //X ось
	drawList.push_back(DrawableLine(5, 0, 5, histHeight)); // Y ось
	histHeight+=5;

	//Draw scale 0.1
	histWidth -= 5;
	for (int j = 1; j < 10; j++){
		drawList.push_back(DrawableLine(
			(histWidth * j / 10) + 5,
			histHeight,
			(histWidth * j / 10) + 5,
			histHeight - 10));
	}
	histWidth += 5;

	hist_image.draw(drawList);
	drawList.clear();

	// // for all binary images we segment do:
	// for (int i=0; i<seg_number; i++){}
	// у нас сейчас этого нет, мы тут делаем для текущего bi
		
	//PI = 0 - обнуление?!
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			PI[row][column] = 0.0;
			pi[row][column] = 0.0;					
			pi_average[row][column] = 0.0;					
		}
	}	

	//pi1=pi2			
	//test
	// int index = 0;
	// double test = 0.0;
	for ( ssize_t row = frameHeightHalf; row < height - frameHeightHalf; row++ ){
		for ( ssize_t column = frameWidthHalf; column < width - frameWidthHalf; column++ ){
			element = 1;
			sequences = 1;
			pi[row][column] = 0.0;
			// test = 0.0;
			for ( ssize_t frameRow = row - frameHeightHalf+1; frameRow < row + frameHeightHalf; frameRow++ ){
				for ( ssize_t frameColumn = column - frameWidthHalf+1; frameColumn < column + frameWidthHalf; frameColumn++ ){
					// index++;
					// if (frameRow>0)
					// 	if(comporator&channel_R[frameRow][frameColumn]&channel_R[frameRow-1][frameColumn] || comporator&!channel_R[frameRow][frameColumn]&!channel_R[frameRow-1][frameColumn])
					// 		test += 1.0;
						

					element++;
					if(frameColumn == (column - frameWidthHalf) && frameRow > (row - frameHeightHalf)){
						if ((channel_R[frameRow][frameColumn]&channel_R[frameRow-1][column + frameWidthHalf]&comporator)||(!channel_R[frameRow][frameColumn]&!channel_R[frameRow-1][column + frameWidthHalf]&comporator)){
							element++;
							// test+=1;
						}
						else{
							sequences++;
							
						}
					}
					else{
						if ((channel_R[frameRow][frameColumn]&channel_R[frameRow][frameColumn-1]&comporator)||(!channel_R[frameRow][frameColumn]&!channel_R[frameRow][frameColumn-1]&comporator)){
							element++;
							// test+=1;
						}
						else{
							sequences++;
							
						}
					}
					// pi для центрального элемента += текущее хи
					pi[row][column] += 1 - 1 / (element / sequences);
				}
			}

			// test /= frameSquare;
			// f<<"plain test: " + to_string(test)<<endl;
			// f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
			pi[row][column] /= frameSquare;
			// pi[row][column] = test;
			// if(index>frameSquare)
				// dump(index);
			// index=0;
			// //warning-dump
			// if (pi[row][column] < 0){
			// 	f<<"warning: pi<0: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
			// 	f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
			// 	pi[row][column] = 0.001;
			// }
			// if (pi[row][column] > 1.0){
			// 	f<<"warning: pi>1: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
			// 	// f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;

			// 	pi[row][column] = 0.999;
			// }				
		}
	}
	dump("pi1 and pi2 counted...");

	//@TODO: исправить вычисление ^
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){								
			if (pi[row][column] < 0){
				f<<"warning: pi<0: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
				f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
				pi[row][column] = 0.001;
			}
			if (pi[row][column] > 1.0){
				f<<"warning: pi>1: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
				f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
				pi[row][column] = 0.999;
			}						
		}
	}		
	dump("pi checked...");

	for ( ssize_t row = 1; row < height ; row++ ){
		for ( ssize_t column = 1; column < width ; column++ ){
			//0?1 1?0
			if (channel_R[row-1][column]&channel_R[row][column-1]&comporator){
				PI[row][column] = 0.5;
			}
			//0?0 1?1
			else{
				//000 111
				if (channel_R[row-1][column]&channel_R[row][column]&comporator){
					PI[row][column] = (double) pi[row][column]*pi[row][column]/(pi[row][column]*pi[row][column]+(1-pi[row][column])*(1-pi[row][column]));
				}
				//010 101
				else{
					PI[row][column] = (double) (1-pi[row][column])*(1-pi[row][column])/(pi[row][column]*pi[row][column]+(1-pi[row][column])*(1-pi[row][column]));
				}
			}
		}
	}		
	dump("PI counted ...");

	//pi frame avarage
	for ( ssize_t row = frameHeightHalf; row < height - frameHeightHalf; row++ ){
		for ( ssize_t column = frameWidthHalf; column < width - frameWidthHalf; column++ ){
			pi_average[row][column] = 0;
			for ( ssize_t frameRow = row - frameHeightHalf; frameRow < row + frameHeightHalf; frameRow++ ){
				for ( ssize_t frameColumn = column - frameWidthHalf; frameColumn < column + frameWidthHalf; frameColumn++ ){
					pi_average[row][column] += PI[frameRow][frameColumn];
				}
			}
			pi_average[row][column] /= frameSquare;				
		}
	}
	dump("pi-s frame average counted ...");

	/*for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){			
			pi_average[row][column] = 1 - pi_average[row][column];		
		}
	}*/

	for (int j = 0; j < points_number; j++)
		points[j] = 0;
	dump("created points ...");

	//если неправильно вычислить пи - будет сегфолт, что логично. смотри выше на глюки.
	//нахождение числа одинаковых вероятностей (просто как модуль от деления на шаг)
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			pi_average[row][column] = Round(pi_average[row][column], accuracy);
			// f<<to_string(pi_average[row][column])+ " "<<endl;
		}
	}
	dump("pi rounded ...");

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			current_points_number = (int)(pi_average[row][column] / accuracy);
			if (current_points_number >= points_number){
				dump(to_string(current_points_number));
				dump(to_string(pi_average[row][column]));
				current_points_number = points_number - 1;
				// dump("warning more");
			}
			if (current_points_number<0){
				dump(to_string(current_points_number));
				dump(to_string(pi_average[row][column]));
				current_points_number = 0;
				// dump("warning zero");
			}
			points[current_points_number]++;					
		}
	}
	dump("counted histogramm points ...");

	//нахождение наивысшей точки (максимального количества), нормировка по наивысшей точке    
	max_point = 0;
	for (int j = 0; j < points_number; j++)
		if (max_point < points[j])
			max_point = points[j];
	//прорисовка на гистограмме (пока 1)

	hist_image.strokeColor("black"); // Outline color 		   
	histHeight -= 5;
	histWidth -= 5;
	for (int j = 0; j < points_number; j++){
		points[j] /= max_point;
		drawList.push_back(DrawableLine(
			(histWidth * j / points_number) + 5,
			(int)(histHeight - histHeight * points[j]),
			(histWidth * j / points_number) + 5,
			(int)histHeight));
	}

	for (int j = 1; j <= points_number; j++){
		drawList.push_back(DrawableLine(
			(histWidth * (j-1) / points_number) + 5,
			(int)(histHeight - histHeight * points[j-1]),
			(histWidth * j / points_number) + 5,
			(int)(histHeight - histHeight * points[j])));				
	}

	histHeight += 5;
	histWidth += 5;
	hist_image.draw(drawList);
	drawList.clear();
	dump("drew histogramm ...");

	// minimumy)
	minims[points_number] = true;
	minims[0] = true;
	points_number--;
	minims_counter=1;

	for (int j = 1; j < points_number; j++){
		if ((points[j]<=points[j-1] & points[j]<points[j+1])||(points[j]<points[j-1] & points[j]<=points[j+1])){
			if (minims_counter<8){
				minims[j] = true;
				minims_counter++;
			}
			// dump("There is a minimum in " + to_string(j) + " equal " + to_string(j*accuracy));
			f<<"There is a minimum in " + to_string(j) + " equal " + to_string(j*accuracy)<<endl;
		}
		else
			minims[j] = false;
	}


	//@TODO: реализовать такую штуку: типа если сегментов получилось мало - то и занимать мало разрядов...
	if (comporator == 128)
		color_header = 4;
	if (comporator == 64)
		color_header = 0;
	/*if (comporator == 32)
		color_header = 0;*/

	//segmentation
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			minims_flag = 0;
			for (int j = 0; j <= points_number; j++){
				if (minims[j]){
					minims_flag++;
					// f<<to_string(minims_flag)<<endl;
					if (pi_average[row][column] > j*accuracy){								
						for (int jj = j+1; jj <= points_number; jj++){
							if (minims[jj]){
								// dump("point min is in " + to_string(j) + " equal " + to_string(j*accuracy) + " and in " + to_string(jj) + " equal " + to_string(jj*accuracy) + " pi is " + to_string(pi_average[row][column]));
								if (pi_average[row][column] <= jj*accuracy){
									channel_R_S[row][column] += (minims_flag<<color_header);
								}
							}
						}
					}
				}
			}
		}
	}
	points_number++;
	dump("segmented ... ");

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			channel_R_S[row][column] = 255 - channel_R_S[row][column];
		}
	}	

	/*//frame
	for ( ssize_t row = frameHeightHalf; row < height - frameHeightHalf; row++ ){
		for ( ssize_t column = frameWidthHalf; column < width - frameWidthHalf; column++ ){

			for ( ssize_t frameRow = row - frameHeightHalf; frameRow < row + frameHeightHalf; frameRow++ ){
				for ( ssize_t frameColumn = column - frameWidthHalf; frameColumn < column + frameWidthHalf; frameColumn++ ){

				}
			}

		}
	}*/	

	for(int i = 0; i < height; i++) {
		delete [] pi[i];
		delete [] PI[i];
		delete [] pi_average[i];
	}	
	delete [] pi;
	delete [] PI;
	delete [] pi_average;

	delete [] points;
	delete [] minims;
	hist_image.write(path + "/histogramm " + info + " " + to_string(bi) + ".jpg");

}

void garbge(int argc,char **argv){
	double // #variables
		points_number = 14, //если не более 8ми сегментов, то не более 15ти значений (для 4х - 7)
		accuracy = 1 / (points_number + 1),
		element = 0, //for chi
		sequences = 0;

	// средние вероятности перехода (канал, направление. по 8 шт.). по сути это не надо. скоро отомрёт само.
	double 
		pi1r[8] = {0,0,0,0,0,0,0,0}, 
		pi2r[8] = {0,0,0,0,0,0,0,0}, 
		pi1g[8] = {0,0,0,0,0,0,0,0}, 
		pi2g[8] = {0,0,0,0,0,0,0,0}, 
		pi1b[8] = {0,0,0,0,0,0,0,0}, 
		pi2b[8] = {0,0,0,0,0,0,0,0};
	
	try {

		/*
		// рассчёт средних вероятностей перехода. просто для примера.		
		for ( ssize_t row = 1; row < height ; row++ ){
			for ( ssize_t column = 1; column < width ; column++ ){
				for (int i=0; i<seg_number; i++){
					if(comporator&channel_R[row][column]&channel_R[row-1][column] || comporator&!channel_R[row][column]&!channel_R[row-1][column]){
						pi1r[i]++;
					}
					if(comporator&channel_R[row][column]&channel_R[row][column-1] || comporator&!channel_R[row][column]&!channel_R[row][column-1]){
						pi2r[i]++;
					}
					if(comporator&channel_G[row][column]&channel_G[row-1][column] || comporator&!channel_G[row][column]&!channel_G[row-1][column]){
						pi1g[i]++;
					}
					if(comporator&channel_G[row][column]&channel_G[row][column-1] || comporator&!channel_G[row][column]&!channel_G[row][column-1]){
						pi2g[i]++;
					}
					if(comporator&channel_B[row][column]&channel_B[row-1][column] || comporator&!channel_B[row][column]&!channel_B[row-1][column]){
						pi1b[i]++;
					}
					if(comporator&channel_B[row][column]&channel_B[row][column-1] || comporator&!channel_B[row][column]&!channel_B[row][column-1]){
						pi2b[i]++;
					}
					comporator/=2;
				}
				comporator = 128;
			}
		}		

		for (int i = 0; i<seg_number; i++){
			pi1r[i]/=square;
			pi2r[i]/=square;
			pi1g[i]/=square;
			pi2g[i]/=square;
			pi1b[i]/=square;
			pi2b[i]/=square;
		}
		*/
		/*for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
			}
		}*/		

   
	}    
	catch( Exception &error_ ){       
		cout <<"Caught exception: " << error_.what() << endl;
	}    

}

void channelCompare(string path, string info, unsigned short **channel_R, unsigned short **channel_R_S, unsigned short **channel_R_C, int width, int height){
	ofstream f;
	f.open(path + "/output.txt", ios::app);	
	// dump(info);
	int square = width * height, index = 0;
	double seg_error=0;
	std::list<unsigned short> colours_;
	std::list<unsigned short> colours_S;
	std::list<unsigned short>::iterator it_ = colours_.begin();
	std::list<unsigned short>::iterator it_S = colours_.begin();

	//надо типа не так, а использовать вероятностный подход
	/*std::list<double> part_;
	std::list<double> part_S;
	std::list<double>::iterator it_part_ = part_.begin();
	std::list<double>::iterator it_part_S = part_S.begin();
	double* p;*/
	bool thereis = false;
	
	/*for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){								
			thereis = false;
			it_part_ = part_.begin();  			
  			for (std::list<unsigned short>::iterator it = colours_.begin(); it != colours_.end(); ++it){
  				it_part_++;
  				if (*it == channel_R[row][column]){
  					thereis = true;  					
  					*it_part_ = *it_part_ + 1;
  				}
  			}

  			if (!thereis){
				colours_.push_back(channel_R[row][column]);
				part_.push_back(1);
			}

			thereis = false;
			it_part_S = part_S.begin();
  			for (std::list<unsigned short>::iterator it = colours_S.begin(); it != colours_S.end(); ++it){
  				it_part_S++;
  				if (*it == channel_R_S[row][column]){
  					thereis = true;
  					*it_part_S = *it_part_S + 1;
  				}
  			}
  			if (!thereis){
				colours_S.push_back(channel_R_S[row][column]);
				part_S.push_back(1);
			}
		}
	}*/		

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){								
			thereis = false;
  			for (std::list<unsigned short>::iterator it = colours_.begin(); it != colours_.end(); ++it)
  				if (*it == channel_R[row][column])
  					thereis = true;  		
  			if (!thereis)
				colours_.push_back(channel_R[row][column]);
			thereis = false;
  			for (std::list<unsigned short>::iterator it = colours_S.begin(); it != colours_S.end(); ++it)
  				if (*it == channel_R_S[row][column])
  					thereis = true;
  			if (!thereis)
				colours_S.push_back(channel_R_S[row][column]);  				
		}
	}		

	// dump("push done");

	double *part_ = new double[colours_.size()];
	int *colours_array = new int[colours_.size()];
	double *part_S = new double[colours_S.size()];
	int *colours_array_S = new int[colours_S.size()];

	for (int i = 0; i < colours_.size(); i++)
		part_[i] = 0;

	for (int i = 0; i < colours_S.size(); i++)
		part_S[i] = 0;

	index = 0;
	for (std::list<unsigned short>::iterator it = colours_.begin(); it != colours_.end(); ++it)
		colours_array[index++] = *it;

	index = 0;
	for (std::list<unsigned short>::iterator it = colours_S.begin(); it != colours_S.end(); ++it)
		colours_array_S[index++] = *it;

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			for (int i = 0; i < colours_.size(); i++)
				if (channel_R[row][column] == colours_array[i])
					part_[i]++;
			for (int i = 0; i < colours_S.size(); i++)
				if (channel_R_S[row][column] == colours_array_S[i])
					part_S[i]++;
		}
	}		

    //попробуем просто отсортировать
    for (std::size_t idx_i = 0; idx_i < colours_.size() - 1; idx_i++)
    {
        for (std::size_t idx_j = 0; idx_j < colours_.size() - idx_i - 1; idx_j++)
        {
            if (part_[idx_j + 1] > part_[idx_j])
            {
                std::swap(part_[idx_j], part_[idx_j + 1]);
                std::swap(colours_array[idx_j], colours_array[idx_j + 1]);
            }
        }
    }

    for (std::size_t idx_i = 0; idx_i < colours_S.size() - 1; idx_i++)
    {
        for (std::size_t idx_j = 0; idx_j < colours_S.size() - idx_i - 1; idx_j++)
        {
            if (part_S[idx_j + 1] > part_S[idx_j])
            {
                std::swap(part_S[idx_j], part_S[idx_j + 1]);
                std::swap(colours_array_S[idx_j], colours_array_S[idx_j + 1]);
            }
        }
    }

	f<<"	"+info<<endl;
	f<<"etalon"<<endl;
	for (int i = 0; i < colours_.size(); i++){
		part_[i] = (double) part_[i] * 100 / square;
		f<<to_string(colours_array[i]) + " is colour from 1 part " + to_string(part_[i])<<endl;
	}
		
    f<<"secondary"<<endl;
	for (int i = 0; i < colours_S.size(); i++){
		part_S[i] = (double) part_S[i] * 100 / square;
		f<<to_string(colours_array_S[i]) + " is colour from 2 part " + to_string(part_S[i])<<endl;		
	}

    	/*//playground
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				channel_R_C[row][column] = (channel_R[row][column] + channel_R_S[row][column])/2;
			}
		}*/		

	/*
    //если в эталоне больше цветов
    if(colours_.size()>colours_S.size()){
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				for (int i = 0; i < colours_S.size(); i++)
					if (channel_R[row][column] == colours_array[i]){
						if (channel_R_S[row][column] == colours_array_S[i])
							channel_R_C[row][column] = 255;
						else{
							seg_error++;
							channel_R_C[row][column] = 0;
						}							
					}
			}
		}
	    seg_error = 100*seg_error/square;
		for (int i = colours_S.size(); i < colours_.size(); i++)
			seg_error += part_[i];
    } //если в эталоне цветов меньше или столько же
    else{
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				for (int i = 0; i < colours_.size(); i++)
					if (channel_R_S[row][column] == colours_array_S[i]){
						if (channel_R[row][column] == colours_array[i])	
							channel_R_C[row][column] = 255;
						else{
							seg_error++;
							channel_R_C[row][column] = 0;
						}							
					}
			}
		}
		seg_error = 100*seg_error/square;		
		for (int i = colours_.size(); i < colours_S.size(); i++)
			seg_error += part_S[i];			
    }
    dump(info + " old segmentation error " + to_string(seg_error) + "%");
    f<<"old segmentation error " + to_string(seg_error) + "%"<<endl;			
    */

	//реализуем табличку пересечений (хотя бы для себя)
	int **match = new int*[colours_S.size()];
	//не можешь контролировать ситуацию - создай дублёра
	int **match_ = new int*[colours_S.size()];
	for (int j = 0; j < colours_S.size(); j++){
		match[j] = new int [colours_.size()];
		match_[j] = new int [colours_.size()];		
	}
	for (int i = 0; i < colours_S.size(); i++){
		for (int j = 0; j < colours_.size(); j++){
			match[i][j] = 0;
			match_[i][j] = 0;
		}
	}	

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			for (int i = 0; i < colours_S.size(); i++){
				for (int j = 0; j < colours_.size(); j++){
					if (channel_R[row][column] == colours_array[j]){
						if (channel_R_S[row][column] == colours_array_S[i]){
							match[i][j] += 1;
						}
					}
				}
			}
		}
	}

    f<<"match table"<<endl;
    f<<"           ";
	for (int j = 0; j < colours_.size(); j++){
		f<<"colour " + to_string(colours_array[j])<<' ';
	}
	f<<endl;

	for (int i = 0; i < colours_S.size(); i++){
		f<<"colour " + to_string(colours_array_S[i])<<" | ";
		for (int j = 0; j < colours_.size(); j++){
			f<<match[i][j]<<"      ";
		}
		f<<endl;
	}

	int colours_number = colours_.size();
	if (colours_number > colours_S.size())
		colours_number = colours_S.size();
	int current_max = 0, current_i = 0, current_j = 0;
	seg_error = 0;

	for (int k=0; k<colours_number; k++){
		//найти максимум 
		current_max = 0;
		for (int i = 0; i < colours_S.size(); i++){
			for (int j = 0; j < colours_.size(); j++){
				if (match[i][j] > current_max){
					current_max = match[i][j];
					current_i = i;
					current_j = j;
				}
			}
		}		
		match_[current_i][current_j] = match[current_i][current_j];
		//и зачистить столбец и строку с ним
		for (int i = 0; i < colours_S.size(); i++){
			// if (i != current_i)
				match[i][current_j] = 0;
		}
		for (int j = 0; j < colours_.size(); j++){
			// if (j != current_j)
				match[current_i][j] = 0;
		}
		
	}

    f<<"match table - analyzed"<<endl;
    f<<"           ";
	for (int j = 0; j < colours_.size(); j++){
		f<<"colour " + to_string(colours_array[j])<<' ';
	}
	f<<endl;

	seg_error = 0;
	for (int i = 0; i < colours_S.size(); i++){
		f<<"colour " + to_string(colours_array_S[i])<<" | ";
		for (int j = 0; j < colours_.size(); j++){
			f<<match_[i][j]<<"      ";
			seg_error += match_[i][j];
		}
		f<<endl;
	}

	seg_error = square - seg_error;
	seg_error /= square/100;

    dump(info + " segmentation error " + to_string(seg_error) + "%");
    f<<"segmentation error " + to_string(seg_error) + "%"<<endl;

    //раскраска
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			for (int i = 0; i < colours_S.size(); i++){
				for (int j = 0; j < colours_.size(); j++){
					if (channel_R[row][column] == colours_array[j]){
						if (channel_R_S[row][column] == colours_array_S[i]){
							if (match_[i][j] != 0)
								channel_R_C[row][column] = 255;
							else
								channel_R_C[row][column] = 0;
						}
					}
				}
			}
		}
	}

    for (int j = 0; j < colours_S.size(); j++){
    	delete [] match[j];
    	delete [] match_[j];
    }
    delete [] match;
    delete [] match_;
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
	std::time_t t_start = std::time(0);
	string path = "out/" + to_string(t_start);
	const char *path_ = path.c_str();
	mkdir(path_, 0777);
	
	//описывает поток для записи данных в файл
	ofstream f;
	//открываем файл в режиме записи, режим ios::out устанавливается по умолчанию
	f.open(path + "/output.txt", ios::app);

	int
		x, y, k, m,
		frameWidth = 0, frameWidthHalf = 0,
		frameHeight = 0, frameHeightHalf = 0,
		frameSquare = 0,
		comporator = 64, 
		seg_number = 2, //хитрая переменная: говорит, сколько БИ мы сегментируем. причём одна должна увеличиваться, если в старшем БИ мало сегментов и влезет ещё.. во как.	
		points_number = 15;

	InitializeMagick(*argv);
	// Construct the image object. Seperating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	Image src_image;
	Image seg_test;

	// string imagepath = "in/7637066.jpg"; //a plane
	string imagepath = "in/lenin100.jpg"; 
	// string imagepath = "in/17526af83f295c67-2729bf8c04e7e751.jpg";

	try{
		// Read a file into image object
		src_image.read(imagepath);

		seg_test.read(imagepath); 
		seg_test.segment();
		seg_test.write(path + "/ImageMagick-6 segmentation.jpeg");

		// нарисовать изображения (!)
		// drawBinaryImages(src_image); //(!)

		Pixels src_view(src_image);

		int width = src_image.size().width();
		int height = src_image.size().height();
		int square = width * height;

		//немного про параметры окна log(x)*(x^(1/(log(x))));
		// frameWidth = 21;		
		frameWidth = log(width)*pow(width,(1/(log(width))));
		if ((frameWidth&1) == 0) frameWidth++;
			frameWidthHalf = (frameWidth+1)/2;
		// frameHeight = 21;		
		frameHeight = log(height)*pow(height,(1/(log(height))));;
		if ((frameHeight&1) == 0) frameHeight++;
			frameHeightHalf = (frameHeight+1)/2;
		frameSquare = frameWidth * frameHeight;
		
		//dump
		dump("Initial image width is " + to_string(width) + "px, and height is " + to_string(height) + "px");
		f<<"Initial image width is " + to_string(width) + "px, and height is " + to_string(height) + "px"<<endl;
		dump("So, frame width " + to_string(frameWidth) + "px, " + "frame height " + to_string(frameHeight) + "px");
		f<<"So, frame width " + to_string(frameWidth) + "px, " + "frame height " + to_string(frameHeight) + "px"<<endl;
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
		for(int i = 0; i < height; i++) {
			channel_R[i] = new unsigned short[width];
			channel_G[i] = new unsigned short[width];
			channel_B[i] = new unsigned short[width];
		}

		//Declare a model of a result image.
		unsigned short **channel_R_S = new unsigned short*[height];
		unsigned short **channel_G_S = new unsigned short*[height];
		unsigned short **channel_B_S = new unsigned short*[height];
		for(int i = 0; i < height; i++) {
			channel_R_S[i] = new unsigned short[width];
			channel_G_S[i] = new unsigned short[width];
			channel_B_S[i] = new unsigned short[width];
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

				// test
				/*
				channel_R_S[row][column] = channel_R[row][column];
				channel_G_S[row][column] = channel_G[row][column];
				channel_B_S[row][column] = channel_B[row][column];
				*/
			}
		}		

		//SEGMENTATION
		// void channelSegmentation(unsigned short **channel, unsigned short **channel_S, int width, int height, int bi, int points_number, int frameWidth, int frameHeight)
		/*channelSegmentation(path, "red channel", channel_R, channel_R_S, width, height, 6, points_number, frameWidth, frameHeight);
		channelSegmentation(path, "green channel", channel_G, channel_G_S, width, height, 6, points_number, frameWidth, frameHeight);
		channelSegmentation(path, "blue channel", channel_B, channel_B_S, width, height, 6, points_number, frameWidth, frameHeight);*/

		for (int i=0; i<2; i++){
			channelSegmentation(path, "red channel", channel_R, channel_R_S, width, height, 7-i, points_number, frameWidth, frameHeight);
			channelSegmentation(path, "green channel", channel_G, channel_G_S, width, height, 7-i, points_number, frameWidth, frameHeight);
			channelSegmentation(path, "blue channel", channel_B, channel_B_S, width, height, 7-i, points_number, frameWidth, frameHeight);			
		}

		/*f<<"Среднее значение вероятности перехода для всего изображения: "<<endl;		
		f<<"RED pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1r[i]<<" , "<<1-pi2r[i]<<endl;}
		f<<"GREEN pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1g[i]<<" , "<<1-pi2g[i]<<endl;}
		f<<"BLUE pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1b[i]<<" , "<<1-pi2b[i]<<endl;}*/

		//рисуем итоговую карту
		for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
				pixel_color = ColorRGB((double) channel_R_S[row][column]/255, (double) channel_G_S[row][column]/255, (double) channel_B_S[row][column]/255);
				*(dst_view.get(column,row,1,1)) = pixel_color;
			}
		}		

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

		dst_view.sync();

		compareImages(path, seg_test, dst_image, "magic vs ours");

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
	f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	return 0; 
}