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
	char buffer[BLOCKSIZE];
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

	//fill the block with '0'
	memset(buffer, 0, sizeof(buffer));
	ssufs_writeDataBlock(block_num, buffer);
	//read inode
	ssufs_readInode(inode_num, &new_inode);
	//set inode
	new_inode.status = INODE_IN_USE;
	strcpy(new_inode.name, filename);
	new_inode.direct_blocks[0] = block_num;
	new_inode.file_size = 0;
	//write inode
	ssufs_writeInode(inode_num, &new_inode);

	return inode_num;
}

void ssufs_delete(char *filename){
	int inode_num;
	char buffer[BLOCKSIZE];
	struct inode_t info;

	if((inode_num = open_namei(filename)) == -1){
#if DEBUG
	fprintf(stderr,"The file is not exist\n");
#endif
		return;
	}
	ssufs_readInode(inode_num, &info);
	memset(buffer, 0, sizeof(buffer));
	//data clear
	for(int i = 0; i < MAX_FILE_SIZE; i++){
		if(info.direct_blocks[i] != -1)
			ssufs_writeDataBlock(info.direct_blocks[i], buffer);
	}
	ssufs_freeInode(inode_num);
}

int ssufs_open(char *filename){
	int inode_num, file_handle;

	if((inode_num = open_namei(filename)) == -1){
#if DEBUG
	fprintf(stderr,"The file is not exist\n");
#endif
		return -1;
	}

	file_handle =  ssufs_allocFileHandle();
	if(file_handle != -1){
		file_handle_array[file_handle].inode_number = inode_num;
		file_handle_array[file_handle].offset = 0;
		return file_handle;
	}
	else{
#if DEBUG
	fprintf(stderr,"The file has been open to maximum\n");
#endif
		return -1;
	}
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	int start_block, offset, start_idx, cnt = 0;
	struct inode_t info; 
	char buffer[BLOCKSIZE];

	offset = file_handle_array[file_handle].offset;
	if(ssufs_lseek(file_handle, nbytes) == -1){
#if DEBUG
	fprintf(stderr,"The size to read exceeds the file size\n");
#endif
		return -1;
	}
	ssufs_readInode(file_handle_array[file_handle].inode_number, &info);
	start_block = offset / BLOCKSIZE;
	start_idx = offset % BLOCKSIZE;
	ssufs_readDataBlock(info.direct_blocks[start_block], buffer);


	while(cnt < nbytes){
		buf[cnt++] = buffer[start_idx++];
		if(start_idx == BLOCKSIZE && cnt < nbytes){
			start_block++;
			ssufs_readDataBlock(info.direct_blocks[start_block], buffer);
			start_idx = 0;
		}
	}
	buf[cnt] = '\0';
	return 0;
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	int offset, inode, diff, create_num = 0, cnt = 0;
	int start_block, start_idx, data_index;
	char buffer[BLOCKSIZE];
	struct inode_t info;

	offset = file_handle_array[file_handle].offset;
	if(offset + nbytes > (BLOCKSIZE * MAX_FILE_SIZE)){
#if DEBUG
	fprintf(stderr,"The size to write exceeds the file size\n");
#endif	
		return -1;
	}
	inode = file_handle_array[file_handle].inode_number;
	ssufs_readInode(inode, &info);

	start_block = offset / BLOCKSIZE;
	start_idx = offset % BLOCKSIZE;

	diff = BLOCKSIZE - start_idx;
	while(diff < nbytes){
		create_num++;
		if(info.direct_blocks[start_block + create_num] == -1){
			if((data_index = ssufs_allocDataBlock()) == -1){
				for(int i = 1; i < create_num; i++){
					ssufs_freeDataBlock(info.direct_blocks[start_block + i]);
					info.direct_blocks[start_block + i] = -1;
				}
#if DEBUG
	fprintf(stderr,"Not enough free block\n");
#endif			
				return -1;
			}
			else
				info.direct_blocks[start_block + create_num] = data_index;
		}
		diff += BLOCKSIZE;
	}
	//fill the new block with '0'
	memset(buffer, 0, sizeof(buffer));
	for(int i = 1; i <= create_num; i++)
		ssufs_writeDataBlock(info.direct_blocks[start_block + i], buffer);
	
	ssufs_readDataBlock(info.direct_blocks[start_block], buffer); 
	//overwrite
	if(info.file_size > offset){
		while(offset < info.file_size && cnt < nbytes){
			buffer[start_idx++] = buf[cnt++];
			if(start_idx == BLOCKSIZE){
				ssufs_writeDataBlock(info.direct_blocks[start_block], buffer);
				start_block++;
				if(start_block < MAX_FILE_SIZE)
					ssufs_readDataBlock(info.direct_blocks[start_block], buffer);
				else
					memset(buffer, 0, sizeof(buffer));
				start_idx = 0;
			}
			else if(offset + 1 == info.file_size || cnt == nbytes)
				ssufs_writeDataBlock(info.direct_blocks[start_block], buffer);
			offset++;
		}
	}
	//write
	while(cnt < nbytes){
		buffer[start_idx++] = buf[cnt++];
		if(start_idx == BLOCKSIZE){
			ssufs_writeDataBlock(info.direct_blocks[start_block], buffer);
			start_block++;
			if(start_block < MAX_FILE_SIZE)
				ssufs_readDataBlock(info.direct_blocks[start_block], buffer);
			else
				memset(buffer, 0, sizeof(buffer));
			start_idx = 0;
		}
		else if(cnt == nbytes)
			ssufs_writeDataBlock(info.direct_blocks[start_block], buffer);
		info.file_size++;
	}
	ssufs_writeInode(inode, &info);
	ssufs_lseek(file_handle, nbytes);
	return 0;
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
