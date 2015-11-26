#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/ioctl.h>
#include <errno.h>
#define HCIGETCONNINFO _IOR('H', 213, int) 


int main(int argc, char **argv)
{
    printf("Program Start\n");
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    bdaddr_t bdaddr;
    dev_id = hci_get_route(NULL);
    printf("Device ID : %d\n", dev_id);
    sock = hci_open_dev( dev_id );
    printf("Socket ID : %d\n", sock);
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), 
            name, 0) < 0)
        strcpy(name, "[unknown]");
        printf("%s  %s\n", addr, name);
    
        int8_t rssi;                                                  
        struct hci_conn_info_req *cr;                                    
        cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));         
        if(!cr){                                                      
        perror("Can't allocate memory");                             
        exit(1);                                                     
        } 
        str2ba(addr, &bdaddr);
        bacpy(&cr->bdaddr, &bdaddr);
        ba2str(&cr->bdaddr, addr);
        printf("debug: %s\n", addr); 
	cr->type = ACL_LINK;
	int errorcode = ioctl(sock, HCIGETCONNINFO, (unsigned long) cr);
	if (errorcode < 0) {
		printf("Error code: %d\n", errorcode);
                printf("Error no is: %d\n", errno);
		printf("Error description is : %s\n", strerror(errno));
                perror("Get connection info failed");
		exit(1);
	}                                                               
        int return_rssi = hci_read_rssi(sock, htobs(cr->conn_info->handle), &rssi, 1000);  
        printf("RSSI value: %d\n", rssi);
        printf("RSSI return: %d\n", return_rssi);   
        free(cr);
    }

       
    free( ii );
    close( sock );
    return 0;
}
