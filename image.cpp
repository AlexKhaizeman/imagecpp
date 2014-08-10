#include <ImageMagick-6/Magick++.h>
#include <iostream>
using namespace std;
using namespace Magick;

void dump(string variable)
{
	cout << variable << endl;
}

int main(int argc,char **argv)
{
  // Construct the image object. Seperating image construction from the
  // the read operation ensures that a failure to read the image file
  // doesn't render the image object useless.
  Image src_image;

  try {
    // Read a file into image object
    src_image.read( "test.jpg" );

    Pixels src_view(src_image);

	int width = src_image.size().width();
    int height = src_image.size().height();

	Image dst_image( Geometry(width, height), Color(MaxRGB, MaxRGB, MaxRGB, 0));
	dst_image.type(TrueColorType);
	dst_image.modifyImage();

	Color green("green");
	Pixels dst_view(dst_image);

	dump(to_string(width));
	dump(to_string(height));	

	// Выбираем все пиксели от позиции 0,0 до ширины и высоты (то есть просто все пиксели)
	// PixelPacket *pixels = dst_view.get(0,0,width,height);
	for ( ssize_t row = 0; row < height ; row++ ){
		for ( ssize_t column = 0; column < width ; column++ ){
			Color pixel_color = *(src_view.get(column,row,1,1));
			ColorRGB pixel_rgb(pixel_color);
			
			dump(to_string(pixel_rgb.red()));
			dump(to_string(pixel_rgb.green()));
			dump(to_string(pixel_rgb.blue()));

			*(dst_view.get(column,row,1,1)) = *(src_view.get(column,row,1,1)); 
			// *pixels++=green; 
		}
	}

	dst_view.sync();

	dst_image.write("total.jpg");
  }
  catch( Exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  return 0;
}