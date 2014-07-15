//FSFILE * SDpointer;
//*********************************************
//Function declarations.
//*********************************************
char MDD_task(short int mode);
FSFILE * MDD_Open(char * filename);
char MDD_Close(FSFILE * fileptr);
char MDD_write_power(FSFILE * fileptr);
char MDD_write_title(FSFILE * fileptr);
//char MDD_write(char source[], FSFILE * fileptr);
char MDD_write(char source[], char length, FSFILE * fileptr);
//



