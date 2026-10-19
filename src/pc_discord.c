/* Optional Windows Discord IPC adapter. No SDK DLL, tokens, or network access.
 * Wire format follows Discord's published discord-rpc implementation:
 * little-endian opcode/length, v1 handshake, READY, SET_ACTIVITY, ping/pong.
 * All pipe I/O stays on a worker, with bounded overlapped operations. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "pc_discord.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef TSFP_DISCORD_INTERVAL
#define TSFP_DISCORD_INTERVAL 15000
#endif
#ifndef TSFP_DISCORD_RETRY
#define TSFP_DISCORD_RETRY 5000
#endif
#define MAX_FRAME 65536u
static SRWLOCK lock=SRWLOCK_INIT;
static HANDLE worker,stop_event;
static volatile LONG active;
static PcDiscordSnapshot latest={.scene=101};
static PcDiscordConfig config;

/* On timeout, cancel and drain the operation before its stack storage dies. */
static int transfer(HANDLE pipe,void *buffer,DWORD bytes,int writing)
{
    DWORD done=0;OVERLAPPED ov={0};BOOL ok;
    ov.hEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!ov.hEvent) return 0;
    ok=writing?WriteFile(pipe,buffer,bytes,&done,&ov):ReadFile(pipe,buffer,bytes,&done,&ov);
    if(!ok && GetLastError()==ERROR_IO_PENDING) {
        HANDLE waits[2]={stop_event,ov.hEvent};
        if(WaitForMultipleObjects(2,waits,FALSE,250)!=WAIT_OBJECT_0+1) {
            CancelIoEx(pipe,&ov);GetOverlappedResult(pipe,&ov,&done,TRUE);ok=FALSE;
        } else ok=GetOverlappedResult(pipe,&ov,&done,FALSE);
    }
    CloseHandle(ov.hEvent);
    return ok && done==bytes;
}

static int send_frame(HANDLE pipe,uint32_t opcode,const char *data,size_t size)
{
    unsigned char frame[4096];uint32_t length=(uint32_t)size;
    if(size>sizeof frame-8) return 0;
    memcpy(frame,&opcode,4);memcpy(frame+4,&length,4);memcpy(frame+8,data,size);
    return transfer(pipe,frame,(DWORD)(size+8),1);
}

/* Match only top-level JSON strings; ignore nested user/profile data. */
static int field_is(const char *json,const char *key,const char *value)
{
    int depth=0;
    for(const char *p=json;*p;p++) {
        if(*p=='{' || *p=='[') depth++;
        else if(*p=='}' || *p==']') depth--;
        else if(*p=='"') {
            const char *start=++p;
            while(*p && *p!='"') {if(*p=='\\' && p[1]) p++;p++;}
            if(!*p) return 0;
            if(depth==1 && (size_t)(p-start)==strlen(key) && !memcmp(start,key,strlen(key))) {
                const char *q=p+1;
                while(*q==' ' || *q=='\t' || *q=='\r' || *q=='\n') q++;
                if(*q++!=':') continue;
                while(*q==' ' || *q=='\t' || *q=='\r' || *q=='\n') q++;
                if(*q++=='"' && !strncmp(q,value,strlen(value)) && q[strlen(value)]=='"') return 1;
            }
        }
    }
    return 0;
}

static HANDLE open_pipe(void)
{
    for(unsigned i=0;i<10;i++) {
        wchar_t name[64];HANDLE pipe;
#ifdef TSFP_DISCORD_TEST_PIPE
        swprintf(name,64,L"\\\\.\\pipe\\tsfp-discord-test");
#else
        swprintf(name,64,L"\\\\.\\pipe\\discord-ipc-%u",i);
#endif
        pipe=CreateFileW(name,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED|SECURITY_SQOS_PRESENT|SECURITY_IDENTIFICATION,NULL);
        if(pipe!=INVALID_HANDLE_VALUE) return pipe;
    }
    return INVALID_HANDLE_VALUE;
}

