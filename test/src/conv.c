#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
int main(int argc,char*argv[])
{
   unsigned short host_port = 0x1234;
   unsigned short net_port;

   unsigned long host_addr = 0x12345678;
   unsigned long net_addr;

   net_port = htons(host_port);
   net_addr = htonl(host_addr);

    printf("Host ordered port:%#x\n",host_port );
    printf("Network ordered port:%#x\n",net_port );

    printf("Host ordered address:%#lx\n",host_addr );
    printf("Network ordered address:%#lx\n",net_addr );



    char *addr1 = "1.2.3.4";
    char *addr2 = "1.2.3.256";

    unsigned long conv_addr = inet_addr(addr1);
    if (conv_addr == INADDR_NONE)
    {
    	printf("Error occured!\n");
    }
    else
    {
    	printf("Network ordered integer addr:%#lx\n", conv_addr);
    }

    //每个字节最大的值是255，256为无效IP地址
    conv_addr = inet_addr(addr2);
    if (conv_addr == INADDR_NONE)
    {
    	printf("Error occured!\n");
    }
    else
    {
    	printf("Network ordered integer addr:%#lx\n", conv_addr);
    }


    struct sockaddr_in addr_inet;

    if(inet_aton(addr1,&addr_inet.sin_addr))
    {
    	printf("Network ordered integer addr:%#x\n", addr_inet.sin_addr.s_addr);
    }
    else
    {
    	printf("inet_aton error occured\n");
    }


    struct sockaddr_in saddr1,saddr2;
    char*str_ptr;
    char str_arr[20];

    saddr1.sin_addr.s_addr = htonl(0x10203040);
    saddr2.sin_addr.s_addr = htonl(0x10101010);

    str_ptr = inet_ntoa(saddr1.sin_addr);
    strcpy(str_arr,str_ptr);

    printf("notation1:%s\n", str_ptr);

    inet_ntoa(saddr2.sin_addr);
    printf("notation2:%s\n", str_ptr);
    printf("notation3:%s\n", str_arr);



    return 0;
}
