#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Convert dotted decimal string to uint32_t */
uint32_t dotted_decimal_to_uint32(const char *ip) {
    uint32_t a,b,c,d;
    if (sscanf(ip,"%u.%u.%u.%u",&a,&b,&c,&d)!=4) {
        fprintf(stderr,"Invalid dotted decimal IP\n");
        exit(1);
    }
    return (a<<24)|(b<<16)|(c<<8)|d;
}

/* Convert binary string to uint32_t */
uint32_t binary_to_uint32(const char *ip) {
    if (strlen(ip)!=32) {
        fprintf(stderr,"Binary IP must have 32 bits\n");
        exit(1);
    }
    uint32_t res = 0;
    for(int i=0;i<32;i++){
        if(ip[i]!='0' && ip[i]!='1'){
            fprintf(stderr,"Invalid binary character\n");
            exit(1);
        }
        res = (res<<1) | (ip[i]-'0');
    }
    return res;
}

/* Convert hex string to uint32_t */
uint32_t hex_to_uint32(const char *ip) {
    uint32_t res;
    if (sscanf(ip,"%x",&res)!=1) {
        fprintf(stderr,"Invalid hexadecimal IP\n");
        exit(1);
    }
    return res;
}

/* Print uint32_t as dotted decimal */
void print_dotted_decimal(uint32_t ip) {
    printf("%u.%u.%u.%u\n",
           (ip>>24)&0xFF,
           (ip>>16)&0xFF,
           (ip>>8)&0xFF,
           ip&0xFF);
}

/* Print uint32_t as binary */
void print_binary(uint32_t ip) {
    for(int i=31;i>=0;i--){
        printf("%c", ((ip>>i)&1)?'1':'0');
        if(i%8==0 && i!=0) printf(" ");
    }
    printf("\n");
}

/* Print uint32_t as hexadecimal */
void print_hex(uint32_t ip){
    printf("0x%08X\n", ip);
}

int main(){
    char input[50], input_format[20], output_format[20];
    printf("Enter IPv4 address: ");
    scanf("%s", input);
    printf("Enter input format (binary/dotted/hex): ");
    scanf("%s", input_format);
    printf("Enter desired output format (binary/dotted/hex): ");
    scanf("%s", output_format);

    uint32_t ip;

    /* Convert input to uint32_t */
    if(strcmp(input_format,"binary")==0) ip = binary_to_uint32(input);
    else if(strcmp(input_format,"dotted")==0) ip = dotted_decimal_to_uint32(input);
    else if(strcmp(input_format,"hex")==0) ip = hex_to_uint32(input);
    else { fprintf(stderr,"Invalid input format\n"); return 1; }

    /* Convert uint32_t to desired output */
    if(strcmp(output_format,"binary")==0) print_binary(ip);
    else if(strcmp(output_format,"dotted")==0) print_dotted_decimal(ip);
    else if(strcmp(output_format,"hex")==0) print_hex(ip);
    else { fprintf(stderr,"Invalid output format\n"); return 1; }

    return 0;
}
