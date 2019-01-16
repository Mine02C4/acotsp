/*
 * Filename:	acotsp.c
 * Title:		Parallel Ant Colony Optimization for the Traveling Salesman Problem (Term Project)
 * Course:		CSC337 Spring 2008 (Adhar)
 * Author:		Brett C. Buddin (brett@intraspirit.net)
 * Date:		April 29, 2008
*/

#include "acotsp.h"

int main(int argc, char *argv[])
{
	int i, j, k, max_width, max_height, min;
	double start = 0.0, finish = 0.0;
	MPI_Status status;
	MPI_Datatype MPI_CITY, MPI_BEST;
	
	// Initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// Capture the starting time
	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();
	
	if (!rank) {
		ACO_Load_cities(argv[1], &max_width, &max_height);
		printf("Cities: %d\nProcesses: %d\nAnts: %d\nAlpha: %3.2f\nBeta: %3.2f\nRho: %3.2f\nQ: %d\n\n", NUM_CITIES, procs, NUM_ANTS, ALPHA, BETA, RHO, Q);
		all_best = (ACO_Best_tour *)malloc(sizeof(ACO_Best_tour)*procs);
	}
	
	// Broadcast the cities to all processes
	MPI_Type_contiguous(2, MPI_INT, &MPI_CITY);
	MPI_Type_set_name(MPI_CITY, "MPI_CITY");
	MPI_Type_commit(&MPI_CITY);
	MPI_Bcast(city, NUM_CITIES, MPI_CITY, 0, MPI_COMM_WORLD);
	
	// Construct the city graph and setup/distribute the ants
	ACO_Link_cities();
	ACO_Reset_ants();
	
	// Build the derived data type for communication of the best tour
	ACO_Build_best(&best, &MPI_BEST);
	MPI_Type_set_name(MPI_BEST, "MPI_BEST");
	MPI_Type_commit(&MPI_BEST);
	for(i=0; i<NUM_COMMS; i++) {
		for(j=0; j<NUM_TOURS*NUM_CITIES; j++) {
			ACO_Step_ants();
			if(j % NUM_CITIES == 0 && j != 0) {
				ACO_Update_pheromone();
				ACO_Update_best();
				ACO_Reset_ants();
			}
		}
		
		// Collect best tours from all processes
		MPI_Gather(&best, 1, MPI_BEST, all_best, 1, MPI_BEST, 0, MPI_COMM_WORLD);
		if(!rank) {
			min = 0;
			for(j=1; j<procs; j++) {
				if(all_best[j].distance < all_best[min].distance) { 
					min = j;
				}
			}
			best.distance = all_best[min].distance;

			for(j=0; j<NUM_CITIES; j++) {
				best.path[j] = all_best[min].path[j];
			}
			printf("Best Distance So Far: %.15f\n", best.distance);
			fflush(stdout);
		}
		
		if(i < NUM_CITIES-1) {
			// Broadcast the overall best tour to all processes
			MPI_Bcast(&best, 1, MPI_BEST, 0, MPI_COMM_WORLD);
			for(j=0; j<NUM_CITIES; j++) {
				for(k=0; k<NUM_CITIES; k++) {
					pheromone[j][k] = 1.0/NUM_CITIES;
				}
			}
			// Highlight the overall best tour in the pheromone matrix
			for(j=0; j<NUM_CITIES; j++) {
				if(j < NUM_CITIES-1) {
					pheromone[best.path[j]][best.path[j+1]] += Q/best.distance;
					pheromone[best.path[j+1]][best.path[j]] = pheromone[best.path[j]][best.path[j+1]];
				}
			}
		}
	}
	
	// Capture the ending time
	MPI_Barrier(MPI_COMM_WORLD);
	finish = MPI_Wtime();
	if(!rank) {
		printf("Final Distance (%.15f): %.15f\n", finish-start, best.distance);
		fflush(stdout);
#ifdef PROCESSING
		ACO_Export_processing(max_width, max_height);
#endif
	}
	
	MPI_Type_free(&MPI_CITY);
	MPI_Type_free(&MPI_BEST);
	MPI_Finalize();

#ifdef OPENGL	
	if(!rank) {
		glutInit(&argc, argv); 
		glutInitDisplayMode(GLUT_RGB); 
		glutInitWindowSize(max_width, max_height); 
		glutInitWindowPosition(100, 0);		
		glutCreateWindow("Results");
		glClearColor(0.2, 0.2, 0.2, 1);
		gluOrtho2D(0, max_width, 0, max_height);
		glMatrixMode(GL_PROJECTION); 
		glLoadIdentity();
		glutDisplayFunc(ACO_Display);
		glutMainLoop();
	}
#endif
	return 0;
}

