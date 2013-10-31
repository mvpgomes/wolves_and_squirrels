#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <map>
using namespace std;
/*main function*/
int main(int argc, char **argv){
  int size, n_elements, row, column;
  char element;
  map<int,int> keys;
  map<int,int>::iterator it;
  ofstream file;

  char array[] = {'i','s','t','w'};

  if( argc != 3 ){
    cout << "Wrong number of arguments!" << endl;
    exit(-1);
  } else { 
    size = atoi(argv[2]);
    n_elements = atoi(argv[3]);
  }
  if( n_elements > size * size ){
    cout << "Number of elements exceed the allowed!" << endl;
  } else { file.open(argv[1]); }

  file << size << endl;
  int i = 0;
  while( i < n_elements ){
    // random generator for the positions and elements
    row = rand() % size;
    column = rand() % size;
    element = array[rand()%4];
    if( keys.find(row)->second != column){
      keys.insert(make_pair(row,column));
      file << row << " " << column << " " << element << endl;
      i++;
    }
  }
  file.close();
  return 0;
}
