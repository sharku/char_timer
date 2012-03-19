#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
int main()
{
	int fd;
	ssize_t ret;
	unsigned char val;
	fd = open( "/dev/char_timer1", O_RDWR );
	if (fd < 0)
	{
		printf("can't open device!!\n\r");
		return -1;
	}
	
	ret = read(fd, &val, 1);
	if (ret < 0)
	{
		printf("read error!!\n\r");
		return -1;
	}
	
	printf("val = %d\n\r", val);

    close(fd);
    return 0;
}
