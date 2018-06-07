/*
*filename: pipeline.cpp
**
*** Author: Mario Leonardo Salinas
*** Student Code: 502795
*** Master Degree curriculum: Data and Knowledge - Science and Technologies
**      
*/

/*
*   C++ sequential implementation of a watermark application.
*   Global variables and functions used are all defined in lib/util.cpp
**/
#include "util.cpp"

/*
*   Global variables:
*       - watermark @ it's the variable that will contain the stamp image
*       - path_in, path_out @ are respectively the source and result folder paths
*		- task_queue @ it's a Single-Producer-Multiple-Consumers queue between the farm emitter and the pipelines.
*       - stage2task_queue @ it's a vector of single-prod/cons queues between stage1 and 2, one for each pipeline
*       - stage3task_queue @ the same as stage2 one but it's between stage 2 and 3
*		- nImgs @ variable that, after the emitter has finished, will contain the inputsize
*       - aVg @ boolean used to decide whether or not to apply a non-plain watermark [look at Figure 1a/b in the report for more details]
**/

cimg_library::CImg<int> watermark;
std::string path_in, path_out;
queue<std::string> task_queue; 
std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > > stage2task_queue;
std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > > stage3task_queue;
int nImgs = 0;
bool aVg = false;

/*
*   function that implements the farm emitter task. 
*   It has to:
*       1. retrieve the paths of the images and push them as pipeline tasks
*       2. manage pipeline termination (sends EOS)
**/
void Source(int par_deg){
	struct dirent *entry;
    DIR *dir = opendir(path_in.c_str());
    if (dir == NULL) {
    	for(int j = 0; j < par_deg; j++)
    	task_queue.push("EOS");
        return;
    }
    while ((entry = readdir(dir)) != NULL /*&& it < nImgs*/) {
    	//std::cout << entry->d_name << " read." << std::endl;
    	std::string s (entry->d_name);
    	if (s.compare(".") != 0
    		&& s.compare("..") != 0)
    	{
			std::string file_name(s);
			task_queue.push(file_name);
			nImgs++;
	    }
    }

    for(int j = 0; j <= par_deg; j++)
    	task_queue.push("EOS");

    closedir(dir);
}
/*
*	Stage1_i:
*
*	until EOS 
*		1. reads from task_queue a path
*		2. loads the image and puts in stage2_i_task_queue
*		   a pair<string filename, Cimg<int> img>
*	endw
*	
*	push EOS in stage2_i_task_queue
**/
void stage1(int i){ 
	//std::cout << "[in stage1_" <<  i << "] init with p_in = " <<
	//		  path_in << std::endl;
	while(true){
		std::string file_name = task_queue.pop();
		if(file_name.compare("EOS") == 0){
			//std::cout << "[in stage1_" <<  i << "] EOS recieved" << std::endl;
			std::pair<std::string, cimg_library::CImg<int>> p("EOS", cimg_library::CImg<int>());
			stage2task_queue[i].push(p);
			return;
		}
		std::string path(path_in+file_name);
		//std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
		cimg_library::CImg<int> img(path.c_str());
		std::pair<std::string, cimg_library::CImg<int>> p(file_name, img);
		stage2task_queue[i].push(p);
	}
}
/*
*	Stage2_i:
*
*	until EOS 
*		1. reads from stage2_i_task_queue a 
*		   pair<string filename, CImg<int> img>
*		2. process the image				
*		3. push the watermarked pair in stage3_i_task_queue
*	endw
*	
*	push EOS in stage3_i_task_queue
**/
void stage2(int i){
	//std::cout << "[in stage2_" <<  i << "] init." << std::endl;
	while(true){
		std::pair<std::string, cimg_library::CImg<int> > p = stage2task_queue[i].pop();
		if(p.first.compare("EOS") == 0){
		//	std::cout << "[in stage2_" <<  i << "] EOS recieved" << std::endl;
			stage3task_queue[i].push(p);
			return;
		}
		//std::cout << "[in stage2_" <<  i << "] " << p.first << " extracted." << std::endl;
		pipe_process_image(p.second, watermark, aVg);
		stage3task_queue[i].push(p);
	}	
}
/*
*	Stage3_i:
*
*	until EOS 
*		1. reads from stage3_i_task_queue a 
*		   pair<string filename,CImg<int> img>
*		2. writes the image on disk			
*	endw
*	
*/
void stage3(int i){
	//std::cout << "[in stage3_" <<  i << "] init with p_out = " <<
	//		  path_out << std::endl;
	while(true){
		std::pair<std::string, cimg_library::CImg<int> > p = stage3task_queue[i].pop();
		if(p.first.compare("EOS") == 0){
			//std::cout << "[in stage3_" <<  i << "] EOS recieved" << std::endl;
			return;
		}
		//std::cout << "[in stage3_" <<  i << "] "<< p.first << " extracted." << std::endl;
		std::string result_name(path_out+p.first);
		try{
			p.second.save_jpeg(result_name.c_str(), 100);
		}catch(cimg_library::CImgIOException ex){
			std::cout << "Error occurred while saving " <<
					result_name << std::endl;
			continue;
		}
	}	
}

/*
*	Function that implements the worker buisness logic in the master-worker architecture used in some tests.
*
*	until EOS 
*		1. reads from task_queue a path
*		2. loads the image
*		3. process the image		
*		4. writes the image on disk			
*	endw
*/
void stage123(){
	while(true){
		std::string file_name = task_queue.pop();
		if(file_name.compare("EOS") == 0){
			return;
		}
	    std::string path(path_in+file_name);
	    std::string result_name(path_out+file_name);
	    //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
	    img_t img(path.c_str());
	    pipe_process_image(img, watermark, aVg);
	    try{
	        img.save_jpeg(result_name.c_str(), 100);
	    }catch(cimg_library::CImgIOException ex){
	        std::cerr << "Error occurred while saving " <<
	                result_name << std::endl;
	    }
	}
}