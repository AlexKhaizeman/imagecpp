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
// #include <boost/filesystem.hpp>

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
	int i = 0;
	while ((i + 1) * accuracy < number){
		i++;
	}
	if ((i + 1) * accuracy - number < number - i * accuracy){
		return (i + 1) * accuracy;
	}
	else{
		return i * accuracy;
	}
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
	std::time_t t_start = std::time(0);
	string path = "out/" + to_string(t_start);
	const char *path_ = path.c_str();
	mkdir(path_, 0777);

	double // #variables
		points_number = 14, //если не более 8ми сегментов, то не более 15ти значений (для 4х - 7)
		accuracy = 1 / (points_number + 1),
		element = 0, //for chi
		sequences = 0;

	int
		x, y, k, m,
		frameWidth = 0, frameWidthHalf = 0,
		frameHeight = 0, frameHeightHalf = 0,
		frameSquare = 0,
		comporator = 64, 
		seg_number = 1; //хитрая переменная: говорит, сколько БИ мы сегментируем. причём одна должна увеличиваться, если в старшем БИ мало сегментов и влезет ещё.. во как.

	InitializeMagick(*argv);
	// Construct the image object. Seperating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	Image src_image;
	Image seg_test;

	// средние вероятности перехода (канал, направление. по 8 шт.). по сути это не надо. скоро отомрёт само.
	double 
		pi1r[8] = {0,0,0,0,0,0,0,0}, 
		pi2r[8] = {0,0,0,0,0,0,0,0}, 
		pi1g[8] = {0,0,0,0,0,0,0,0}, 
		pi2g[8] = {0,0,0,0,0,0,0,0}, 
		pi1b[8] = {0,0,0,0,0,0,0,0}, 
		pi2b[8] = {0,0,0,0,0,0,0,0};

	//чтобы рисовать гистограммы
	int histHeight = 650, histWidth = 1400;
	Image hist_image( Geometry(histWidth, histHeight), Color(MaxRGB, MaxRGB, MaxRGB, 0));
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

	try {
		// Read a file into image object
		// src_image.read( "in/test.bmp" );
		// src_image.read( "in/Text0,5_0,95.bmp" );		
		src_image.read( "in/lenin100.jpg" );
		// src_image.read( "in/cards.jpg" );

		/*seg_test.read( "in/test.bmp" ); 
		seg_test.segment();
		seg_test.write("out/segmentation test.jpeg");*/

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
		
		//описывает поток для записи данных в файл
		ofstream f;
		//открываем файл в режиме записи, режим ios::out устанавливается по умолчанию
		f.open(path + "/output.txt", ios::out);
		
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

		double **pi1 = new double*[height];
		double **pi2 = new double*[height];
		double **pi = new double*[height]; // (!) pi - для центра окна - среднее по окну (типа симметричные пи1-пи2)
		double **pi_average = new double*[height];
		double **PI = new double*[height];
		for(int i = 0; i < height; i++) {
			pi1[i] = new double[width];
			pi2[i] = new double[width];
			pi[i] = new double[width];
			PI[i] = new double[width];
			pi_average[i] = new double[width];
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

		/*for ( ssize_t row = 0; row < height ; row++ ){
			for ( ssize_t column = 0; column < width ; column++ ){
			}
		}*/		

		comporator = 64;
		//points
		// #variables
		int 
			numver = points_number + 1, // 0 and 1, 0.001 and 0.999 (?)
			current_numver = 0;
		dump("Accuracy = " + to_string(accuracy));
		f<<"Accuracy = " + to_string(accuracy)<<endl;
		dump("Number of points is " + to_string(numver+1));
		f<<"Number of points is " + to_string(numver+1)<<endl;
		double max_point=0;
		double *points = new double[numver];
		bool *minims = new bool[numver];
		short 
			minims_counter = 0, 
			minims_flag = 0,
			color_header = 0;

		// for all binary images we segment do:
		for (int i=0; i<seg_number; i++){
			
			/*//PI = 0 - обнуление?!
			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					PI[row][column] = 0.0;
					pi[row][column] = 0.0;					
				}
			}*/	

			//pi1=pi2			
			//test
			int index = 0;
			double test = 0.0;
			for ( ssize_t row = frameHeightHalf; row < height - frameHeightHalf; row++ ){
				for ( ssize_t column = frameWidthHalf; column < width - frameWidthHalf; column++ ){
					element = 1;
					sequences = 1;
					pi[row][column] = 0;
					test = 0;
					for ( ssize_t frameRow = row - frameHeightHalf+1; frameRow < row + frameHeightHalf; frameRow++ ){
						for ( ssize_t frameColumn = column - frameWidthHalf+1; frameColumn < column + frameWidthHalf; frameColumn++ ){
							// index++;
							if (frameRow>0)
								if(comporator&channel_R[frameRow][frameColumn]&channel_R[frameRow-1][frameColumn] || comporator&!channel_R[frameRow][frameColumn]&!channel_R[frameRow-1][frameColumn]){
									test += 1;
								}

							/*element++;
							if(frameColumn == (column - frameWidthHalf) && frameRow > (row - frameHeightHalf)){
								if ((channel_R[frameRow][frameColumn]&channel_R[frameRow-1][column + frameWidthHalf]&comporator)||(!channel_R[frameRow][frameColumn]&!channel_R[frameRow-1][column + frameWidthHalf]&comporator)){
									element++;
									test+=1;
								}
								else{
									sequences++;
									
								}
							}
							else{
								if ((channel_R[frameRow][frameColumn]&channel_R[frameRow][frameColumn-1]&comporator)||(!channel_R[frameRow][frameColumn]&!channel_R[frameRow][frameColumn-1]&comporator)){
									element++;
									test+=1;
								}
								else{
									sequences++;
									
								}
							}*/
							// pi для центрального элемента += текущее хи
							// pi[row][column] += 1 - 1 / (element / sequences);
						}
					}

					test /= frameSquare;
					// f<<"plain test: " + to_string(test)<<endl;
					// f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
					// pi[row][column] /= frameSquare;
					pi[row][column] = test;
					// if(index>frameSquare)
						// dump(index);
					// index=0;
					//warning-dump
					if (pi[row][column] < 0){
						f<<"warning: pi<0: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
						f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;
						pi[row][column] = 0.001;
					}
					if (pi[row][column] > 1.0){
						f<<"warning: pi>1: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column)<<endl;
						// f<<" " + to_string(element) + " / " + to_string(sequences) + " / " + to_string(frameSquare) + " = " + to_string(element / sequences / frameSquare)<<endl;

						pi[row][column] = 0.999;
					}				
				}
			}
			dump("pi1 and pi2 counted...");

			//@TODO: исправить вычисление ^
			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					// if (pi[row][column] < 0){
					// 	dump("warning: pi<0: " + to_string(pi[row][column])+" row: " + to_string(row) + " column: " + to_string(column));						
					// 	pi[row][column] = 0.001;
					// }
					// if (pi[row][column] > 1.0){
					// 	dump(pi[row][column]);
					// 	pi[row][column] = 0.999;
					// 	// dump("warning: pi>1");
					// }										
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
					pi_average[row][column] = Round(pi_average[row][column], accuracy);
				}
			}
			dump("pi-s frame average counted ...");

			/*for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){			
					pi_average[row][column] = 1 - pi_average[row][column];		
				}
			}*/

			for (int j = 0; j < numver; j++)
				points[j] = 0;

			//если неправильно вычислить пи - будет сегфолт, что логично. смотри выше на глюки.
			//нахождение числа одинаковых вероятностей (просто как модуль от деления на шаг)
			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					current_numver = (int)(pi_average[row][column] / accuracy);
					if (current_numver >= numver)
						current_numver = numver - 1;
					points[current_numver]++;					
				}
			}
			dump("counted histogramm points ...");

			//нахождение наивысшей точки (максимального количества), нормировка по наивысшей точке    
			max_point = 0;
			for (int j = 0; j < numver; j++)
				if (max_point < points[j])
					max_point = points[j];
			//прорисовка на гистограмме (пока 1)

			hist_image.strokeColor("black"); // Outline color 		   
			histHeight -= 5;
			histWidth -= 5;
			for (int j = 0; j < numver; j++){
				points[j] /= max_point;
				drawList.push_back(DrawableLine(
					(histWidth * j / numver) + 5,
					(int)(histHeight - histHeight * points[j]),
					(histWidth * j / numver) + 5,
					(int)histHeight));
			}

			for (int j = 1; j <= numver; j++){
				drawList.push_back(DrawableLine(
					(histWidth * (j-1) / numver) + 5,
					(int)(histHeight - histHeight * points[j-1]),
					(histWidth * j / numver) + 5,
					(int)(histHeight - histHeight * points[j])));				
			}

			histHeight += 5;
			histWidth += 5;
			hist_image.draw(drawList);
			drawList.clear();
			dump("drew histogramm ...");

			// minimumy)
			minims[numver] = true;
			minims[0] = true;
			numver--;
			minims_counter=1;

			for (int j = 1; j < numver; j++){
				if ((points[j]<=points[j-1] & points[j]<points[j+1])||(points[j]<points[j-1] & points[j]<=points[j+1])){
					if (minims_counter<8){
						minims[j] = true;
						minims_counter++;
					}
					dump("There is a minimum in " + to_string(j) + " equal " + to_string(j*accuracy));
					f<<"There is a minimum in " + to_string(j) + " equal " + to_string(j*accuracy)<<endl;
				}
				else
					minims[j] = false;
			}

			//@TODO: реализовать такую штуку: типа если сегментов получилось мало - то и занимать мало разрядов...
			if (comporator == 128)
				color_header = 5;
			if (comporator == 64)
				color_header = 2;
			if (comporator == 32)
				color_header = 0;

			//segmentation
			for ( ssize_t row = 0; row < height ; row++ ){
				for ( ssize_t column = 0; column < width ; column++ ){
					minims_flag = 0;
					for (int j = 0; j <= numver; j++){
						if (minims[j])	{
							minims_flag++;
							// f<<to_string(minims_flag)<<endl;
							if (pi_average[row][column] > j*accuracy){								
								for (int jj = j+1; jj <= numver; jj++){
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
			numver++;
			dump("segmented ... ");

			comporator/=2;
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

		f<<"Среднее значение вероятности перехода для всего изображения: "<<endl;		
		f<<"RED pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1r[i]<<" , "<<1-pi2r[i]<<endl;}
		f<<"GREEN pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1g[i]<<" , "<<1-pi2g[i]<<endl;}
		f<<"BLUE pi1 , pi2: "<<endl;		
		for (int i = 0; i<8; i++) {f<<1-pi1b[i]<<" , "<<1-pi2b[i]<<endl;}

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

		delete [] points;
		delete [] minims;

		dst_view.sync();

		//запись результата
		dst_image.write(path + "/total.jpg");
		hist_image.write(path + "/histogramm.jpg");

		f.close();

		dump("Done");    
	}    
	catch( Exception &error_ ){       
		cout <<"Caught exception: " << error_.what() << endl;
		return 1;
	}    
	std::time_t t_finish = std::time(0);
	dump(to_string(t_finish - t_start) + " seconds past.");
	// f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	return 0; 
}