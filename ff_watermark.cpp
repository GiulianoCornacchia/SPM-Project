#include "./lib/ff_farm.cpp"

int main(int argc, char * argv[]){
	if(argc <= 4) {
		std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file par_deg nImgs" <<
		std::endl << "*************************************************" << 
		std::endl << "*** Images and watermark must have SAME SIZE. ***" << 
		std::endl << "*************************************************" << std::endl;
		return(0);
	}

	watermark = cimg_library::CImg<int>(argv[3]);
	path_in = std::string(argv[1]);
	path_out = std::string(argv[2]);
	nImgs = std::stol(argv[5]);
	const size_t nworkers = std::stol(argv[4]);

    source src;
   
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i){ //W.push_back(make_unique<secondStage>());
    	stage1* first = new stage1();
    	stage2* second = new stage2();
    	stage3* third = new stage3();
    	W.push_back(make_unique<ff_Pipe<float>>(first, second, third));
    }

    ff_Farm<float> farm(std::move(W));
 
    farm.add_emitter(src);
    farm.remove_collector();
    std::cerr << "ff_watermark\n";
    ffTime(START_TIME);
    if (farm.run_and_wait_end()<0) {
        error("Running farm ");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << 
    			" nImgs: " << nImgs << " par_deg: " <<
					nworkers << " source: " << path_in <<"\n";

	return 0;
	}
