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
    int scatterSize;
    int *inputPixels;
    int *recvPixels;
    double *dotProducts;
    double *sendDotProducts;
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
        int **inputPixels2d = (int **)calloc(rowCount, sizeof(int*));
        
        for (i = 0; i < rowCount; i++)
        {
            inputPixels2d[i] = (int *)calloc(rowCount * 3, sizeof(int));
        }
        inputPixels = (int *)calloc(rowCount * rowCount * 3, sizeof(int));
        dotProducts = (double *)calloc(rowCount * rowCount, sizeof(double));
        
        for (i = 0; i < rowCount; i++)
        {
            for (j = 0; j < rowCount * 3; j += 3)
            {
                fscanf(file, "%d,%d,%d\n", &inputPixels2d[i][j], &inputPixels2d[i][j + 1], &inputPixels2d[i][j + 2]);
            }
        }
        fclose(file);

        int indexPointer = 0;
        int incrementer = gridRowSize * gridRowSize * 3;
        for (i = 0; i < numprocs; i++)
        {
            int col = (i % sqrtNumprocs) * gridRowSize * 3;
            int row = (i / sqrtNumprocs) * gridRowSize;
            for (j = 0; j < gridRowSize; j++)
            {
                for (k = 0; k < gridRowSize * 3; k++)
                {
                    inputPixels[(j * gridRowSize * 3) + k + indexPointer] = inputPixels2d[row + j][col + k];
                }
            }
            indexPointer += incrementer;
        }
    }
    //Bcast gridRowSize
    MPI_Bcast((void *) &gridRowSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    rowCount = gridRowSize * sqrtNumprocs;

    scatterSize = gridRowSize * gridRowSize * 3;

    recvPixels = (int *)calloc(scatterSize, sizeof(int));
    //Scatter data
    MPI_Scatter((void *)&inputPixels[0], scatterSize, MPI_INT, recvPixels, scatterSize, MPI_INT, 0, MPI_COMM_WORLD);

    sendDotProducts = (double *)calloc(scatterSize / 3, sizeof(double));

    //Make calculations
    for (i = 0; i < scatterSize; i += 3)
    {
        sendDotProducts[i / 3] = recvPixels[i] * dotProductMult[0] + recvPixels[i + 1] * dotProductMult[1] + recvPixels[i + 2] * dotProductMult[2];
    }

    //Gather data
    MPI_Gather(&sendDotProducts[0], scatterSize / 3, MPI_DOUBLE, &dotProducts[0], scatterSize / 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    free(recvPixels);
    free(sendDotProducts);

    if (rank == 0)
    {
        double **dotProducts2d = (double **)calloc(rowCount, sizeof(double*));

        for (i = 0; i < rowCount; i++)
        {

            dotProducts2d[i] = (double *)calloc(rowCount, sizeof(double*));
        }

        int indexPointer = 0;
        int incrementer = gridRowSize * gridRowSize;
        for (i = 0; i < numprocs; i++)
        {
            int col = (i % sqrtNumprocs) * gridRowSize;
            int row = (i / sqrtNumprocs) * gridRowSize;
            //printf("row: %d\ncol: %d\n", row, col);
            for (j = 0; j < gridRowSize; j++)
            {
                for (k = 0; k < gridRowSize; k++)
                {
                    dotProducts2d[row + j][col + k] = dotProducts[(j * gridRowSize) + k + indexPointer];
                }
            }
            indexPointer += incrementer;
        }

        FILE *file = fopen("grayscale-mpi-v3-output.txt", "w");
        fprintf(file, "%d\n", rowCount);
        for (i = 0; i < rowCount; i++)
        {
            for (j = 0; j < rowCount; j++)
            {
                fprintf(file, "%f\n", dotProducts2d[i][j]);
            }
        }
        fclose(file);
        for (i = 0; i < rowCount; i++){
            free(dotProducts2d[i]);
        }
        free(dotProducts2d);
        free(dotProducts);
        free(inputPixels);


    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}