#include "./lib/ff_farm.cpp"

struct firstStage: ff_node_t<float> {
	firstStage(const size_t len):len(len) {}
		float* svc(float *) {
		for(long i=0;i<len;++i)
			ff_send_out(new float(i));
		return EOS; // End-Of-Stream
	}
	const size_t len;
};

// 2nd stage
struct secondStage: ff_node_t<float> {
	float* svc(float *task ) {
		float &t = *task;
		t = t*t;
		return task;
}};
// 3rd stage
struct thirdStage: ff_node_t<float> {
	float* svc(float *task ) {
		float &t = *task;
		sum +=t;
		delete task;
		return GO_ON;
	}
	void svc_end() { std::cout << "sum = " << sum << "\n"; }
	float sum = {0.0};
};

int main(int argc, char * argv[]){
	
	const size_t nworkers = std::stol(argv[1]);

	firstStage  first(nworkers);

	//ff_Pipe<float> pipe(first, second, third);
    std::vector<std::unique_ptr<ff_node> > W1, W2;
    // workers are multi-output nodes
    for(size_t i=0;i<10;++i){ //W.push_back(make_unique<secondStage>());
    	secondStage* sec = new secondStage();
    	thirdStage* third = new thirdStage();
    	W1.push_back(make_unique<ff_Pipe<float>>(sec, third));
    	//W2.push_back(make_unique<thirdStage>());
    }
    ff_Farm<float> farm1(std::move(W1));
    //ff_Farm<float> farm2(std::move(W2));
    // the Emitter needs the internal load-balancer  
    farm1.add_emitter(first);
    farm1.remove_collector();  // removing the default collector
    //farm.wrap_around();       // creating a feedback channel between workers and emitter

    //ff_Pipe<> pipe(first, farm1, farm2);  // build the pipeline


    std::cout << "Starting computatation.\n"; 
    ffTime(START_TIME);
    if (farm1.run_and_wait_end()<0) {
        error("Running farm ");
        return -1;
    }
    /*
    if (farm.run_and_wait_end()<0) {
        error("Running farm ");
        return -1;
    }
    */
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << " (ms)\n";
    //std::cout << "Time: " << ffTime(GET_TIME) << 
    //			" nImgs: " << nImgs << " par_deg: " <<
    //			nworkers << " source: " << path_in <<"\n";

	return 0;
}