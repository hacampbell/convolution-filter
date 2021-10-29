/******************************************************************************
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
*****************************************************************************/

#include <iostream>     // Basic IO
#include <pthread.h>    // Threads
#include <sys/types.h>  // Thread ID types  
#include <string>       // Strings
#include <math.h>       // Used for sqrt

using namespace std;

/******************************************************************************
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
 * RETURNS:         0 if the given arguments were valid, 1 with console output
 *                  otherwise.
 *****************************************************************************/ 
int ProcessArguments (int argc, char** argv, string* file, int* dpth, int* nTh) {
    // Check we've been given the correct number of arguments
    if (argc < 4) {
        cout << "[ERROR] Invalid number of arguments given." << endl;
        cout << "Usage:" << endl;
        cout << "\tconvolution [matrixFile] [filterDepth] [numThreads]" << endl;
        
        return 1;
    }

    // Check we've been given numbers for depth and numThreads
    if (atoi(argv[2]) == 0 || atoi(argv[3]) == 0) {
        cout << "[ERROR] Invalid values given for depth or numThreads" << endl;
        cout << "Usage:" << endl;
        cout << "\tconvolution [matrixFile] [filterDepth] [numThreads]" << endl;
        cout << "Where filterDepth and numThreads are ints > 0" << endl;
        
        return 1;
    }

    *file = argv[1];
    *dpth = atoi(argv[2]);
    *nTh = atoi(argv[3]);

    return 0;
}

/******************************************************************************
 * NAME:            GetMatrixDimension
 * 
 * DESCRIPTION:     Gets the dimension used in a given matrix file. Note that
 *                  this function will not work for files over 2 GB, and is
 *                  only applicable to square matrixes e.g. a 4x4 matrix
 * 
 * PARAMETERS:      char* filename - the name of the matrix file
 * 
 * RETURNS:         int - the dimension of the matrix e.g. 5 for a 5x5 matrix
 *****************************************************************************/
int GetMatrixDimension (/*char* filename[STRLEN]*/ string filenameStr) {
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


/******************************************************************************
 * NAME:            main
 * DESCRIPTION:     Entrypoint for the program.
 * PARAMETERS:      None
 * RETURNS:         0 on success, an error status otherwise.
 *****************************************************************************/ 
int main (int argc, char** argv) {
    string filename;
    int filterDepth;
    int numThreads;
    int matrixDimension;

    if (ProcessArguments(argc, argv, &filename, &filterDepth, &numThreads) != 0) {
        cout << "\nProgram exiting..." << endl;
        return 0;
    }

    cout << "file: " << filename << " depth: " << filterDepth << " threads: ";
    cout << numThreads << endl;


    matrixDimension = GetMatrixDimension(filename);

    printf("Matrix dimension for '%s' was %d\n", filename.c_str(), matrixDimension);

	return 0;
}
