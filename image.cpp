#include <ImageMagick-6/Magick++.h>
#include <iostream>
using namespace std;
using namespace Magick;
#include <fstream>
#include <iomanip>

#include <list>     // подключаем заголовок списка
#include <iterator> // заголовок итераторов

/*
TODO:
1. преобразование изображения в массивы (12 двумерных - список массивов)
2. функция "собирания" картинки из массива (?)
3. сегментация одного "массива" - уже есть: в дипломе
4. что-то там про линейную фильтрацию "массива"
*/

void dump(string variable)
{
	cout << variable << endl;
}

void drawBinaryImages(Image src_image){
	   Pixels src_view(src_image);
		int width = src_image.size().width();
	   int height = src_image.size().height();
	   unsigned char r=0;

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
			currentBIpointer->write("out/bi"+to_string(++i)+".jpg");	//Выполняем метод myAction объекта Object
			iterator++;		//Берём следующий элемент
		}
}

int main(int argc,char **argv)
{
	//немного вспомогательных переменных
	double accuracy = 0.0055;
   //координаты и размеры окна
   int
      x, y, k, m,
      frameWidth = 0, frameWidthHalf = 0,
      frameHeight = 0, frameHeightHalf = 0,
      frameSquare = 0;

	InitializeMagick(*argv);
   // Construct the image object. Seperating image construction from the
   // the read operation ensures that a failure to read the image file
   // doesn't render the image object useless.
   Image src_image;
   Image seg_test;
   /*
	unsigned char 
		r = 0, g = 0, b = 0, 
		r1 = 0, g1 = 0, b1 = 0, 
		r2 = 0, g2 = 0, b2 = 0, 
		temp1r = 0, temp2r = 0, temp1g = 0, 
		temp2g = 0, temp1b = 0, temp2b = 0;
	*/		
	/*
	double 
		pi1r[8] = {0,0,0,0,0,0,0,0}, 
		pi2r[8] = {0,0,0,0,0,0,0,0}, 
		pi1g[8] = {0,0,0,0,0,0,0,0}, 
		pi2g[8] = {0,0,0,0,0,0,0,0}, 
		pi1b[8] = {0,0,0,0,0,0,0,0}, 
		pi2b[8] = {0,0,0,0,0,0,0,0};
	*/

	//чтобы рисовать гистограммы
	int histHeight = 350, histWidth = 800;
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
   for (int j = 1; j < 10; j++)
   {
       drawList.push_back(DrawableLine(
           ((histWidth - 5) * j / 10) + 5,
           histHeight,
           ((histWidth - 5) * j / 10) + 5,
           histHeight - 10));
   }

   hist_image.draw(drawList);
   drawList.clear();		

   try {
   	// Read a file into image object
  		src_image.read( "in/test.bmp" );
  		// src_image.read( "in/cards.jpg" );
  		
  		//seg_test.read( "in/test.bmp" );  		
  		//seg_test.segment();
  		//seg_test.write("out/segmentation test.jpeg");

  		//нарисовать изображения (!)
  		// drawBinaryImages(src_image); //(!)

	   Pixels src_view(src_image);

		int width = src_image.size().width();
	   int height = src_image.size().height();
	   int square = width * height;

	   //немного про параметры окна
      frameWidth = sqrt(width);
      if ((frameWidth&1) == 0) frameWidth--;
      frameWidthHalf = (frameWidth-1)/2;
      frameHeight = sqrt(height);
      if ((frameHeight&1) == 0) frameHeight--;
      frameHeightHalf = (frameHeight-1)/2;
	   frameSquare = frameWidth * frameHeight;
		//dump
		dump("Initial image width is " + to_string(width) + "px, and height is " + to_string(height) + "px");
		dump("So, frame width " + to_string(frameWidth) + "px, " + "frame height " + to_string(frameHeight) + "px");

		//описывает поток для записи данных в файл
		ofstream f;
		//открываем файл в режиме записи, режим ios::out устанавливается по умолчанию
		f.open("out/output.txt", ios::out);

		f<<"Initial image width: "<<to_string(width)<<"px"<<endl;
		f<<"Initial image height: "<<to_string(height)<<"px"<<endl;
		//dump end
	   //@TODO: научиться создавать картинки нормально
		Image dst_image(Geometry(width, height), Color(MaxRGB, MaxRGB, MaxRGB, 0));
		dst_image.type(TrueColorType);
		dst_image.modifyImage();
		Pixels dst_view(dst_image);
		/*
		Image b_image(Geometry(width, height), Color(MaxRGB, MaxRGB, MaxRGB, 0));		
		b_image.type(TrueColorType);
		b_image.modifyImage();
		b_image.strokeColor("black"); // Outline color 
	   b_image.fillColor("white"); // Fill color 
	   b_image.strokeWidth(1);
	   */
		// Выбираем все пиксели от позиции 0,0 до ширины и высоты (то есть просто все пиксели)
		PixelPacket *pixels = dst_view.get(0,0,width,height);

		//Declare a model of an image.
		unsigned char **channel_R = new unsigned char*[height];
		unsigned char **channel_G = new unsigned char*[height];
		unsigned char **channel_B = new unsigned char*[height];
		for(int i = 0; i < height; i++) {
			//dump("got here " + to_string(i));
	    	channel_R[i] = new unsigned char[width];
	    	channel_G[i] = new unsigned char[width];
	    	channel_B[i] = new unsigned char[width];
		}

		for ( ssize_t row = 0; row < height ; row++ ){
			//dump(to_string(100*row/height)+"% done");
			for ( ssize_t column = 0; column < width ; column++ ){
				
				Color pixel_color = *(src_view.get(column,row,1,1));
				ColorRGB pixel_rgb(pixel_color);
								
				channel_R[row][column] = (unsigned char) 255*pixel_rgb.red();
				channel_G[row][column] = (unsigned char) 255*pixel_rgb.green();
				channel_B[row][column] = (unsigned char) 255*pixel_rgb.blue();

				/*
				Color pixel_color = *(src_view.get(column,row,1,1));
				ColorRGB pixel_rgb(pixel_color);
				r = (unsigned char) 255*pixel_rgb.red();
				comporator=255;
				for (int ii=0; ii<4; ii++){
					if (comporator^r){
						*(dst_view.get(column,row,1,1)) = Color("black"); 
					}
					comporator>>=1;					
				}
				*/
				//*(dst_view.get(column,row,1,1)) = *(src_view.get(column,row,1,1)); 
				// *pixels++=green; 
			}
		}

		// dump("fun " + to_string(channel_R[10][20]));

		for ( ssize_t row = 1; row < height ; row++ ){
			for ( ssize_t column = 1; column < width ; column++ ){
				/*
				//need to use different logic. use chi. dont be proud.
				Color pixel_color = *(src_view.get(column,row,1,1));		
				ColorRGB pixel_rgb(pixel_color);		

				Color pixel_color1 = *(src_view.get(column-1,row,1,1));		
				ColorRGB pixel_rgb1(pixel_color1);		

				Color pixel_color2 = *(src_view.get(column,row-1,1,1));		
				ColorRGB pixel_rgb2(pixel_color2);		

				r = (unsigned char) 255*pixel_rgb.red();
				r1 = (unsigned char) 255*pixel_rgb1.red();
				r2 = (unsigned char) 255*pixel_rgb2.red();

				g = (unsigned char) 255*pixel_rgb.green();
				g1 = (unsigned char) 255*pixel_rgb1.green();
				g2 = (unsigned char) 255*pixel_rgb2.green();

				b = (unsigned char) 255*pixel_rgb.blue();
				b1 = (unsigned char) 255*pixel_rgb1.blue();
				b2 = (unsigned char) 255*pixel_rgb2.blue();

				temp1r = r^r1;
				temp2r = r^r2;
				temp1b = g^g1;
				temp2b = g^g2;
				temp1g = b^b1;
				temp2g = b^b2;
				*/						
				// вот это по-любому не работает... ибо что-то не так с этим умножением
				/*
				for (int i = 0; i<8; i++)
				{
					if ((1<<i)^temp1r)			
						pi1r[i]+=1;
					if ((1<<i)^temp2r)
						pi2r[i]+=1;				
					if ((1<<i)^temp1b)				
						pi1b[i]+=1;				
					if ((1<<i)^temp2b)				
						pi2b[i]+=1;				
					if ((1<<i)^temp1g)				
						pi1g[i]+=1;				
					if ((1<<i)^temp2g)				
						pi2g[i]+=1;												
				}
				*/

				//*(dst_view.get(column,row,1,1)) = *(src_view.get(column,row,1,1)); 
				//*pixels++=green; 					
			}
		}

		//???
		//*(dst_view.get(0,0,width,height)) = *(src_view.get(0,0,width,height)); 

		//test
		//dump(to_string(2&253));

		//Destructing the model of an image
		/*
		for(int i = 0; i < height; ++i) {
	    	delete [] channel_R[i];
		}
		delete [] channel_R;
		*/
		/*
		f<<"RED GREEN BLUE pi1,pi2: "<<endl;
		for (int i = 0; i<8; i++)
		{
			pi1r[i]/=square;
			pi2r[i]/=square;
			//f<<pi1r[i]<<","<<pi2r[i]<<" ";

			pi1g[i]/=square;
			pi2g[i]/=square;
			//f<<pi1g[i]<<","<<pi2g[i]<<" ";

			pi1b[i]/=square;
			pi2b[i]/=square;
			//f<<pi1b[i]<<","<<pi2b[i]<<endl;
		}
		*/

		dst_view.sync();

		//запись результата
		dst_image.write("out/total.jpg");
		hist_image.write("out/histogramm.jpg");

		f.close();

      dump("Done");    
   }    
   catch( Exception &error_ )    
   {       
   	cout <<"Caught exception: " << error_.what() << endl;       
   	return 1;    
   }    
   return 0; 
}