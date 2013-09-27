#ifndef ThreadsafeQueue_class
#define ThreadsafeQueue_class
//! destroy mutex and destroy cond is missing
#include <pthread.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

pthread_mutex_t mutex_string_queue = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t      cond_string_queue  = PTHREAD_COND_INITIALIZER;

template < class T >
class ThreadsafeQueue {
	public: 
	
	ThreadsafeQueue(){
		pthread_mutex_init (&mutex , NULL );
	}
	
	//std::string front(){
	T front(){
		pthread_mutex_lock(&mutex);
		T ret=queue.front();
		pthread_mutex_unlock(&mutex);
		return ret;	
		
	}
	bool empty ( ) const{
		bool ret;
		pthread_mutex_lock(&mutex);
		ret=queue.empty();
		pthread_mutex_unlock(&mutex);
		return ret;						
	}
	//void push ( const std::string& str){
	void push ( const T& str){
		pthread_mutex_lock(&mutex);
		queue.push(str);
		pthread_mutex_unlock(&mutex);	
	}
	void pop ( ){
		pthread_mutex_lock(&mutex);
		queue.pop();
		pthread_mutex_unlock(&mutex);
	}
	size_t size(){
		pthread_mutex_lock(&mutex);
		int size=queue.size();		
		pthread_mutex_unlock(&mutex);
		return size;
		
	}
	
	private:
		//std::queue<std::string> queue;

		//pthread_mutex_t mutex;
		std::queue<T> queue;
	
};

class StringQueue{
public:	
	void push(const std::string str){
		pthread_mutex_lock(&mutex_string_queue);
		queue.push(str);
		int rc = pthread_cond_broadcast(&cond_string_queue);
		pthread_mutex_unlock(&mutex_string_queue);

	}
	std::string pop(){
		pthread_mutex_lock(&mutex_string_queue);
		
		while(queue.empty()){
			//std::cout <<"queue blockieren \n";
			//we wait for a new message...
			int rc=pthread_cond_wait(&cond_string_queue, &mutex_string_queue);
			//std::cout <<"weiter \n";
		}
		
		std::string	ret=queue.front();
		queue.pop();
		//std::cout<<" Eingangsqueuesize: "<<queue.size();
		pthread_mutex_unlock(&mutex_string_queue);
		return ret;

	}	

	std::string pop_no_blocking(){
		if(queue.empty()) return "";
		else{
			//we need a mutex here! cause 2 threads could make a front() and get the same 1 data... then they both make
			// a pop() and will erase 2!!! datas 
			pthread_mutex_lock(&mutex_string_queue);
			std::string	ret=queue.front();
			queue.pop();
			pthread_mutex_unlock(&mutex_string_queue);
			return ret;
		}	
	}

private:
	ThreadsafeQueue<std::string> queue;
	
};

#endif
