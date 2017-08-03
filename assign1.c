#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h> 
#include <unistd.h>
#include <fcntl.h>

//byte sort

unsigned long byte_sort (unsigned long arg)
{
  unsigned long argbyte[8];
  int i;
  //printf("argument passed = %llx\n", arg);

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long)); i++)
  argbyte[i] = (arg>>i*8) & 0xff;

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long))-1; i++)
  {
     unsigned long min = argbyte[i];
     int index = i, j;

     for (j=i+1; j<(sizeof(argbyte)/sizeof(unsigned long)); j++)
     {
        if (argbyte[j]<min)
        {
           min = argbyte[j];
           index = j;
        }
     }
     unsigned long temp = argbyte[i];
     argbyte[i] = min;
     argbyte[index] = temp;
  }

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long)); i++)
  {
    if(i>0) 
    argbyte[i] = (argbyte[i]<<(i*8))+argbyte[i-1];
   
    //if(i==7) 
    //printf("Output of sorted bytes= %llx\n",argbyte[i]);
  }

  return argbyte[7];

}
/*
void main ()
{
  byte_sort (0x0403deadbeef0201);
}
*/

//nibble sort

unsigned long nibble_sort (unsigned long arg)
{
  unsigned long argbyte[16];
  int i;
  //printf("argument passed = %llx\n", arg);

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long)); i++)
  argbyte[i] = (arg>>i*4) & 0xf;

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long))-1; i++)
  {
     unsigned long min = argbyte[i];
     int index = i, j;

     for (j=i+1; j<(sizeof(argbyte)/sizeof(unsigned long)); j++)
     {
        if (argbyte[j]<min)
        {
           min = argbyte[j];
           index = j;
        }
     }
     unsigned long temp = argbyte[i];
     argbyte[i] = min;
     argbyte[index] = temp;
  }

  for (i=0; i<(sizeof(argbyte)/sizeof(unsigned long)); i++)
  {
    if(i>0)
    argbyte[i] = (argbyte[i]<<(i*4))+argbyte[i-1];

    //if(i==15)
    //printf("Output of sorted bytes = %llx\n",argbyte[i]);
  }

  return argbyte[15];
}
/*
void main ()
{
nibble_sort (0x0403deadbeef0201);
}
*/

//name_list

struct elt 
{
  char val;
  struct elt *link;
};

struct elt *name_list (void)
{
   char name[] = "SHIVA VULPALA";
   struct elt *first = (struct elt *)malloc(sizeof(struct elt));

   if(first == NULL)
   {
     //printf("Malloc failed so exit\n");
     return NULL;
   }

   struct elt *current = first;

   if(sizeof(name)-1 > 0)
   {
      first->val = name[0];
      first->link = NULL;
   }
   else
   {
      //printf("No input string is given\n");
      return NULL;
   }

   current = first;
   unsigned int i = 0;

   for(i = 1; i < sizeof(name)-1; i++)
   {
      struct elt *temp = (struct elt*)malloc(sizeof(struct elt));

      if(temp == NULL)
      {
         current = first;
         //printf("Malloc failed\n");

         while(first != NULL)
         {
            current = current->link;
            free(first);
            first = current;
         }

         return NULL;
      }

      temp->val = name[i];
      temp->link = NULL;
      current->link = temp;
      current = current->link;
    }

    //printf("\nMy name is ");

    while(first != NULL)
    {
        //printf("%c",first->val);
        first = first->link;
    }

    //printf("\n");
    return first;
}
/*
void main()
{
 name_list();
}
*/

//convert

enum format_t 
{
  OCT = 66, BIN, HEX
};

void convert (enum format_t mode, unsigned long value)
{ 
	char result[64] = "0000000000000000000000000000000000000000000000000000000000000000";
	char result_str[] = "The hexadecimal/octal/binary value result";
	int count = 0,ind_start = 0,ind_end = 0;
  
	if(mode == 66)
	{
  		int index_val = 21;
    		static char octal_values[] = "01234567";    
    		while (value > 0 && count < 22) 
    		{
      			result[index_val--] = octal_values[(value & 7)];
      			value >>= 3;
      			count++;
    		}
    		ind_start = 0;
    		ind_end = 22;
  	}
  	else if(mode == 67)
  	{
    		int index_val = 63;
    		static char binary_values[] = "01";
    		while (value > 0 && count <64) 
    		{
      			result[index_val--] = binary_values[(value & 1)];
      			value >>= 1;
      			count++;
    		}
   		ind_start = 0;
    		ind_end = 64;
  	}
  	else if(mode == 68)
  	{	
    		int index_val = 15;
		static char hex_values[] = "0123456789abcdef"; 
		while (value > 0 && count < 16 ) {
	                result[index_val--] = hex_values[(value & 0xF)];
                	value >>= 4;
			count++;
        	}

		ind_start = 0;
		ind_end = 16;
	}

	else{
		//printf("\nMode can only be HEX,OCT or BIN");
		return;
	}
	int j = 0;

	for(j = ind_start ; j < ind_end ; j++)
	{
		putc(result[j],stdout);
	}
	//printf("\\n");
        char newline[3] = "\\n";
        for(j=0; j<3; j++)
        putc(newline[j],stdout);
	return;

}

/*
void main()
{
  convert(HEX, 0xdeadbeef);
}
*/

//draw me
void draw_me (void)
{

char filename[] = "me.txt";
int filedesc = syscall(2,filename,O_WRONLY |  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IXUSR);

if (filedesc < 0)
{
  syscall(60,EXIT_FAILURE);
}

char art[] = "                                                   _               \n                                                 @  @             \n                                                @    @            \n                                               @      @      \n                                              @        @         \n                                             @   0__0   @         \n                                            @            @         \n                                           @              @         \n                                           @              @         \n                                           @              @        \n                                           @              @         \n                                           @              @         \n                                           @              @         \n                                           @              @         \n                                           @______________@         \n";


int ret = syscall(1,filedesc,art,1023);

if(ret != 1023)
{
  syscall(3,filedesc);
  syscall(87,filename);
  syscall(60,EXIT_FAILURE);
}
syscall(3,filedesc);
return;
}

/*
void main()
{
draw_me();
}
*/



