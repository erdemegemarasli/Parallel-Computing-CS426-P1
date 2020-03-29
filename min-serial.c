#include <stdio.h>
#include <limits.h>

int main(int argc, char **argv)
{
    int curInt;
    int min = INT_MAX;
    FILE *file = fopen("input.txt", "r");
    if (file == NULL){
            printf("No File Exist named: \"input.txt\"\n");
            return -1;
    }
    while (!feof(file))
    {
        fscanf(file, "%d", &curInt);
        if(curInt < min)
            min = curInt;
    }
    fclose(file);
    printf("Min: %d\n", min);
    return 0;
}