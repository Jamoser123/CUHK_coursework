#include "call.h"
const char *HD = "HD";

inode* read_inode(int fd, int i_number){
	inode* ip = malloc(sizeof(inode));
	int currpos=lseek(fd, I_OFFSET + i_number * sizeof(inode), SEEK_SET);
	if(currpos<0){
		printf("Error: lseek()\n");
		return NULL;
	}
	
	//read inode from disk
	int ret = read(fd, ip, sizeof(inode));
	if(ret != sizeof (inode) ){
		printf("Error: read()\n");
		return NULL;
	}
	return ip;
}

int min(int a, int b){
	if(a < b) return a;
	else return b;
}

int max(int a, int b){
	if(a < b) return b;
	else return a;
}

inode* get_inode(int fd, char* filename, inode* curr)
{
	DIR_NODE* dir_block = (DIR_NODE*)malloc(BLK_SIZE);
	int inode_number = -1;
	int entries;
	int entries_per_block = BLK_SIZE/sizeof(DIR_NODE);

	//read direct data blocks
	for(int blk_id = 0; blk_id < 2; blk_id++){
		if(inode_number > -1){
			break;
		}
		pread(fd, dir_block, BLK_SIZE, D_OFFSET + curr->direct_blk[blk_id] * BLK_SIZE);

		entries = min(curr->sub_f_num - blk_id * entries_per_block, entries_per_block);
		for(int i = 0; i < entries; i++){
			if(strcmp(filename, dir_block[i].f_name) == 0){
				inode_number = dir_block[i].i_number;
				break;
			}
		}
	}

	//read indirect data blocks
	if(inode_number == -1){
		int* p_block = (int*)malloc(BLK_SIZE);
		pread(fd, p_block, BLK_SIZE, D_OFFSET + curr->indirect_blk * BLK_SIZE);
		
		for(int i = 0; i < curr->blk_number - 2; i++){
			if(inode_number > -1){
				break;
			}
			pread(fd, dir_block, BLK_SIZE, D_OFFSET + p_block[i] * BLK_SIZE);

			entries = min(curr->sub_f_num - (i + 2) * entries_per_block, entries_per_block);
			for(int j = 0; j < entries; j++){
				if(strcmp(filename, dir_block[j].f_name) == 0){
					inode_number = dir_block[j].i_number;
					break;
				}
			}
		}
		free(p_block);
	}
	free(dir_block);

	return read_inode(fd, inode_number);
}


int open_recursive(int fd, char *path, inode* curr)
{
	if(curr == NULL){ //error
		return -1;
	}

	if(path[0] == '\0'){
		return curr->i_number;
	}

	char* sub = strchr(path, '/');
	sub[0] = '\0';
	inode* next = get_inode(fd, path, curr);

	return open_recursive(fd, sub + 1, next);
}

int open_t(char *pathname)
{
	if(strcmp(pathname, "/") == 0){
		return 0;
	}

	int fd = open("./HD", O_RDONLY);
	inode* root = read_inode(fd, 0);
	strcat(pathname, "/"); 

	int inode_number = open_recursive(fd, pathname + 1, root);
	close(fd);
	return inode_number;
}


int read_t(int i_number, int offset, void *buf, int count)
{
	int read_bytes = 0;
	int fd = open("./HD", O_RDONLY);
	int blk_id = offset/BLK_SIZE;
	inode* inode = read_inode(fd, i_number);
	int init_offset = offset % BLK_SIZE;

	//check for overflow and adjust how many bytes to read
	if(offset + count > inode->f_size){
		count = max(inode->f_size - offset, 0);
	}

	//read direct blocks
	for(; blk_id < 2; blk_id++){
		if(count < read_bytes + BLK_SIZE - init_offset){
			read_bytes += pread(fd, buf, count - read_bytes, D_OFFSET + init_offset + inode->direct_blk[blk_id] * BLK_SIZE);
			break;
		}
		read_bytes += pread(fd, buf, BLK_SIZE - init_offset, D_OFFSET + init_offset + inode->direct_blk[blk_id] * BLK_SIZE);
		buf += BLK_SIZE - init_offset;
		init_offset = 0;
	}

	//read indirect blocks
	if(read_bytes != count){
		int* p_block = (int*)malloc(BLK_SIZE);
		pread(fd, p_block, BLK_SIZE, D_OFFSET + inode->indirect_blk * BLK_SIZE);

		for(blk_id -= 2; blk_id < inode->blk_number-2; blk_id++){
			if(count < read_bytes + BLK_SIZE - init_offset){
				read_bytes += pread(fd, buf, count - read_bytes, D_OFFSET + init_offset + p_block[blk_id] * BLK_SIZE);
				break;
			}
			read_bytes += pread(fd, buf, BLK_SIZE - init_offset, D_OFFSET + init_offset + p_block[blk_id] * BLK_SIZE);
			buf += BLK_SIZE - init_offset;
			init_offset = 0;
		}
		free(p_block);
	}

	close(fd);

	return read_bytes; 
}