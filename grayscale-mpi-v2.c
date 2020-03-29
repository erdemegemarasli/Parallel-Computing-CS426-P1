#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    //mpi initilazations
    int rank, numprocs, i, j, k;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size_of_data;
    double **dotProducts;
    const double dotProductMult [] = {0.21, 0.71, 0.07};

    int gridRowSize;

    int rowCount;
    int sqrtNumprocs = sqrt(numprocs);
    

    
    //Master
    if (rank == 0)
    {
        int dataCursor = 0;
        
        FILE *file = fopen("image.txt", "r");
        if (file == NULL){
            printf("No File Exist named: \"image.txt\"\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        if (numprocs / sqrtNumprocs != sqrtNumprocs || numprocs % sqrtNumprocs != 0)
        {
            printf("Number of processors used is not perfect square\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fscanf(file, "%d\n", &rowCount);
        if (rowCount % sqrtNumprocs != 0){
            printf("Matrix dimensions are not divisible by the square root of the number of processors.\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        if (numprocs > rowCount){
            printf("Number of processors are more than number of row \n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        gridRowSize = rowCount / sqrtNumprocs;
        int **inputPixels = (int **)calloc(rowCount, sizeof(int*));
        
        dotProducts = (double **)calloc(rowCount, sizeof(double*));

        for (i = 0; i < rowCount; i++)
        {
            inputPixels[i] = (int *)calloc(rowCount * 3, sizeof(int));
            dotProducts[i] = (double *)calloc(rowCount, sizeof(double*));
        }
        
        
        for (i = 0; i < rowCount; i++)
        {
            for (j = 0; j < rowCount * 3; j += 3)
            {
                fscanf(file, "%d,%d,%d\n", &inputPixels[i][j], &inputPixels[i][j + 1], &inputPixels[i][j + 2]);
            }
        }
        fclose(file);
        
        // Master calculations
        for (i = 0; i < gridRowSize; i++)
        {
            for (j = 0; j < gridRowSize * 3; j += 3)
            {
                dotProducts[i][j / 3] = inputPixels[i][j] * dotProductMult[0] + inputPixels[i][j + 1] * dotProductMult[1] + inputPixels[i][j + 2] * dotProductMult[2];
            }
        }

        int *sendPixels = (int *)calloc(gridRowSize * gridRowSize * 3, sizeof(int));

        int send_size = gridRowSize * gridRowSize * 3;

        // Send data to processors
        for (i = 1; i < numprocs; i++)
        {
            int col = (i % sqrtNumprocs) * gridRowSize * 3;
            int row = (i / sqrtNumprocs) * gridRowSize;
            //printf("row: %d\ncol: %d\n", row, col);
            for (j = 0; j < gridRowSize; j++)
            {
                for (k = 0; k < gridRowSize * 3; k++)
                {
                    sendPixels[(j * gridRowSize * 3) + k] = inputPixels[row + j][col + k];
                }
            }
            //Send
            MPI_Send((void *) &gridRowSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send((void *) &sendPixels[0], send_size, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
        
        for (i = 0; i < rowCount; i++){
            free(inputPixels[i]);
        }
        free(inputPixels);

        free(sendPixels);

        int recv_size = gridRowSize * gridRowSize;

        double *recvDotProducts = (double *)calloc(recv_size, sizeof(double));

        for (i = 1; i < numprocs; i++){
            int col = (i % sqrtNumprocs) * gridRowSize;
            int row = (i / sqrtNumprocs) * gridRowSize;
            MPI_Recv((void *) &recvDotProducts[0], recv_size, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (j = 0; j < gridRowSize; j++)
            {
                for (k = 0; k < gridRowSize; k++)
                {
                    dotProducts[row + j][col + k] = recvDotProducts[(j * gridRowSize) + k];
                }
            }
        }

        file = fopen("grayscale-mpi-v2-output.txt", "w");
        fprintf(file, "%d\n", rowCount);
        for (i = 0; i < rowCount; i++)
        {
            for (j = 0; j < rowCount; j++)
            {
                fprintf(file, "%f\n", dotProducts[i][j]);
            }
        }
        fclose(file);
        for (i = 0; i < rowCount; i++){
            free(dotProducts[i]);
        }
        free(dotProducts);
        free(recvDotProducts);
    }
    else
    {
        MPI_Recv((void *) &gridRowSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        size_of_data = gridRowSize * gridRowSize * 3;

        int *recvPixels = (int *)calloc(size_of_data, sizeof(int));
        double *sendDotProducts = (double *)calloc(gridRowSize * gridRowSize, sizeof(double));

        MPI_Recv((void *) &recvPixels[0], size_of_data, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Make calculations
        for (i = 0; i < size_of_data; i += 3)
        {
            sendDotProducts[i / 3] = recvPixels[i] * dotProductMult[0] + recvPixels[i + 1] * dotProductMult[1] + recvPixels[i + 2] * dotProductMult[2];
        }
        int send_size = gridRowSize * gridRowSize;
        MPI_Send((void *) &sendDotProducts[0], send_size, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
        free(sendDotProducts);
        free(recvPixels);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}