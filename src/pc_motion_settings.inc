/* Windows in-game preferences. Keep the runtime's file and unknown keys intact;
 * the presentation clock and monitor adapter consume these on next launch. */
#include <ctype.h>
extern int recomp_settings_path(uint32_t title_id,char *out,size_t bytes);
extern uint32_t recomp_config_title_id(void);
typedef struct PcMotionSettings { int fps, refresh; } PcMotionSettings;
static int motion_key(const char *line,size_t length,const char *key)
{
    size_t n=strlen(key),i=0;
    while(i<length && (line[i]==' ' || line[i]=='\t'))i++;
    if(length-i<n || memcmp(line+i,key,n))return 0;
    i+=n;while(i<length && (line[i]==' ' || line[i]=='\t'))i++;
    return i<length && line[i]=='=';
}
static char *motion_read_file(const char *path,size_t *size)
{
    FILE *file=fopen(path,"rb");char *text;long n;
    *size=0;
    if(!file) {
        if(GetFileAttributesA(path)!=INVALID_FILE_ATTRIBUTES || GetLastError()!=ERROR_FILE_NOT_FOUND)return NULL;
        return calloc(1,1);
    }
    if(fseek(file,0,SEEK_END) || (n=ftell(file))<0 || n>65536 || fseek(file,0,SEEK_SET)) { fclose(file);return NULL; }
    text=malloc((size_t)n+1);
    if(!text) { fclose(file);return NULL; }
    if(fread(text,1,(size_t)n,file)!=(size_t)n || memchr(text,0,(size_t)n)) { free(text);fclose(file);return NULL; }
    fclose(file);text[n]=0;*size=(size_t)n;return text;
}
static int motion_value(const char *text,const char *key,int fallback)
{
    for(const char *p=text;*p;) {
        const char *end=strchr(p,'\n');size_t n=end?(size_t)(end-p):strlen(p);
        if(motion_key(p,n,key)) fallback=atoi(strchr(p,'=')+1);
        p+=n;if(*p)p++;
    }
    return fallback;
}
static int pc_motion_load(PcMotionSettings *settings)
{
    char path[1024];size_t n;char *text;int target;
    *settings=(PcMotionSettings){0,0};
    if(!recomp_settings_path(recomp_config_title_id(),path,sizeof path) || !(text=motion_read_file(path,&n)))return 0;
    target=motion_value(text,"frame_target",0);
    settings->fps=target==120 || target==144?target:0;
    if(!settings->fps && motion_value(text,"frame_interp",1)==1 && motion_value(text,"frame_cap",0)==60)settings->fps=60;
    target=motion_value(text,"refresh_rate",0);
    settings->refresh=target==120 || target==144?target:0;
    free(text);return 1;
}
static int pc_motion_save(PcMotionSettings settings)
{
    char path[1024],temp[1100],*text,*out;size_t n,used=0;
    HANDLE file;DWORD written;int ok=0,exists;
    if((settings.fps!=0 && settings.fps!=60 && settings.fps!=120 && settings.fps!=144) ||
       (settings.refresh!=0 && settings.refresh!=120 && settings.refresh!=144))return 0;
    if(!recomp_settings_path(recomp_config_title_id(),path,sizeof path) || !(text=motion_read_file(path,&n)))return 0;
    exists=GetFileAttributesA(path)!=INVALID_FILE_ATTRIBUTES;
    if(exists && (GetFileAttributesA(path)&FILE_ATTRIBUTE_READONLY)) { free(text);return 0; }
    out=malloc(n+256);if(!out){free(text);return 0;}
    for(const char *p=text;*p;) {
        const char *end=strchr(p,'\n');size_t length=end?(size_t)(end-p)+1:strlen(p);
        int replace=motion_key(p,length,"refresh_rate");
        if(settings.fps) replace|=motion_key(p,length,"frame_target") || motion_key(p,length,"frame_cap");
        if(settings.fps==60)replace|=motion_key(p,length,"frame_interp");
        if(settings.fps>60)replace|=motion_key(p,length,"fullscreen") || motion_key(p,length,"vrr");
        if(!replace){memcpy(out+used,p,length);used+=length;}p+=length;
    }
    if(used && out[used-1]!='\n')out[used++]='\n';
    used+=sprintf(out+used,"refresh_rate = %d\n",settings.refresh);
    if(settings.fps) used+=sprintf(out+used,"frame_target = %d\nframe_cap = 60\n",settings.fps==60?0:settings.fps);
    if(settings.fps==60)used+=sprintf(out+used,"frame_interp = 1\n");
    if(settings.fps>60)used+=sprintf(out+used,"fullscreen = 1\nvrr = 1\n");
    snprintf(temp,sizeof temp,"%s.motion-%lu.tmp",path,(unsigned long)GetCurrentProcessId());
    file=CreateFileA(temp,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file!=INVALID_HANDLE_VALUE) {
        ok=WriteFile(file,out,(DWORD)used,&written,NULL) && written==used && FlushFileBuffers(file);
        if(!CloseHandle(file))ok=0;
        if(ok)ok=exists?ReplaceFileA(path,temp,NULL,0,NULL,NULL):MoveFileExA(temp,path,MOVEFILE_WRITE_THROUGH);
        if(!ok)DeleteFileA(temp); /* Only the temporary file created above. */
    }
    free(out);free(text);return ok;
}
