#include <windows.h>
#include "pc_discord.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv)
{
    PcDiscordSnapshot sample={.scene=101};ULONGLONG start;
    if(argc!=2) return 2;
    _putenv_s("TSFP_DISCORD_CONFIG",argv[1]);pc_discord_start();
    start=GetTickCount64();
    while(GetTickCount64()-start<3000) {
        ULONGLONG age=GetTickCount64()-start;
        if(age>=600 && age<1200) { sample.scene=0;sample.flags=0;sample.mode=4;strcpy(sample.mission,"Time to Split");sample.difficulty=2;sample.humans=1; }
        else if(age>=1200 && age<2400) {
            sample.scene=48;sample.flags=0;sample.network=2;sample.role=1;
            strcpy(sample.rules,"Deathmatch");strcpy(sample.map,"Zeppelin");
            sample.humans=age<1800?2:3;sample.bots=4;sample.capacity=16;
        }
        else if(age>=2400) { sample.scene=101;sample.flags=0;sample.network=0;sample.menu_mode=0; }
        pc_discord_submit(&sample);Sleep(20);
    }
    start=GetTickCount64();pc_discord_stop();
    if(GetTickCount64()-start>1500) return 3;
    puts("PASS: Discord worker stopped");return 0;
}
