#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include<time.h>
#include<math.h>
int rows;//variable that contains the # of rows that will partitioned to each worker thread
int cols;//variable that contains the # of cols that will partitioned to each worker thread
int ROWS,COLS;// constants to hold the array ROW and array COL size program assumes that programs are the square
int start;// variable that is the starting row that each worker thread will begin executing on
int end ;// variable that is the starting row that each worker thread will finish executing on
int size; 
int comm_sz;
int my_rank;	
MPI_Status status;
int modulo;
clock_t time_start ;
clock_t time_end ;
double time_spent;

int main (int argc, char *argv[])
{
	
srand(time(NULL));//sets seed to NULL allowing generation of random numbers	
	  

if(argc !=2 && argc !=5)
{
	printf("Errorz!");
	exit(1);
}
//program has 5 cl args
if(argc == 5)
{
	//variables to handle file I\O	
	FILE *fptr; 
	double buffer;	
	char* matrixA;
	FILE *f;
	char* matrixB;
	//init MPI
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	//read in values from files	
	matrixA = argv[2];	
	if((fptr = fopen(matrixA,"r")) ==NULL)
	{
		printf("Error!");
		exit(1);
	}

	char* writeFile  = argv[4];	
	if((f = fopen(writeFile,"w")) ==NULL)
	{
		printf("Error!");
		exit(1);
	}

	fscanf(fptr,"%d",&rows);
	fscanf(fptr,"%d",&cols);
	//dynamically allocate arrays 
	double *arrayA[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayA[i] = (double *)malloc(cols * sizeof(double));
	double *arrayB[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayB[i] = (double *)malloc(cols * sizeof(double));
	double *arrayC[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayC[i] = (double *)malloc(cols * sizeof(double));
	
	for (int i = 0 ; i < rows; i++)
	{
		for(int j = 0; j <cols; j++)
		{
			fscanf(fptr, "%lf",&buffer);
			arrayA[i][j] = buffer;	 
	
		}
				
	}
	matrixB = argv[3];	
	if((fptr = fopen(matrixB,"r")) ==NULL)
	{
		printf("Error!");
		exit(1);
	}
	fscanf(fptr,"%d",&rows);
	fscanf(fptr,"%d",&cols);
	size = rows * cols ;
	ROWS = rows;
	COLS = cols; 
	for (int i = 0 ; i < rows; i++)
	{
		for(int j =0; j < cols; j++)
		{
			fscanf(fptr, "%lf",&buffer);
			arrayB[i][j] = buffer; 
		}
		
	}
	//start timer and broadcast arrays C and B to every thread
	time_start = clock();
	for(int i=0;i<ROWS;i++)
	{
	  MPI_Bcast((double **)&(arrayB[i][0]),COLS,MPI_DOUBLE,0,MPI_COMM_WORLD);
	  MPI_Bcast((double **)&(arrayC[i][0]),COLS,MPI_DOUBLE,0,MPI_COMM_WORLD);
	}
	
	

	//thread 0 will act as root and distribute everything as well as handle clean up 
	if(my_rank ==0 )
	{
		
		rows = rows/(comm_sz-1)	;	
		start = 0;
		end = rows;
		//send start , rows of arrayA , and end to worker threads except thread comm_sz-1 b/c this thread will handle additional rows
		for(int i = 1; i < comm_sz -1; i ++,end+=rows)
		{
			MPI_Send(&start,1, MPI_INT,i,i,MPI_COMM_WORLD);
			MPI_Send(&end, 1, MPI_INT,i,i,MPI_COMM_WORLD);
			for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,i,i,MPI_COMM_WORLD);	
				start ++;			
			}
			
		}
			
		// send start, end, and rows of Array  A to thread comm_sz -1 
		MPI_Send(&start,1, MPI_INT,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);
		end =ROWS;		
		MPI_Send(&end, 1, MPI_INT,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);	
			
			}
		//recieve allocated rows of C , start, and end back from each worker thread
		for(int i = 1; i < comm_sz; i ++)
		{
			MPI_Recv(&start, 1, MPI_INT,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			MPI_Recv(&end, 1, MPI_INT,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j = start; j < end;j++)
				MPI_Recv((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				
		}
		//write contents out to file
		for(int i = 0; i < ROWS;i++){
			for(int j = 0; j < COLS; j++)
				fprintf(f,"%f	", arrayC[i][j]);
		fprintf(f,"\n");		
		}

		//cleanup
		fclose(f);
		for (int i=0; i<ROWS; i++) 
			 	{free(arrayC[i]) ;
				free(arrayA[i]) ;
				free(arrayB[i]) ;}	
		
		
	
		
	
		
		fclose(fptr);
		time_end = clock();
		time_spent = (double)(time_end - time_start) / CLOCKS_PER_SEC;
		printf("Time : %f",time_spent);
	}
	//worker thread com_sz -1 will handle additional rows
	else if (my_rank == comm_sz-1)
	{
		//recieve values from root thread
		MPI_Recv(&start, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);
				
		MPI_Recv(&end, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);		
				
		for(int j = start; j < end;j++)
			MPI_Recv((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD,MPI_STATUS_IGNORE);		
		//perform matrix multiplication
		for (int s = 0; s < COLS;s++ )
			for(int i = start ; i < end; i++){
				for(int j = 0; j < COLS; j++)
					arrayC[i][s] += arrayA[i][j] *arrayB[j][s];
		
		}
		//send back start, end , and modified rows of array C
		MPI_Send(&start,1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		MPI_Send(&end, 1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD);				
		}		
	}
	else{
		//worker threads except comm_sz -1 recieve values
		MPI_Recv(&start, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);
				
		MPI_Recv(&end, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);		
			
		for(int j = start; j < end;j++)
			MPI_Recv((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD,MPI_STATUS_IGNORE);		
		//perform matrix multiplication
		for (int s = 0; s < COLS;s++ )
			for(int i = start ; i < end; i++){
				for(int j = 0; j < COLS; j++)
					arrayC[i][s] += arrayA[i][j] *arrayB[j][s];
		
		}
		//send back values to root thread
		MPI_Send(&start,1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		MPI_Send(&end, 1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD);				
			}	
	}

}//end of arg 5
//enter here if cl arguments =2 
//structure is the same as if argc = 5   
if (argc == 2){
	rows = atoi(argv[1]);
	cols = atoi(argv[1]);
	ROWS = rows;
	COLS = cols; 
	double *arrayA[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayA[i] = (double *)malloc(cols * sizeof(double));
	double *arrayB[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayB[i] = (double *)malloc(cols * sizeof(double));
	double *arrayC[rows]; 
    	for (int i=0; i<rows; i++) 
         	arrayC[i] = (double *)malloc(cols * sizeof(double));

	for (int i = 0 ; i < rows; i++)
	{	
		for(int j = 0 ; j < cols; j++){
			arrayA[i][j] = rand() %100; 
			
		}		
	}	
	
	for (int i = 0 ; i < rows; i++)
	{	
		for(int j = 0 ; j < cols; j++){
			arrayB[i][j] = rand()%100; 
			
		}		
	}
	for (int i = 0 ; i < rows; i++)
	{	
		for(int j = 0 ; j < cols; j++){
			arrayC[i][j] = 0; 
			
		}		
	}	


	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

	time_start = clock();
	for(int i=0;i<ROWS;i++)
	{
	  MPI_Bcast((double **)&(arrayB[i][0]),COLS,MPI_DOUBLE,0,MPI_COMM_WORLD);
	  MPI_Bcast((double **)&(arrayC[i][0]),COLS,MPI_DOUBLE,0,MPI_COMM_WORLD);
	}
	
	


	if(my_rank ==0 )
	{
		
		rows = rows/(comm_sz-1)	;	
		start = 0;
		end = rows;
		
		for(int i = 1; i < comm_sz -1; i ++,end+=rows)
		{
			MPI_Send(&start,1, MPI_INT,i,i,MPI_COMM_WORLD);
			MPI_Send(&end, 1, MPI_INT,i,i,MPI_COMM_WORLD);
			for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,i,i,MPI_COMM_WORLD);	
				start ++;			
			}
			
		}
		
		MPI_Send(&start,1, MPI_INT,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);
		
		end =ROWS;		
		MPI_Send(&end, 1, MPI_INT,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,comm_sz-1,comm_sz-1,MPI_COMM_WORLD);	
			
			}

		for(int i = 1; i < comm_sz; i ++)
		{
			MPI_Recv(&start, 1, MPI_INT,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			MPI_Recv(&end, 1, MPI_INT,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j = start; j < end;j++)
				MPI_Recv((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,i,i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				
		}
		
			//uncomment out to output arrayC
		/*
		for(int i = 0; i < ROWS;i++){
			for(int j = 0; j < COLS; j++)
				printf("%f	", arrayC[i][j]);
		printf("\n");		
		}
*/
		
		
		for (int i=0; i<ROWS; i++) 
			 	{free(arrayC[i]) ;
				free(arrayA[i]) ;
				free(arrayB[i]) ;}	
		
		
	
		
		time_end = clock();
		time_spent = (double)(time_end - time_start) / CLOCKS_PER_SEC;
		printf("Time : %f",time_spent);
		
		


	}
	else if (my_rank == comm_sz-1)
	{
		
		MPI_Recv(&start, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);
				
		MPI_Recv(&end, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);		
				
		for(int j = start; j < end;j++)
			MPI_Recv((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD,MPI_STATUS_IGNORE);		
		
		for (int s = 0; s < COLS;s++ )
			for(int i = start ; i < end; i++){
				for(int j = 0; j < COLS; j++)
					arrayC[i][s] += arrayA[i][j] *arrayB[j][s];
		
		}
		MPI_Send(&start,1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		MPI_Send(&end, 1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD);				
		}		
	}
	else{
		
		MPI_Recv(&start, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);
				
		MPI_Recv(&end, 1, MPI_INT,0,my_rank, MPI_COMM_WORLD,&status);		
				
		for(int j = start; j < end;j++)
			MPI_Recv((double **)&(arrayA[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD,MPI_STATUS_IGNORE);		
		for (int s = 0; s < COLS;s++ )
			for(int i = start ; i < end; i++){
				for(int j = 0; j < COLS; j++)
					arrayC[i][s] += arrayA[i][j] *arrayB[j][s];
		
		}
		MPI_Send(&start,1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		MPI_Send(&end, 1, MPI_INT,0,my_rank,MPI_COMM_WORLD);
		for(int j = start; j < end;j++){
				MPI_Send((double **)&(arrayC[j][0]),COLS,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD);				
			}	
	}


}
MPI_Finalize();
return 0;
//end of main//
}
