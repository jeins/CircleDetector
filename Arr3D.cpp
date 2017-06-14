#include <stdio.h>
#include <stdlib.h>  
#include <time.h> 
#include <mpi.h>


void generate3D(int n, int x, int y, int*** &result)
{
    result = new int**[n];
    for(int i=0; i<n; i++){
        result[i] = new int*[x];

        for(int j=0; j<x; j++){
            result[i][j] = new int[y];

            for(int k=0; k<y; k++){
                result[i][j][k] = rand();
            }
        }
    }
}

void print3DArray(int n, int x, int y, int*** &arr3D)
{
    for(int i=0; i<n; i++){
        for(int j=0; j<x; j++){
            for(int k=0; k<y; k++){
                printf("n: %d | x: %d | y: %d ==> %d \n", i, j, k, arr3D[i][j][k]);
            }
        }
    }
}

int free3D(int ***arr, int l,int m) {
    for(int i=0;i<l;i++)
    {
        for(int j=0;j<m;j++)
        {
                free(arr[i][j]);
        }
        free(arr[i]);
    }
    free(arr);
}

int ***alloc3D(int n, int x, int y)
{
    int ***arr = (int ***)malloc(n * sizeof(int));
    for (int i=0; i<n; i++){
         arr[i] = (int **)malloc(x * sizeof(int));
         for(int j=0; j<x; j++){
             arr[i][j] = (int *)malloc(y * sizeof(int));
         }
    }
}

int main()
{
    MPI_Init(NULL, NULL);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int a = 3;
    int b = 3;
    int c = 3;

    //custom data type
    int sizes[3]    = {a*size, b*size, c*size};        
    int subsizes[3] = {a, b, c};   
    int starts[3]   = {0,0,0};                       
    MPI_Datatype type, subarrtype;
    MPI_Type_create_subarray(3, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &type);
    MPI_Type_create_resized(type, 0, a*sizeof(int), &subarrtype);
    MPI_Type_commit(&subarrtype);

    int *tmp = (int*)malloc(sizeof(int));
    int *n = NULL;

    if(rank == 0){
        int t[] = {1};
        n = t;
    }

    MPI_Scatter(n, 1, MPI_INT, tmp, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int ***arr3D;
    generate3D(a, b, c, arr3D);
    print3DArray(a, b, c, arr3D);

    int ***glbArr=NULL;
    if(rank == 0){
        glbArr = &(arr3D[0][0][0]);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gatherv(&(arr3D[0][0][0]), a*b*c,  MPI_INT,
                 glbArr, a*size, subarrtype,
                 0, MPI_COMM_WORLD);


    MPI_Type_free(&subarrtype);

    if(rank == 0){
        printf("hasil : %d\n", glbArr[0][0][0]);
    }
    free3D(glbArr, b*size,c*size);
    free3D(arr3D, b, c);
    free(tmp);
    free(n);

    MPI_Finalize();
}