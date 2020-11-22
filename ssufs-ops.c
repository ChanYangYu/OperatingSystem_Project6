#include "ssufs-ops.h"
#define DEBUG 1

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	int inode_num;
	int block_num;
	struct inode_t new_inode;

	if(open_namei(filename) != -1){
#if DEBUG
	fprintf(stderr,"File is already exist!\n");
#endif
		return -1;
	}
	if((inode_num = ssufs_allocInode()) == -1){
#if DEBUG
	fprintf(stderr,"The number of file is full\n");
#endif
		return -1;
	}
	if((block_num = ssufs_allocDataBlock()) == -1){
#if DEBUG
	fprintf(stderr,"There is No FreeBlock\n");
#endif
		ssufs_freeInode(inode_num);
		return -1;
	}
	//read inode
	ssufs_readInode(inode_num, &new_inode);
	//set inode
	new_inode.status = INODE_IN_USE;
	strcpy(new_inode.name, filename);
	new_inode.direct_blocks[0] = block_num;
	//write inode
	ssufs_writeInode(inode_num, &new_inode);

	return inode_num;
}

void ssufs_delete(char *filename){
	int inode_num;

	if((inode_num = open_namei(filename)) == -1){
#if DEBUG
	fprintf(stderr,"The file is not exist\n");
#endif
		return;
	}
	ssufs_freeInode(inode_num);
}

int ssufs_open(char *filename){
	int inode_num;

	if((inode_num = open_namei(filename)) == -1){
#if DEBUG
	fprintf(stderr,"The file is not exist\n");
#endif
		return -1;
	}

	for(int i = 0; i < MAX_OPEN_FILES; i++){
		if(file_handle_array[i].inode_number = -1){
			file_handle_array[i].inode_number = inode_num;
			file_handle_array[i].offset = 0;
			return i;
		}
	}
#if DEBUG
	fprintf(stderr,"The file has been open to maximum\n");
#endif
	return -1;
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	/* 4 */
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	/* 5 */
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
