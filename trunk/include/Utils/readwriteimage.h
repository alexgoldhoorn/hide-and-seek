#ifndef READWRITEIMAGE_H
#define READWRITEIMAGE_H


typedef struct _PGMData {
    int row;
    int col;
    int max_gray;
    int **matrix;
} PGMData;

//int **allocate_dynamic_matrix(int row, int col);

void deallocate_dynamic_matrix(int **matrix, int row);


//void SkipComments(FILE *fp);

/*for reading:*/
PGMData* readPGM(const char *file_name, PGMData *data);

/*and for writing*/
void writePGM(const char *filename, const PGMData *data);


#endif // READWRITEIMAGE_H
