#include "./lib/ff_farm.cpp"

int main(int argc, char * argv[]){
    if(argc <= 6) {
        std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file par_deg nImgs on(0)/off(1)(par_for) [max_nw_pf chunk_sz]" <<
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
    enable_pf = "off";
    if(std::atoi(argv[6]) == 0 && argc == 9){
        //std::cout << "Parallel for enabled\n";
        max_nw = std::atoi(argv[7]);
        chuck_sz = std::atoi(argv[8]);
        enable_pf = std::string("on");
    }

    source src;
   
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i){ //W.push_back(make_unique<secondStage>());
        stage12_opt* first = new stage12_opt();
        stage3* third = new stage3();
        W.push_back(make_unique<ff_Pipe<float>>(first, third));
    }

    ff_Farm<float> farm(std::move(W));
 
    farm.add_emitter(src);
    farm.remove_collector();

    std::cerr << "ff_computatation_opt.\n"; 
    ffTime(START_TIME);
    if (farm.run_and_wait_end()<0) {
        error("Running farm ");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << 
                " nImgs: " << nImgs << " par_deg: " <<
                nworkers << " source: " << path_in << " "<<enable_pf ;
    if (enable_pf == "on")
        std::cout << " nw_par_for: " << max_nw << " chunk: " << chuck_sz;
    std::cout<<"\n";

    return 0;
}
