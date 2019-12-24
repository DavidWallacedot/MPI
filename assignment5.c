#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

const int MAX_STRING =100;
int main (int argc, char *argv[])
{
	
	if(argc != 3)
		{
			printf("usage: %s string int\n",argv[0]);
			return 1;
		}

	char message[MAX_STRING];
	char appending[MAX_STRING];
	int comm_sz;
	int my_rank;
	int i = atoi(argv[2]) ;
	  
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	memset(message,0,5);
	strcat(message, argv[1]);
	if(my_rank >0 && my_rank <= comm_sz-2)
	{	
		MPI_Recv(&i,1,MPI_INT,my_rank-1,my_rank-1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(message,MAX_STRING,MPI_CHAR,my_rank-1,my_rank-1+20,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		sprintf(appending, "%d", my_rank);	
		strcat(message,appending);
		i++;
		MPI_Send(message,strlen(message)+1,MPI_CHAR,my_rank+1,my_rank+20,MPI_COMM_WORLD);
		MPI_Send(&i, 1 , MPI_INT,my_rank+1,my_rank,MPI_COMM_WORLD);
	}	
	else if(my_rank ==comm_sz-1)
	{
		MPI_Recv(message,MAX_STRING,MPI_CHAR,my_rank-1,my_rank-1+20,MPI_COMM_WORLD,MPI_STATUS_IGNORE);	
		MPI_Recv(&i,1,MPI_INT,my_rank-1,my_rank-1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		sprintf(appending, "%d", my_rank);	
		strcat(message,appending);	
		i++;
		MPI_Send(message,strlen(message)+1,MPI_CHAR,0,my_rank+20,MPI_COMM_WORLD);
		MPI_Send(&i, 1 , MPI_INT,0,my_rank,MPI_COMM_WORLD);
	}
	else{
		
		MPI_Send(&i, 1 , MPI_INT,1,my_rank,MPI_COMM_WORLD);
		MPI_Send(message,strlen(message)+1,MPI_CHAR,my_rank+1,my_rank+20,MPI_COMM_WORLD);
		MPI_Recv(message,MAX_STRING,MPI_CHAR,comm_sz-1,comm_sz-1+20,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&i,1,MPI_INT,comm_sz-1,comm_sz-1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		sprintf(appending, "%d", my_rank);		
		strcat(message,appending);	
		i++;		
		printf("\n%i\n%s\n",i,message);
			
			
		}
	MPI_Finalize();
	return 0 ;



}                 
