#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <stddef.h>

/* Constants */
#define EMPTY ' '
#define WOLF 'w'
#define SQUIRREL 's'
#define TREE 't'
#define ICE 'i'
#define TREEWSQUIRREL '$'
#define RED 0
#define BLACK 1

#define UPDSQL 3965
#define UPDWLF 9865

/* Global variables */
int wolf_breeding_period;
int squirrel_breeding_period;
int wolf_starvation_period;
int side_size;
int nprocs;
int id;
int chunk;

MPI_Datatype mpiworld;

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
struct world {
  char type;
  int row;
  int column;
  int breeding_period;
  int starvation_period;
} *world, **rows; 

/* Lists functions */

/* new_list(int row, int column) : creates a new list with the first element */
void new_list(int row, int column, struct list_pos* list) {
  struct position* pos = (struct position*)malloc(sizeof(struct position));

  pos->row = row;
  pos->column = column;
  pos->next = NULL;

  list->first = pos;
  list->last = pos;
  list->num_elems = 1;
}

/* add_elem(int row, int column, struct list_pos* list) : adds a new element of the position type to the list */
void add_elem(int row, int column, struct list_pos* list) {
  if(list->num_elems == 0) {
	return new_list(row, column, list);
  }

  struct position* last;
  struct position* pos = (struct position*) malloc(sizeof(struct position));

  pos->row = row;
  pos->column = column;
  pos->next = NULL;

  last = list->last;
  last->next = pos;
  list->last = pos;

  list->num_elems++;
}

/* get_element(struct list_pos* list, int n) : returns the nth element in the list */
struct position* get_element(struct list_pos* list, int n) {
  int i;
  struct position* pos = list->first;

  for(i = 0; i < n; i++) {

	pos = pos->next;
  }

  return pos;
}

/* initialize_rows(FILE *file) : function responsible for allocate mememory for the 2-D grid.*/
void initialize_world(FILE *file) {
  int i, j;

  if(fscanf(file, "%d", &side_size) == 1) {
	world = (struct world *) malloc(side_size * side_size * sizeof(struct world ));
  }

  for(i = 0; i < side_size; i++) {
	for(j = 0; j < side_size; j++) {
	  world[i*side_size+j].type = EMPTY;
	}
  }

}

/* populate_rows(FILE *file) : function responsible for populate the struct rows according the input file. */
void populate_world(FILE *file) {
  int i, size;
  int row, column;
  char type;

  while(fscanf(file, "%d %d %c", &row, &column, &type) == 3) {
	switch(type) {
	case ICE:
	  world[row*side_size+column].type = type;
	  break;
	case TREE:
	  world[row*side_size+column].type = type;
	  break;
	case SQUIRREL:
	  world[row*side_size+column].type = type;
	  world[row*side_size+column].breeding_period = squirrel_breeding_period;
	  break;
	case WOLF:
	  world[row*side_size+column].type = type;
	  world[row*side_size+column].breeding_period = wolf_breeding_period;
	  world[row*side_size+column].starvation_period = wolf_starvation_period;
	  break;
	}
  }

} 

/* compute_direction(int row, int column, int p): function responsible to select the direction of moviment. */
int select_direction(int row, int column, int p) {
  int c = row * side_size + column;
  int cell = c % p;
  return cell;
}

/* compute_wolf_movement(int row, int column): */
struct list_pos* compute_wolf_movement(int row, int column, struct world **rows) {
  struct list_pos* list = (struct list_pos*) malloc(sizeof(struct list_pos));
  list->first = NULL;
  list->last = NULL;
  list->num_elems = 0;
	
  if(row > 0 && (rows[row - 1][column].type == EMPTY || rows[row - 1][column].type == SQUIRREL || rows[row - 1][column].type == WOLF)) {
	add_elem(row - 1, column, list);
  }
  if(column < (side_size - 1) && (rows[row][column + 1].type == EMPTY || rows[row][column + 1].type == SQUIRREL || rows[row][column + 1].type == WOLF)) {
	add_elem(row, column + 1, list);
  }
  if(row < (side_size - 1) && (rows[row + 1][column].type == EMPTY || rows[row + 1][column].type == SQUIRREL || rows[row + 1][column].type == WOLF)) {
	add_elem(row + 1, column, list);
  }
  if(column > 0 && (rows[row][column - 1].type == EMPTY || rows[row][column - 1].type == SQUIRREL || rows[row][column - 1].type == WOLF)) {
	add_elem(row, column - 1, list);
  }
	