static DWORD WINAPI run(void *unused)
{
    char *incoming=(char *)malloc(MAX_FRAME+9u),payload[2048];
    unsigned nonce=0;int64_t started=(int64_t)time(NULL);
    int reported_ready=0,reported_error=0,reported_activity=0;
    (void)unused;
    if(!incoming) return 0;
    while(WaitForSingleObject(stop_event,0)==WAIT_TIMEOUT) {
        HANDLE pipe=open_pipe();
        if(pipe!=INVALID_HANDLE_VALUE) {
            int n=snprintf(payload,sizeof payload,"{\"v\":1,\"client_id\":\"%s\"}",config.application_id);
            int connected=send_frame(pipe,0,payload,(size_t)n),ready=0,have_sent=0;
            size_t used=0;uint64_t opened=GetTickCount64(),sent_at=0;
            PcDiscordActivity previous={0};
            while(connected && WaitForSingleObject(stop_event,100)==WAIT_TIMEOUT) {
                DWORD available=0;
                if(!PeekNamedPipe(pipe,NULL,0,NULL,&available,NULL)) break;
                if(available) {
                    DWORD take=available;
                    if(take>MAX_FRAME+8u-used) take=(DWORD)(MAX_FRAME+8u-used);
                    if(!take || !transfer(pipe,incoming+used,take,0)) break;
                    used+=take;
                    incoming[used]=0;
                    while(used>=8) {
                        uint32_t opcode,length;size_t frame;
                        memcpy(&opcode,incoming,4);memcpy(&length,incoming+4,4);
                        if(length>MAX_FRAME) {connected=0;break;}
                        frame=(size_t)length+8;
                        if(used<frame) break;
                        /* Preserve the next frame's first byte while parsing. */
                        char saved=incoming[frame];incoming[frame]=0;
                        if(opcode==2) connected=0;
                        else if(opcode==3) {
                            uint32_t pong=4;
                            memcpy(incoming,&pong,4);
                            connected=transfer(pipe,incoming,(DWORD)frame,1);
                        } else if(opcode==1) {
                            if(field_is(incoming+8,"evt","ERROR")) {
                                if(!reported_error) fprintf(stderr,"[PC-DISCORD] Discord rejected presence; check the Application ID.\n");
                                reported_error=1;
                                connected=0;
                            } else if(field_is(incoming+8,"cmd","DISPATCH") && field_is(incoming+8,"evt","READY")) {
                                ready=1;
                                if(!reported_ready) fprintf(stderr,"[PC-DISCORD] Connected to the Discord desktop client.\n");
                                reported_ready=1;
                            } else if(field_is(incoming+8,"cmd","SET_ACTIVITY") && !reported_activity) {
                                fprintf(stderr,"[PC-DISCORD] Discord accepted Rich Presence.\n");reported_activity=1;
                            }
                        } else if(opcode!=4) connected=0;
                        incoming[frame]=saved;
                        used-=frame;memmove(incoming,incoming+frame,used);
                        if(!connected) break;
                    }
                }
                if(!ready && GetTickCount64()-opened>10000) break;
                if(connected && ready && (!have_sent || GetTickCount64()-sent_at>=TSFP_DISCORD_INTERVAL)) {
                    PcDiscordSnapshot sample;PcDiscordActivity activity;
                    AcquireSRWLockShared(&lock);sample=latest;ReleaseSRWLockShared(&lock);
                    pc_discord_activity(&sample,&activity);
                    if(!have_sent || strcmp(previous.details,activity.details) || strcmp(previous.state,activity.state)) {
                        size_t bytes=pc_discord_payload(payload,sizeof payload,&activity,GetCurrentProcessId(),started,++nonce);
                        connected=bytes && send_frame(pipe,1,payload,bytes);
                        if(connected) {
                            previous=activity;sent_at=GetTickCount64();have_sent=1;
                            fprintf(stderr,"[PC-DISCORD] Activity: %s / %s\n",activity.details,activity.state);
                        }
                    }
                }
            }
            /* Pipe disconnect also clears presence. Attempt an explicit clear
             * on normal shutdown without allowing a slow peer to hold exit. */
            if(ready && WaitForSingleObject(stop_event,0)==WAIT_OBJECT_0) {
                size_t bytes=pc_discord_payload(payload,sizeof payload,NULL,GetCurrentProcessId(),started,++nonce);
                if(bytes) send_frame(pipe,1,payload,bytes);
            }
            CloseHandle(pipe);
        }
        if(WaitForSingleObject(stop_event,TSFP_DISCORD_RETRY)!=WAIT_TIMEOUT) break;
    }
    free(incoming);return 0;
}

void pc_discord_start(void)
{
    char path[32768],text[256];const char *p;FILE *file;size_t bytes;
    if(worker) return;
    p=getenv("TSFP_DISCORD_CONFIG");
    if(p && *p) snprintf(path,sizeof path,"%s",p);
    else {
        char *a,*b,*slash;
        p=getenv("RECOMP_INPUT_CONFIG");snprintf(path,sizeof path,"%s",p&&*p?p:"pc-controls.json");
        a=strrchr(path,'/');b=strrchr(path,'\\');slash=a&&b?(a>b?a:b):a?a:b;
        if(slash) snprintf(slash+1,sizeof path-(size_t)(slash+1-path),"pc-discord.ini");
        else snprintf(path,sizeof path,"pc-discord.ini");
    }
    file=fopen(path,"rb");if(!file) return;
    bytes=fread(text,1,sizeof text-1,file);text[bytes]=0;
    int valid=!ferror(file) && feof(file) && !memchr(text,0,bytes);
    fclose(file);
    if(!valid || !pc_discord_parse_config(text,&config)) {
        fprintf(stderr,"[PC-DISCORD] Invalid settings; presence disabled.\n");return;
    }
    if(!config.enabled) return;
    stop_event=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!stop_event) return;
    worker=CreateThread(NULL,0,run,NULL,0,NULL);
    if(!worker) {CloseHandle(stop_event);stop_event=NULL;return;}
    InterlockedExchange(&active,1);atexit(pc_discord_stop);
    fprintf(stderr,"[PC-DISCORD] Optional Rich Presence enabled.\n");
}

void pc_discord_submit(const PcDiscordSnapshot *sample)
{
    if(InterlockedCompareExchange(&active,0,0) && TryAcquireSRWLockExclusive(&lock)) {
        latest=*sample;ReleaseSRWLockExclusive(&lock);
    }
}

void pc_discord_stop(void)
{
    if(!worker) return;
    InterlockedExchange(&active,0);SetEvent(stop_event);
    WaitForSingleObject(worker,INFINITE);
    CloseHandle(worker);CloseHandle(stop_event);worker=stop_event=NULL;
}
