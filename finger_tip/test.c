#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "tip_point.h"

#define max_packet_size 1400
#define max_buf_size 40000

static struct sockaddr_in server_addr, client_addr;
static int server_sock_fd;

int udp_server_init(void)
{
    char recv_buf[100];
    int nbytes = 0;

    /* 创建Server Socket */
    server_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock_fd < 0)
    {
        printf("服务端Socket创建失败");
        return -1;
    }
    printf("服务端Socket创建成功\n");

    /* 绑定ip和端口 */
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY指机器所有的IP
    server_addr.sin_port = htons(3333);//指定端口号
    bind(server_sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    printf("服务端Socket绑定成功\n");
    return 0;
}

int main(){
    int i,x=0,y=0;
    int last_index = 0,this_index = 0;
    int recv_nbytes;
    int fb_write_position = 0;
    unsigned char recv_buf[max_packet_size+1];
    unsigned char *fb_buf;
    
    //为buf分配内存并初始化为0
    fb_buf = (unsigned char *)calloc(max_buf_size, sizeof(unsigned char));

    if(udp_server_init())
        printf("udp_server_init error \n");

    socklen_t len = sizeof(client_addr);

    while(1){
        len = sizeof(client_addr);
        recv_nbytes = recvfrom(server_sock_fd, recv_buf, max_buf_size+1, 0, (struct sockaddr *)&client_addr, &len);
        //recv_buf[nbytes] = '\0';
        recv_nbytes = recv_nbytes - 1;//接受位的大小
        this_index = (int)recv_buf[recv_nbytes];//将序号字节强制转换为int
       // printf("recv %d bytes data,index=%d ,fb_pos%d\n",recv_nbytes, this_index, fb_write_position);
        //printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        //将数据拷贝到fb_buf
        memcpy((void *)(fb_buf+fb_write_position), recv_buf, recv_nbytes);
        fb_write_position = fb_write_position + recv_nbytes;

        /* 判断是否是最后一包 */
        if ( this_index!=255 ){//没到最后一包就拷贝
            if( ((this_index-1) == last_index) || last_index == 0 )//序号连续就正常循环
                last_index = this_index;
            else
                continue;
        }
        else{//相等表示是最后一包
            //printf("fb loca c:%d\n", fb_buf);
            if(tip_posi(fb_buf, fb_write_position, &x, &y))//调用c++函数计算指尖位置
            {
                printf("tip_x=%d, tip_y=%d \n", x, y);
                //free(fb_buf);
                //break;
                //printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
            }
            else
                printf("tip not found \n");
            /* 初始化参数，准备下一张图片的接收 */
            memset(fb_buf, 0, fb_write_position);
            fb_write_position = 0;
            last_index = 0;
        }
        //printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    }
    free(fb_buf);//释放内存

    return 0;
}  

