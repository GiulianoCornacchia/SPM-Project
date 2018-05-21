#include "util.cpp"

img_t watermark;
queue<std::string> task_queue; 
std::vector<queue<std::pair<std::string, img_t* > *> > stage2task_queue;
std::vector<queue<std::pair<std::string, img_t* > *> > stage3task_queue;
std::string path_in;
std::string path_out;

void Source(int nImgs, int par_deg){
	std::string eos_string("EOS");
	struct dirent *entry;
    DIR *dir = opendir(path_in.c_str());
    if (dir == NULL) {
    	for(int j = 0; j < par_deg; j++)
    	task_queue.push(eos_string);
        return;
    }
    int it = 0;
    while ((entry = readdir(dir)) != NULL && it < nImgs) {
    	//std::cout << entry->d_name << " read." << std::endl;
    	std::string s (entry->d_name);
    	if (s.compare(".") != 0
    		&& s.compare("..") != 0)
    	{
			std::string file_name(s);
			//std::string result_name(path_out+s);
			task_queue.push(file_name);
			it++;
	    }
    }
    for(int j = 0; j <= par_deg; j++){
        task_queue.push(eos_string);
    }
    closedir(dir);
    return;
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
*/
void stage1(int i){ 
	//std::cout << "[in stage1_" <<  i << "] init with p_in = " <<
	//		  path_in << std::endl;
	while(true){
		std::string task = task_queue.pop();  
		//std::string &file_name = *task;
		if(task.compare("EOS") == 0){
			// std::cout << "[in stage1_" <<  i << "] EOS recieved" << std::endl;
			img_t *eos_img = new img_t();
			std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>("EOS", eos_img);
			stage2task_queue[i].push(p);
			return;
		}
		std::string path(path_in+task);
		//std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
		img_t *img = new img_t(path.c_str());
		std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(task, img);
		stage2task_queue[i].push(p);
	}
}

/*
*	Stage2_i:
*
*	until EOS 
*		1. reads from stage2_i_task_queue a 
*		   parir<string filename, CImg<int> img>
*		2. process the image				
*		3. push the watermarked pair in stage3_i_task_queue
*	endw
*	
*	push EOS in stage3_i_task_queue
*/
void stage2(int i){
	
	while(true){
		std::pair<std::string, img_t*>* task = stage2task_queue[i].pop();
		std::pair<std::string, img_t*> &p = *task;
		//std::cout << "[in stage2_" <<  i << "] recieved " << p.first << std::endl;

		if(p.first.compare("EOS") == 0){
			stage3task_queue[i].push(task);
			return;
		}
		pipe_process_image(*(p.second), watermark, false);
		stage3task_queue[i].push(task);
	}	
}
/*
*	Stage3_i:
*
*	until EOS 
*		1. reads from stage3_i_task_queue a 
		   pair<string filename,CImg<int> img>
*		2. writes the image on disk			
*	endw
*	
*/
void stage3(int i){
	while(true){
		std::pair<std::string, img_t*>* task = stage3task_queue[i].pop();
		std::pair<std::string, img_t*> &p = *task;
		if(p.first.compare("EOS") == 0){
			delete task;
			//std::cout << "[in stage3_" <<  i << "] EOS recieved" << std::endl;
			return;
		}
		//std::cout << "[in stage3_" <<  i << "] "<< p.first << " extracted." << std::endl;
		std::string result_name(path_out+(p.first));
		try{
			(*(p.second)).save_jpeg(result_name.c_str(), 100);
		}catch(cimg_library::CImgIOException ex){
			std::cout << "Error occurred while saving " <<
					result_name << std::endl;
			continue;
		}
		delete task;
	}
}

void stage12(int i){ 
    while(true){
		std::string task = task_queue.pop();  
		//std::string &file_name = *task;
		if(task.compare("EOS") == 0){
			//std::cout << "[in stage1_" <<  i << "] EOS recieved" << std::endl;
			img_t * eos_img = new img_t();
			std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>("EOS", eos_img);
			stage3task_queue[i].push(p);
			return;
		}
		std::string path(path_in+task);
		//std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
		img_t *img = new img_t(path.c_str());
		std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(task, img);
		pipe_process_image((*img), watermark, false);
		stage3task_queue[i].push(p);
	}
};