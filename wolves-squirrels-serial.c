#include <stdio.h>
#include <stdlib.h>

/* Constants */
#define EMPTY 'e'

/* Global variables */
int wolf_breeding_period;
int squirrel_breeding_period;
int wolf_starvation_period;
int side_size;

/* Structure that represents a position in the grid. */
struct position{
  int row;
  int column;
};
/* Array of structures that represents a 2-D grid. */
struct world{
  char type;
  int breeding_period;
  int starvation_period;
} **world;

/* initialize_world(FILE *file) : function responsible for allocate mememory for the 2-D grid.*/
void initialize_world(FILE *file){
  int i, j;
  // Initialize the 2-D grid.
  if( fscanf(file, "%d", &side_size) == 1){
    world = (struct world **) malloc(side_size * sizeof(struct world *));
    for(i=0; i<side_size; i++)
      world[i] = (struct world *) malloc(side_size * sizeof(struct world));
  }
  // fill the 2-D grid of empty positions
  for(i=0; i<side_size; i++){
    for(j=0;j<side_size; j++){
      world[i][j].type = EMPTY;
    }
  }
}
/* populate_world(FILE *file) : function responsible for populate the struct world according the input file. */
void populate_world(FILE *file){
  int i, size;
  int row, column;
  char type;
  // Reads the file and populate the 2-D grid. 
  while( fscanf(file, "%d %d %c", &row, &column, &type) == 3){
    switch(type){
    case 'i':
      world[row][column].type = type;
      break;
    case 't':
      world[row][column].type = type;
      break;
    case 's':
      world[row][column].type = type;
      world[row][column].breeding_period = squirrel_breeding_period;
 break;
    case 'w':
      world[row][column].type = type;
      world[row][column].breeding_period = wolf_breeding_period;
      world[row][column].starvation_period = wolf_starvation_period;
      break;
    }
  }
} 
/* compute_direction(int row, int column, int p): function responsible to select the direction of moviment. */
int select_direction(int row, int column, int p){
  // compute cell
  int c = row*side_size + column;
  // calculate the moviment
  int cell = c % p;
  return cell;
}
/* compute_wolf_moviment(int row, int column): function responsible to find the possible moviments for the wolf. */
struct position* compute_wolf_moviment(int row, int column){
  int i= 0;
  struct position* array = (struct position *)malloc(sizeof(struct position) * 4);
  
  if(world[row-1][column].type == 'e'  || world[row-1][column].type == 's'){
    array[i].row = row-1;
    array[i].column = column;
    i++;
  }
  if(world[row][column+1].type == 'e' || world[row][column+1].type == 's'){
    array[i].row = row;
    array[i].column = column+1;
    i++;
  }
  if(world[row+1][column].type == 'e' || world[row+1][column].type == 's'){
    array[i].row = row+1;
    array[i].column = column;
    i++;
  }
  if(world[row][column-1].type == 'e' || world[row][column-1].type == 's'){
    array[i].row = row;
    array[i].column = column-1;
  }
  return array;
}
/* compute_squirrel_moviment(int row, int column): function responsible to find the possible moviments for the squirrel. */
struct position* compute_squirrel_moviment(int row, int column){
  int i= 0;
  struct position* array = (struct position *)malloc(sizeof(struct position) * 4);
  
  if(world[row-1][column].type == 'e' || world[row-1][column].type == 't'){
    array[i].row = row-1;
    array[i].column = column;
    i++;
  }
  if(world[row][column+1].type == 'e' || world[row][column+1].type == 't'){
    array[i].row = row;
    array[i].column = column+1;
    i++;
  }
  if(world[row+1][column].type == 'e' || world[row+1][column].type == 't'){
    array[i].row = row+1;
    array[i].column = column;
    i++;
  }
  if(world[row][column-1].type == 'e' || world[row][column-1].type == 't'){
    array[i].row = row;
    array[i].column = column-1;
  }
  return array;
}
/* print_world() : function that's print the actual state of the world*/
void print_world(){
  int i, j, k;
  
  for(i=0;i<side_size;i++){
    printf("----");
  }
  printf("\n");
  for(i=0;i<side_size;i++){
    for(j=0;j<side_size;j++){
      printf("| %c ", world[i][j].type); 
    }
    printf("|\n");
    for(k=0;k<side_size;k++){
      printf("----");
    }
    printf("\n");
  }
}
/* main : function responsible for the main interaction of the program. */
int main(int argc, char *argv[]){
  char *file_name;
  int n_generations;
  FILE *file;
  // Verifies the input parameters of the program and initializes the variables.
  if(argc == 6){
    file_name = argv[1];
    wolf_breeding_period = atoi(argv[2]);
    squirrel_breeding_period = atoi(argv[3]);
    wolf_starvation_period = atoi(argv[4]);
    n_generations = atoi(argv[5]);
  }
  else { 
    printf("Error! : Incorrect number of arguments.\n");
    exit(-1);
  }
  // Open the input file and initialize the world.
  file = fopen(file_name,"r");
  
  if(file == NULL){
    printf("Error! : Could not open the file.");
    exit(-1);
  }
  else {
    initialize_world(file);
    populate_world(file);
    fclose(file);
  }
  // prints the world
  print_world();
  
  return 0;
}
