//CMPSC 311 FA22
//LAB 2

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"

extern int cur_block;
extern int cur_disk;

//would pack bytes go here?
uint32_t pack_bytes(uint32_t diskID, uint32_t blockID, uint32_t cmmd, uint32_t rsvd)
{
	//setup local value
	uint32_t retval = 0x0, tempdiskID, tempblockID, tempcmmd, temprsvd;
	
	tempdiskID = (diskID&0xff) << 8;
	tempblockID = blockID&0xff;
	tempcmmd = cmmd&0xff << 12;
	temprsvd = rsvd&0xff << 18;

	//return retval
	return retval;
}

// do a jbod operation with op and block buffer in parameters
int jbod_operation(uint32_t op, uint8_t *block);

//write given op using command diskID and blockID in parameters
int jbod_op(uint32_t cmmd, uint32_t diskID, uint32_t blockID);

//initialize mount
int mount = 0;

int mdadm_mount(void) {
	//if its not already been set to 1,
	if(mount)
	{
		return -1;
	}
	int op = jbod_op(JBOD_MOUNT()<<12, 0,0);
	jbod_operation(op,NULL);
	mount = 1;
	return 1;

 }

int mdadm_unmount(void) {
		//if its not already been set to 1,
	if(!mount)
	{
		return -1;
	}
	int op = jbod_op(JBOD_UNMOUNT()<<12, 0,0);
	jbod_operation(op,NULL);
	mount = 1;
	return 1;
	
 }

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
	//initialize variables
	int read_bytes;
	int cur_addr = start_addr;
	//set temp to 256 bytes
	int *temp_buf = malloc(sizeof(int)*64);
	int offset;

	// check for errors
	if (!mount)
	{
		return -1;
	}
	if (read_len>2048)
	{
		return -1;
	}
	//make sure there is no out of bounds linear address
	if((start_addr + read_len) > 1048576)
	{
		return -1;
	}

	//current disk using start address divided by disk size, then times disk size
	int cur_disk = start_addr / 256 * 256;
	//current block modulus 256 from start address, divided by disk size
	int cur_block = start_addr % 256 / 256;

	//jbod operation to seek to block and disk, using packBytes
	jbod_operation(pack_bytes(cur_disk,0,SEEK_TO_DISK,0),NULL);
	jbod_operation(pack_bytes(0,cur_block,SEEK_TO_BLOCK,0),NULL);

	//forloop to go through and read from the devices
	for(int x = 0; i<read_len; x += read_bytes)
	{
		//set offset equal to current address modulus a disk space
		offset = cur_addr % 256;
		jbod_operation(pack_bytes(0,0,READ_BLOCK,0), *temp_buf);
		//check if it is a partial block
		if(read_len<256)
		{
			mem_copy(*temp_buf[offset],read_buf[read_bytes],(read_len-read_bytes));
			//if it ends in the middle of a block
			if (read_len<256)
			{
				read_bytes = read_len;
			}
			else {read_bytes += 256 - offset;}
		}
		if(read_len == 256)
		{
			mem_copy(*temp_buf[0], read_buf[read_bytes],256);
			read_bytes += 256;
		}
		curr_addr += read_bytes;
		if (read_bytes == 256) // how would we find the end of disk 
		{
			jbod_operation(pack_bytes(cur_disk,0,SEEK_TO_DISK,0),NULL);
			jbod_operation(pack_bytes(0,cur_block,SEEK_TO_BLOCK,0),NULL);
		}
	}
	return read_bytes;
}
