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
    //master
    if (rank == 0)
    {
        int curInt;
        int numberCount = 0;
        FILE *file = fopen("input.txt", "r");
        if (file == NULL){
            printf("No File Exist named: \"input.txt\"\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        while (!feof(file))
        {
            numberCount++;
            fscanf(file, "%d\n", &curInt);
        }
        fclose(file);

        int divider = numberCount / numprocs;
        int excess = numberCount % numprocs;

        file = fopen("input.txt", "r");

        //Master Calculations
        size_of_data = divider;
        if (rank < excess)
        {
            size_of_data++;
        }
        for (i = 0; i < size_of_data; i++)
        {
            fscanf (file, "%d\n", &curInt);
            if (localMin > curInt)
            {
                localMin = curInt;
            }
        }
        //Send data to other processors
        for (i = 1; i < numprocs; i++)
        {
            size_of_data = divider;
            if (i < excess)
            {
                size_of_data++;
            }
            //Send size of data
            MPI_Send((void *) &size_of_data, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            num_arr = (int*)calloc(size_of_data, sizeof(int));
            for (j = 0; j < size_of_data; j++)
            {
                fscanf (file, "%d\n", &curInt);
                num_arr[j] = curInt;
            }
            //Send data
            MPI_Send((void *) &num_arr[0], size_of_data, MPI_INT, i, 1, MPI_COMM_WORLD);
            free(num_arr);
            num_arr = NULL;
        }
        fclose(file);

        int recv_local_min;
        //Receive local mins from other processors and calculate total min
        for (i = 1; i < numprocs; i++)
        {
            MPI_Recv((void *) &recv_local_min, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (localMin > recv_local_min)
            {
                localMin = recv_local_min;
            }
        }
        printf("Min: %d\n", localMin);
    }
    //Other processors receive data calculates local mins and sends local mins to master
    else
    {
        MPI_Recv((void *) &size_of_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        num_arr = (int*)calloc(size_of_data, sizeof(int));
        MPI_Recv((void *) &num_arr[0], size_of_data, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < size_of_data; i++)
        {
            if (localMin > num_arr[i])
            {
                localMin = num_arr[i];
            }
        }
        free(num_arr);
        num_arr = NULL;

        MPI_Send((void *) &localMin, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}