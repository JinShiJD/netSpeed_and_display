#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/if_link.h>
#include <linux/if.h>
#include <errno.h>


// 解析属性列表
static void parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while(RTA_OK(rta, len))
    {
        if(rta->rta_type <= max)
        {
            tb[rta->rta_type] = rta;
        }
        rta = RTA_NEXT(rta, len);
    }
}


// 检测网卡是否处于连接状态并获取速率
void get_link_status_and_speed(const char *interface_name)
{
    struct sockaddr_nl sa;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg = {0};
    int sock;
    char buf[8192];  //接收netlink消息
    int len;


    //创建netlink通信的socket
    sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(sock < 0)
    {
        perror("Socket create failed!");
        return;
    }

    // 绑定地址
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = getpid();    //进程标识

    // 构建请求消息
    typedef struct nlmsghdr * n_addr;
    nlh = (n_addr)buf;
    //memset(nlh, 0, sizeof(struct nlmsghdr));

    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    nlh->nlmsg_type = RTM_GETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = 1;
    nlh->nlmsg_pid = sa.nl_pid;

    // 填充接口信息
    struct ifinfomsg *ifimsg = (struct ifinfomsg *)NLMSG_DATA(nlh);
    //ifimsg = (struct ifinfomsg *)NLMSG_DATA(nlh);
    ifimsg->ifi_family = AF_UNSPEC;   //获取所有接口
    ifimsg->ifi_change = 0;		// 仅查询，不监听变化

    // 设置消息的向量和地址
    iov.iov_base = nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //发送netlink请求
    if(sendmsg(sock, &msg, 0) < 0)
    {
        perror("Send Msg Failed !");
        close(sock);
        return;
    }

    // 接收netlink响应
    while(len = recv(sock, buf, sizeof(buf), 0))
	{
        if(len < 0)
        {
			perror("Recv Msg failed !");
			close(sock);
			return;
        }

        /****
        // 解析返回的netlink 信息
        // #define NLMSG_OK(nlh, len)  ((len) >= sizeof(struct nlmsghdr) && (len) >= (nlh)->nlmsg_len) 
        // #define NLMSG_NEXT(nlh, len)  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
         ****/	
        // 遍历所有消息头
        for(nlh = (struct nlmsghdr *)buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
        {
            //结束标记
            if(nlh->nlmsg_type == NLMSG_DONE)
            {
                goto done;
            }

            // 处理 RTA_NEWLINK 类型的消息
            if(nlh->nlmsg_type == RTM_NEWLINK)
            {
                struct ifinfomsg *ifi = NLMSG_DATA(nlh);
                struct rtattr *attrs[IFLA_MAX + 1];
                parse_rtattr(attrs, IFLA_MAX, IFLA_RTA(ifi), nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi)));
                //struct rtattr *attr = (struct rtattr *)( ((char*)ifi) + NLMSG_ALIGN(sizeof(struct ifinfomsg)));

                // 获取接口名称
                char *name = NULL;
                if(attrs[IFLA_IFNAME])
                {
                    name = (char *)RTA_DATA(attrs[IFLA_IFNAME]);
                }

                // 处理目标接口
                if(!name || strcmp(name, interface_name) != 0)
                {
                    continue;
                }

                // 获取状态、速率
                int status = 0;
                __u32 speed = 0;

                if(attrs[IFLA_IFNAME])
                {
                    status = (*((unsigned char *)RTA_DATA(attrs[IFLA_OPERSTATE])) == IF_OPER_UP);
                }

                if(attrs[IFLA_INFO_SPEED])
                {
                    speed = *((__u32 *)RTA_DATA(attrs[IFLA_LINK_SPEED]));
                }

                //输出结果
                printf("Interface: %s\n", name);
                printf(" Status: %s\n", status ? "Up" : "Down");
                if(speed > 0)
                {
                    printf(" Speed: %u Mbps\n", speed);
                }
                else
                {
                    printf(" Speed: Unkonwn\n");
                }
            }
        }
    }

done:	
    close(sock);
}




int main()
{
    const char *interface = "ens37";
    get_link_status_and_speed(interface);
    return 0;

}
