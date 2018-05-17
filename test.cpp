#include "./lib/util.cpp"

int main(int argc, char * argv[]){
	if(argc <= 4) {
		std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file nImgs" <<
		std::endl << "*************************************************" << 
		std::endl << "*** Images and watermark must have SAME SIZE. ***" << 
		std::endl << "*************************************************" << std::endl;
		return(0);
	}
	cimg_library::CImg<int> watermark(argv[3]);
	std::string path_in = argv[1];
	std::string path_out = argv[2];
	int nImgs = atoi(argv[4]);

	auto start   = std::chrono::high_resolution_clock::now();
	
    seq_wm(nImgs, path_in, path_out, watermark);

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Elapsed time " << msec << " msecs on " <<
		nImgs << " imgs" << std::endl;

	return 0;
}