  return list;
}

/* compute_squirrel_movement(int row, int column): */
struct list_pos* compute_squirrel_movement(int row, int column, struct world **rows) {

  struct list_pos* list = (struct list_pos*) malloc(sizeof(struct list_pos));
  list->first = NULL;
  list->last = NULL;
  list->num_elems = 0;

  if(row > 0 && (rows[row - 1][column].type != ICE)) {
	add_elem(row - 1, column, list);
  }
  if(column < (side_size - 1) && (rows[row][column + 1].type != ICE)) {
	add_elem(row, column + 1, list);
  }
  if(row < (side_size - 1) && (rows[row + 1][column].type != ICE)) {
	add_elem(row + 1, column, list);
  }
  if(column > 0 && (rows[row][column - 1].type != ICE)) {
	add_elem(row, column - 1, list);
  }
	
  return list;
}

/* exchange_cells(struct world **copy, int new_row, int new_column, int row, int column) : switches two cells */
void exchange_cells(struct world **copy, int new_row, int new_column, int row, int column) {
  struct world aux_cell;
  aux_cell = copy[new_row][new_column];
  copy[new_row][new_column] = copy[row][column];
  copy[row][column] = aux_cell;
}

/* process_squirrel(int row, int column, struct world **rows) */
void process_squirrel(int row, int column, struct world **rows) {
  int p, next_row, next_column;
  struct position* next_pos;
  struct world aux_cell;
  MPI_Request request_pos;
  MPI_Status pos_status;

  struct list_pos* list = compute_squirrel_movement(row, column, rows);

  if(list->num_elems == 0) {
	return;
  } else {
	p = select_direction(row, column, list->num_elems);
  }

  next_pos = get_element(list, p);
  next_row = next_pos->row;
  next_column = next_pos->column;

  if( next_row == id*chunk + chunk )
	{
	  rows[row][column].row = next_row;
	  rows[row][column].column = next_column;
	  MPI_Isend(&rows[row][column], 1, mpiworld, id + 1, UPDWLF, MPI_COMM_WORLD, &request_pos);
	  MPI_Wait(&request_pos, &pos_status);
	  if( !(rows[row][column].breeding_period < 1) )
		{
		  rows[row][column].type = EMPTY;
		}
	  return;
	}
  if( next_row == id*chunk - 1)
	{
	  rows[row][column].row = next_row;
	  rows[row][column].column = next_column;
	  MPI_Isend(&rows[row][column], 1, mpiworld, id - 1, UPDWLF, MPI_COMM_WORLD, &request_pos);
	  MPI_Wait(&request_pos, &pos_status);
	  if( !(rows[row][column].breeding_period < 1) )
		{
		  rows[row][column].type = EMPTY;
		}
	  return;
	}
  
  if(rows[next_row][next_column].type == SQUIRREL) {
	exchange_cells(rows, next_row, next_column, row, column);

	if(rows[row][column].breeding_period > rows[next_row][next_column].breeding_period) {
	  rows[next_row][next_column].breeding_period = rows[row][column].breeding_period;
	}
		
	if(rows[next_row][next_column].breeding_period < 1) {
	  rows[row][column].breeding_period = squirrel_breeding_period + 1;
	  rows[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
	  rows[row][column].type = SQUIRREL;
	  rows[next_row][next_column].type = SQUIRREL;
	} else {
	  rows[row][column].type = EMPTY;
	  rows[next_row][next_column].type = SQUIRREL;
	}
		
	return;
  }
	
  if(rows[next_row][next_column].type == WOLF) {

	if(rows[row][column].type == TREEWSQUIRREL) {
	  rows[row][column].type = TREE;
	} else {
	  rows[row][column].type = EMPTY;
	}
		
	rows[next_row][next_column].starvation_period = wolf_starvation_period + 1;
		
	return;
  }

  if(rows[next_row][next_column].type == TREE) {
	exchange_cells(rows, next_row, next_column, row, column);
		
	if(rows[next_row][next_column].breeding_period < 1) {
	  rows[row][column].breeding_period = squirrel_breeding_period + 1;
	  rows[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
	  rows[row][column].type = SQUIRREL;
	  rows[next_row][next_column].type = TREEWSQUIRREL;
	} else {
	  rows[row][column].type = EMPTY;
	  rows[next_row][next_column].type = TREEWSQUIRREL;
	}

	return;
  }
	
  if(rows[row][column].type == TREEWSQUIRREL && rows[next_row][next_column].type == EMPTY) {
	exchange_cells(rows, next_row, next_column, row, column);

	if(rows[next_row][next_column].breeding_period < 1) {
	  rows[row][column].breeding_period = squirrel_breeding_period + 1;
	  rows[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
	  rows[row][column].type = TREEWSQUIRREL;
	  rows[next_row][next_column].type = SQUIRREL;
	} else {
	  rows[row][column].type = TREE;
	  rows[next_row][next_column].type = SQUIRREL;
	}

	return;
  }

  exchange_cells(rows, next_row, next_column, row, column);
	
  if(rows[next_row][next_column].breeding_period < 1) {
	rows[row][column].breeding_period = squirrel_breeding_period + 1;
	rows[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
	rows[row][column].type = SQUIRREL;
	rows[next_row][next_column].type = SQUIRREL;
  }
}

/* process_new_squirrel(struct world *recv_pos) */
void process_new_squirrel(struct world *recv_pos) {

  if(rows[recv_pos->row][recv_pos->column].type == SQUIRREL) {

	if(rows[recv_pos->row][recv_pos->column].breeding_period < recv_pos->breeding_period) {
	  rows[recv_pos->row][recv_pos->column] = *recv_pos;
	  rows[recv_pos->row][recv_pos->column].type = SQUIRREL;
	}
		
	if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	  rows[recv_pos->row][recv_pos->column].breeding_period = squirrel_breeding_period + 1;
	}
		
	return;
  }
	
  if(rows[recv_pos->row][recv_pos->column].type == WOLF) {
	
	rows[recv_pos->row][recv_pos->column].starvation_period = wolf_starvation_period + 1;
		
	return;
  }

  if(rows[recv_pos->row][recv_pos->column].type == TREE) {

	rows[recv_pos->row][recv_pos->column] = *recv_pos;
	rows[recv_pos->row][recv_pos->column].type = TREEWSQUIRREL;
		
	if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	  rows[recv_pos->row][recv_pos->column].breeding_period = squirrel_breeding_period + 1;
	}

	return;
  }
	
  if(recv_pos->type == TREEWSQUIRREL && rows[recv_pos->row][recv_pos->column].type == EMPTY) {

	rows[recv_pos->row][recv_pos->column] = *recv_pos;
	rows[recv_pos->row][recv_pos->column].type = SQUIRREL;
		
	if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	  rows[recv_pos->row][recv_pos->column].breeding_period = squirrel_breeding_period + 1;
	}

	return;
  }

  rows[recv_pos->row][recv_pos->column] = *recv_pos;
  rows[recv_pos->row][recv_pos->column].type = SQUIRREL;
  if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	rows[recv_pos->row][recv_pos->column].breeding_period = squirrel_breeding_period + 1;
  }
}
/* process_new_wolf(struct world *recv_pos) */
void process_new_wolf(struct world *recv_pos) {

  if(rows[recv_pos->row][recv_pos->column].type == SQUIRREL) {
	rows[recv_pos->row][recv_pos->column] = *recv_pos;

	if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	  rows[recv_pos->row][recv_pos->column].breeding_period = wolf_breeding_period + 1;
	}

	rows[recv_pos->row][recv_pos->column].starvation_period = wolf_starvation_period + 1;

	return;
  }

  if(rows[recv_pos->row][recv_pos->column].type == WOLF) {

	if(rows[recv_pos->row][recv_pos->column].starvation_period < recv_pos->starvation_period) {
	  rows[recv_pos->row][recv_pos->column] = *recv_pos;
	} else if (rows[recv_pos->row][recv_pos->column].starvation_period == recv_pos->starvation_period) {
	  if(rows[recv_pos->row][recv_pos->column].breeding_period < recv_pos->breeding_period) {
	rows[recv_pos->row][recv_pos->column] = *recv_pos;
	  }
	}
		
	if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	  rows[recv_pos->row][recv_pos->column].breeding_period = wolf_breeding_period + 1;
	}

	rows[recv_pos->row][recv_pos->column].starvation_period = wolf_starvation_period + 1;

	return;
  }

  rows[recv_pos->row][recv_pos->column] = *recv_pos;
  if(rows[recv_pos->row][recv_pos->column].breeding_period < 1) {
	rows[recv_pos->row][recv_pos->column].breeding_period = wolf_breeding_period + 1;
  }
}

