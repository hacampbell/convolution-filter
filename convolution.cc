/***********************************************************************************
 * FILENAME:        convolution.cc
 * 
 * AUTHOR:          Henry Campbell
 * 
 * DESCRIPTION:     A simple threaded implementation of a convolution filter
 *                  for a given square matrix.
 * 
 * ARGUMENTS:       matrixFile  - the filename of the file containing a matrix
 *                  depth       - the neighbourhood depth to use for the filter
 *                  numThreads  - the number of threads to use for the program 
 * 
 * USAGE:           Compile the program using either g++ or makefile
 *                      > g++ -pthread convolution.cc -o convolution
 *                  OR
 *                      > make
 * 
 *                  You can then run the program using either of the following
 *                      > ./convolution [matrixFile] [depth] [numThreads]
 *                  OR
 *                      > make run
***********************************************************************************/

// TODO:    
//          Pad matrix with 0's so we can more easily work with it
//          Split matrix rows between given number of threads
//          Have threads calculate their the new values for their given row(s)
//          Have threads insert their new rows into a new 2D array
//          Display the new array, and we're done

#include <iostream>     // Basic IO
#include <pthread.h>    // Threads
#include <sys/types.h>  // Thread ID types  
#include <string>       // Strings
#include <math.h>       // Used for sqrt, ceil
#include "matrix.h"     // Used for matrix operations
#include <fcntl.h>      // Used for file reading
#include <unistd.h>     // Used for O_RDONLY

using namespace std;

/***********************************************************************************
 * NAME:            PrettyPrintMatrix
 * 
 * DESCRIPTION:     Pretty prints a given square matrix (represented by a 2D array)
 *                  out to the console
 * 
 * PARAMETERS:      int**   :   matrix      -   the matrix to print
 *                  int     :   matrixDim   -   the dimension of the matrix           
 *        
 * RETURNS:         Void, but exits the program if incorrect values have been
 *                  given
 **********************************************************************************/ 
void PrettyPrintMatrix (int** matrix, int matrixDim) {
    for (int i = 0; i < matrixDim; i++) {
        for (int j = 0; j < matrixDim; j++) {
            cout << matrix[i][j] << "\t";
        }
        cout << endl;
    }
}

/***********************************************************************************
 * NAME:            ProcessArguments
 * 
 * DESCRIPTION:     Used to process and check the validity of the programs
 *                  command line arguments.
 * 
 * PARAMETERS:      int     :   argc    -   number of command line arguments
 *                  char**  :   argv    -   the command line arguments
 *                  string* :   file    -   varible to store matrix filename
 *                  int*    :   dpth    -   variable to store the filter depth
 *                  int*    :   nTh     -   varible to store number of threads             
 *        
 * RETURNS:         Void, but exits the program if incorrect values have been
 *                  given
 **********************************************************************************/ 
void ProcessArguments (int argc, char** argv, string* file, int* dpth, int* nTh) {
    // Check we've been given the correct number of arguments
    if (argc < 4) {
        cout << "[ERROR] Invalid number of arguments given." << endl;
        cout << "Usage:" << endl;
        cout << "\tconvolution [matrixFile] [filterDepth] [numThreads]" << endl;
        
        exit(EXIT_FAILURE);
    }

    // Check we've been given numbers for depth and numThreads
    if (atoi(argv[2]) == 0 || atoi(argv[3]) == 0) {
        cout << "[ERROR] Invalid values given for depth or numThreads" << endl;
        cout << "Usage:" << endl;
        cout << "\tconvolution [matrixFile] [filterDepth] [numThreads]" << endl;
        cout << "Where filterDepth and numThreads are ints > 0" << endl;
        
        exit(EXIT_FAILURE);
    }

    *file = argv[1];
    *dpth = atoi(argv[2]);
    *nTh = atoi(argv[3]);
}

/***********************************************************************************
 * NAME:            GetMatrixDimension
 * 
 * DESCRIPTION:     Gets the dimension used in a given matrix file. Note that
 *                  this function will not work for files over 2 GB, and is
 *                  only applicable to square matrixes e.g. a 4x4 matrix
 * 
 * PARAMETERS:      string  :   filenameStr    -   the name of the matrix file
 * 
 * RETURNS:         int - the dimension of the matrix e.g. 5 for a 5x5 matrix
 **********************************************************************************/
int GetMatrixDimension (string filenameStr) {
    int size;
    int dimension;

    const char* filename = filenameStr.c_str();

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("[ERROR] Could not open file '%s'\n", filename);
        exit(1);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        printf("[ERROR] Could not get dimension for file '%s'\n", filename);
        exit(1);
    }

    size = ftell(file);
    if (size == -1) {
        printf("[ERROR] Could not get dimension for file '%s'\n", filename);
        exit(1);
    }

    fclose(file);

    dimension = size / 4;
    return sqrt(dimension);
}

/***********************************************************************************
 * NAME:            ReadMatrixFile
 * 
 * DESCRIPTION:     Reads in a matrix from a given file
 * 
 * PARAMETERS:      string  :   filenameStr -   the name of the matrix file
 *                  int     :   matDim      -   dimension of square matrix
 * 
 * RETURNS:         int**   : matrix2D      -   a pointer to the 2D array
 **********************************************************************************/