/** ACO_Build_best
	Build an MPI derived type for communications of the best tour.
	
	@param tour
	Pointer to the place we are storing our best tour (for the current process).
	
	@param mpi_type
	Pointer to the MPI derived type we are building.
	
	@return void
*/
void ACO_Build_best(ACO_Best_tour *tour, MPI_Datatype *mpi_type /*out*/)
{
	int block_lengths[2];
	MPI_Aint displacements[3];
	MPI_Datatype typelist[3];
	MPI_Aint start_address;
	MPI_Aint address;
	
	block_lengths[0] = 1;
	block_lengths[1] = NUM_CITIES;
	
	typelist[0] = MPI_DOUBLE;
	typelist[1] = MPI_INT;
	
	displacements[0] = 0;
	
	MPI_Address(&(tour->distance), &start_address);
	MPI_Address(tour->path, &address);
	displacements[1] = address - start_address;
	
	MPI_Type_struct(2, block_lengths, displacements, typelist, mpi_type);
	MPI_Type_commit(mpi_type);
}

/** ACO_Display
	Draws and displays the city coordinates and best tour loop to the screen. If OPENGL
	isn't defined during compile this function won't be available.
	
	@return void
*/
#ifdef OPENGL
void ACO_Display() 
{ 
	int i;
	
	glClear(GL_COLOR_BUFFER_BIT); 
	glPointSize(4.0);
	glLineWidth(2.0);
	
	glColor3f(0.35, 0.35, 0.35);	
	glBegin(GL_LINE_LOOP); 
	for(i=0; i<NUM_CITIES; i++) glVertex2f(city[best.path[i]].x, city[best.path[i]].y);
	glEnd();
	
	glColor3f(1, 1, 1);	
	glBegin(GL_POINTS); 
	for(i=0; i<NUM_CITIES; i++)	glVertex2f(city[best.path[i]].x, city[best.path[i]].y);
	glEnd();
	glFlush();
}
#endif

/** ACO_Step_ants
	Iterates each ant one step in their tours.
	
	@return void
*/
void ACO_Step_ants()
{
	int i;
	
	for(i=0; i<NUM_ANTS; i++) {
		if(ant[i].path_index < NUM_CITIES) {
			ant[i].next_city = ACO_Next_city(i);  // Pick our next city
			ant[i].tour_distance += distance[ant[i].city][ant[i].next_city];
			ant[i].path[ant[i].path_index++] = ant[i].next_city;
			ant[i].tabu[ant[i].next_city] = 1;
			ant[i].city = ant[i].next_city; // Move the ant
			
			if(ant[i].path_index == NUM_CITIES) {
				ant[i].tour_distance += distance[ant[i].path[NUM_CITIES-1]][ant[i].path[0]];
			}
		}
	}
}

/** ACO_Reset_ants
	Reset each ant's tour information and uniformly distribute each ant accross the city graph.
	
	@return void
*/
void ACO_Reset_ants()
{
	int i, j, uniform = 0;
	
	for(i=0; i<NUM_ANTS; i++) {
		if(uniform == NUM_CITIES) uniform = 0;
		ant[i].city = uniform;
		ant[i].path_index = 1;
		ant[i].tour_distance = 0.0;
		
		for(j=0; j<NUM_CITIES; j++) {
			ant[i].tabu[j] = 0;
			ant[i].path[j] = -1;
		}
		ant[i].tabu[ant[i].city] = 1;
		ant[i].path[0] = ant[i].city;
		
		uniform++;
	}
}

/** ACO_Next_city
	Chooses which city to visit next based on path pheromones and distances for a specific ant.
	
	@param ant_index [0 .. NUM_ANTS]
	Index of the ant we are deciding the next move for.
	
	@return city index if no devide by zero error.  Else -1.
*/
int ACO_Next_city(int ant_index)
{
	double denominator = 0.0, c = 0.0, r;
	int i;
	struct timeval time;
	
	gettimeofday(&time, 0);
	srandom((int)(time.tv_usec * 1000000 + time.tv_sec)+rank);
	r = (double)random()/(double)RAND_MAX;
	
	for(i=0; i<NUM_CITIES; i++) {
		if(!ant[ant_index].tabu[i]) denominator += ACO_Prob_product(ant[ant_index].city, i);
	}
	
	if(denominator != 0.0) {	
		for(i=0; i<NUM_CITIES; i++) {
			if(!ant[ant_index].tabu[i]) {
				c += ACO_Prob_product(ant[ant_index].city, i)/denominator;
				if(r <= c) break;
			}
		}

		return i;
	} else {
		return -1;
	}
}

