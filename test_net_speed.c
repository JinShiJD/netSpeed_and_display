#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INTERFACE_NAME_LEN 100

// 使用 ethool 查询网卡速率的函数
int get_link_speed(const char *interface)
{
    char command[MAX_INTERFACE_NAME_LEN + 50];
    FILE *fp;
    int speed = 0;

    // 构造ethtool命令来查询网卡速率
    snprintf(command, sizeof(command), "ethtool %s | grep 'Speed:'", interface);

    fp = popen(command, "r");
    if(fp == NULL)
    {
        perror("Error opening pipe to run ethtool");
        return -1;
    }

    // 读取输出结果
    char line[256];
    if (fgets(line, sizeof(line), fp) != NULL)
    {
        if(strstr(line, "1000Mb/s"))
        {
            speed = 1000; // 1000mbps(千兆)
        }
        else if(strstr(line, "100Mb/s"))
        {
            speed = 100; ///100Mbps(百兆)
        }
    }

    fclose(fp);
    return speed;

}


// 检查网卡是否在线的函数
int is_interface_up(const char *interface)
{
    char command[MAX_INTERFACE_NAME_LEN + 50];
    FILE *fp;
    int status = 0;

    // 构造命令来检查网卡状态
    snprintf(command, sizeof(command), "cat /sys/class/net/%s/operstate", interface);

    fp = popen(command, "r");
    if(fp == NULL)
    {
        perror("Error opening pipe to check interface status");
        return -1;
    }



    // 读取状态
    char line[256];
    if(fgets(line, sizeof(line), fp) != NULL)
    {
        if (strstr(line, "up"))
        {
            status = 1;  //网卡在线
        }
        else
        {
            status = 0; //网卡离线
        }
    }


    fclose(fp);
    return status;
}


int main()
{
    const char *interface = "ens37";  // 网卡名称，根据实际情况修改

    if (is_interface_up(interface))
    {
        printf("网卡 %s 处于连接状态。 \n", interface);

        // 获取并显示网卡速率
        int speed = get_link_speed(interface);
        if(speed == -1)
        {
            printf("无法获取网卡的速率信息。 \n");
        }
        else if(speed == 1000)
        {
            printf("网卡速率是： 千兆(1000Mbps)。\n");
        }
        else if(speed == 100)
        {
            printf("网卡速率是：百兆(100Mbps)。\n");
        }
        else
        {
            printf("网卡的速率无法识别。 \n");
        }
    }
    else
    {
        printf("网卡 %s 不处于连接状态。\n", interface);
    }

    return 0;
}