/* process_wolf(int row, int column, struct world **rows) */
void process_wolf(int row, int column, struct world **rows) {
  int p, next_row, next_column, nextid = id;
  struct position* next_pos;
  struct world aux_cell;
  MPI_Request request_pos;
  MPI_Status pos_status;
		
  struct list_pos* list = compute_wolf_movement(row, column, rows);

  if (list->num_elems == 0) {
	return;
  }
	
  p = select_direction(row, column, list->num_elems);
  next_pos = get_element(list, p);

  next_row = next_pos->row;
  next_column = next_pos->column;

  if( next_row == id*chunk + chunk )
	{
	  rows[row][column].row = next_row;
	  rows[row][column].column = next_column;
	  MPI_Isend(&rows[row][column], 1, mpiworld, id + 1, UPDWLF, MPI_COMM_WORLD, &request_pos);
	  MPI_Wait(&request_pos, &pos_status);
	  if( !(rows[row][column].breeding_period < 1) )
		{
		  rows[row][column].type = EMPTY;
		}
	  return;
	}
  
  if(next_row == id*chunk - 1)
	{
	  rows[row][column].row = next_row;
	  rows[row][column].column = next_column;
	  MPI_Isend(&rows[row][column], 1, mpiworld, id - 1, UPDWLF, MPI_COMM_WORLD, &request_pos);
	  MPI_Wait(&request_pos, &pos_status);
	  if( !(rows[row][column].breeding_period < 1) )
		{
		  rows[row][column].type = EMPTY;
		}
	  return;
	}

  if(rows[next_row][next_column].type == SQUIRREL) {
	exchange_cells(rows, next_row, next_column, row, column);

	if(rows[next_row][next_column].breeding_period < 1) {
	  rows[row][column].breeding_period = wolf_breeding_period + 1;
	  rows[next_row][next_column].breeding_period = wolf_breeding_period + 1;
	  rows[row][column].type = WOLF;
	  rows[next_row][next_column].type = WOLF;
	  rows[row][column].starvation_period = wolf_starvation_period + 1;
	} else {
	  rows[row][column].type = EMPTY;
	}

	rows[next_row][next_column].starvation_period = wolf_starvation_period + 1;

	return;
  }

  if(rows[next_row][next_column].type == WOLF) {
	exchange_cells(rows, next_row, next_column, row, column);

	if(rows[row][column].starvation_period > rows[next_row][next_column].starvation_period) {
	  rows[next_row][next_column].starvation_period = rows[row][column].starvation_period;
	  rows[next_row][next_column].breeding_period = rows[row][column].breeding_period;
	} else if (rows[row][column].starvation_period == rows[next_row][next_column].starvation_period) {
	  if(rows[row][column].breeding_period > rows[next_row][next_column].breeding_period) {
	rows[next_row][next_column].breeding_period = rows[row][column].breeding_period;
	  }
	}
		
	if(rows[next_row][next_column].breeding_period < 1) {
	  rows[row][column].breeding_period = wolf_breeding_period + 1;
	  rows[row][column].starvation_period = wolf_starvation_period + 1;
	  rows[next_row][next_column].breeding_period = wolf_breeding_period + 1;
	  rows[row][column].type = WOLF;
	  rows[next_row][next_column].type = WOLF;
	} else {
	  rows[row][column].type = EMPTY;
	  rows[next_row][next_column].type = WOLF;
	}
		
	return;
  }

  exchange_cells(rows, next_row, next_column, row, column);
 
  if(rows[next_row][next_column].breeding_period < 1) {
	rows[row][column].breeding_period = wolf_breeding_period + 1;
	rows[next_row][next_column].breeding_period = wolf_breeding_period + 1;
	rows[row][column].type = WOLF;
	rows[next_row][next_column].type = WOLF;
	rows[row][column].starvation_period = wolf_starvation_period + 1;
  }
}

