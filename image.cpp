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

// g++ image.cpp -o image `Magick++-config --cppflags --cxxflags --ldflags --libs` && ./image

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

void channelSegmentation(string path, string info, unsigned short **channel_R, unsigned short **channel_R_S, int width, int height, int bi, int points_number, int frameWidth, int frameHeight, int color_header){
	ofstream f;
	f.open(path + "/output.txt", ios::app);
	f<<"	" +info<<endl;
	dump(info);
	// f<<to_string(bi)+" binary image"<<endl;
	f<<to_string(bi)+" бинарное изображение"<<endl;
	dump(to_string(bi)+" binary image");

	// InitializeMagick(*argv);
	Image dst_image(Geometry(width, height), ColorRGB(MaxRGB, MaxRGB, MaxRGB));
	
	dst_image.modifyImage();
	Pixels dst_view(dst_image);

	//0<bi<7 - number of a binary image
	int 
		comporator = 1<<bi,
		frameSquare = frameHeight * frameWidth,
		//points
		current_points_number = 0,
		frameHeightHalf = frameHeight / 2,
		frameWidthHalf = frameWidth / 2;

	// dump("So, frame half width " + to_string(frameWidthHalf) + "px, " + "frame half height " + to_string(frameHeightHalf) + "px");
	// f<<("So, frame half width " + to_string(frameWidthHalf) + "px, " + "frame half height " + to_string(frameHeightHalf) + "px")<<endl;

	double 
		accuracy = 1 / ((double) points_number),
		max_point = 0,
		element = 0,
		sequences = 0;

	//points
	double *points = new double[points_number];
	bool *minims = new bool[points_number];
	short 
		minims_counter = 0, 
		minims_flag = 0; //кто ты теперь?
		// color_header = 0;

	// double **pi1 = new double*[height];
	// double **pi2 = new double*[height];
	double **pi = new double*[height]; // (!) pi - для центра окна - среднее по окну (типа симметричные пи1-пи2)
	double **pi_average = new double*[height];
	double **PI = new double*[height]; //PI = pi1*pi2/pi3
	for(int i = 0; i < height; i++) {
		// pi1[i] = new double[width];
		// pi2[i] = new double[width];
		pi[i] = new double[width];
		PI[i] = new double[width];
		pi_average[i] = new double[width]; //PI average
	}		

	// f<<"Accuracy = " + to_string(accuracy)<<endl;
	f<<"Точность округления вероятности = " + to_string(accuracy)<<endl;
	// f<<"Number of points is " + to_string(points_number)<<endl;
	f<<"Количество точек " + to_string(points_number)<<endl;

	//чтобы рисовать гистограммы
	int histHeight = 600, histWidth = 1800;
	Image hist_image(Geometry(histWidth, histHeight), Color(MaxRGB, MaxRGB, MaxRGB, 0));
	histHeight -= 30;
	histWidth -= 5;
	hist_image.strokeColor("olive"); // Outline color 
	hist_image.fillColor("green"); // Fill color 
	hist_image.strokeWidth(1);

	// Construct drawing list 
	std::list<Magick::Drawable> drawList;	   

	histHeight-=25;
	drawList.push_back(DrawableLine(5, histHeight, histWidth, histHeight)); //X ось
	drawList.push_back(DrawableLine(5, 0, 5, histHeight)); // Y ось
	drawList.push_back(DrawableLine((histWidth+5)/2, 0, histWidth/2+5, histHeight)); // Y ось 0.5
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
	histHeight+=20;

	hist_image.draw(drawList);
	drawList.clear();

	//Declare a COPY model of an image for current BI
	unsigned short **channel_R_ = new unsigned short*[height];
	for(int i = 0; i < height; i++) {
		channel_R_[i] = new unsigned short[width];
	}	

	//обнуление
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			PI[row][column] = 0.0;
			pi[row][column] = 0.0;					
			pi_average[row][column] = 0.0;					
			channel_R_[row][column] = channel_R[row][column]&comporator; //! вычленение бита сразу
		}
	}	

	unsigned short  *biElementsFrame = new unsigned short [frameSquare];
	// unsigned short  *frameElements = new unsigned short [frameSquare];
	// unsigned short  *frameSequences = new unsigned short [frameSquare];

	/*for (int j=0; j<frameSquare; j++)
		biElementsFrame[j] = 0;*/

	int 
		index,
		frameWidthHalfLocal = 1,
		frameHeightHalfLocal = 1,
		frameSquareLocal = 1;

	for ( ssize_t row = 0; row < height ; row++ )//вероятность pi1 pi2
	{
		for ( ssize_t column = 0; column < width ; column++ )
		{
            if (column > frameWidthHalf && row > frameHeightHalf && column < (width - frameWidthHalf) && row < (height - frameHeightHalf))
            {            	
                index = 0; //make a 1d copy
                for (ssize_t frameRow = (row - frameHeightHalf); frameRow <= (row + frameHeightHalf); frameRow++)//frame circle
                {
                    for (ssize_t frameColumn = (column - frameWidthHalf); frameColumn <= (column + frameWidthHalf); frameColumn++)
                    {                    	
                        biElementsFrame[index] = channel_R_[frameRow][frameColumn];
                        index++;
                    }
                }

                // good but long
                /*frameElements[0] = 1;
                frameSequences[0] = 1;

                for (int j = 1; j < frameSquare; j++)
                {
                    frameElements[j] = frameElements[j - 1];
                    frameSequences[j] = frameSequences[j - 1];
                    if (biElementsFrame[j - 1] == biElementsFrame[j])
                    { frameElements[j]++; }
                    else
                    { frameSequences[j]++; }
                    frameElements[j]++;
                }
                
                 // var 1
                pi[row][column] = frameSquare;
                for (index = 0; index < frameSquare; index++)
                {
                    pi[row][column] -= (double) frameSequences[index] / frameElements[index];
                }*/

                /* // var 2
                pi[row][column] = 0.0;
                for (index = 0; index < frameSquare; index++)
                {
                    pi[row][column] -= (double) frameSequences[index] / (double) frameElements[index];
                }
                pi[row][column] += (double) frameSquare;*/
				
				//good
				element = 1;
                sequences = 1;
                pi[row][column] = frameSquare;
                for (int j = 1; j < frameSquare; j++)
                {
                    if (biElementsFrame[j - 1] == biElementsFrame[j])
                    	{ element++; }
                    else
                		{ sequences++; }
                    element++;
					pi[row][column] -= (double) sequences / (double) element;
                }                
	            pi[row][column] = (double) pi[row][column]/ (double) frameSquare;      
            }
            else
            {
        		if (column <= frameWidthHalf){
					frameWidthHalfLocal = column;
        		}
        		if (column >= (width - frameWidthHalf)){
					frameWidthHalfLocal = width - column - 1;
        		}
				if (row <= frameHeightHalf){
					frameHeightHalfLocal = row;
				}
				if (row >= (height - frameHeightHalf)){
					frameHeightHalfLocal = height - row - 1;
				}				
				frameSquareLocal = (2 * frameHeightHalfLocal + 1) * (2 * frameWidthHalfLocal + 1);

	            index = 0; //make a 1d copy
	            for (ssize_t frameRow = (row - frameHeightHalfLocal); frameRow <= (row + frameHeightHalfLocal); frameRow++)//frame circle
	            {
	                for (ssize_t frameColumn = (column - frameWidthHalfLocal); frameColumn <= (column + frameWidthHalfLocal); frameColumn++)
	                {                    	
	                    biElementsFrame[index] = channel_R[frameRow][frameColumn];
	                    index++;
	                }
	            }    

				//good
				element = 1;
                sequences = 1;
                pi[row][column] = frameSquareLocal;
                for (int j = 1; j < frameSquareLocal; j++)
                {
                    if (biElementsFrame[j - 1] == biElementsFrame[j])
                    	{ element++; }
                    else
                		{ sequences++; }
                    element++;
					pi[row][column] -= (double) sequences / (double) element;
                }                
            	pi[row][column] = (double) pi[row][column]/ (double) frameSquareLocal;                  	
            }
        }
    }

	dump("pi1 and pi2 counted...");

	//@TODO: исправить вычисление ^ :)
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
	dump("pi1 and pi2 checked ...");

	double alpha, aalpha, pi3;

	for ( ssize_t row = 1; row < height ; row++ ){
		for ( ssize_t column = 1; column < width ; column++ ){

            alpha = pi[row][column];
            aalpha = 1 - alpha;

			//0x1 1x0
			if (channel_R_[row-1][column] != channel_R_[row][column-1]) {
				PI[row][column] = 0.5;
				// dump(0.5);
			}
			//0x0 1x1
			else{
	            pi3 = (double) (alpha * alpha + aalpha * aalpha);

				//000 111
				if (channel_R_[row][column] == channel_R_[row][column-1]){
					PI[row][column] = (double) (alpha * alpha / pi3); 
				}
				//010 101
				else{					
					PI[row][column] = (double) (aalpha * aalpha / pi3); 
				}
			}
		}
	}		
	dump("PI 3 counted ...");

	//pi frame avarage
	for ( ssize_t row = 0; row < height ; row++ )//вероятность pi1 pi2
	{
		for ( ssize_t column = 0; column < width ; column++ )
		{
			pi_average[row][column] = 0;
            if (column > frameWidthHalf && row > frameHeightHalf && column < (width - frameWidthHalf) && row < (height - frameHeightHalf))
            {
				for ( ssize_t frameRow = row - frameHeightHalf; frameRow <= row + frameHeightHalf; frameRow++ ){
					for ( ssize_t frameColumn = column - frameWidthHalf; frameColumn <= column + frameWidthHalf; frameColumn++ ){
						pi_average[row][column] += (double) PI[frameRow][frameColumn];
					}
				}
				pi_average[row][column] = (double) pi_average[row][column] / (double) frameSquare;				                        	
			}
			else
			{
        		if (column <= frameWidthHalf){
					frameWidthHalfLocal = column;
        		}
        		if (column >= (width - frameWidthHalf)){
					frameWidthHalfLocal = width - column - 1;
        		}
				if (row <= frameHeightHalf){
					frameHeightHalfLocal = row;
				}
				if (row >= (height - frameHeightHalf)){
					frameHeightHalfLocal = height - row - 1;
				}				
				frameSquareLocal = (2 * frameHeightHalfLocal + 1) * (2 * frameWidthHalfLocal + 1);

				for ( ssize_t frameRow = row - frameHeightHalfLocal; frameRow <= row + frameHeightHalfLocal; frameRow++ ){
					for ( ssize_t frameColumn = column - frameWidthHalfLocal; frameColumn <= column + frameWidthHalfLocal; frameColumn++ ){
						pi_average[row][column] += (double) PI[frameRow][frameColumn];
					}
				}
				pi_average[row][column] = (double) pi_average[row][column] / (double) frameSquareLocal;				                        	
			}
		}
	}	
	// for ( ssize_t row = frameHeightHalf; row < height - frameHeightHalf; row++ ){
	// 	for ( ssize_t column = frameWidthHalf; column < width - frameWidthHalf; column++ ){
	// 	}
	// }
	dump("Pi3-s frame average counted ...");

	for (int j = 0; j < points_number; j++)
		points[j] = 0;
	dump("created points ...");

	//если неправильно вычислить пи - будет сегфолт, что логично. смотри выше на глюки.
	//нахождение числа одинаковых вероятностей (просто как модуль от деления на шаг)
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			pi_average[row][column] = Round(pi_average[row][column], accuracy);
			// f<<to_string(pi_average[row][column])+ " "<<endl;
			/*	if (pi_average[row][column] < 0.5)
				pi_average[row][column] = 0.5;*/
			if (pi_average[row][column] <= 0){
				pi_average[row][column] = 0.001;
			}
			if (pi_average[row][column] >= 1.0){
				pi_average[row][column] = 0.999;
			}				
		}
	}
	dump("pi rounded ...");

	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			current_points_number = (int)(pi_average[row][column] / accuracy);
			if (current_points_number >= points_number){
				f<<"warning more: in " + to_string(current_points_number)+" pi is: " + to_string(pi_average[row][column])<<endl;
				current_points_number = points_number - 1;
			}
			if (current_points_number<0){
				f<<"warning zero: in " + to_string(current_points_number)+" pi is: " + to_string(pi_average[row][column])<<endl;				
				current_points_number = 0;
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
	histHeight -= 25;
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
	histHeight += 25;

	//add some text
	// histHeight -= 10;
	DrawableFont font = DrawableFont("Times New Roman",
	                                  NormalStyle,
	                                  400,
	                                  SemiCondensedStretch
	                                 );
	// drawList.push_back(DrawableFont("-misc-fixed-medium-o-semicondensed—13-*-*-*-c-60-iso8859-1"));
	drawList.push_back(font);	
 	drawList.push_back(DrawableStrokeColor(Color("black")));
 	drawList.push_back(DrawableFillColor(Color(0, 0, 0, MaxRGB)));	
	drawList.push_back(DrawableText(5,histHeight,"0"));
	drawList.push_back(DrawableText(histWidth,histHeight,"1"));	
	drawList.push_back(DrawableText(histWidth/2,histHeight,"0.5"));
	// drawList.push_back(DrawableText(3*histWidth/4,histHeight+10,"вероятность перехода"));
	drawList.push_back(DrawableText(10,15,""+to_string((int) max_point)+" px"));
	// drawList.push_back(DrawableText(histWidth/2+10,histHeight/2,"количество "));
	// drawList.push_back(DrawableText(histWidth/2+10,histHeight/2 + 15," пикселей"));
	histHeight -= 10;
	// histHeight += 5;
	// histWidth += 5;
	hist_image.draw(drawList);
	drawList.clear();

	// minimumy)
	minims[points_number] = true;
	minims[0] = true;
	points_number--;
	minims_counter=1;

	// int vicinity = (int) points_number/20;
	int vicinity = (int) pow(points_number, 0.5);
	
	//a try
/*	int vicinity = 1;
	bool a_vicinity_flag = false;
	for (int j = 1; j < points_number - 1; j++){
			for (int k = 1; k < vicinity+1; k++){
				if (!((points[j]<=points[j-k] & points[j]<points[j+k])||(points[j]<points[j-k] & points[j]<=points[j+k]))){
					
				}
			}		
	}
*/
	for (int j = 1; j < points_number; j++){
		minims[j] = false;
	}

	drawList.push_back(DrawableStrokeColor(Color("red")));
	if (minims_counter<16){
		for (int j = vicinity; j < points_number - vicinity; j++){
			minims[j] = true;
			for (int k = 1; k < vicinity; k++){
				if (!((points[j]<=points[j-k] & points[j]<points[j+k])||(points[j]<points[j-k] & points[j]<=points[j+k]))){
					minims[j] = false;
				}
			}
			if (minims[j]){				
				if ((j*accuracy) > 0.5){
					minims_counter++;
					// f<<"There is a minimum in " + to_string(j) + " equal " + to_string(j*accuracy)<<endl;
					f<<"Есть минимум в точке " + to_string(j) + " равный " + to_string(j*accuracy)<<endl;
					drawList.push_back(DrawableText(j*accuracy*histWidth,histHeight,to_string(j*accuracy)));
					drawList.push_back(DrawableLine(
						(histWidth * j / points_number) + 5,
						5,
						(histWidth * j / points_number) + 5,
						(int)histHeight-10));
				}
				else{
					minims[j] = false;
				}
			}
		}
	}
	hist_image.draw(drawList);
	drawList.clear();

	histWidth += 10;
	histHeight += 40;
	// dump(histWidth); //2800
	// dump(histHeight); //900

	hist_image.write(path + "/histogramm " + info + " " + to_string(bi) + ".jpg");
	// hist_image.write(path + "/histogramm " + to_string(bi) + ".jpg");
	hist_image.crop(Geometry(histWidth/2+10, histHeight, histWidth/2-5, 0));
	hist_image.write(path + "/histogramm _ " + info + " " + to_string(bi) + ".jpg");
	// hist_image.write(path + "/histogramm half " + to_string(bi) + ".jpg");
	dump("drew histogramm ...");

	//normal colours 255/counter
	unsigned short  *Colours = new unsigned short [minims_counter];
	Colours[0] = 0;
	// f<<"Colour " + to_string(0) + " is " + to_string(Colours[0])<<endl;
	f<<"Цвет номер " + to_string(0) + " яркости " + to_string(Colours[0])<<endl;
	for (int ii = 1; ii < minims_counter; ii++){
		Colours[ii] = (unsigned short) (255/ii);
		// f<<"Colour " + to_string(ii) + " is " + to_string(Colours[ii])<<endl;
		f<<"Цвет номер " + to_string(ii) + " яркости " + to_string(Colours[ii])<<endl;
	}

	//segmentation
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			channel_R_S[row][column] = 0;
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
									channel_R_S[row][column] = Colours[minims_flag];
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

	Color pixel_color = *(dst_view.get(1,1,1,1));

	//рисуем итоговую карту "слоя"
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			pixel_color = ColorRGB((double) channel_R_S[row][column]/255, (double) channel_R_S[row][column]/255, (double) channel_R_S[row][column]/255);
			*(dst_view.get(column,row,1,1)) = pixel_color;
		}
	}	
	dst_view.sync();
	//запись результата
	dst_image.write(path + "/output " + info + " " + to_string(bi) +"bi.bmp");

	/*for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){			
			if (pi_average[row][column] <= 0.7){
				channel_R_S[row][column] = 255;
			}
			channel_R_S[row][column] = 255 - channel_R_S[row][column];
		}
	}*/

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

	//Destructing the model of an image
	for(int i = 0; i < height; ++i) {
		delete [] channel_R_[i];
	}
	delete [] channel_R_;
}

