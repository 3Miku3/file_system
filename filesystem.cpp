#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
    磁盘管理
*/

// 磁盘空间  //1MB
#define DISK_SIZE   1024*1024
// 物理块大小
#define BLOCK_SIZE  512
// 物理块个数
#define NUM_BLOCK   (DISK_SIZE/BLOCK_SIZE)

typedef struct {
    char data[BLOCK_SIZE];
} block_t;

// 模拟的磁盘空间
block_t disk[NUM_BLOCK];

short map[46][46];

block_t* malloc_block(){
	int i,j;
	for(i = 1;i<46;i++){
		for(j = 1;j<46;j++){
				if(map[i][j] == 0){
					map[i][j] = 1;
					int index = 45*(i-1) + j;
					return &disk[index-1];
				}
		}
	}

}
void free_block(block_t* block){
	int index = block - disk ;
	int i = (index/45) + 1;
	int j= (index%45) + 1;
	map[i][j] = 0;
}

void init_file_system(){
	int i,j;
	for(i = 0;i<46;i++){
		for(j = 0;j<46;j++)
			map[i][j] = 0;
	}
}
/*


*/






/*
    文件管理
*/

#define MAX_NAME  32
#define MAX_FILE  10

// 文件结点定义
typedef struct {
    char name[MAX_NAME]; // 文件名
    int size;            // 文件大小
    block_t* block;      // 存放数据的物理块
} file_node_t;

// 目录表定义
typedef struct {
    file_node_t nodes[MAX_FILE];
    int count;
} dir_table_t;

/*
  二级目录结构体
*/
#define MAX_DIR 10
 typedef struct{
	char name[MAX_NAME];
	dir_table_t* dir;
 } usr_node_t;


typedef struct{
	usr_node_t nodes[MAX_DIR]; //10个文件目录表
	int count ; //initialize the count value to 0
 } usr_table_t ;

 usr_table_t tables;//chu shi hua jie gou ti tables.
// 系统目录表
dir_table_t root_dir = {0};
// 当前目录
usr_node_t* workdir;
dir_table_t* work_dir;


// 根据文件名查找文件结点
file_node_t* find_file_node(const char* filename) {
    int i;
    for (i = 0; i < work_dir->count; i++)
        if (strcmp(work_dir->nodes[i].name, filename) == 0)
            return &work_dir->nodes[i];
    return NULL;
}

// 根据文件名创建文件
file_node_t* create_file(const char* filename) {
    file_node_t* node = find_file_node(filename);
    if (node == NULL) {
        node = &work_dir->nodes[work_dir->count];
        strcpy(node->name, filename);
        node->size = 0;
        node->block = malloc_block();
        work_dir->count++;
    }
    return node;
}

// 根据文件名删除文件
void remove_file(const char* filename) {
    file_node_t* node = find_file_node(filename);
    if (node == NULL) {
        printf("file not exist!\n");
        return;
    }

    free_block(node->block);

    int index = node - work_dir->nodes;
    memcpy(&work_dir->nodes[index], &work_dir->nodes[index+1],
            (work_dir->count - index - 1) * sizeof(file_node_t));  //重建数组
    work_dir->count--;
}

// 打印文件内容
void print_file(const char* filename) {
    file_node_t* node = find_file_node(filename);
    if (node == NULL) {
        printf("file not exist!\n");
        return;
    }

    printf("%s\n", node->block->data);
}

// 根据文件名创建文件，并将数据写入文件中
void write_file(const char* filename, const char* data) {
    file_node_t* node = create_file(filename);
    strcpy(node->block->data, data);
    node->size = strlen(data);
}

// 打印当前目录
void print_dir() {
    printf("CurDir: %s\n",workdir->name);
    printf("Files: ");
    int i;
    for (i = 0; i < work_dir->count; i++)
        printf("%s ", work_dir->nodes[i].name);
    printf("\n");
}
/*
  文件打开表
*/

