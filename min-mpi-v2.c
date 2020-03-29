#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    //mpi initilazations
    int rank, numprocs, i, j;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int localMin = INT_MAX;
    int *num_arr;
    int size_of_data;
    //master calculates size
    if (rank == 0)
    {
        int curInt;
        size_of_data = 0;
        FILE *file = fopen("input.txt", "r");
        if (file == NULL){
            printf("No File Exist named: \"input.txt\"\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        while (!feof(file))
        {
            size_of_data++;
            fscanf(file, "%d\n", &curInt);
        }
        fclose(file);
    }

    //Broadcast size
    MPI_Bcast((void *) &size_of_data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    num_arr = (int *)calloc(size_of_data, sizeof(int));

    // master inserts data to Bcast
    if (rank == 0)
    {
        int curInt;
        FILE *file = fopen("input.txt", "r");
        for (i = 0; i < size_of_data; i++)
        {
            fscanf (file, "%d\n", &curInt);
            num_arr[i] = curInt;
        }
        fclose(file);
    }

    MPI_Bcast((void *) &num_arr[0], size_of_data, MPI_INT, 0, MPI_COMM_WORLD);

    //All processors calculates min
    for (i = 0; i < size_of_data; i++)
    {
        if (localMin > num_arr[i])
        {
            localMin = num_arr[i];
        }
    }


    free(num_arr);
    num_arr = NULL;

    printf("Process %d says min: %d\n", rank, localMin);
    MPI_Finalize();
    return 0;
}