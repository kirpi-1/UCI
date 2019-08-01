template <typename T>
class SharedData{
public:
	SharedData(unsigned int maxSize){MAX_SIZE=maxSize;data = new T[MAX_SIZE];};
	~SharedData(){delete[] T;};
	T* next();
	T* prev();
	T* current();
	T* beg();
	T* end();
	unsigned int size(){return size;};	
	bool isReady(){return ready;};
	//T operator[]
protected:
	bool ready;
	bool resize();
	unsigned int curSize;
	unsigned int MAX_SIZE;
	int index;
	T *data;
};