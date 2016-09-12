//-*-c++-*-
#ifndef _Filter_h_
#define _Filter_h_

using namespace std;

class Filter {
  
public:
  Filter(int _dim);
  int get(int r, int c);
  void set(int r, int c, int value);
	
	int divisor;
  int dim;
  int *data;

  int getDivisor();
  void setDivisor(int value);

  int getSize();
};

#endif