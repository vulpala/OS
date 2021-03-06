#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>


static uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p;

	p = buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}
char *file_array[1024];
struct monitor_vars{
	pthread_cond_t criticalSecAvail;
	pthread_mutex_t lock;
	uint32_t* checkSum;
	char **finFiles;
	int current_ind;
	int upd_ind;
	int count;
};

struct monitor_vars *mv;
char *name_f;
volatile int n_threads = 0;
volatile int n_files = 0;
void *thread_func(void *arg)
{
			  int ind = (intptr_t)arg;
			  char current_file[1024];
			  strcpy(current_file,file_array[ind]);
			  int error_check;		  
			  struct stat statbuf;
			  char currentPath[FILENAME_MAX];
			  do
			  {
			  FILE *fileptr;
  			  char file_path[FILENAME_MAX];
			  char* buffer;
			  int c;
			  strcpy(file_path,name_f);
			  strcat(file_path,"/");
			  strcat(file_path,current_file);
			  uint32_t crc = 0;
	    	 	  strcat(currentPath, "/");
		 	  strcat(currentPath,name_f);
    		 	  strcat(currentPath,"/");
    			  strcat(currentPath, current_file);
		      	  if(stat(currentPath, &statbuf) == -1){
			      printf("Error occured in stat\n");
			      exit(0);
			    }		
    		   	  int file_len = (int)statbuf.st_size;
			  buffer = (char *)malloc(file_len*sizeof(char));
			  if(buffer == NULL)
			  {
				printf("Error during memory allocation for buffer\n");
				exit(0);
			  }
			  fileptr = fopen(file_path,"r");
			  if(fileptr == NULL)
			  {
				printf("Error occured\n");
				return 0;
		          }
			  int n = 0;
			  while(1){
			  c = fgetc(fileptr);
	  		  if(feof(fileptr)){
					fclose(fileptr);		
					break;
				}
				buffer[n++] = c;
			  }
			  if(file_len > 1)
				file_len = file_len-1;
			  crc = crc32(crc,buffer,file_len);
			  
			  error_check = pthread_mutex_lock(&mv->lock);
			  if(error_check != 0)
			  {
				printf("Error in locking the thread\n");
				exit(0);
			  }
			  while(mv->count != 0)
				pthread_cond_wait(&mv->criticalSecAvail,&mv->lock);
			  mv->count = 1;
			  mv->finFiles[mv->upd_ind] = (char*)malloc(sizeof(char)*strlen(current_file));
			  if(mv->finFiles[mv->upd_ind] == NULL)
			  {
				printf("Error in memory allocation for finished file names\n");
				exit(0);
			  }
			  strcpy(mv->finFiles[mv->upd_ind],current_file);
			  mv->checkSum[mv->upd_ind] = crc;
			  mv->upd_ind+=1;
			  if(mv->current_ind >= n_files)
			  {
				    mv->count = 0;
				    pthread_cond_signal(&mv->criticalSecAvail);
				    pthread_mutex_unlock(&mv->lock);
				    pthread_exit(NULL);
			  }
			  strcpy(current_file,file_array[mv->current_ind]);
			  mv->current_ind+=1;
			  mv->count = 0;
			  pthread_cond_signal(&mv->criticalSecAvail);
			  error_check = pthread_mutex_unlock(&mv->lock);
			  if(error_check != 0)
			  {
				printf("Error in unlocking the thread\n");
				exit(0);
			  }
			  
		}while(mv->current_ind-1 < n_files);
	return NULL;
}

