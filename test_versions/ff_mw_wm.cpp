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
    //nImgs = std::stol(argv[5]);
    const size_t nworkers = std::stol(argv[4]);
    aVg = to_bool(argv[5]);

    source src;
   
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i)
    	W.push_back(make_unique<stage123>());


    ff_Farm<float> farm(std::move(W));
 
    farm.add_emitter(src);
    farm.remove_collector();
    std::cerr << "ff_master-worker_watermark\n";
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