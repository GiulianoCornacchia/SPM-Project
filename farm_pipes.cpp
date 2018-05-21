#include "./lib/pipeline.cpp"

int main(int argc, char * argv[]){
	if(argc < 6) {
		std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file par_deg nImgs [on (enable opt)]" <<
		std::endl << "*************************************************" << 
		std::endl << "*** Images and watermark must have SAME SIZE. ***" << 
		std::endl << "*************************************************" << std::endl;
		return(0);
	}

	watermark = cimg_library::CImg<int>(argv[3]);
	path_in = std::string(argv[1]);
	path_out = std::string(argv[2]);
	int nImgs = atoi(argv[5]),
		par_deg = atoi(argv[4]);

	stage2task_queue = std::vector<queue<std::pair<std::string, img_t*>* > >(par_deg);
	stage3task_queue = std::vector<queue<std::pair<std::string, img_t*>* > >(par_deg);

  	std::vector<std::thread> tids;
  	std::cerr << "farm_pipes\n";
	auto start   = std::chrono::high_resolution_clock::now();

	for(int i=0; i<par_deg; i++) {
		if(argc == 7 && std::string(argv[6]).compare("on")==0)
			tids.push_back(std::thread(stage12, i));
		else{
			tids.push_back(std::thread(stage1, i));
			tids.push_back(std::thread(stage2, i));
		}
		tids.push_back(std::thread(stage3, i));
	}
	tids.push_back(std::thread(Source, nImgs, par_deg));

	for(std::thread& t: tids)
		t.join();
	
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Time: " << msec << 
                " nImgs: " << nImgs << " par_deg: " <<
                par_deg << " source: " << path_in << " stage12: ";

    if(argc == 7 && std::string(argv[6]).compare("on")==0)
    	std::cout << "on\n";
    else std::cout << "off\n";

	return 0;
}