int main(int argc, char *argv[]){

  if(argc != 3)
	{
		printf("Number of arguments passed are invalid\n");
		return 0;
	}
  n_threads = atoi(argv[2]);
  if(n_threads < 1 || n_threads > 99)
	{
		printf("Number of threads value should lie between 1 and 99.\n");
		return 0;
	}
  int len = strlen(argv[1]);
  name_f = (char*)malloc(sizeof(char)*len);
  if(name_f == NULL)
	{
		printf("Error occured during memory allocation\n");
		return EXIT_FAILURE;
	}
  if(argv[1][len-1] == '/')
	{
		strncpy(name_f,argv[1],len-1);
	}
	else
	{
		strcpy(name_f,argv[1]);
	}
  mv = (struct monitor_vars*)malloc(sizeof(struct monitor_vars));
  if(mv == NULL)
  {
	printf("Error occured during memory allocation\n");
	return EXIT_FAILURE;
  }
  DIR *dip;
  int m = 0;
  struct dirent *dit;
  struct stat statbuf;
  char currentPath[FILENAME_MAX];
  dip = opendir(name_f);
   
  if(dip == NULL)
	{
		printf("Error opening the directory.\n");    	
		return EXIT_FAILURE;
	}

  while((dit = readdir(dip)) != NULL){
   
    if(strcmp(dit->d_name, ".") == 0 || strcmp(dit->d_name, "..") == 0)
      continue;
    strcat(currentPath, "/");
    strcat(currentPath,name_f);
    strcat(currentPath,"/");
    strcat(currentPath, dit->d_name);
    if(stat(currentPath, &statbuf) == -1){
      printf("Error in stat\n");
      return errno;
    }
    if(S_ISREG(statbuf.st_mode))
	   {
			int str_len = strlen(dit->d_name);  
			file_array[m] = (char *)malloc(sizeof(char)*str_len);
			if(file_array[m] == NULL)
			  {
				printf("Error occured during memory allocation\n");
				exit(0);
			  }
			strcpy(file_array[m],dit->d_name);
			m++;		  
	   }
  }
  closedir(dip);
  int x = 0, y = 0 , z = 0;
  char temp[1024],value[1024];
  for(x=0 ; x<m ; x++){
		z=x;
		strcpy(value,file_array[x]);
		for(y=x+1 ; y<m ; y++){
			if(strcmp(value,file_array[y])>0){
				strcpy(value,file_array[y]);
				z = y;
			}
		}
		strcpy(temp,file_array[x]);
		strcpy(file_array[x],value);
		strcpy(file_array[z],temp);
	}
  pthread_t thread[n_threads];
  mv->finFiles = (char**)malloc(sizeof(char*)*m);
  if(mv->finFiles == NULL)
			  {
				printf("Error occured during memory allocation\n");
				return EXIT_FAILURE;
			  }
  mv->checkSum = (uint32_t *)malloc(sizeof(uint32_t)*m);
  if(mv->checkSum == NULL)
			  {
				printf("Error occured during memory allocation\n");
				return EXIT_FAILURE;
			  }
  int n = 0, val = 0;
  mv->current_ind = 0;
  mv->upd_ind = 0;
  mv->count = 0;
  n_files = m;
  int active_thread = n_threads;
  if(n_files < n_threads)
  active_thread = n_files;
  while(n < n_threads && mv->current_ind < m)
  {
	 if((val = pthread_create(&thread[n],NULL,thread_func,(void*)(intptr_t)mv->current_ind))) {
		printf("Failure in creating the thread\n");
		return EXIT_FAILURE;
  	 }
     	 n+=1;
 	 val = pthread_mutex_lock(&mv->lock);
	 if(val != 0)
			  {
				printf("Error occured while locking the main thread\n");
			    return EXIT_FAILURE;
			  }
	 mv->current_ind+=1;
	 val = pthread_mutex_unlock(&mv->lock);
	 if(val != 0)
			  {
				printf("Error occured while unlocking the main thread\n");
				return EXIT_FAILURE;
			  }
  }
	
  for( n = 0 ; n < active_thread ; n++ )
  {
	 void *ret_val;
	 if((val = pthread_join(thread[n],&ret_val)))
	 {
		printf("Failure in joining the thread\n");
		return EXIT_FAILURE;
	 }
  }
  for(n=0;n<n_files;n++)
  {
		for(m = 0; m < n_files ; m++)
		{
			if(strcmp(file_array[n],mv->finFiles[m]) == 0)
			{			  
				printf("File: %s\tChecksum: %08X\n",mv->finFiles[m],mv->checkSum[m]);
			}
		}
  }
  return 0;
}



