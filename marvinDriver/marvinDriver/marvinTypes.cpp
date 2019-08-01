#include "marvinTypes.h"

template <typename T>
MyBuffer<T>::MyBuffer(int ms){
	initialize(ms);
}
template <typename T>
MyBuffer<T>::~MyBuffer(){
	kill();
}
template <typename T>
void MyBuffer<T>::kill(){
	if(data!=NULL)
		delete[] data;
}	
template <typename T>
void MyBuffer<T>::initialize(int ms){
	maxSize = ms;
	pos = 0;
	size = 0;
	if(data!=NULL)
		delete[] data;
	data = new T[maxSize];
	read = true;
}

template <typename T>
void MyBuffer<T>::push(T d){
	size++;
	if(size>maxSize){
		T *temp = new T[maxSize*2];
		for(int i=0;i<size;i++)
			temp[i] = data[i];
		delete[] data;
		data = temp;
		maxSize = 2*maxSize;
	}
	data[size-1]=d;	
}
MyBuffer<SharedData<Command>>::MyBuffer(int ms){
	initialize(ms);
	for(int i=0;i<this->maxSize;i++)
		this->data[i].data=NONE;
}
MyBuffer<SharedData<Command>>::~MyBuffer(){
	kill();
}

