//CMPSC 311 FA22
//LAB 2

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"


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
	int op = jbod_op(JBOD_MOUNT(), 0,0);
	jbod_operation(op,NULL);
	mount = 1;
	return 1;

 }

int mdadm_unmount(void) {
	return 0;
 }

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
	return 0;
}
