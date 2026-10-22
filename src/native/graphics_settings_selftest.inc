/* Real modal controls and GPU resolution changes, using only runner paths. */
int tsfp_native_graphics_settings_test(SDL_Window *window) {
    @autoreleasepool {
        const char *path=getenv("TSFP_DISPLAY_CONFIG"),*root=getenv("RECOMP_SAVE_DIR");
        if(!path || !root)return 0;
        NSString *file=[NSString stringWithUTF8String:path];
        NSString *directory=[NSString stringWithUTF8String:root];
        if(![file.stringByDeletingLastPathComponent.stringByResolvingSymlinksInPath
             isEqual:directory.stringByResolvingSymlinksInPath])return 0;
        NSData *original=[NSData dataWithContentsOfFile:file];
        PcDisplayMode before=pc_display_current();int before_fov=pc_display_fov();
        PcDisplayMode choices[]={{0,0},{640,480}},filtered[2];int selected;
        if(pc_display_filter_modes(choices,2,0,choices[1],filtered,2,&selected)!=2 || selected!=1)return 0;
        if(pc_display_filter_modes(choices,2,0,choices[0],filtered,2,&selected)!=2 || selected!=0)return 0;
        __block int ok=pc_display_apply((PcDisplayMode){1000,600});
        if(!ok)return 0;
        graphics_test_ready=^(TSFPSettings *dialog,NSTabView *tabs,NSPopUpButton *aspect,NSPopUpButton *resolution,NSSlider *fov){
            [tabs selectTabViewItemWithIdentifier:@"graphics"];
            ok &= [resolution.titleOfSelectedItem isEqual:@"1000 × 600"] && fov.intValue==before_fov;
            ok &= fov.minValue==PC_FOV_MIN && fov.maxValue==PC_FOV_MAX;
            for(int group=1;group<PC_DISPLAY_ASPECT_COUNT;group++) {
                [aspect selectItemAtIndex:group];[dialog aspectChanged:nil];
                ok &= resolution.numberOfItems>0;
                for(NSMenuItem *item in resolution.itemArray) {
                    PcDisplayMode mode={0};
                    if([item.title hasPrefix:@"Auto"])mode=(PcDisplayMode){1,1};
                    else if(![item.title hasPrefix:@"Original"])
                        ok &= sscanf(item.title.UTF8String,"%u × %u",&mode.width,&mode.height)==2;
                    ok &= pc_display_aspect(mode)==group;
                }
            }
            [dialog defaults:nil];
            ok &= aspect.indexOfSelectedItem==0 && [resolution.titleOfSelectedItem hasPrefix:@"Original"] && fov.intValue==PC_FOV_DEFAULT;
            [aspect selectItemAtIndex:3];[dialog aspectChanged:nil];
            [resolution selectItemWithTitle:@"1280 × 720"];fov.intValue=90;[dialog fovChanged:nil];
            [dialog cancel:nil];
        };
        tsfp_native_settings_show(window);
        ok &= pc_display_current().width==1000 && pc_display_fov()==before_fov;
        NSData *unchanged=[NSData dataWithContentsOfFile:file];
        ok &= original?[original isEqualToData:unchanged]:unchanged==nil;
        graphics_test_ready=^(TSFPSettings *dialog,NSTabView *tabs,NSPopUpButton *aspect,NSPopUpButton *resolution,NSSlider *fov){
            [tabs selectTabViewItemWithIdentifier:@"graphics"];
            [aspect selectItemAtIndex:3];[dialog aspectChanged:nil];
            [resolution selectItemWithTitle:@"1280 × 720"];fov.intValue=90;[dialog fovChanged:nil];
            [dialog save:nil];
        };
        tsfp_native_settings_show(window);
        ok &= pc_display_current().width==1280 && pc_display_current().height==720 && pc_display_fov()==90;
        NSData *saved=[NSData dataWithContentsOfFile:file];
        ok &= [saved isEqualToData:[@"TSFP_DISPLAY_V2 1280 720 90\n" dataUsingEncoding:NSUTF8StringEncoding]];
        struct stat st;
        if(stat(path,&st) || chmod(path,0400))ok=0;
        else {
            char error[512]={0};settings_test_error=error;
            graphics_test_ready=^(TSFPSettings *dialog,NSTabView *tabs,NSPopUpButton *aspect,NSPopUpButton *resolution,NSSlider *fov){
                [tabs selectTabViewItemWithIdentifier:@"graphics"];
                ok &= fov.intValue==90;
                [resolution selectItemWithTitle:@"1920 × 1080"];fov.intValue=100;
                [dialog save:nil];[dialog cancel:nil];
            };
            tsfp_native_settings_show(window);settings_test_error=NULL;
            ok &= strstr(error,"previous resolution and FOV")!=NULL;
            ok &= pc_display_current().width==1280 && pc_display_fov()==90;
            ok &= [saved isEqualToData:[NSData dataWithContentsOfFile:file]];
            ok &= chmod(path,st.st_mode&0777)==0;
        }
        graphics_test_ready=nil;
        ok &= pc_display_apply(before) && pc_display_set_fov(before_fov);
        if(original)ok &= [original writeToFile:file atomically:YES];
        else ok &= unlink(path)==0;
        fprintf(stderr,"[native-graphics-settings-test] aspect groups, custom mode, FOV/defaults, Cancel/Save/reopen and failed-save rollback: %s\n",ok?"PASS":"FAIL");
        return ok;
    }
}
