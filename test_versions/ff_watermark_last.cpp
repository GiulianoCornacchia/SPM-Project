#include "./lib/ff_farm.cpp"

//crontab -e

int main(int argc, char * argv[]){
    if(argc <= 5) {
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
   	/*
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i){ //W.push_back(make_unique<secondStage>());
        stage12_opt* first = new stage12_opt();
        stage3* third = new stage3();
        W.push_back(make_unique<ff_Pipe<float>>(first, third));
    }

    ff_Farm<float> farm(std::move(W));
 
    farm.add_emitter(src);
    farm.remove_collector();
*/  std::vector<std::unique_ptr<ff_node> > W1, W2;
    for(size_t i=0;i<nworkers;i++){
    	W1.push_back(make_unique<stage12>());
    	W2.push_back(make_unique<stage3>());
    	W2.push_back(make_unique<stage3>());
    }
    ff_Farm<float> farm1(std::move(W1));
    farm1.add_emitter(src);
	ff_Farm<float> farm2(std::move(W2));

    ff_Pipe<> pipe(farm1, farm2);  // build the pipeline
 
    std::cerr << "ff_watermark_opt.\n"; 
    ffTime(START_TIME);
    if (pipe.run_and_wait_end()<0) {
        error("Running pipe ");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << 
                " nImgs: " << nImgs << " par_deg: " <<
                nworkers << " source: " << path_in << "\n";

    return 0;
}