/* process_sub_world(int redBlack)*/
void process_sub_world(int redBlack) {
  int i,k, flag_sql, flag_wlf;
  MPI_Status status_sql, status_wlf;
  struct world pos_from_outside;
  
  for(i = id*chunk; i < (id*chunk + chunk) && i < side_size; i++) {
	for(k = (i + redBlack) % 2; k < side_size; k += 2) {
	  
	  //printf("O processo activo é o %d\n.", id);

	  MPI_Iprobe(MPI_ANY_SOURCE, UPDSQL, MPI_COMM_WORLD, &flag_sql, &status_sql);
	  MPI_Iprobe(MPI_ANY_SOURCE, UPDWLF, MPI_COMM_WORLD, &flag_wlf, &status_wlf);
	  
	  while(flag_sql || flag_wlf) {
	//printf("proc %d is receiving a squirrel:%d ; wolf:%d\n", id, flag_sql, flag_wlf);

	if(flag_sql) {
	  MPI_Recv(&pos_from_outside, 1, mpiworld, status_sql.MPI_SOURCE, UPDSQL, MPI_COMM_WORLD, &status_sql);
	  process_new_squirrel(&pos_from_outside);
	  flag_sql = 0;
	  //printf("proc %d received from %d a squirrel\n", id, status_sql.MPI_SOURCE);
	} else {
	  MPI_Recv(&pos_from_outside, 1, mpiworld, status_wlf.MPI_SOURCE, UPDWLF, MPI_COMM_WORLD, &status_wlf);
	  process_new_wolf(&pos_from_outside);
	  flag_wlf = 0;
	  //printf("proc %d received from %d a wolf\n", id, status_wlf.MPI_SOURCE);
	}

	MPI_Iprobe(MPI_ANY_SOURCE, UPDSQL, MPI_COMM_WORLD, &flag_sql, &status_sql);
	MPI_Iprobe(MPI_ANY_SOURCE, UPDWLF, MPI_COMM_WORLD, &flag_wlf, &status_wlf);

	  }

	  if(rows[i][k].type == EMPTY || rows[i][k].type == TREE || rows[i][k].type == ICE) { 
	continue;
	  } else if(rows[i][k].type == SQUIRREL || rows[i][k].type == TREEWSQUIRREL) { 
	process_squirrel(i, k, rows);
	  } else { 
	process_wolf(i, k, rows);
	  }
	}
  }
}