int** ReadMatrixFile (string filenameStr, int matDim) {
    int fd;
    int **matrix2D = 0;
    matrix2D = new int*[matDim];

    const char* filename = filenameStr.c_str();

    printf("Reading matrix from file '%s'\n", filename);

    if((fd = open(filename, O_RDONLY)) == -1){
        cout << "Failed to read file descriptor for " << filename << endl;
        exit(1); 
    }

    for (int i = 0; i < matDim; i++) {
        matrix2D[i] = new int[matDim];
        get_row(fd, matDim, i+1, matrix2D[i]);
    }

   return matrix2D;

}

/***********************************************************************************
 * NAME:            CleanupMatrix
 * DESCRIPTION:     Cleans up the memory allocated for a 2D matrix
 * PARAMETERS:      int**   :   matrix      - the matrix to cleanup
 *                  int     ;   matrixDim   - the dimension of the matrix
 * RETURNS:         void
 **********************************************************************************/ 
void CleanupMatrix (int** matrix, int matrixDim) {
    for (int i = 0; i < matrixDim; i++) {
        delete[] matrix[i];
    }

    delete[] matrix;
    matrix = 0;
}

/***********************************************************************************
 * NAME:            GetMatrixWork
 * 
 * DESCRIPTION:     Determines the start and end point a given thread should 
 *                  perform calculations on
 * 
 * PARAMETERS:      int     :   matrixDim   -   the dimension of the matrix
 *                  int     :   numT        -   the number of threads being used
 *                  int     :   tid         -   the ID for this thread
 *                  int*    :   startP      -   variable to store start point
 *                  int*    :   endP        -   variable to store end point
 * 
 * RETURNS:         
 **********************************************************************************/ 
void GetMatrixWork (int matrixDim, int numT, int tid, int* startP, int* endP) {
    int remainder = 0;
    int workload = 0;
    int start;
    int end;

    // Determine the number of rows a thread has to perform work on, and how many
    // rows are going to be left over after initial even distribution
    if (numT >= matrixDim) {
        workload = 1;
    } else {
        remainder = matrixDim % numT;
        workload = floor(matrixDim / numT);
    }

    // Determine the start and end rows that we should be working on
    start = workload * tid;
    end = workload * (tid + 1);

    // If we're the final thread handle any remainder
    if (tid == numT - 1) {
        end += remainder;
    }

    // Special case for when we're supposed to make more threads than rows in matrix
    if (tid >= matrixDim) {
        start = -1;
        end = -1;
    }

    //cout << "Thread " << tid << " start " << start << " end " << end << endl;
    *startP = start;
    *endP = end;
}

/***********************************************************************************
 * NAME:            CalculateFilter
 * 
 * DESCRIPTION:     Calculates the new convolution filter values for the matrix
 * 
 * PARAMETERS:      int**   :   matrix      -   the matrix to work on
 *                  int     :   matrixDim   -   the dimension of the matrix
 *                  int     :   numT        -   the number of threads being used
 *                  int     :   tid         -   the ID for this thread
 * 
 * RETURNS:         
 **********************************************************************************/ 
void CalculateFilter (int** matrix, int matrixDim, int numT, int tid) {
    int start;
    int end;

    // Find our start and end points
    GetMatrixWork(matrixDim, numT, tid, &start, &end);
    
    // If we have no work, break out
    if (start == -1 && end == -1)
        return;
    
    // Just print out the rows we're supposed to work on for now
    printf("Thread %d of %d\n", tid, numT);

    for (int row = start; row < end; row++) {
        for (int i = 0; i < matrixDim; i++) {
            cout << matrix[row][i] << "\t";
        }
        cout << endl;
    }

    // Next step is to break this out into being a thread entry point and to
    // have the main function create the threads and have each of them call this.

}

/***********************************************************************************
 * NAME:            main
 * DESCRIPTION:     Entrypoint for the program.
 * PARAMETERS:      None
 * RETURNS:         0 on success, an error status otherwise.
 **********************************************************************************/ 
int main (int argc, char** argv) {
    string filename;
    int filterDepth;
    int numThreads;
    int matrixDimension;
    int** matrix;

    // Check we've been given good arguments
    ProcessArguments(argc, argv, &filename, &filterDepth, &numThreads);

    cout << "\nfile: " << filename << " depth: " << filterDepth << " threads: ";
    cout << numThreads << endl;


    // Get our matrix dimensions
    matrixDimension = GetMatrixDimension(filename);
    printf("Matrix dimension for '%s' was %d\n", filename.c_str(), matrixDimension);

    // Read the matrix file itself
    matrix = ReadMatrixFile(filename, matrixDimension);

    // Distribute Matrix Work
    for (int i = 0; i < numThreads; i++) {
        CalculateFilter(matrix, matrixDimension, numThreads, i);
    }


    cout << "Whole Matrix" << endl;
    PrettyPrintMatrix(matrix, matrixDimension);

    // Clean up before we exit, no memory leaks please
    CleanupMatrix(matrix, matrixDimension);
	return 0;
}