#define MAX_OPEN_FILE 100
char buffer[512];
typedef struct {
    file_node_t* nodes[MAX_OPEN_FILE];
} opened_file_table_t;

opened_file_table_t opened_file_table = {0};


// 获得一个空闲的文件描述符FCB
int get_unused_file_descriptor() {
    int i;
    for (i = 0; i < MAX_OPEN_FILE; i++) {
        if (opened_file_table.nodes[i] == NULL) return i;
    }
    return -1;
}

// 打开文件，成功返回文件描述符，失败返回 -1;
int my_open(char* filename, int mode) {
    file_node_t* node = find_file_node(filename);
    if (node == NULL) return -1;

    int fd = get_unused_file_descriptor();
    opened_file_table.nodes[fd] = node;
    return fd;
}
void my_close(int fd){
	printf("close file!\n");
	opened_file_table.nodes[fd] = NULL;
}
//read from a file
int my_read(int fd,char* buffer,int size){
	if(opened_file_table.nodes[fd] == NULL){
		printf("file is not opened yet!\n");
		return -1;
	}
	file_node_t* node = opened_file_table.nodes[fd];
	char* content = node->block->data; //content指向字符数组
	int nodeS = node->size;
	int realS = nodeS > size ? size : nodeS;
	int i;
	for(i = 0 ;i < nodeS ;i++){
		buffer[i] = '\0';  //buffer initialize ,if not that buffer will make random char;
	}
	for( i = 0 ; i < realS ; i++){
		buffer[i] = content[i];
	}
	return realS;
}
int my_write(int fd,char* buffer,int size){ //写入size个字节到fd内
	if(opened_file_table.nodes[fd] == NULL){
		printf("file is not opened yet!\n");
		return -1;
	}
	file_node_t* node = opened_file_table.nodes[fd];
	char* content = node->block->data; //content指向字符数组
 	int length = node->size;
	node->size = length + 1 + size;
	int i = length;
	content[i++] = '\n';  //length [\n 0..size]
	int j = 0;
	while(i < BLOCK_SIZE && j < size){
		content[i] = buffer[j];
		i++;
		j++;
	}
	return (j == size) ? size : j;
}
/*
  二级目录
*/

void initTables(){
	tables.count = 1;
	strcpy(tables.nodes[0].name,"root");
	tables.nodes[0].dir = &root_dir;
	workdir = &tables.nodes[0];
	work_dir = workdir->dir;
}
usr_node_t* find_usr_node(char* dirname){
	int i;
	for(i = 0;i < tables.count;i++){
		if(strcmp(tables.nodes[i].name,dirname) == 0){
			return &tables.nodes[i];
		}
	}
	return NULL;
 }
 usr_node_t* create_usr_node(char* dirname){
	usr_node_t* node = find_usr_node(dirname);
	int counts = tables.count;
	if(node == NULL){
		node = &tables.nodes[counts];
		strcpy(node->name ,dirname);
		node->dir = (dir_table_t*)malloc(sizeof(dir_table_t));
		tables.count++;
	}
	return node;
 }
int remove_usr_node(char* dirname){
 	usr_node_t* node = find_usr_node(dirname);
 	if(node == NULL){
		printf("directory is not exist\n");
		return -1;
	 }else{
		if(strcmp(workdir->name,dirname) == 0){
			printf("Do not delete the current dir in use\n");
			return -1;
		}
		free(node->dir);
		int index = node - tables.nodes; //数组名
		memcpy(&tables.nodes[index],&tables.nodes[index+1],
		(tables.count-index-1) * sizeof(usr_node_t));   //count-(index+1)
		//从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中		
		// cuo wu : mei jia kuo hao , yue jie cuo wu (Segmentation fault)
		tables.count--;
		printf("Delete Dir OK\n");
		return 1;
	 }
 }
 void print_dirs(){
	int i = 0;
	printf("Useful Dir: ");
	for(i = 0 ; i < tables.count;i++){
		printf("%s  ",tables.nodes[i].name);
	}
	printf("\n");
	printf("Directories count: %d ", tables.count);
	printf("\n");
	return;
 }

