#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/pc_motion_settings.inc"
static const char *config_path;
uint32_t recomp_config_title_id(void) { return 0x45410066; }
int recomp_settings_path(uint32_t title,char *out,size_t n)
{ return snprintf(out,n,"%s",config_path)>0; }
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;} }while(0)
int main(int argc,char **argv)
{
    PcMotionSettings settings;size_t n;char *text;FILE *f;
    CHECK(argc==2);config_path=argv[1];
    f=fopen(config_path,"wb");CHECK(f);
    fputs("# personal comment\r\nframe_target = 120\r\n frame_target=144\nframe_interp = 3\nfuture = retain\nresolution_scale = 4\n",f);fclose(f);
    CHECK(pc_motion_save((PcMotionSettings){120,144}));
    CHECK(pc_motion_load(&settings) && settings.fps==120 && settings.refresh==144);
    text=motion_read_file(config_path,&n);CHECK(text);
    CHECK(strstr(text,"# personal comment\r\n") && strstr(text,"future = retain") && strstr(text,"resolution_scale = 4"));
    CHECK(strstr(text,"frame_interp = 3") && strstr(text,"vrr = 1") && strstr(text,"fullscreen = 1"));
    CHECK(!strstr(text,"frame_target=144"));free(text);
    CHECK(pc_motion_save((PcMotionSettings){0,120}));CHECK(pc_motion_load(&settings) && settings.fps==120 && settings.refresh==120);
    CHECK(pc_motion_save((PcMotionSettings){60,0}));CHECK(pc_motion_load(&settings) && settings.fps==60 && settings.refresh==0);
    text=motion_read_file(config_path,&n);CHECK(text && strstr(text,"frame_interp = 1"));
    CHECK(!pc_motion_save((PcMotionSettings){145,0}));
    CHECK(SetFileAttributesA(config_path,FILE_ATTRIBUTE_READONLY));CHECK(!pc_motion_save((PcMotionSettings){144,144}));
    CHECK(SetFileAttributesA(config_path,FILE_ATTRIBUTE_NORMAL));
    {size_t length;char *after=motion_read_file(config_path,&length);CHECK(after && length==n && !memcmp(text,after,n));free(after);}
    free(text);
    /* A pre-existing temporary file is not ours to remove or overwrite. */
    char temp[1100];snprintf(temp,sizeof temp,"%s.motion-%lu.tmp",config_path,(unsigned long)GetCurrentProcessId());
    f=fopen(temp,"wb");CHECK(f);fputs("owned elsewhere",f);fclose(f);
    CHECK(!pc_motion_save((PcMotionSettings){144,144}));CHECK(GetFileAttributesA(temp)!=INVALID_FILE_ATTRIBUTES);
    CHECK(DeleteFileA(temp));CHECK(pc_motion_load(&settings) && settings.fps==60);
    puts("PASS: performance settings atomic save, duplicate keys, preservation, legacy, validation and write failures");return 0;
}
