brew install imagemagick --with-magic-plus-plus
g++ image.cpp -o image `Magick++-config --cppflags --cxxflags --ldflags —libs`