/** ACO_Update_best
	Decides which ant has the best tour and stores that information.
	
	@return void
*/
void ACO_Update_best()
{
	int i, j;
	
	for(i=0; i<NUM_ANTS; i++) {
		if(ant[i].tour_distance < best.distance || best.distance == 0.0) {
			best.distance = ant[i].tour_distance;
			for(j=0; j<NUM_CITIES; j++) best.path[j] = ant[i].path[j];
		}
	}
}

/** ACO_Update_pheromone
	Updates the pheromone trail matrix.  First, it evaporates the current trails,
	and then deposit new pheromone amounts for each ant's tour.
	
	@return void
*/
void ACO_Update_pheromone()
{
	int i, j, from, to;
	
	// Evaporate pheromone
	for(i=0; i<NUM_CITIES; i++) {
		for(j=0; j<NUM_CITIES; j++) {
			if(i != j) {
				pheromone[i][j] *= 1.0-RHO;
				if(pheromone[i][j] < 0.0) pheromone[i][j] = 1.0/NUM_CITIES;
			}
		}
	}
	
	// Deposit pheromone
	for(i=0; i<NUM_ANTS; i++) {
		for(j=0; j<NUM_CITIES; j++) {
			from = ant[i].path[j];
			
			if(j < NUM_CITIES-1) to = ant[i].path[j+1];
			else to = ant[i].path[0];
			
			pheromone[from][to] += Q/ant[i].tour_distance;
			pheromone[to][from] = pheromone[from][to];			
		}
	}
}

/** ACO_Load_cities
	Opens a file that contains city coordinates and load that information into 
	the city matrix to be used by the program.
	
	@param filename
	Character array for the filename of the city file.
	
	@param max_width
	Maximum width of the points in the file.
	
	@param max_height
	Maximum height of the points in the file.
	
	@return void
*/
void ACO_Load_cities(char *filename, int *max_width /*out*/, int *max_height /*out*/)
{
	FILE *fp;
	int i;
	
	fp = fopen(filename, "r");
	fscanf(fp, "%dx%d", max_width, max_height);
	for(i=0; i<NUM_CITIES; i++) {
		fscanf(fp, "%d,%d", &city[i].x, &city[i].y);
	}
	
	fclose(fp);
}

/** ACO_Link_cities
	Constructs the fully connected graph of cities by defining the 
	distance and pheromone matrices.  By default pheromone levels are set to
	(1.0 / the number of cities).
	
	@return void
*/
void ACO_Link_cities()
{
	int i, j;
	
	for(i=0; i<NUM_CITIES; i++) {
		for(j=0; j<NUM_CITIES; j++) {
			distance[i][j] = 0.0;
			if(i != j) {
				distance[i][j] = distance[j][i] = ACO_Distance(city[i].x, city[i].y, city[j].x, city[j].y);
			}
			pheromone[i][j] = 1.0/NUM_CITIES;
		}
	}
}

/** ACO_Prob_product
	Calculates the pheromone/distance product for use in the ACO probability function.
	
	@param from [0 .. NUM_CITIES]
	Index of a starting city
	
	@param to [0 .. NUM_CITIES]
	Index of a ending city
	
	@return double value for product
*/
double ACO_Prob_product(int from, int to)
{
	return pow(pheromone[from][to], ALPHA) * pow((1.0/distance[from][to]), BETA);
}

/** ACO_Distance
	Calculates the distance between two points on the cartesian plane.
	
	@param x1
	X component for the first point
	
	@param y1
	Y component for the first point
	
	@param x2
	X component for the second point
	
	@param y2
	Y component for the second point
	
	@return double value for distance
*/
double ACO_Distance(int x1, int y1, int x2, int y2)
{
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

/** ACO_Export_processing
	Prints out a script to be pasted into Processing (http://processing.org/) for visualization.
	This can be used if OPENGL and GLUT aren't available.
	
	@param max_width
	Width to use for the size of window.
	
	@param max_height
	Height to use for the size of window.
	
	@return void
*/
void ACO_Export_processing(int max_width, int max_height)
{
	int i, j;
	printf("\nsize(%d, %d);\nbackground(20);\nstroke(60);\nfill(255);\n", max_width, max_height);
	for(i=0; i<NUM_CITIES; i++) {
		if(i<NUM_CITIES-1) printf("line(%d, %d, %d, %d);\n", city[best.path[i]].x, city[best.path[i]].y, city[best.path[i+1]].x, city[best.path[i+1]].y);
		else printf("line(%d, %d, %d, %d);\n", city[best.path[i]].x, city[best.path[i]].y, city[best.path[0]].x, city[best.path[0]].y);
	}
	for(i=0; i<NUM_CITIES; i++) {
		printf("ellipse(%d, %d, 4, 4);\n", city[i].x, city[i].y);
	}
	fflush(stdout);
}
