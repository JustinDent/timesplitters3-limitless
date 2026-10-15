#include "texture_pack_format.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"failed line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void put(unsigned char *p,uint32_t x) {p[0]=(unsigned char)x;p[1]=(unsigned char)(x>>8);p[2]=(unsigned char)(x>>16);p[3]=(unsigned char)(x>>24);}
int main(int argc, char **argv) {
    unsigned char p[176]={0},data[8]={0,1,2,3,4,5,6,7};TsfpDDS d;
    put(p,0x20534444);put(p+4,124);put(p+12,4);put(p+16,4);put(p+28,3);
    put(p+76,32);put(p+80,4);put(p+84,0x35545844);
    CHECK(tsfp_dds_parse(p,sizeof p,&d));CHECK(d.bytes==48 && d.levels==3 && d.size[2]==16);
    CHECK(!tsfp_dds_parse(p,175,&d));CHECK(!tsfp_dds_parse(p,127,&d));
    put(p+112,0x200);CHECK(!tsfp_dds_parse(p,sizeof p,&d));put(p+112,0);
    put(p+28,1);CHECK(!tsfp_dds_parse(p,sizeof p,&d));put(p+28,3);
    put(p+16,8192);CHECK(!tsfp_dds_parse(p,sizeof p,&d));put(p+16,3);
    CHECK(!tsfp_dds_parse(p,sizeof p,&d));put(p+16,4);
    put(p+84,0x30315844);CHECK(!tsfp_dds_parse(p,sizeof p,&d));
    CHECK(tsfp_texture_key(12,4,4,data,8)==UINT64_C(0x1fc92492b76218f1));
    puts("PASS: DDS truncation, dimensions, mip chain, cube/format rejection, content identity");
    for(int i=1;i<argc;i++) {
        FILE *f=fopen(argv[i],"rb");long n;unsigned char *data;
        CHECK(f!=NULL);CHECK(fseek(f,0,SEEK_END)==0);n=ftell(f);
        CHECK(n>=128 && n<=24*1024*1024);rewind(f);data=(unsigned char *)malloc(n);CHECK(data!=NULL);
        CHECK(fread(data,1,n,f)==(size_t)n);fclose(f);
        CHECK(tsfp_dds_parse(data,n,&d));free(data);
        printf("PASS: %s %ux%u %u mips %u bytes\n",argv[i],d.width,d.height,d.levels,d.bytes);
    }
    return 0;
}