void garbge(int argc,char **argv){
	double // #variables
		points_number = 14, //если не более 8ми сегментов, то не более 15ти значений (для 4х - 7)
		accuracy = 1 / (points_number + 1),
		element = 0, //for chi
		sequences = 0;

/*	// средние вероятности перехода (канал, направление. по 8 шт.). по сути это не надо. скоро отомрёт само.
	double 
		pi1r[8] = {0,0,0,0,0,0,0,0}, 
		pi2r[8] = {0,0,0,0,0,0,0,0}, 
		pi1g[8] = {0,0,0,0,0,0,0,0}, 
		pi2g[8] = {0,0,0,0,0,0,0,0}, 
		pi1b[8] = {0,0,0,0,0,0,0,0}, 
		pi2b[8] = {0,0,0,0,0,0,0,0};*/
	
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

	int
		x, y, k, m,
		frameWidth = 0, frameWidthHalf = 0,
		frameHeight = 0, frameHeightHalf = 0,
		frameSquare = 0,
		comporator = 64, 
		seg_number = 2, //хитрая переменная: говорит, сколько БИ мы сегментируем. причём одна должна увеличиваться, если в старшем БИ мало сегментов и влезет ещё.. во как.	
		points_number = 1000; //ok 500

	InitializeMagick(*argv);
	// Construct the image object. Seperating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	Image src_image;
	Image seg_test;

	//misc
	// string imagepath = "in/test.bmp";
	// string imagepath = "in/";
	// string imagepath = "in/1.jpg";
	string imagepath = "in/2.jpg";
	// string imagepath = "in/3.jpg";
	// string imagepath = "in/4.bmp";
	// string imagepath = "in/5.jpg";
	// string imagepath = "in/6.jpg";
	// string imagepath = "in/12 small.jpg";
	// string imagepath = "in/0_c2405_7b7a3647_orig.jpg";
	// string imagepath = "in/7637066.jpg"; //a plane

	//test 1
	// string imagepath = "in/Text0,5_0,95.bmp";
	// string imagepath = "in/Text0,5_0,95_filtr0.bmp"; 
	// string imagepath = "in/Text0,5_0,95_filtr-3.bmp"; 
	// string imagepath = "in/Text0,5_0,95_filtr-6.bmp"; 

	//test 2
	// string imagepath = "in/Text0,7_0,95.bmp";
	// string imagepath = "in/Text0,7_0,95_filtr0.bmp";
	// string imagepath = "in/Text0,7_0,95_filtr-3.bmp";
	// string imagepath = "in/Text0,7_0,95_filtr-6.bmp";

	//test 3 (4)
	// string imagepath = "in/Text2_9,7,6,5.bmp";
	// string imagepath = "in/Text2_9,7,6,5_filtr0.bmp";
	// string imagepath = "in/Text2_9,7,6,5_filtr-3.bmp";
	// string imagepath = "in/Text2_9,7,6,5_filtr-6.bmp";

	//test 4 (4)
	// string imagepath = "in/Text5_95,8,7,55.bmp";	
	// string imagepath = "in/Text5_95,8,7,55_filtr0.bmp";
	// string imagepath = "in/Text5_95,8,7,55_filtr-3.bmp";
	// string imagepath = "in/Text5_95,8,7,55_filtr-6.bmp";

	//test 5	
	// string imagepath = "in/Text0,8_0,9.bmp";
	// string imagepath = "in/Text0,8_0,9_filtr0.bmp";
	// string imagepath = "in/Text0,8_0,9_filtr-3.bmp";
	// string imagepath = "in/Text0,8_0,9_filtr-6.bmp";

	//test 6
	// string imagepath = "in/Text0,6_0,8.bmp";
	// string imagepath = "in/Text0,6_0,8_filtr0.bmp";
	// string imagepath = "in/Text0,6_0,8_filtr-3.bmp";
	// string imagepath = "in/Text0,6_0,8_filtr-6.bmp";

	//test 7
	// string imagepath = "in/Text0,75_85_1000.bmp";
	// string imagepath = "in/Text0,75_85_1000_filtr0.bmp";
	// string imagepath = "in/Text0,75_85_1000_filtr-3.bmp";
	// string imagepath = "in/Text0,75_85_1000_filtr-6.bmp";

	//test 8
	// string imagepath = "in/Text0,85_0,65.bmp";
	// string imagepath = "in/Text0,85_0,65_filtr0.bmp";
	// string imagepath = "in/Text0,85_0,65_filtr-3.bmp";
	// string imagepath = "in/Text0,85_0,65_filtr-6.bmp";

	//Троицк
	// string imagepath = "in/0_c2405_7b7a3647_orig.jpg"; 

	//lenin
	// string imagepath = "in/lenin100.bmp";
	// string imagepath = "in/lenin100_filtr-3.bmp";
	// string imagepath = "in/lenin100_filtr-6.bmp";

	//sputnik
	// string imagepath = "in/sputnik.jpg"; 
	// string imagepath = "in/sputnik.bmp"; 
	// string imagepath = "in/sputnik_filtr0.bmp"; 
	// string imagepath = "in/sputnik_filtr-3.bmp"; 
	// string imagepath = "in/sputnik_filtr-6.bmp"; 

	//initial
	// string imagepath = "in/initial.bmp"; 
	// string imagepath = "in/initial_filtr0.bmp"; 
	// string imagepath = "in/initial_filtr-3.bmp"; 
	// string imagepath = "in/initial_filtr-6.bmp"; 

	bool colourfull = false;

	try{
		// Read a file into image object
		src_image.read(imagepath);

		/*seg_test.read(imagepath); 
		seg_test.segment();
		seg_test.write(path + "/ImageMagick-6 segmentation.jpeg");*/

		Pixels src_view(src_image);

		int width = src_image.size().width();
		int height = src_image.size().height();
		int square = width * height;

		//немного про параметры окна log(x)*(x^(1/(log(x))));
		// frameWidth = 21;		
		// frameWidth = pow(log(width)/log(sqrt(2)), 1.11);		
		frameWidth = log(width)/log(sqrt(2));		
		if ((frameWidth&1) == 0) frameWidth++;
		frameWidthHalf = (frameWidth+1)/2;
		// frameHeight = 21;		
		// frameHeight = pow(log(height)/log(sqrt(2)), 1.11);
		frameHeight = log(height)/log(sqrt(2));
		if ((frameHeight&1) == 0) frameHeight++;
		frameHeightHalf = (frameHeight+1)/2;
		frameSquare = frameWidth * frameHeight;
		
		//dump
		dump("Initial image width is " + to_string(width) + "px, and height is " + to_string(height) + "px");
		// f<<"Initial image width is " + to_string(width) + "px, and height is " + to_string(height) + "px"<<endl;
		f<<"Ширина исходного изображения " + to_string(width) + "px, высота " + to_string(height) + "px"<<endl;
		// f<<"Ширина исходного изображения " + to_string(width) + "px, and height is " + to_string(height) + "px"<<endl;
		dump("So, frame width " + to_string(frameWidth) + "px, " + "frame height " + to_string(frameHeight) + "px");
		// f<<"So, frame width " + to_string(frameWidth) + "px, " + "frame height " + to_string(frameHeight) + "px"<<endl;
		f<<"Поэтому ширина окна " + to_string(frameWidth) + "px, " + "высота окна " + to_string(frameHeight) + "px"<<endl;
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

		// colourfull = true;
		//SEGMENTATION
		if (colourfull)
		{
			for (int i=7; i>3; i--){	
				// f<<"Segmenting all channels."<<endl;
				f<<"Сегментируем все каналы."<<endl;
				dump("Segmenting all channels.");
				// last argument is color_header. is 4 for bright colours, 0 for fade colours... (())
				channelSegmentation(path, "R", channel_R, channel_R_S, width, height, i, points_number, frameWidth, frameHeight, 4);
				channelSegmentation(path, "G", channel_G, channel_G_S, width, height, i, points_number, frameWidth, frameHeight, 4);
				channelSegmentation(path, "B", channel_B, channel_B_S, width, height, i, points_number, frameWidth, frameHeight, 4);			
			}
			// //рисуем итоговую карту
			// for ( ssize_t row = 0; row < height ; row++ ){
			// 	for ( ssize_t column = 0; column < width ; column++ ){
			// 		pixel_color = ColorRGB((double) channel_R_S[row][column]/255, (double) channel_G_S[row][column]/255, (double) channel_B_S[row][column]/255);
			// 		*(dst_view.get(column,row,1,1)) = pixel_color;
			// 	}
			// }		
		}
		else
		{
			// f<<"Segmenting grey."<<endl;
			// dump("Segmenting grey.");		
			for (int i=7; i>6; i--){	
				channelSegmentation(path, "grey", channel_Grey, channel_Grey_S, width, height, i, points_number, frameWidth, frameHeight, 4);
/*				//рисуем итоговую карту "слоя"
				for ( ssize_t row = 0; row < height ; row++ ){
					for ( ssize_t column = 0; column < width ; column++ ){
						pixel_color = ColorRGB((double) channel_Grey_S[row][column]/255, (double) channel_Grey_S[row][column]/255, (double) channel_Grey_S[row][column]/255);
						*(dst_view.get(column,row,1,1)) = pixel_color;
					}
				}	
				dst_view.sync();
				//запись результата
				dst_image.write(path + "/output " + to_string(i) +"bi.bmp");
*/			}
		}

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

		// dst_view.sync();s
		//запись результата
		// dst_image.write(path + "/output.bmp");
		src_image.write(path + "/initial.jpg");

		

		dump("Done"); 		
	}
	catch( Exception &error_ ){       
		cout << "Caught exception: " << error_.what() << endl;
		f << "Caught exception: " << error_.what() << endl;
	}    

	std::time_t t_finish = std::time(0);
	f<<to_string(t_finish - t_start) + " seconds past."<<endl;
	f<<"Прошло " + to_string(t_finish - t_start) + " секунд."<<endl;
	dump(to_string(t_finish - t_start) + " seconds past.");
	f.close();
	return 0; 
}