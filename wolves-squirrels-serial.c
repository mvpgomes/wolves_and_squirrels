#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define EMPTY 'e'
#define RED 0
#define BLACK 1

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
} *world;

struct world **rows; 

/* initialize_rows(FILE *file) : function responsible for allocate mememory for the 2-D grid.*/
void initialize_world(FILE *file){
  int i, j;
  // Initialize the 2-D grid.
  if( fscanf(file, "%d", &side_size) == 1){
    world = (struct world *) malloc(side_size * side_size * sizeof(struct world ));
    rows = (struct world **) malloc(side_size * sizeof(struct world *));
    for(i=0; i<side_size; i++)
      rows[i] = &world[i*side_size];
  }
  // fill the 2-D grid of empty positions
  for(i=0; i<side_size; i++){
    for(j=0;j<side_size; j++){
      rows[i][j].type = EMPTY;
    }
  }
}
/* populate_rows(FILE *file) : function responsible for populate the struct rows according the input file. */
void populate_world(FILE *file){
  int i, size;
  int row, column;
  char type;
  // Reads the file and populate the 2-D grid. 
  while( fscanf(file, "%d %d %c", &row, &column, &type) == 3){
    switch(type){
    case 'i':
      rows[row][column].type = type;
      break;
    case 't':
      rows[row][column].type = type;
      break;
    case 's':
      rows[row][column].type = type;
      rows[row][column].breeding_period = squirrel_breeding_period;
 break;
    case 'w':
      rows[row][column].type = type;
      rows[row][column].breeding_period = wolf_breeding_period;
      rows[row][column].starvation_period = wolf_starvation_period;
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

/* find_squirrels(int * array): function that receives an array with positions and return the squirrel position if that exists. */
struct position * find_squirrels(struct position * array){
  int i;
  int size = 4;
  struct position * position = (struct position *)malloc(sizeof(struct position));
  for(i=0; i < size; i++){
    int row = array[i].row;
    int column = array[i].column;
    if(rows[row][column].type == 's')
      position = &array[i];
  }
  return position;
}

/* compute_wolf_moviment(int row, int column): function responsible to find the possible moviments for the wolf. */
struct position* compute_wolf_moviment(int row, int column){
  int i= 0;
  struct position* array = (struct position *)malloc(sizeof(struct position) * 4);
  
  if(row > 0 && (rows[row-1][column].type == 'e'  || rows[row-1][column].type == 's')){
    array[i].row = row-1;
    array[i].column = column;
    i++;
  }
  if(column < side_size && (rows[row][column+1].type == 'e' || rows[row][column+1].type == 's')){
    array[i].row = row;
    array[i].column = column+1;
    i++;
  }
  if(row < side_size && (rows[row+1][column].type == 'e' || rows[row+1][column].type == 's')){
    array[i].row = row+1;
    array[i].column = column;
    i++;
  }
  if(column > 0 && (rows[row][column-1].type == 'e' || rows[row][column-1].type == 's')){
    array[i].row = row;
    array[i].column = column-1;
  }
  return array;
}
/* compute_squirrel_moviment(int row, int column): function responsible to find the possible moviments for the squirrel. */
struct position* compute_squirrel_moviment(int row, int column){
  int i= 0;
  struct position* array = (struct position *)malloc(sizeof(struct position) * 4);
  
  if(row > 0 && (rows[row-1][column].type == 'e' || rows[row-1][column].type == 't')){
    array[i].row = row-1;
    array[i].column = column;
    i++;
  }
  if(column < side_size && (rows[row][column+1].type == 'e' || rows[row][column+1].type == 't')){
    array[i].row = row;
    array[i].column = column+1;
    i++;
  }
  if(row < side_size && (rows[row+1][column].type == 'e' || rows[row+1][column].type == 't')){
    array[i].row = row+1;
    array[i].column = column;
    i++;
  }
  if(column > 0 && (rows[row][column-1].type == 'e' || rows[row][column-1].type == 't')){
    array[i].row = row;
    array[i].column = column-1;
  }
  return array;
}
/* process_squirrel(int row, int column, struct world **rows) */
process_squirrel(int row, int column, struct world **rows) {
}
/* process_wolf(int row, int column, struct world **rows) */
process_wolf(int row, int column, struct world **rows) {
}
/* process_sub_world(int redBlack)*/
void process_sub_world(int redBlack){
  int i,k;
  struct world * copy;
  memcpy(copy, world, sizeof(struct world) * side_size * side_size);
  struct world ** rows_copy = malloc(side_size * sizeof(struct world *));
  for (i=0; i < side_size; i++) 
    rows_copy[i] = &copy[i*side_size];

  for(i=0; i<side_size; i++) {
    for(k=(i+redBlack)%2; k<side_size; k+=2){
      if(rows_copy[i][k].type == 'e' || rows_copy[i][k].type == 't' || rows_copy[i][k].type == 'i') continue;
      else if(rows_copy[i][k].type == 's' || rows_copy[i][k].type == '$') process_squirrel(i, k, rows_copy);
      else process_wolf(i, k, rows_copy);
    }
  }
  memcpy(world, copy, sizeof(struct world) * side_size * side_size);
}
/* print_rows() : function that's print the actual state of the rows*/
void print_world(){
  int i, j, k;
  
  for(i=0;i<side_size;i++){
    printf("----");
  }
  printf("\n");
  for(i=0;i<side_size;i++){
    for(j=0;j<side_size;j++){
      printf("| %c ", rows[i][j].type); 
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
  int n_generations,i;
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
  // Open the input file and initialize the rows.
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

  for (i=0; i<n_generations; i++) {
    process_sub_world(RED);
    process_sub_world(BLACK);
  }
  // prints the rows
  print_world();
  
  return 0;
}
