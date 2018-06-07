/*
*filename: ff_watermark.cpp
**
*** Author: Mario Leonardo Salinas
*** Student Code: 502795
*** Master Degree curriculum: Data and Knowledge - Science and Technologies
**      
*/

/*
*   C++&Fastflow implementation of a parallel watermark application.
*   The ff_node business-logic code can can be found in lib/ff_farm.cpp.
*   Lines 26-30 contain some global variables defined also in ff_farm.cpp.
**/
#include "./lib/ff_farm.cpp"

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
	const size_t nworkers = std::stol(argv[4]);
    aVg = to_bool(argv[5]);

    source src;
   
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i){ 
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