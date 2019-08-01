#include <cstdlib>
#include <cmath>


using namespace std;

#ifndef pi
#define pi (3.14159265f)
#endif
const int GABORCENTER = 5;
const float GABORSD = 2.25;
class outOfBoundsError{
public:
	//image size and x,y coordinate that was trying to be reached
	int w,h,x,y;
	outOfBoundsError(){w=h=x=y=-1;};
	outOfBoundsError(int wd,int ht,int X,int Y){w=wd;h=ht;x=X;y=Y;};
};

struct Position{
	int x;
	int y;
};

struct Pixel{
	Position pos;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

class Gabor{
private:
	float *d, f, a;
	unsigned int s;
public:
	Gabor(){d=NULL;f=a=s=0;};
	Gabor(int sz, float fr, float an,float (*func)(float));
	~Gabor();
	void setup(int sz, float fr, float an,float (*func)(float));
	int size(){return s;};
	float freq(){return f;};
	float angle(){return a;};
	float data(int x, int y);
	float data(Position p);
	
};

template <class T>
class ImageData{
private:
	T *d;
	int h,w,c;
public:
	ImageData(){d=NULL;h=w=c=0;};
	ImageData(T *src, int wd, int ht, int ch){setup(src, wd, ht, ch);};
	ImageData(T *src, int wd, int ht){setup(src,wd,ht,1);};
	ImageData(T *src, int size){setup(src, size, size, 1);};
	~ImageData(){delete[] d;};
	void setup(T *src, int wd, int ht, int ch){d=src;h=ht;w=wd;c=ch;};
	int height(){return h;};
	int width(){return w;};
	int Nchannels(){return c;};
	T data(int x, int y);
	T data(Position p){return data(p.x, p.y);};
	T *dataPointer(){return d;};
	void set(int x, int y, T value);
	void set(Position p, T value){set(p.x, p.y, value);};
};
