#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rowCount;
    const double dotProductMult [] = {0.21, 0.71, 0.07};
    int pixel [3];
    FILE *file = fopen("image.txt", "r");
    fscanf(file, "%d\n", &rowCount);
    double *dotProducts = (double *)calloc(rowCount * rowCount, sizeof(double)); 
    int i = 0;
    while (!feof(file))
    {
        fscanf(file, "%d,%d,%d\n", &pixel[0], &pixel[1], &pixel[2]);

        dotProducts[i] = pixel[0] * dotProductMult[0] + pixel[1] * dotProductMult[1] + pixel[2] * dotProductMult[2];
        i++;
    }

    fclose(file);

    file = fopen("grayscale-serial-output.txt", "w");
    fprintf(file, "%d\n", rowCount);
    for (i = 0; i < rowCount * rowCount; i++)
    {
        fprintf(file, "%f\n", dotProducts[i]);
    }
    free(dotProducts);
    fclose(file);
    return 0;
}