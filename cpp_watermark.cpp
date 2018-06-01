#include "./lib/pipeline.cpp"

int main(int argc, char * argv[]){
    if(argc < 6) {
        std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file par_deg avg" <<
        std::endl << "*************************************************" << 
        std::endl << "*** Images and watermark must have SAME SIZE. ***" << 
        std::endl << "*************************************************" << std::endl;
        return(0);
    }

    watermark = cimg_library::CImg<int>(argv[3]);
    path_in = std::string(argv[1]);
    path_out = std::string(argv[2]);
    //nImgs = std::stol(argv[5]);
    int par_deg = std::stol(argv[4]);
    aVg = to_bool(argv[5]);

	stage2task_queue = std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > >(par_deg);
	stage3task_queue = std::vector<queue<std::pair<std::string, cimg_library::CImg<int> > > >(par_deg);

  	std::vector<std::thread> tids;
  	std::cerr << "cpp_watermark\n";
	auto start   = std::chrono::high_resolution_clock::now();



	for(int i=0; i<par_deg; i++) {
		tids.push_back(std::thread(stage1, i));
		tids.push_back(std::thread(stage2, i));
		tids.push_back(std::thread(stage3, i));
	}
	tids.push_back(std::thread(Source, par_deg));

	for(std::thread& t: tids)
		t.join();
	
	/*
	std::thread farm = std::thread(Farm, path_in, path_out, par_deg, nImgs);
	farm.join();
	*/
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Time: " << msec << 
                " nImgs: " << nImgs << " par_deg: " <<
                par_deg << " source: " << path_in << "\n";


	return 0;
}
