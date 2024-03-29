//CMPSC 311 FA22
//LAB 2

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "mdadm.h"
#include "jbod.h"

int cur_block;
int cur_disk;

//write given op using command,diskID,clock ID
uint32_t op(uint32_t diskID, uint32_t blockID, uint32_t cmmd)
{
	//setup local value
	uint32_t tempdiskID, tempblockID, tempcmmd;

	
	tempdiskID = (diskID) << 8;
	tempblockID = blockID;
	tempcmmd = cmmd << 12;

	uint32_t retval = tempdiskID|tempblockID|tempcmmd;

	//return retval
	return retval;
}

// do a jbod operation with op and block buffer in parameters
int jbod_operation(uint32_t op, uint8_t *block);


//initialize mount
int mount = 0;
int jbod_op = 0;

int mdadm_mount(void) {
  if(mount ==0)
    {
  int jbod_op = op(0,0,JBOD_MOUNT);
	//use the op function and the operation function to mount 
  if(jbod_operation (jbod_op,NULL)== 0)
	{
		mount = 1;
		return 1;
	}
  else
    {
      //return -1 if it has not been mounted
      return -1;
    }
    }
  else {
    return -1;
 }
}

int mdadm_unmount(void) {
  if(mount ==1)//if it has already been mounted you now unmount it
    {
      
  int jbod_op = op(0,0,JBOD_UNMOUNT);
		//if its not already been set to 1,
  if(jbod_operation(jbod_op,NULL)==0)
	{
	mount = 0;
	return 1;
	}
  
  else
    {//if unsuccessfully unmounted, return -1
      return -1;
    }
    }
  else
    {
      return -1;
    }
	
 }
//I am so sorry I tried so hard to make this but i kept getting stack smashing and infinite loops and I will fix it this upcoming week I promise! 
int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {


	// check for errors
	if( read_buf == NULL && read_len == 0)
	  {
	    return 0;
	  }
	//mount should fail on an anmounted system
        if (mount!=1)
	  {
	    return -1;
	  }
	if (read_len == 0)
	  {
	    return -1;
	  }
	
	//of the length is bigger that a disk
	if (read_len>2048)
	{
		return -1;
	}
	//make sure there is no out of bounds linear address
	if((start_addr + read_len) > 1048576)
	{
		return -1;
	}
	//if read buf is equal to null and read length is not equal to 0
	if(read_buf == NULL && read_len != 0)
	  {
	    return -1;
	  }

	
	 //initialize variables
  	int cur_addr = start_addr;
	//current disk using start address divided by disk size, then times disk size
	int cur_disk = start_addr / 65536;
	//current block modulus 256 from start address, divided by disk size
	int cur_block = start_addr % 65536 / 256 ;
	int offset = cur_addr % 256;
	int read_bytes = 0;
	//set temp to 256 bytes
  	uint8_t temp_buf[256];
      	//make a need to read variable for the remainders
      	int need_to_read = 0;
	//while loop to  see if the read length is greater than what has been read
	while(read_bytes < read_len)
	{

	//jbod operation to seek to block and disk, using packBytes
	int jbod_disk = op(cur_disk,0,JBOD_SEEK_TO_DISK);
	jbod_operation(jbod_disk,NULL);
	int jbod_block = op(0,cur_block,JBOD_SEEK_TO_BLOCK);
	jbod_operation(jbod_block,NULL);
	int jbod_read = op(0,0,JBOD_READ_BLOCK);
	jbod_operation(jbod_read,temp_buf);
	//set need to read to remainder of read
	need_to_read = read_len - read_bytes;
	//if offset plus remainder is less than a block
		if(offset+need_to_read<256)
		{
		//do mem copy for strating place to the rest of block
			memcpy(read_buf+read_bytes,temp_buf+offset,need_to_read);
			read_bytes += need_to_read;
		}
		//do the next block if initial one has not been completely read
		else
		{
			memcpy(read_buf+read_bytes,temp_buf+offset,256-offset);
			read_bytes += 256-offset;
		}
	
  	//current address is added to the read_bytes
       	cur_addr += read_bytes;
//update current disk and block
       	cur_disk = cur_addr/65536;
       	cur_block = cur_addr%65536 /256;
	//update the disk and block
	int update_disk = op(cur_disk,0,JBOD_SEEK_TO_DISK);
         jbod_operation(update_disk,NULL);
	 int update_block = op(0,cur_block,JBOD_SEEK_TO_BLOCK);
         jbod_operation(update_block,NULL);
         offset = 0;
         
		       
	}
	
	//return the final bytes read
	 return read_bytes;
}

