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
    int size_of_data;
    double *dotProducts;
    const double dotProductMult [] = {0.21, 0.71, 0.07};

    int rowCount;   

    
    //Master
    if (rank == 0)
    {
        int dataCursor = 0;
        
        FILE *file = fopen("image.txt", "r");
        if (file == NULL){
            printf("No File Exist named: \"image.txt\"\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fscanf(file, "%d\n", &rowCount);
        int *inputPixels = (int *)calloc(rowCount * rowCount * 3, sizeof(int)); 
        
        
        i = 0;
        while (!feof(file))
        {
            fscanf(file, "%d,%d,%d\n", &inputPixels[i], &inputPixels[i + 1], &inputPixels[i + 2]);
            i += 3;
        }

        fclose(file);

        int divider = rowCount / numprocs;
        int excess = rowCount % numprocs;

        int numOfResponsibleRows = divider;

        

        if (excess > rank)
        {
            numOfResponsibleRows++;
        }
        
        size_of_data = numOfResponsibleRows * rowCount * 3;

        //Master calculation
        dotProducts = (double *)calloc(rowCount * rowCount, sizeof(double));
        for (i = 0; i < size_of_data; i += 3)
        {
            dotProducts[i / 3] = inputPixels[i] * dotProductMult[0] + inputPixels[i + 1] * dotProductMult[1] + inputPixels[i + 2] * dotProductMult[2];
        }

        dataCursor += size_of_data;
        
        for (i = 1; i < numprocs; i++)
        {
            int tempNumOfResponsibleRows = divider;

            if (excess > i)
            {
                tempNumOfResponsibleRows++;
            }

            int temp_size_of_data = tempNumOfResponsibleRows * rowCount * 3;
            
            MPI_Send((void *) &temp_size_of_data, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            MPI_Send((void *) &inputPixels[dataCursor], temp_size_of_data, MPI_INT, i, 1, MPI_COMM_WORLD);


            dataCursor += temp_size_of_data;
        }

        free(inputPixels);

        dataCursor = numOfResponsibleRows * rowCount;

        for (i = 1; i < numprocs; i++)
        {
            int tempNumOfResponsibleRows = divider;

            if (excess > i)
            {
                tempNumOfResponsibleRows++;
            }

            int temp_size_of_data = tempNumOfResponsibleRows * rowCount;
        
            MPI_Recv((void *) &dotProducts[dataCursor], temp_size_of_data, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            dataCursor += temp_size_of_data;
        }

        file = fopen("grayscale-mpi-v1-output.txt", "w");
        fprintf(file, "%d\n", rowCount);
        for (i = 0; i < rowCount * rowCount; i++)
        {
            fprintf(file, "%f\n", dotProducts[i]);
        }
        fclose(file);
        free(dotProducts);
        

        
    }
    //Other processors receive and process
    else 
    {
        MPI_Recv((void *) &size_of_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int *pixels = (int *)calloc(size_of_data, sizeof(int));
        int size_of_data_divided = size_of_data / 3;
        
        dotProducts = (double *)calloc(size_of_data_divided, sizeof(double));
        MPI_Recv((void *) &pixels[0], size_of_data, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //Make calculations
        for (i = 0; i < size_of_data; i += 3)
        {
            dotProducts[i / 3] = pixels[i] * dotProductMult[0] + pixels[i + 1] * dotProductMult[1] + pixels[i + 2] * dotProductMult[2];
        }

        int chunk_num = size_of_data_divided / 100;

        int last_chunk = size_of_data_divided % 100;

        

        MPI_Send((void *) &dotProducts[0], size_of_data_divided, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
        
        free(pixels);
        free(dotProducts);
    }
    MPI_Barrier(MPI_COMM_WORLD);  
    MPI_Finalize();
    return 0;
}