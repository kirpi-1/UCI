#include "gabor.h"
#include <iostream>


void Gabor::setup(int sz, float fr, float an, float(*func)(float)){
	s = sz;
	f = fr;
	a = an;
	d = new float[s*s];
	
	double cosor=cos(a);
	double sinor=sin(a);
	double arg;
	printf("freq:%f\tangle:%f\n",f,a);
	//printf("cosor:%f\tsinor:%f\n",cosor,sinor);
	for(int r=0;r<s;r++){
		for(int c=0;c<s;c++){
			arg=((r-GABORCENTER)*sinor+(c-GABORCENTER)*cosor);
			float tmp=(c-GABORCENTER)*(c-GABORCENTER)+(r-GABORCENTER)*(r-GABORCENTER)/(GABORSD*GABORSD);
			d[r*s+c] = func(2.0f*pi*f*arg)*exp(-0.5*tmp);
			//printf("arg:%f\t",arg);
			//printf("%f  ",d[r*s+c]);
		}
		//printf("\n\n");
	}
}
Gabor::Gabor(int sz, float fr, float an, float(*func)(float)){
	setup(sz,fr,an,func);
}
Gabor::~Gabor(){
	delete[] d;
}
float Gabor::data(int x, int y){
	if(x>=0 && x<s && y>=0 && y<s)
		return d[y*s+x];
	else{		
		outOfBoundsError e(s,s,x,y);
		throw e;
	}
}

float Gabor::data(Position p){
	return data(p.x, p.y);
}

template <class T>
T ImageData<T>::data(int x, int y){
	if(x>=0 && x<w && y>=0 && y<h)
		return d[y*w+x];
	else{		
		outOfBoundsError e(w,h,x,y);
		throw e;
	}
}

template <class T>
void ImageData<T>::set(int x, int y, T value){
	if(x>=0 && x<w && y>=0 && y<h)
		d[y*w+x]=value;
	else{
		outOfBoundsError e(w,h,x,y);
		throw e;
	}		
}
//need these to avoid linker errors when using the type ImageData in main
template class ImageData<int>;
template class ImageData<float>;
template class ImageData<char>;
template class ImageData<unsigned char>;
