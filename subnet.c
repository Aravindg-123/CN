#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Function to determine class based on first octet */
char get_class(int first_octet) {
    if (first_octet >=1 && first_octet <=126) return 'A';
    else if (first_octet >=128 && first_octet <=191) return 'B';
    else if (first_octet >=192 && first_octet <=223) return 'C';
    else if (first_octet >=224 && first_octet <=239) return 'D';
    else return 'E';
}

/* Function to convert dotted decimal string to uint32_t */
uint32_t dotted_to_uint32(const char *ip) {
    unsigned int a,b,c,d;
    if(sscanf(ip,"%u.%u.%u.%u",&a,&b,&c,&d)!=4) {
        printf("Invalid IP address format\n");
        exit(1);
    }
    return (a<<24)|(b<<16)|(c<<8)|d;
}

/* Function to print uint32_t as dotted decimal */
void print_dotted(uint32_t ip) {
    printf("%u.%u.%u.%u", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF);
}

/* Function to calculate subnet mask for given number of subnets */
uint32_t calculate_subnet_mask(char class, int num_subnets) {
    int default_mask_bits;
    if(class=='A') default_mask_bits=8;
    else if(class=='B') default_mask_bits=16;
    else if(class=='C') default_mask_bits=24;
    else {
        printf("Class D/E not supported for subnetting\n");
        exit(1);
    }
    int subnet_bits = ceil(log2(num_subnets));
    int mask_bits = default_mask_bits + subnet_bits;
    uint32_t mask = 0xFFFFFFFF << (32 - mask_bits);
    return mask;
}

/* Function to calculate network ID */
uint32_t calculate_network_id(uint32_t ip, uint32_t mask) {
    return ip & mask;
}

/* Function to calculate host ID */
uint32_t calculate_host_id(uint32_t ip, uint32_t mask) {
    return ip & ~mask;
}

/* Function to calculate total addresses */
uint32_t total_addresses(uint32_t mask) {
    int host_bits = 32 - __builtin_popcount(mask);
    return (1U << host_bits);
}

int main() {
    char ip_str[50], type[10];
    int num_subnets;

    printf("Enter IP address: ");
    scanf("%s", ip_str);

    printf("Enter IP type (IPv4 or IPv6): ");
    scanf("%s", type);

    if(strcmp(type,"IPv6")==0) {
        printf("IPv6 support not implemented in this program.\n");
        return 0;
    }

    printf("Enter number of subnets: ");
    scanf("%d",&num_subnets);

    uint32_t ip = dotted_to_uint32(ip_str);
    int first_octet = (ip>>24)&0xFF;

    char class = get_class(first_octet);
    printf("Class: %c\n", class);

    uint32_t subnet_mask = calculate_subnet_mask(class, num_subnets);
    printf("Subnet Mask: ");
    print_dotted(subnet_mask);
    printf("\n");

    uint32_t network_id = calculate_network_id(ip, subnet_mask);
    printf("Network ID: ");
    print_dotted(network_id);
    printf("\n");

    uint32_t host_id = calculate_host_id(ip, subnet_mask);
    printf("Host ID: ");
    print_dotted(host_id);
    printf("\n");

    uint32_t total_addr = total_addresses(subnet_mask);
    printf("Total address space: %u\n", total_addr);

    uint32_t first_addr = network_id + 1;
    uint32_t last_addr = network_id + total_addr - 2;
    printf("First usable address: ");
    print_dotted(first_addr);
    printf("\n");
    printf("Last usable address: ");
    print_dotted(last_addr);
    printf("\n");

    uint32_t usable_hosts = total_addr - 2;
    printf("Total usable hosts: %u\n", usable_hosts);

    return 0;
}
