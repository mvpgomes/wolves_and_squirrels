#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define EMPTY ' '
#define WOLF 'w'
#define SQUIRREL 's'
#define TREE 't'
#define ICE 'i'
#define TREEWSQUIRREL '$'
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
struct world {
	char type;
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
		rows = (struct world **) malloc(side_size * sizeof(struct world *));
		for(i = 0; i < side_size; i++) {
			rows[i] = &world[i * side_size];
		}
	}

	for(i = 0; i < side_size; i++) {
		for(j = 0; j < side_size; j++) {
			rows[i][j].type = EMPTY;
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
			rows[row][column].type = type;
			break;
		case TREE:
			rows[row][column].type = type;
			break;
		case SQUIRREL:
			rows[row][column].type = type;
			rows[row][column].breeding_period = squirrel_breeding_period;
			break;
		case WOLF:
			rows[row][column].type = type;
			rows[row][column].breeding_period = wolf_breeding_period;
			rows[row][column].starvation_period = wolf_starvation_period;
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
struct list_pos* compute_wolf_movement(int row, int column, struct world **rows_copy) {
	struct list_pos* list = (struct list_pos*) malloc(sizeof(struct list_pos));
	list->first = NULL;
	list->last = NULL;
	list->num_elems = 0;
	
	if(row > 0 && (rows_copy[row - 1][column].type == EMPTY || rows_copy[row - 1][column].type == SQUIRREL || rows_copy[row - 1][column].type == WOLF)) {
		add_elem(row - 1, column, list);
	}
	if(column < (side_size - 1) && (rows_copy[row][column + 1].type == EMPTY || rows_copy[row][column + 1].type == SQUIRREL || rows_copy[row][column + 1].type == WOLF)) {
		add_elem(row, column + 1, list);
	}
	if(row < (side_size - 1) && (rows_copy[row + 1][column].type == EMPTY || rows_copy[row + 1][column].type == SQUIRREL || rows_copy[row + 1][column].type == WOLF)) {
		add_elem(row + 1, column, list);
	}
	if(column > 0 && (rows_copy[row][column - 1].type == EMPTY || rows_copy[row][column - 1].type == SQUIRREL || rows_copy[row][column - 1].type == WOLF)) {
		add_elem(row, column - 1, list);
	}
	
	return list;
}

/* compute_squirrel_movement(int row, int column): */
struct list_pos* compute_squirrel_movement(int row, int column, struct world **rows_copy) {
	struct list_pos* list = (struct list_pos*) malloc(sizeof(struct list_pos));
	list->first = NULL;
	list->last = NULL;
	list->num_elems = 0;

	if(row > 0 && (rows_copy[row - 1][column].type != ICE)) {
		add_elem(row - 1, column, list);
	}
	if(column < (side_size - 1) && (rows_copy[row][column + 1].type != ICE)) {
		add_elem(row, column + 1, list);
	}
	if(row < (side_size - 1) && (rows_copy[row + 1][column].type != ICE)) {
		add_elem(row + 1, column, list);
	}
	if(column > 0 && (rows_copy[row][column - 1].type != ICE)) {
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
void process_squirrel(int row, int column, struct world **rows_copy) {
	int p, next_row, next_column;
	struct position* next_pos;
	struct world aux_cell;

	struct list_pos* list = compute_squirrel_movement(row, column, rows_copy);

	if(list->num_elems == 0) {
		return;
	} else {
		p = select_direction(row, column, list->num_elems);
	}

	next_pos = get_element(list, p);
	next_row = next_pos->row;
	next_column = next_pos->column;

	if(rows_copy[next_row][next_column].type == SQUIRREL) {
		exchange_cells(rows_copy, next_row, next_column, row, column);

		if(rows_copy[row][column].breeding_period > rows_copy[next_row][next_column].breeding_period) {
			rows_copy[next_row][next_column].breeding_period = rows_copy[row][column].breeding_period;
		}
		
		if(rows_copy[next_row][next_column].breeding_period < 1) {
			rows_copy[row][column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[row][column].type = SQUIRREL;
			rows_copy[next_row][next_column].type = SQUIRREL;
		} else {
			rows_copy[row][column].type = EMPTY;
			rows_copy[next_row][next_column].type = SQUIRREL;
		}
		
		return;
	}
	
	if(rows_copy[next_row][next_column].type == WOLF) {

		if(rows_copy[row][column].type == TREEWSQUIRREL) {
			rows_copy[row][column].type = TREE;
		} else {
			rows_copy[row][column].type = EMPTY;
		}
		
		rows_copy[next_row][next_column].starvation_period = wolf_starvation_period + 1;
		
		return;
	}

	if(rows_copy[next_row][next_column].type == TREE) {
		exchange_cells(rows_copy, next_row, next_column, row, column);
		
		if(rows_copy[next_row][next_column].breeding_period < 1) {
			rows_copy[row][column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[row][column].type = SQUIRREL;
			rows_copy[next_row][next_column].type = TREEWSQUIRREL;
		} else {
			rows_copy[row][column].type = EMPTY;
			rows_copy[next_row][next_column].type = TREEWSQUIRREL;
		}

		return;
	}
	
	if(rows_copy[row][column].type == TREEWSQUIRREL && rows_copy[next_row][next_column].type == EMPTY) {
		exchange_cells(rows_copy, next_row, next_column, row, column);

		if(rows_copy[next_row][next_column].breeding_period < 1) {
			rows_copy[row][column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
			rows_copy[row][column].type = TREEWSQUIRREL;
			rows_copy[next_row][next_column].type = SQUIRREL;
		} else {
			rows_copy[row][column].type = TREE;
			rows_copy[next_row][next_column].type = SQUIRREL;
		}

		return;
	}

	exchange_cells(rows_copy, next_row, next_column, row, column);
	
	if(rows_copy[next_row][next_column].breeding_period < 1) {
		rows_copy[row][column].breeding_period = squirrel_breeding_period + 1;
		rows_copy[next_row][next_column].breeding_period = squirrel_breeding_period + 1;
		rows_copy[row][column].type = SQUIRREL;
		rows_copy[next_row][next_column].type = SQUIRREL;
	}
}

/* process_wolf(int row, int column, struct world **rows_copy) */
void process_wolf(int row, int column, struct world **rows_copy) {
	int p, next_row, next_column;
	struct position* next_pos;
	struct world aux_cell;
		
	struct list_pos* list = compute_wolf_movement(row, column, rows_copy);

	if (list->num_elems == 0) {
		return;
	}
	
	p = select_direction(row, column, list->num_elems);
	next_pos = get_element(list, p);

	next_row = next_pos->row;
	next_column = next_pos->column;

	if(rows_copy[next_row][next_column].type == SQUIRREL) {
		exchange_cells(rows_copy, next_row, next_column, row, column);

		if(rows_copy[next_row][next_column].breeding_period < 1) {
			rows_copy[row][column].breeding_period = wolf_breeding_period + 1;
			rows_copy[next_row][next_column].breeding_period = wolf_breeding_period + 1;
			rows_copy[row][column].type = WOLF;
			rows_copy[next_row][next_column].type = WOLF;
			rows_copy[row][column].starvation_period = wolf_starvation_period + 1;
		} else {
			rows_copy[row][column].type = EMPTY;
		}

		rows_copy[next_row][next_column].starvation_period = wolf_starvation_period + 1;

		return;
	}

	if(rows_copy[next_row][next_column].type == WOLF) {
		exchange_cells(rows_copy, next_row, next_column, row, column);

		if(rows_copy[row][column].starvation_period > rows_copy[next_row][next_column].starvation_period) {
			rows_copy[next_row][next_column].starvation_period = rows_copy[row][column].starvation_period;
			rows_copy[next_row][next_column].breeding_period = rows_copy[row][column].breeding_period;
		} else if (rows_copy[row][column].starvation_period == rows_copy[next_row][next_column].starvation_period) {
			if(rows_copy[row][column].breeding_period > rows_copy[next_row][next_column].breeding_period) {
				rows_copy[next_row][next_column].breeding_period = rows_copy[row][column].breeding_period;
			}
		}
		
		if(rows_copy[next_row][next_column].breeding_period < 1) {
			rows_copy[row][column].breeding_period = wolf_breeding_period + 1;
			rows_copy[row][column].starvation_period = wolf_starvation_period + 1;
			rows_copy[next_row][next_column].breeding_period = wolf_breeding_period + 1;
			rows_copy[row][column].type = WOLF;
			rows_copy[next_row][next_column].type = WOLF;
		} else {
			rows_copy[row][column].type = EMPTY;
			rows_copy[next_row][next_column].type = WOLF;
		}
		
		return;
	}

	exchange_cells(rows_copy, next_row, next_column, row, column);

	if(rows_copy[next_row][next_column].breeding_period < 1) {
		rows_copy[row][column].breeding_period = wolf_breeding_period + 1;
		rows_copy[next_row][next_column].breeding_period = wolf_breeding_period + 1;
		rows_copy[row][column].type = WOLF;
		rows_copy[next_row][next_column].type = WOLF;
		rows_copy[row][column].starvation_period = wolf_starvation_period + 1;
	}
}

/* process_sub_world(int redBlack)*/
void process_sub_world(int redBlack) {
	int i,k;
	struct world * copy = (struct world *) malloc( sizeof(struct world) * side_size * side_size);
	memcpy(copy, world, sizeof(struct world) * side_size * side_size);
	struct world ** rows_copy = malloc(side_size * sizeof(struct world *));

	for (i = 0; i < side_size; i++) {
		rows_copy[i] = &copy[i*side_size];
	}

	for(i = 0; i < side_size; i++) {
		for(k = (i + redBlack) % 2; k < side_size; k += 2) {
			if(rows_copy[i][k].type == EMPTY || rows_copy[i][k].type == TREE || rows_copy[i][k].type == ICE) { 
				continue;
			} else if(rows_copy[i][k].type == SQUIRREL || rows_copy[i][k].type == TREEWSQUIRREL) { 
				process_squirrel(i, k, rows_copy);
			} else { 
				process_wolf(i, k, rows_copy);
			}
		}
	}

	memcpy(world, copy, sizeof(struct world) * side_size * side_size);
}

/* kill_wolves() : updates wolves starvation period */
void kill_wolves() {
	int i, k;

	for(i = 0; i < side_size; i++) {
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

	for(i = 0; i < side_size; i++) {
		for(k = 0; k < side_size; k++) {
			if(rows[i][k].type == SQUIRREL || rows[i][k].type == TREEWSQUIRREL || rows[i][k].type == WOLF)
				rows[i][k].breeding_period--;
		}
	}
}

/* print_world_pos() : function that prints a list in the form (row,column,type) with the objects on the world map */
void print_world_pos()
{
	int i, k;

	for(i = 0; i < side_size; i++)
		{
			for(k = 0; k < side_size; k++)
	{
		if(rows[i][k].type != EMPTY)
			printf("%d %d %c\n", i, k, rows[i][k].type);
	}
		}
}

/* main : function responsible for the main interaction of the program. */
int main(int argc, char *argv[]) {
	char *file_name;
	int n_generations,i;
	FILE *file;

	if(argc == 6) {
		file_name = argv[1];
		wolf_breeding_period = atoi(argv[2]);
		squirrel_breeding_period = atoi(argv[3]);
		wolf_starvation_period = atoi(argv[4]);
		n_generations = atoi(argv[5]);
	} else { 
		printf("Error! : Incorrect number of arguments.\n");
		exit(-1);
	}

	file = fopen(file_name,"r");
	
	if(file == NULL) {
		printf("Error! : Could not open the file.");
		exit(-1);
	} else {
		initialize_world(file);
		populate_world(file);
		fclose(file);
	}

	for (i = 0; i < n_generations; i++) {
		process_sub_world(RED);

		process_sub_world(BLACK);

		kill_wolves();
		update_breeding_period();

	}

	print_world_pos();
	return 0;
}