/*
//forloop to go through and read from the devices
	for(int x = 0; x<read_len; x +=read_len)
	{
	  
	  printf("loop %d\n",x);
       	//set offset equal to current address modulus a disk space
	 offset = cur_addr % 256;
	int op_read = op(0,0,JBOD_READ_BLOCK);
	jbod_operation(op_read,temp_buf);

	  printf("%d offset: %d",cur_addr, offset);
       	//check if it is a partial block
       		if(offset == 0)
       		{
       		printf("curr addr: %d , offset: %d",cur_addr,offset);
	 	 memcpy(read_buf+read_bytes,temp_buf+offset,read_len-read_bytes);
	     	//if it ends in the middle of a block
	 		 if (read_bytes+offset<256)
  			{
  
	      		read_bytes = read_len;
	      		printf("%d--", read_bytes);
       			}
		  
			 else
			 {
		      	read_bytes += 256 - offset;
		 	}
		}	
	 	if(read_len == 256)//if its a full block incrememtn read bytes 
	   	{
	     	memcpy(read_buf+read_bytes,temp_buf+0,256);
	     	read_bytes += 256;
	   	}
       	
	  	if(offset==0)
	  	 {
	     	memcpy(read_buf+read_bytes,temp_buf,read_len);
	      	 if(read_len<JBOD_BLOCK_SIZE)
	    	{
	      	read_bytes = read_len;
	      	free(temp_buf);
	     	 return read_bytes;
	    	}
	  	else
	  	{
	      	read_bytes +=256;
	  	}
	     
	  
	
       	cur_addr += read_bytes;

       	cur_disk = cur_addr/JBOD_DISK_SIZE;
       	cur_block = (cur_addr%JBOD_DISK_SIZE) /JBOD_BLOCK_SIZE;
	
	int update_disk = op(cur_disk,0,JBOD_SEEK_TO_DISK);
         jbod_operation(update_disk,NULL);
	 int update_block = op(0,cur_block,JBOD_SEEK_TO_BLOCK);
         jbod_operation(update_block,NULL);
         
         printf("%d\n", read_bytes);
		       
	}
	
	
	 return read_len;
}


//forloop to go through and read from the devices
	for(int x = 0; x<read_len; x +=read_len)
	{
	  
	  printf("loop %d\n",x);
       	//set offset equal to current address modulus a disk space
	 offset = cur_addr % 256;
	int op_read = op(0,0,JBOD_READ_BLOCK);
	jbod_operation(op_read,temp_buf);

	  printf("%d offset: %d",cur_addr, offset);
       	//check if it is a partial block
       		if(offset == 0)
       		{
       		printf("curr addr: %d , offset: %d",cur_addr,offset);
	 	 memcpy(read_buf+read_bytes,temp_buf+offset,read_len-read_bytes);
	     	//if it ends in the middle of a block
	 		 if (read_bytes+offset<256)
  			{
  
	      		read_bytes = read_len;
	      		printf("%d--", read_bytes);
       			}
		  
			 else
			 {
		      	read_bytes += 256 - offset;
		 	}
		}	
	 	if(read_len == 256)//if its a full block incrememtn read bytes 
	   	{
	     	memcpy(read_buf+read_bytes,temp_buf+0,256);
	     	read_bytes += 256;
	   	}
       	
	  	if(offset==0)
	  	 {
	     	memcpy(read_buf+read_bytes,temp_buf,read_len);
	      	 if(read_len<JBOD_BLOCK_SIZE)
	    	{
	      	read_bytes = read_len;
	      	free(temp_buf);
	     	 return read_bytes;
	    	}
	  	else
	  	{
	      	read_bytes +=256;
	  	}
	     
	  
	
	       
  	
       	cur_addr += read_bytes;

       	cur_disk = cur_addr/JBOD_DISK_SIZE;
       	cur_block = (cur_addr%JBOD_DISK_SIZE) /JBOD_BLOCK_SIZE;
	
	int update_disk = op(cur_disk,0,JBOD_SEEK_TO_DISK);
         jbod_operation(update_disk,NULL);
	 int update_block = op(0,cur_block,JBOD_SEEK_TO_BLOCK);
         jbod_operation(update_block,NULL);
         
         printf("%d\n", read_bytes);
		       
	}
	
	
	 return read_len;
*/
