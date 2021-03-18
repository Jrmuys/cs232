/*
 * Author: Joel Muyskens
 * Date 02/20/2021
 * Class: CS232
 * Written for: Homework02
  
 * Sources used:
 * https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
 * https://www.programiz.com/c-programming/c-file-input-output
 * https://www.tutorialspoint.com/c_standard_library/c_function_fgetc.htm
 * https://www.tutorialspoint.com/c_standard_library/c_function_perror.htm
 * https://www.geeksforgeeks.org/access-command-in-linux-with-examples/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char *argv[] )  {

   if( argc == 3 ) {
      printf("The first argument supplied is %s\n", argv[1]);
      printf("The second argument supplied is %s\n", argv[2]);

      //Open the file from the first command line argument
      FILE *srcPtr;
      FILE *destPtr;
      int copyChar;

      // Open the source file
      srcPtr = fopen(argv[1], "r");
      //If the file doesn't exist
      if (srcPtr == NULL) {
          perror("Error, file not found!");
          return(-1);
      };

      printf("Can access? %d\n", access(argv[2], F_OK));

      // Test if the destination file already exists
     
      if (access(argv[2], F_OK) ==0) {
          perror("Error, destination file already exists!");
          return(-1);
      };

      // Create/open destination file
      destPtr = fopen(argv[2],"w");

      if (destPtr == NULL) {
          perror("Could not create file");
          return(-1);
      }

      while (1) {
          copyChar = fgetc(srcPtr); // Copy character from source
          if (feof(srcPtr)) { // Break if end of file is detected
              break;
          }
          fputc(copyChar, destPtr); // Write character to dest file
      }

      fclose(destPtr);
            
   // Test for correct quantity of argument
   }
   else if( argc > 3 ) {
      printf("Too many arguments supplied.\n");
   }
   else {
      printf("Two argument expected.\n");
   }

   return 0;
}