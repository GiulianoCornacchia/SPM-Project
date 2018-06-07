/*
*filename: ff_farm.cpp
**
*** Author: Mario Leonardo Salinas
*** Student Code: 502795
*** Master Degree curriculum: Data and Knowledge - Science and Technologies
**      
*/

/*
*   Library containing global variables and ff_node definitions
*   used in the Fastflow version of the application.
*   From line on there are also pieces of code used in the design and test phases. 
**/
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>
#include "util.cpp"

using namespace ff;
/*
*   Global variables, where img_t it's an alias for Cimg<int>:
*       - watermark @ it's the variable that will contain the stamp image
*       - path_in, path_out @ are respectively the source and result folder paths
*       - enable_pf @ string used to decide whether or not to apply a parallel for [user choice in a test version]
*       - nImgs @ variable that, after the emitter has finished, will contain the inputsize
*       - max_nw, chunk_sz @ parallel for parameters
*       - aVg @ boolean used to decide whether or not to apply a non-plain watermark [look at Figure 1a/b in the report for more details]
**/
img_t watermark;
std::string path_in, path_out, enable_pf;
size_t  nImgs  = 0;
int max_nw = 0,
    chunk_sz = 0;
bool aVg = false;
/*
*   ff_node that implements the farm emitter. 
*   Its tasks are:
*       1. retrieve the paths of the images and push them as pipeline tasks
*       2. manage pipeline termination (sends EOS)
**/
struct source: ff_node_t<std::string> {

    std::string* svc(std::string *in) {
        struct dirent *entry;
        DIR *dir = opendir(path_in.c_str());
        if (dir == NULL) {
            return EOS;
        }
        while ((entry = readdir(dir)) != NULL/* && it < nImgs*/) {
            //std::cout << entry->d_name << " read." << std::endl;
            std::string s (entry->d_name);
            if (s.compare(".") != 0
                && s.compare("..") != 0)
            {
                std::string* file_name = new std::string(s);
                ff_send_out(file_name);
                nImgs++;
            }
        }
        closedir(dir);

        return EOS;
    }
};
/*
*   ff_node that implements the first stage of the pipeline. 
*   It has to:
*       1. retrieve a path from the emitter
*       2. load the corresponding image into a CImg object
*       3. create a pair
*           <file_name, image_pointer>
*          the file name is needed by the third stage in order to save the image
*       4. send to the second stage a pointer to the pair
**/
struct stage1: ff_node_t<std::string, std::pair<std::string, img_t* >> {
        
    std::pair<std::string, img_t*>* svc(std::string * task) { 
        
        std::string &file_name = *task;
        //Path reconstruction
        std::string path(path_in+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t* img = new img_t(path.c_str());
        std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(file_name, img);
        delete task;
        return p;
    }
};
/*
*   ff_node that implements the second stage of the pipeline. 
*   It has to:
*       1. retrieve a pair from the first stage that contains the image to be processed and the correspondig file name
*       2. watermark the image
*       3. send the pair to the third stage
**/
struct stage2: ff_node_t<std::pair<std::string, img_t*>> {

    std::pair<std::string, img_t*>* svc(
        std::pair<std::string, img_t*> * task) { 
        
        std::pair<std::string, img_t*> &p = *task;
        //Image watermarking
        pipe_process_image((*(p.second)), watermark, aVg);
        return task; 
    }
};
/*
*   ff_node that implements the third and last stage of the pipeline. 
*   This last node is in charge of saving the watermarked images recived from the second stage.
**/
struct stage3: ff_node_t<std::pair<std::string, img_t*>> {
    std::pair<std::string, img_t*>* svc(
        std::pair<std::string, img_t*> * task) { 
        
        std::pair<std::string, img_t*> &p = *task;
        //Path reconstruction that uses the name of the file, namely the first element of the pairs created in the first stage.
        std::string result_name(path_out+p.first);
        try{
            (*(p.second)).save_jpeg(result_name.c_str(), 100);
        }catch(cimg_library::CImgIOException ex){
            std::cerr << "Error occurred while saving " <<
                    result_name << std::endl;
        }
        delete p.second;
        delete task;
        return GO_ON; 
    }
};
/*
*   This portion of code is the one used during the various tests.
*   -------------------------
*   Stage 1&2 merged.
**/
struct stage12: ff_node_t<std::string, std::pair<std::string, img_t*>> {
    std::pair<std::string, img_t*>* svc(
        std::string * task) { 
        
        std::string &file_name = *task;
        std::string path(path_in+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t* img = new img_t(path.c_str());
        std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(file_name, img);
        delete task;
        pipe_process_image((*(p->second)), watermark, aVg);
        return p; 
    }
};

/*
*   Function that applies a parallel watermark 
*   using a Fastflow parallel for.
**/
void ff_process_image(
    img_t &img, 
    img_t watermark,
    int chunk,
    bool average
    ){
    ParallelFor pf(max_nw);
    int width = img.width(),
        height = img.height();
    pf.parallel_for_idx(0,height,1, chunk, [&](const long start, const long stop, const long thid) {        for(long r=start;r<stop;r++)
        for(int r = start; r < stop; r++)
            for (int c = 0; c < width; c++)
                if(watermark(c, r) < 50)
                    apply_watermark(average, img, c, r);
    });
}

/*
*   Stage 1&2 merged and parallel for.
**/
struct stage12_opt: ff_node_t<std::string, std::pair<std::string, img_t*>> {
    std::pair<std::string, img_t*>* svc(
        std::string * task) { 
        
        std::string &file_name = *task;
        std::string path(path_in+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t* img = new img_t(path.c_str());
        std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(file_name, img);
        
        delete task;
        if(enable_pf.compare("on")==0)
            ff_process_image((*(p->second)), watermark, chunk_sz, aVg);
        else
            pipe_process_image((*(p->second)), watermark, false);
        return p; 
        
    }
};

/*
*   Stage123 for the master worker implementation
**/
struct stage123: ff_node_t<std::string> {
        
    std::string* svc(std::string * task) { 

        std::string &file_name = *task;
        std::string path(path_in+file_name);
        std::string result_name(path_out+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t img(path.c_str());
        pipe_process_image(img, watermark, aVg);
        try{
            img.save_jpeg(result_name.c_str(), 100);
        }catch(cimg_library::CImgIOException ex){
            std::cerr << "Error occurred while saving " <<
                    result_name << std::endl;
        }
        return task;
    }
};
