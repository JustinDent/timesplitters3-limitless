#include "recomp_config.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    RecompSettings settings;
    FILE *file;
    assert(argc==2);
    recomp_settings_defaults(&settings);
    assert(settings.frame_target==0 && settings.frame_interp==1);
    settings.frame_target=144;settings.frame_interp=2;settings.refresh_rate=120;
    assert(recomp_settings_write(argv[1], &settings));
    assert(recomp_settings_read(argv[1], &settings));
    assert(settings.frame_target==144 && settings.frame_interp==2 && settings.refresh_rate==120);
    settings.frame_target=120;
    assert(recomp_settings_write(argv[1], &settings));
    assert(recomp_settings_read(argv[1], &settings) && settings.frame_target==120);
    file=fopen(argv[1],"w");assert(file);
    fputs("frame_target = 145\nframe_interp = 3\n",file);fclose(file);
    assert(recomp_settings_read(argv[1], &settings));
    assert(settings.frame_target==0 && settings.frame_interp==3);
    puts("PASS: target settings persist without changing legacy preferences");
    return 0;
}
