// crt_realloc.c  
// This program allocates a block of memory for  
// buffer and then uses _msize to display the size of that  
// block. Next, it uses realloc to expand the amount of  
// memory used by buffer and then calls _msize again to  
// display the new amount of memory allocated to buffer.  
  //https://msdn.microsoft.com/en-us/library/xbebcx7d.aspx
#include <stdio.h>  
#include <malloc.h>  
#include <stdlib.h>  
//  
//int main( void )  
//{  
//   long *buffer, *oldbuffer;  
//   size_t size;  
//  
//   if( (buffer = (long *)malloc( 1000 * sizeof( long ) )) == NULL )  
//      exit( 1 );  
//  
//   size = _msize( buffer );  
//   printf_s( "Size of block after malloc of 1000 longs: %u\n", size );  
//  
//   // Reallocate and show new size:  
//   oldbuffer = buffer;     // save pointer in case realloc fails  
//   if( (buffer = realloc( buffer, size + (1000 * sizeof( long )) ))   
//        ==  NULL )  
//   {  
//      free( oldbuffer );  // free original block  
//      exit( 1 );  
//   }  
//   size = _msize( buffer );  
//   printf_s( "Size of block after realloc of 1000 more longs: %u\n",   
//            size );  
//  
//   free( buffer );  
//   exit( 0 );  
//}  


//http://www.cplusplus.com/reference/cstdlib/realloc/
//int main()
//{
//	int input, n;
//	int count = 0;
//	int* numbers = NULL;
//	int* more_numbers = NULL;
//
//	do {
//		printf("Enter an integer value (0 to end): ");
//		scanf("%d", &input);
//		count++;
//
//		more_numbers = (int*)realloc(numbers, count * sizeof(int));
//
//		if (more_numbers != NULL) {
//			numbers = more_numbers;
//			numbers[count - 1] = input;
//		}
//		else {
//			free(numbers);
//			puts("Error (re)allocating memory");
//			exit(1);
//		}
//	} while (input != 0);
//
//	printf("Numbers entered: ");
//	for (n = 0; n < count; n++) printf("%d ", numbers[n]);
//	free(numbers);
//
//	return 0;
//}