ifndef FF_ROOT 
FF_ROOT		= ./lib
endif

CXX		= g++ -std=c++11 -D cimg_display=0 #-DNO_DEFAULT_MAPPING 
INCLUDES	= -I $(FF_ROOT) 
CXXFLAGS  	= 

LDFLAGS 	= -pthread
OPTFLAGS	= -O3 -finline-functions

TARGETS		= seq_watermark      \
		  cpp_watermark  \
		  ff_watermark

.PHONY: all clean cleanall
.SUFFIXES: .cpp 


%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

all		: $(TARGETS)
	echo "\n---------------------------------\nSource files succesfully compiled.\n---------------------------------\n \nUsage is:\n./obj_file_name source_folder result_folder watermark_file [par_deg] avg\n \n*************************************************\n*** Images and watermark must have SAME SIZE. ***\n*************************************************";
clean		: 
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~



