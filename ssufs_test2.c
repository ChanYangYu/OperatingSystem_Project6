#include "ssufs-ops.h"

int main()
{
    char str[] = "!-------32 Bytes of Data-------!!-------32 Bytes of Data-------!";
	char test1[256], test2[256];

	for(int i = 0; i < 255; i++)
		test1[i] = 'A';
    ssufs_formatDisk();

    ssufs_create("f1.txt");
    int fd1 = ssufs_open("f1.txt");

	ssufs_create("f2.txt");
	int fd2 = ssufs_open("f2.txt");

    printf("Write Data: %d\n", ssufs_write(fd1, str, BLOCKSIZE));
	printf("Write Data: %d\n", ssufs_write(fd2, test1, sizeof(test1)));
    printf("Seek: %d\n", ssufs_lseek(fd2, -100));
	ssufs_read(fd2, test2, 100);
	printf("Read Data : %s\n", test2);
    printf("Seek: %d\n", ssufs_lseek(fd2, -100));

	for(int i = 0; i < 100; i++)
		test1[i] = 'B';
	printf("Write Data: %d\n", ssufs_write(fd2, test1, 100));
	printf("Seek: %d\n", ssufs_lseek(fd2, -100));
	ssufs_read(fd2, test2, 100);
	printf("Read Data : %s\n", test2);
    ssufs_dump();
    ssufs_delete("f1.txt");
    ssufs_dump();
	ssufs_delete("f2.txt");
	ssufs_dump();
}