/****/
/*
    SHELL命令
*/

// 保存当前的命令行输入
char line[256];
// 根据命令行输入等到的命令参数
char args[10][80];
// 命令参数个数
int narg;
 int create_dir(char* name){
	if (narg != 2) {
        printf("Please input DirectoryName!\n");
        return -1;
    	}
	usr_node_t* node =  create_usr_node(name);
	if(node != NULL) return 1;
	return -1;
 }
 int remove_dir(char* name){
	if (narg != 2) {
        printf("Please input DirectoryName!\n");
        return -1;
    	}
		int res = remove_usr_node(name);
		return res;
 }
 int change_dir(char* name){
	if (narg != 2) {
        printf("Please input DirectoryName!\n");
        return -1;
    	}
	usr_node_t* curnode =  find_usr_node(name);
	if(curnode == NULL) return -1;
	workdir = curnode;  //change Main File Diectory;
	work_dir = workdir->dir;//change User File Directory;
	return 1;
 }
void exec_cat() {
    if (narg != 2) {
        printf("Please input filename!\n");
        return;
    }

    print_file(args[1]);
}

void exec_new() {
    if (narg != 2) {
        printf("Please input filename!\n");
        return;
    }

    const char* data = gets(line);
    write_file(args[1], data);
}

void exec_rm() {
    if (narg != 2) {
        printf("Please input filename!\n");
        return;
    }

    remove_file(args[1]);
}

void exec_change(){
	if (narg != 2) {
        printf("Please input filename!\n");
        return;
    	}
	int fd = my_open(args[1],1);
	int i = 0;
	int size = 0;
	int byte = 0;
	printf("please input a number to choose function ---- write:1 read:0 none:2 \n");
	scanf("%d",&i);
	getchar(); //吸收输入缓冲区的回车
	if(i == 2){
		my_close(fd);
		return;
	}
	else if( i == 0){
		printf("input you read-size\n");
		scanf("%d",&size);
		getchar();
		byte = my_read(fd,buffer,size);
		printf("buffer = %s, bytes = %d\n",buffer,byte);
	}else{
		printf("input what you write\n");
		scanf("%s",buffer);
		getchar();
		size = strlen(buffer);
		byte = my_write(fd,buffer,size);
		printf("%d bytes you write\n",byte);
	}
	my_close(fd);
 }


int main() {
    init_file_system();
printf("******************************************\n");
printf("**              文件系统                 **\n");
printf("**           1.新建文件 new  	        **\n");
printf("**           2.删除文件 rm    	        **\n");
printf("**           3.查看目录 ls    	        **\n");
printf("**           4.查看文件 cat    	        **\n");
printf("**           4.修改文件 mdf    	        **\n");
printf("**           5.退出系统 quit             **\n");
printf("**           6.新建目录 mkdir            **\n");
printf("**           7.删除目录 rmdir            **\n");
printf("**           8.改变目录 cd               **\n");
printf("******************************************\n");
    printf("> ");
    initTables();
    while (gets(line)) {
        narg = sscanf(line, "%s%s%s", args[0], args[1], args[2]);

        if (strcmp(args[0], "quit") == 0) {
            break;
        } else if (strcmp(args[0], "ls") == 0) {
	    print_dirs();//	
            print_dir();
        } else if (strcmp(args[0], "cat") == 0) {
            exec_cat();
        } else if (strcmp(args[0], "new") == 0) {
            exec_new();
        } else if (strcmp(args[0], "rm") == 0) {
            exec_rm();
        }else if(strcmp(args[0],"mdf") == 0){
		exec_change();
 	}else if(strcmp(args[0],"mkdir") == 0){
		create_dir(args[1]);
	} else if(strcmp(args[0],"cd") == 0){
		change_dir(args[1]);
	}else if(strcmp(args[0],"rmdir") == 0){
		remove_dir(args[1]);
	}else {
            printf("command not found\n");
        }
        printf("> ");
    }
}
