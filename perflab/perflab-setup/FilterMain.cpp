#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
 
using namespace std;
 
#include "rtdsc.h"
 
// Testing rsync
 
//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);
 
int main(int argc, char **argv)
{
 
  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }
 
  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];
 
  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }
 
  Filter *filter = readFilter(filtername);
 
    double sum = 0.0;
    int samples = 0;
    double sample;// declare sample outside
    struct cs1300bmp *input = new struct cs1300bmp;//--------
    struct cs1300bmp *output = new struct cs1300bmp;//------- both declared out of loops to stop repetitive defines.
 
  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
 
    // Replace ok variable with direct function call
     
    if (cs1300bmp_readfile( (char *) inputFilename.c_str(), input)) { // took out variable "ok" and replaced its definiton where it was being utilized
      sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);
 
}
 
struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());
 
  if ( ! input.bad() ) {
    int size = 0;
    input >> size; //size
    Filter *filter = new Filter(size);
    int div;
    input >> div;//divisor
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
    int value;
    input >> value;// value
    filter -> set(i,j,value);
      }
    }
    return filter;
  }
  return NULL;//return NULL to correct error
}
 
 
double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{
    long long cycStart, cycStop;
    cycStart = rdtscll();
    
    output -> width = input -> width;
    output -> height = input -> height;
    
   // int theSize = filter -> dim;
    int theDivisor = filter -> divisor; 
    int theWidth = (input -> width ) -1;    // declared outside of loops
    int theHeight = (input -> height ) -1;    
    int v, a, l =0; // inititalized int v,a,l
     
    
     // unrolling original loops
    for(int plane = 0; plane <3; plane++){          //plane
		for(int col = 1; col < theWidth; col++){//column
			for(int row =1; row < theHeight; row++){//row
             
            v = input -> color[col - 1][plane][row - 1] * filter->data[0]; //column 1-1 =0 plane = 0 row 1-1 = 0 * data[0]
            a = input -> color[col][plane][row - 1] * filter->data[1];//column 2-1 =1 plane=1 row2-1 = 1 * data[1]
            l = input -> color[col + 1][plane][row - 1] * filter->data[2];
             
             
            v += input -> color[col - 1][plane][row] * filter->data[3];
            a += input -> color[col][plane][row] * filter->data[4];
            l += input -> color[col + 1][plane][row] * filter->data[5];
             
             
            v += input -> color[col - 1][plane][row + 1] * filter->data[6];
            a += input -> color[col][plane][row + 1] * filter->data[7]; 										//1  2  3
																												//~  ~  ~
            l += input -> color[col + 1][plane][row + 1] * filter->data[8];     // 9 indexs total.. 3x3 matrix -> 01 02 03  (1)
																												//10 11 12  (2)
																												//20 21 22  (3)
      
        int value = v+a+l;  //value add all of them added
        
             if(theDivisor > 1)
				value /= theDivisor;
				
             if(value <0)
				value = 0;
				
             if (value > 255)
				value = 255;
				
             output -> color[col][plane][row] = value;
           
     }
 }
}
    cycStop = rdtscll();
    double diff = cycStop - cycStart;
    double diffPerPixel = diff / (output -> width * output -> height);
    
    
    fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
    diff, diff / (output -> width * output -> height));
    return diffPerPixel;
}

