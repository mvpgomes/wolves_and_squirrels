#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define EMPTY ' '
#define RED 0
#define BLACK 1

/* Global variables */
int wolf_breeding_period;
int squirrel_breeding_period;
int wolf_starvation_period;
int side_size;

/* Structure that represents a position in the grid. */
struct position {
  int row;
  int column;
  struct position* next;
};
/* Structure that represents the list of positions to where the animal can move */
struct list_pos {
  struct position* first;
  struct position* last;
  int num_elems;
};

/* Array of structures that represents a 2-D grid. */
struct world{
  char type;
  int breeding_period;
  int starvation_period;
} *world, **rows; 

/* lists functions */
/* new_list(int row, int column) : creates a new list with the first element */
void new_list(int row, int column, struct list_pos* list)
{
  struct position* pos = (struct position*)malloc(sizeof(struct position));

  pos->row = row;
  pos->column = column;
  pos->next = NULL;

  list->first = pos;
  list->last = pos;
  list->num_elems = 1;

}
/* add_elem(int row, int column, struct list_pos* list) : adds a new element of the position type to the list */
void add_elem(int row, int column, struct list_pos* list)
{
  if(list->num_elems == 0)
    return new_list(row, column, list);

  struct position* last;
  struct position* pos = (struct position*)malloc(sizeof(struct position));

  pos->row = row;
  pos->column = column;
  pos->next = NULL;

  last = list->last;
  last->next = pos;
  list->last = pos;

}
/* get_element(struct list_pos* list, int n) : returns the nth element in the list */
struct position* get_element(struct list_pos* list, int n)
{
  int i;
  struct position* pos = list->first;

  for(i = 0; i < n; i++)
      pos = pos->next;

  return pos;
}

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
  
  if(row > 0 && (rows[row-1][column].type == EMPTY  || rows[row-1][column].type == 's')){
    array[i].row = row-1;
    array[i].column = column;
    i++;
  }
  if(column < side_size && (rows[row][column+1].type == EMPTY || rows[row][column+1].type == 's')){
    array[i].row = row;
    array[i].column = column+1;
    i++;
  }
  if(row < side_size && (rows[row+1][column].type == EMPTY || rows[row+1][column].type == 's')){
    array[i].row = row+1;
    array[i].column = column;
    i++;
  }
  if(column > 0 && (rows[row][column-1].type == EMPTY || rows[row][column-1].type == 's')){
    array[i].row = row;
    array[i].column = column-1;
  }
  return array;
}


/* compute_squirrel_moviment(int row, int column): function responsible to find the possible moviments for the squirrel. */
/*
struct position* compute_squirrel_movement(int row, int column){
  int i= 0;
  struct position* array = (struct position *)malloc(sizeof(struct position) * 4);
  
  if(row > 0 && (rows[row-1][column].type == 'e' || rows[row-1][column].type == 't')){
    array[i].row = row-1;
    array[i].column = column;
    array[i].p = i;
    i++;
  }
  if(column < (side_size - 1) && (rows[row][column+1].type == 'e' || rows[row][column+1].type == 't')){
    array[i].row = row;
    array[i].column = column+1;
    array[i].p = i;
    i++;
  }
  if(row < (side_size - 1) && (rows[row+1][column].type == 'e' || rows[row+1][column].type == 't')){
    array[i].row = row+1;
    array[i].column = column;
    array[i].p = i;
    i++;
  }
  if(column > 0 && (rows[row][column-1].type == 'e' || rows[row][column-1].type == 't')){
    array[i].row = row;
    array[i].column = column-1;
    array[i].p = i;
  }
  return array;
}
*/
/* alternate compute_squirrel_movement(int row, int column): */
struct list_pos* compute_squirrel_movement(int row, int column)
{
  struct list_pos* list = (struct list_pos*)malloc(sizeof(struct list_pos));
  list->first = NULL;
  list->last = NULL;
  list->num_elems = 0;

  if(row > 0 && (rows[row-1][column].type == EMPTY || rows[row-1][column].type == 't'))
    {
      add_elem(row-1, column, list);
    }
  if(column < (side_size - 1) && (rows[row][column+1].type == EMPTY || rows[row][column+1].type == 't'))
    {
      add_elem(row, column+1, list);
    }
  if(row < (side_size - 1) && (rows[row+1][column].type == EMPTY || rows[row+1][column].type == 't'))
    {
      add_elem(row+1, column, list);
    }
  if(column > 0 && (rows[row][column-1].type == EMPTY || rows[row][column-1].type == 't'))
    {
      add_elem(row, column-1, list);
    }
  
  return list;
}


void exchange_cells(struct world **copy, int new_row, int new_column, int row, int column)
{
  struct world aux_cell;
  aux_cell = copy[new_row][new_column];
  copy[new_row][new_column] = copy[row][column];
  copy[row][column] = aux_cell;
}
/* process_squirrel(int row, int column, struct world **rows) */
void process_squirrel(int row, int column, struct world **rows_copy) 
{
  int p, next_row, next_column;
  struct position* next_pos;
  struct world aux_cell;

  struct list_pos* list = compute_squirrel_movement(row, column);
  p = select_direction(row, column, list->num_elems);

  next_pos = get_element(list, p);
  next_row = next_pos->row;
  next_column = next_pos->column;

  if(rows_copy[next_row][next_column].type == 't')
    {
      exchange_cells(rows_copy, next_row, next_column, row, column);
      rows_copy[row][column].type = EMPTY;
      rows_copy[next_row][next_column].type = '$';
      return;
    }
  if(rows_copy[row][column].type == '$' && rows_copy[next_row][next_column].type == EMPTY)
    {
      exchange_cells(rows_copy, next_row, next_column, row, column);
      rows_copy[row][column].type = 't';
      rows_copy[next_row][next_column].type = 's';
      return;
    }

  exchange_cells(rows_copy, next_row, next_column, row, column);
}


/* process_wolf(int row, int column, struct world **rows) */
void process_wolf(int row, int column, struct world **rows) {
}
/* process_sub_world(int redBlack)*/
void process_sub_world(int redBlack){
  int i,k;
  struct world * copy = (struct world *)malloc( sizeof(struct world) * side_size * side_size);
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

  print_world();
  for (i=0; i<n_generations; i++) {
    process_sub_world(RED);
    process_sub_world(BLACK);
    printf("Iteration %d\n", i);
    print_world();
  }
  // prints the rows
  // print_world();
  
  return 0;
}
