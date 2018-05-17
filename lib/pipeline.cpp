#include "util.cpp"

cimg_library::CImg<int> watermark;
queue<std::string> task_queue; 
std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > > stage2task_queue;
std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > > stage3task_queue;
std::string path_in;
std::string path_out;

void Source(int nImgs, int par_deg){
	struct dirent *entry;
    DIR *dir = opendir(path_in.c_str());
    if (dir == NULL) {
    	for(int j = 0; j < par_deg; j++)
    	task_queue.push("EOS");
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
*/
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
*		   parir<string filename, CImg<int> img>
*		2. process the image				
*		3. push the watermarked pair in stage3_i_task_queue
*	endw
*	
*	push EOS in stage3_i_task_queue
*/
void stage2(int i){
	//std::cout << "[in stage2_" <<  i << "] init." << std::endl;
	while(true){
		std::pair<std::string, cimg_library::CImg<int> > p = stage2task_queue[i].pop();
		if(p.first.compare("EOS") == 0){
		//	std::cout << "[in stage2_" <<  i << "] EOS recieved" << std::endl;
		//	std::pair<std::string, cimg_library::CImg<int> > p1("EOS", cimg_library::CImg<int>());
			stage3task_queue[i].push(p);
			return;
		}
		//std::cout << "[in stage2_" <<  i << "] " << p.first << " extracted." << std::endl;
		pipe_process_image(p.second, watermark, false);
		stage3task_queue[i].push(p);
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