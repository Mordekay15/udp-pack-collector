#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define Q_PERMISSIONS 0660
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

int main()
{
    mqd_t qd_server, qd_client;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open ("/server", O_RDONLY | O_CREAT, Q_PERMISSIONS, &attr)) == -1) {
        perror ("Util2: mq_open (util2)");
        exit (1);
    }

    char in_buffer[MSG_BUFFER_SIZE];

    while(1) {
	if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Util2: mq_receive");
            exit (1);
        }

	char sep[2] = " ";
	char* istr;
	istr = strtok(in_buffer, sep);

	printf("Total packets: %s\t ", istr);

	istr = strtok(NULL, sep);
	printf("Total bytes: %s\n", istr);

    }
    return 0;
}
