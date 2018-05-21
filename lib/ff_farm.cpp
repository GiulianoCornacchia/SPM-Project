#include "./ff/pipeline.hpp"
#include "./ff/farm.hpp"
#include <ff/parallel_for.hpp>
#include "util.cpp"

using namespace ff;

img_t watermark;
std::string path_in, path_out, enable_pf;
size_t  nImgs  = 0;
int max_nw = 0,
    chuck_sz = 0;

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

struct source: ff_node_t<std::string> {
    
    std::string* svc(std::string *in) {
        struct dirent *entry;
        DIR *dir = opendir(path_in.c_str());
        if (dir == NULL) {
            return EOS;
        }
        int it = 0;
        while ((entry = readdir(dir)) != NULL && it < nImgs) {
            //std::cout << entry->d_name << " read." << std::endl;
            std::string s (entry->d_name);
            if (s.compare(".") != 0
                && s.compare("..") != 0)
            {
                std::string* file_name = new std::string(s);
                //std::string result_name(path_out+s);
                ff_send_out(file_name);
                it++;
            }
        }
        closedir(dir);
        return EOS;
    }
};

struct stage1: ff_node_t<std::string, std::pair<std::string, img_t* >> {
        
    std::pair<std::string, img_t*>* svc(std::string * task) { 
        
        std::string &file_name = *task;
        std::string path(path_in+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t* img = new img_t(path.c_str());
        std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(file_name, img);
        delete task;
        return p;
    }
};

struct stage2: ff_node_t<std::pair<std::string, img_t*>> {

    std::pair<std::string, img_t*>* svc(
        std::pair<std::string, img_t*> * task) { 
        
        std::pair<std::string, img_t*> &p = *task;
        //Process string here
        pipe_process_image((*(p.second)), watermark, false);
        return task; 
    }
};

struct stage3: ff_node_t<std::pair<std::string, img_t*>> {
    std::pair<std::string, img_t*>* svc(
        std::pair<std::string, img_t*> * task) { 
        
        std::pair<std::string, img_t*> &p = *task;
        //Process string here
        std::string result_name(path_out+p.first);
        try{
            (*(p.second)).save_jpeg(result_name.c_str(), 100);
        }catch(cimg_library::CImgIOException ex){
            std::cout << "Error occurred while saving " <<
                    result_name << std::endl;
        }
        delete p.second;
        delete task;
        return GO_ON; 
    }

    //void svc_end() { if( errors > 0 ) std::cout << "#errors = " << errors << "\n"; }
};

/*
*   Stage 1&2 merged and parallel for.
**/
struct stage12: ff_node_t<std::string, std::pair<std::string, img_t*>> {
    std::pair<std::string, img_t*>* svc(
        std::string * task) { 
        
        std::string &file_name = *task;
        std::string path(path_in+file_name);
        //std::cout << "[in stage1_" <<  i << "] " << path << " extracted." << std::endl;
        img_t* img = new img_t(path.c_str());
        std::pair<std::string, img_t*>* p = new std::pair<std::string, img_t*>(file_name, img);
        //delete &path;
        delete task;
        pipe_process_image((*(p->second)), watermark, false);
        return p; 
    }
};

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
            ff_process_image((*(p->second)), watermark, chuck_sz, false);
        else
            pipe_process_image((*(p->second)), watermark, false);
        return p; 
        
    }
};