/* kill_wolves() : updates wolves starvation period */
void kill_wolves() {
  int i, k;
  for(i = id*chunk; i < (id*chunk + chunk) && i < side_size; i++) {
	for(k = 0; k < side_size ;k++) {
	  if(rows[i][k].type == WOLF && --rows[i][k].starvation_period < 0) {
	rows[i][k].type = EMPTY;
	  }
	}
  }
}

/* update_breeding_perior() : updates breeding period */
void update_breeding_period() {
  int i, k;

  for(i = id*chunk; i < (id*chunk + chunk) && i < side_size; i++) {
	for(k = 0; k < side_size; k++) {
	  if(rows[i][k].type == SQUIRREL || rows[i][k].type == TREEWSQUIRREL || rows[i][k].type == WOLF)
	rows[i][k].breeding_period--;
	}
  }
}

/* print_world_pos() : function that prints a list in the form (row,column,type) with the objects on the world map */
void print_world_pos(struct world *to_print)
{
  int i, k;

  for(i = 0; i < side_size; i++)
	{
	  for(k = 0; k < side_size; k++)
	{
	  if(to_print[i*side_size+k].type != EMPTY)
		printf("%d %d %c\n", i, k, to_print[i*side_size+k].type);
	}
	}
}

/* main : function responsible for the main interaction of the program. */
int main(int argc, char *argv[]) {
  char *file_name;
  int n_generations,i;
  FILE *file;

  int blocklens[5] = {1, 1, 1, 1, 1};
  MPI_Aint indices[5];
  MPI_Datatype old_types[5] = {MPI_CHAR, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Status status;

  MPI_Init(&argc, &argv);
	 
  MPI_Comm_rank (MPI_COMM_WORLD, &id);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);

  if(argc == 6) {
	file_name = argv[1];
	wolf_breeding_period = atoi(argv[2]);
	squirrel_breeding_period = atoi(argv[3]);
	wolf_starvation_period = atoi(argv[4]);
	n_generations = atoi(argv[5]);
  } else { 
	printf("Error! : Incorrect number of arguments.\n");
	MPI_Finalize();
	exit(-1);
  }

  if(!id) {
	file = fopen(file_name,"r");
	
	if(file == NULL) {
	  printf("Error! : Could not open the file.");
	  MPI_Finalize();
	  exit(-1);
	} else {
	  initialize_world(file);
	  populate_world(file);
	  fclose(file);
	}
  }


  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Bcast(&side_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  chunk = ceil((double)side_size / (double)nprocs);

  if(id)
	world = (struct world *) malloc(side_size * side_size * sizeof(struct world ));


  /* Define mpiworld datatype */

  /* Make relative */
  indices[0] = offsetof(struct world, type);
  indices[1] = offsetof(struct world, row);
  indices[2] = offsetof(struct world, column);
  indices[3] = offsetof(struct world, breeding_period);
  indices[4] = offsetof(struct world, starvation_period);

  MPI_Type_create_struct( 5, blocklens, indices, old_types, &mpiworld );
  MPI_Type_commit( &mpiworld );
  /* End of mpiworld datatype */

  MPI_Bcast(world, side_size*side_size, mpiworld, 0, MPI_COMM_WORLD);


  rows = (struct world **) malloc(side_size * sizeof(struct world *));
  for(i = 0; i < side_size; i++) {
	rows[i] = &world[i * side_size];
  }

  for (i = 0; i < n_generations; i++) {
	process_sub_world(RED);
	MPI_Barrier(MPI_COMM_WORLD);
	process_sub_world(BLACK);
	//printf("iteration %d on %d\n", i, id);
	MPI_Barrier(MPI_COMM_WORLD);
	kill_wolves();
	update_breeding_period();
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Send processed prositions to root

  // Number of positions to send
  int send_count;

  // Calculate where the process positions start and end
  int begin = id*chunk*side_size;
  int end = id*chunk*side_size + chunk*side_size;

  // Calculate how many positions to send (if end passes the matrix, set as the matrix size - begin)
  if (end > side_size*side_size) {
    send_count = side_size*side_size - begin;
  } else {
    send_count = chunk * side_size;
  }

  // If, for some reason, 1 process doesn't process anything and the send_count is negative, define as 0
  if (send_count < 0) {
    send_count = 0;
  }

  struct world *send_buff;
 
  if (send_count != 0) {
    send_buff = &world[id*chunk*side_size];
  } else {
    send_buff = world;
  }

  struct world *recv_buff = (struct world *) malloc (side_size * side_size * sizeof(struct world));

  // Gather all parts of the matrix on the root process
  MPI_Gather(send_buff, send_count, mpiworld, recv_buff, side_size*chunk, mpiworld, 0, MPI_COMM_WORLD);

  if(!id) {
   print_world_pos(recv_buff);
  }

  MPI_Type_free(&mpiworld);
  MPI_Finalize();
  return 0;
}
