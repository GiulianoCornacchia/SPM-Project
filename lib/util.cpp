/*
*filename: util.cpp
**
*** Author: Mario Leonardo Salinas
*** Student Code: 502795
*** Master Degree curriculum: Data and Knowledge - Science and Technologies
**      
*/

/*
*   Library containing:
*		a. the definition of the blocking queue used 
*			in the plain C++ implementation
*		b. all the other functions and procedures 
*			used by all the implementations.
**/
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <cstddef>
#include <string>
#include <thread>
#include <cmath> 
#include <vector>
#include <chrono>
#include <algorithm>
#include <dirent.h>
#include <sys/types.h>	
#include "CImg.h"
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>

#define EOS NULL

using img_t = cimg_library::CImg<int>;

template <typename T>
class queue{
	private:

	  mutable std::mutex      d_mutex;
	  mutable std::condition_variable d_condition;
	  std::deque<T>           d_queue;

	public:
	  queue(){}
	  /*
	  * Constructor overriding to manage non-movable mutex:
	  *
	  * Move Constructor 
	  */
	  queue(queue&& a)
	    {
	        std::unique_lock<std::mutex> lock(a.d_mutex);
	        d_queue = std::move(a.d_queue);
	    }
	  /*
	  * Move Assignment 
	  */
	  queue& operator=(queue&& a)
	    {
	      if (this != &a)
	      {
	        std::unique_lock<std::mutex> lock(d_mutex, std::defer_lock);
	        d_queue = std::move(a.d_queue);
	      }
	      return *this;
	    }
	  /*
	  * Copy Constructor
	  */
	  queue(const queue& a)
	    {
	        std::unique_lock<std::mutex> lock(a.d_mutex);
	        d_queue = a.d_queue;
	    }
	  /*
	  * Copy Assignment 
	  */
	  queue& operator=(const queue& a)
	    {
	        if (this != &a)
	        {
	          std::unique_lock<std::mutex> lock(a.d_mutex, std::defer_lock);
	          d_queue = a.d_queue;
	        }
	        return *this;
	    }

	  void push(T value) {
	    {
	      std::unique_lock<std::mutex> lock(this->d_mutex);
	      d_queue.push_front(value);
	    }
	    this->d_condition.notify_one();
	  }

	  T pop() {
	    std::unique_lock<std::mutex> lock(this->d_mutex);
	    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
	    T rc = this->d_queue.back();
	    this->d_queue.pop_back();
	    return rc;
	  }
};

bool to_bool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}

void apply_watermark(bool avg, img_t &img, int c, int r){
	for(int ch = 0; ch < img.spectrum(); ch++)
		if(avg)
			img(c,r,0,ch) = img(c,r,0,ch)/(1.8);
		else
			img(c,r,0,ch) = 0;

}

void process_image(
	const char* file_name,
 	img_t img, 
 	img_t watermark,
 	bool average,
 	float &avg_proc,
 	float &avg_save)
{
  	int width = img.width();
    int height = img.height();

	auto start   = std::chrono::high_resolution_clock::now();

	for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
        	if(watermark(c, r) < 50)
        		apply_watermark(average, img, c, r);

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	avg_proc += (float)msec;
    
	start   = std::chrono::high_resolution_clock::now();

  	img.save_jpeg(file_name, 100);
  		
  	elapsed = std::chrono::high_resolution_clock::now() - start;
	msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	avg_save += (float) msec;
};

void pipe_process_image(
	img_t &img, 
 	img_t watermark,
 	bool average){

	int width = img.width(),
    	height = img.height();

    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
        	if(watermark(c, r) < 50)
        		apply_watermark(average, img, c, r);
}

void pipe_process_image(
	img_t &img, 
 	img_t watermark,
 	bool average,
 	bool is_stage_2){

	int width = img.width(),
    	height = img.height(),
    	lim_y = height/2,
    	start_y = 0;

    if (!is_stage_2){
    	start_y =  lim_y;
    	lim_y = height;
    }

    for (int r = start_y; r < lim_y; r++)
        for (int c = 0; c < width; c++)
        	if(watermark(c, r) < 50)
        		apply_watermark(average, img, c, r);
}

void populate_task(
	int nImgs,
	int par_deg,
	const char *path_in,
	queue<std::string> &task_queue
	){
	struct dirent *entry;
    DIR *dir = opendir(path_in);
    if (dir == NULL) {
        return;
    }
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < nImgs) {
    	//std::cout << entry->d_name << " read." << std::endl;
    	std::string s (entry->d_name);
    	if (s.compare(".") != 0
    		&& s.compare("..") != 0)
    	{
    		//std::cout << "Processing: " << s << "["<< i << "]" << std::endl;
			std::string file_name(s);
			task_queue.push(file_name);
			i++;
	    }
    }

    for(int j = 0; j < par_deg; j++)
    	task_queue.push("EOS");

    closedir(dir);
}

void seq_wm(
	int &nImgs,
	std::string path_in,
	std::string path_out, 
	cimg_library::CImg<int> watermark,
	bool avg,
	float &avg_load, 
	float &avg_proc, 
	float &avg_save)
{
    struct dirent *entry;
    DIR *dir = opendir(path_in.c_str());
    if (dir == NULL) {
        return;
    }
	auto start = 0, elapsed = 0, msec = 0;
    while ((entry = readdir(dir)) != NULL /*&& i < nImgs*/) {
    	//std::cout << entry->d_name << " read." << std::endl;
    	std::string s (entry->d_name);
    	if (s.compare(".") != 0
    		&& s.compare("..") != 0)
    	{
			std::string file_name(path_in+s);
			std::string result_name(path_out+s);

			auto start   = std::chrono::high_resolution_clock::now();
	    	cimg_library::CImg<int> img(file_name.c_str());
	    	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	    	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	    	avg_load += (float) msec;

	    	process_image(result_name.c_str(), img, watermark, avg, avg_proc, avg_save);
	    	nImgs++;
	    }
    }
	if(nImgs > 0){
	    avg_proc /= nImgs;
	    avg_save /= nImgs;
	    avg_load /= nImgs;
	    //std::cout << "avgs[L, P, S]: " << avg_load << ", " << avg_proc << ", " <<
		//	avg_save << std::endl;
	}
    closedir(dir);
}
