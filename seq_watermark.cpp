/*
*filename: seq_watermark.cpp
**
*** Author: Mario Leonardo Salinas
*** Student Code: 502795
*** Master Degree curriculum: Data and Knowledge - Science and Technologies
**      
*/

/*
*   C++ sequential implementation of a watermark application.
*	Global variables and functions used are all defined in lib/util.cpp
**/
#include "./lib/util.cpp"

int main(int argc, char * argv[]){
    if(argc < 5) {
        std::cout << "Usage is: " << argv[0] << " source_folder result_folder watermark_file avg" <<
        std::endl << "*************************************************" << 
        std::endl << "*** Images and watermark must have SAME SIZE. ***" << 
        std::endl << "*************************************************" << std::endl;
        return(0);
    }

	cimg_library::CImg<int> watermark(argv[3]);
	std::string path_in = argv[1];
	std::string path_out = argv[2];
	bool aVg = to_bool(argv[4]);
	int nImgs = 0;
	float avg_load = 0.0, avg_proc = 0.0, avg_save = 0.0;

	std::cerr << "seq_watermark\n";
	auto start   = std::chrono::high_resolution_clock::now();
	
    seq_wm(nImgs, path_in, path_out, watermark, aVg, avg_load, avg_proc, avg_save);

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Time: " << msec << " nImgs: " <<
		nImgs << " source: " << path_in <<
		"avgs: " << avg_load << ", " << avg_proc << ", " <<
		avg_save << std::endl;

	return 0;
}
