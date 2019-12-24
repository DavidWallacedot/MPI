#include<stdio.h>
#include <mpi.h>
int main(int argc, char ** argv)
{
//int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,MPI_Comm comm)
// gather sends to rank 0 as well
int isend, irecv[3];
int rank, size; 
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);
isend = rank+1;
MPI_Gather(&isend, 1, MPI_INT,&irecv,1,MPI_INT,0,MPI_COMM_WORLD);
if(rank ==0 )
{
	printf("%d %d %d\n",irecv[0],irecv[1],irecv[2]);
}

MPI_Finalize();
return 0 ; 
}

