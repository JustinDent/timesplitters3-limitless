#import <CoreText/CoreText.h>
#include <sys/stat.h>
#include <unistd.h>
/* Text rasterization uses thread-safe CoreText/CG objects, never AppKit views
 * on the rendering thread. Pixels are opaque RGBA for a separate host overlay. */
int tsfp_native_debug_bitmap(const char *text,unsigned char *pixels,int width,int height,int *used_width) {
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGContextRef c=CGBitmapContextCreate(pixels,width,height,8,width*4,space,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(space);if(!c)return 0;
    CGContextSetRGBFillColor(c,.03,.03,.03,1);CGContextFillRect(c,CGRectMake(0,0,width,height));
    CTFontRef font=CTFontCreateWithName(CFSTR("Menlo"),22,NULL);
    CGColorRef white=CGColorCreateGenericRGB(1,1,1,1);
    CFStringRef string=CFStringCreateWithCString(NULL,text,kCFStringEncodingUTF8);
    const void *keys[]={kCTFontAttributeName,kCTForegroundColorAttributeName};const void *values[]={font,white};
    CFDictionaryRef attrs=CFDictionaryCreate(NULL,keys,values,2,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef attributed=CFAttributedStringCreate(NULL,string,attrs);
    CTLineRef line=CTLineCreateWithAttributedString(attributed);
    double measured=CTLineGetTypographicBounds(line,NULL,NULL,NULL);
    if(measured>width-16) {
        CFAttributedStringRef dots=CFAttributedStringCreate(NULL,CFSTR("…"),attrs);
        CTLineRef token=CTLineCreateWithAttributedString(dots);
        CTLineRef truncated=CTLineCreateTruncatedLine(line,width-16,kCTLineTruncationEnd,token);
        if(truncated){CFRelease(line);line=truncated;}
        CFRelease(token);CFRelease(dots);
        measured=CTLineGetTypographicBounds(line,NULL,NULL,NULL);
    }
    if(used_width)*used_width=(int)fmin(width,ceil(measured)+16);
    CGContextSetTextPosition(c,8,12);CTLineDraw(line,c);
    CFRelease(line);CFRelease(attributed);CFRelease(attrs);CFRelease(string);CGColorRelease(white);CFRelease(font);CGContextRelease(c);
    return 1;
}
@interface TSFPDebug : NSObject <NSWindowDelegate> {
    NSWindow *panel;NSButton *checks[PC_DEBUG_OPTION_COUNT];NSTextField *stats;NSTextView *recent;
}
- (void)show;
- (void)refresh;
- (void)save:(id)sender;
- (void)cancel:(id)sender;
- (void)clear:(id)sender;
- (void)logs:(id)sender;
@end
static TSFPDebug *debug_dialog;
static NSTimer *debug_poll_timer;
static void (^debug_test_ready)(TSFPDebug *,NSButton * __strong *);
@implementation TSFPDebug
- (void)refresh {
    if(!panel)return;
    PcDebugSnapshot value;pc_debug_snapshot(&value);
    stats.stringValue=[NSString stringWithFormat:@"Game: %.1f fps    Shown: %.1f fps    Interpolated: %.1f/s\nGuest interval: %.2f ms    Slow (>50 ms): %lu    Errors: %lu    Warnings: %lu",value.fps,value.shown_fps,fmax(0,value.shown_fps-value.fps),value.frame_ms,value.slow_frames,value.errors,value.warnings];
    NSString *text=[NSString stringWithUTF8String:value.recent];
    if(!text)text=@"Log contains text that could not be decoded.";
    if(!value.log_available)text=@"Log unavailable. Start with the native launcher or isolated runner.";
    if(![recent.string isEqualToString:text])recent.string=text;
}
- (void)show {
    if(panel){[panel makeKeyAndOrderFront:nil];return;}
    tsfp_native_settings_release_input(game_window);
    int height=500+((int)pc_debug_option_count(0)-4)*30;
    panel=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,740,height) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:NO];
    panel.title=@"TimeSplitters — Debug & Performance";panel.delegate=self;panel.releasedWhenClosed=NO;
    NSView *v=panel.contentView;PcDebugOptions options;pc_debug_get(&options);
    unsigned row=0;
    for(unsigned i=0;i<PC_DEBUG_OPTION_COUNT;i++) {
        checks[i]=nil;
        /* The native port has no network-latency provider yet. Preserve its
         * saved value, and expose it automatically when that capability exists. */
        if(!pc_debug_option_available(i,0))continue;
        NSString *title=[NSString stringWithUTF8String:pc_debug_option(i)->label];
        checks[i]=[NSButton checkboxWithTitle:title target:nil action:NULL];
        checks[i].frame=NSMakeRect(20,height-40-(int)row++*30,690,25);
        checks[i].state=pc_debug_option_value(&options,i);[v addSubview:checks[i]];
    }
    stats=label(v,@"",NSMakeRect(20,302,700,65));
    label(v,@"Shown counts successful host presents, including interpolation, not display scanout.\nLog matches flag possible issues; they do not detect every visual or gameplay problem.",NSMakeRect(20,244,700,48));
    NSScrollView *scroll=[[NSScrollView alloc] initWithFrame:NSMakeRect(20,90,700,150)];scroll.hasVerticalScroller=YES;
    recent=[[NSTextView alloc] initWithFrame:scroll.bounds];recent.editable=NO;recent.font=[NSFont monospacedSystemFontOfSize:11 weight:NSFontWeightRegular];scroll.documentView=recent;[v addSubview:scroll];
    NSString *buttons[]={@"Open logs folder",@"Clear counters",@"Cancel",@"Save & Close"};SEL actions[]={@selector(logs:),@selector(clear:),@selector(cancel:),@selector(save:)};
    NSRect frames[]={NSMakeRect(20,35,150,32),NSMakeRect(175,35,135,32),NSMakeRect(495,35,100,32),NSMakeRect(600,35,120,32)};
    for(int i=0;i<4;i++){NSButton *b=[NSButton buttonWithTitle:buttons[i] target:self action:actions[i]];b.frame=frames[i];[v addSubview:b];}
    [self refresh];[panel center];[panel makeKeyAndOrderFront:nil];
    NSTimer *testTimer=nil;
    if(debug_test_ready){testTimer=[NSTimer timerWithTimeInterval:.05 repeats:NO block:^(NSTimer *timer){debug_test_ready(self,checks);}];
        [[NSRunLoop mainRunLoop] addTimer:testTimer forMode:NSModalPanelRunLoopMode];}
    [NSApp runModalForWindow:panel];[testTimer invalidate];[panel orderOut:nil];panel=nil;
    if(game_window)SDL_RaiseWindow(game_window);tsfp_native_settings_release_input(game_window);
}
- (void)save:(id)sender {
    PcDebugOptions options;pc_debug_get(&options);
    for(unsigned i=0;i<PC_DEBUG_OPTION_COUNT;i++)
        if(checks[i])pc_debug_option_set(&options,i,checks[i].state==NSControlStateValueOn);
    char error[512];if(!pc_debug_config_save(&options,error,sizeof error)){alert_text(error);return;}[NSApp stopModal];
}
- (void)cancel:(id)sender {[NSApp stopModal];}
- (BOOL)windowShouldClose:(NSWindow *)sender {[NSApp stopModal];return NO;}
- (void)clear:(id)sender {pc_debug_clear();[self refresh];}
- (void)logs:(id)sender {if(!pc_debug_open_logs(NULL))alert_text("The log folder is unavailable.");}
@end
void pc_debug_show(HWND owner) {if(!debug_dialog)debug_dialog=[TSFPDebug new];[debug_dialog show];}
int pc_debug_open_logs(HWND owner) {
    const char *path=getenv("RECOMP_LOG_FILE");if(!path || !*path)return 0;
    NSString *file=[NSString stringWithUTF8String:path];
    return file && [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:file.stringByDeletingLastPathComponent isDirectory:YES]];
}
static void native_debug_attach(void) {
    if(debug_poll_timer)return;
    pc_debug_init((HWND)game_window);
    debug_poll_timer=[NSTimer timerWithTimeInterval:.5 repeats:YES block:^(NSTimer *timer){pc_debug_poll();[debug_dialog refresh];}];
    [[NSRunLoop mainRunLoop] addTimer:debug_poll_timer forMode:NSRunLoopCommonModes];
    [[NSRunLoop mainRunLoop] addTimer:debug_poll_timer forMode:NSModalPanelRunLoopMode];
}

int tsfp_native_debug_test(void) {
    @autoreleasepool {
        const char *root=getenv("RECOMP_SAVE_DIR");if(!root)return 1;
        char path[4096],moved[4096],error[512],overlay[320];
        if(snprintf(path,sizeof path,"%s/debug-test.err",root)>=sizeof path ||
           snprintf(moved,sizeof moved,"%s/debug-test-old.err",root)>=sizeof moved)return 1;
        FILE *f=fopen(path,"w");if(!f)return 1;
        fputs("[INFO] startup\n[WARN] unsupported fixture\n[ERROR] failed fixture\n[PC-DEBUG] warning counters\n",f);fclose(f);
        setenv("RECOMP_LOG_FILE",path,1);[NSApplication sharedApplication];pc_debug_init(NULL);
        PcDebugSnapshot snap;PcDebugOptions before={0},got,on={1,1,1,0,1};pc_debug_set(&before);
        pc_debug_snapshot(&snap);int ok=snap.log_available && snap.errors==1 && snap.warnings==1;
        pc_debug_poll();pc_debug_snapshot(&snap);ok &= snap.errors==1 && snap.warnings==1;
        f=fopen(path,"a");if(!f)return 1;fputs("fatal",f);fclose(f);pc_debug_poll();pc_debug_snapshot(&snap);ok &= snap.errors==1;
        f=fopen(path,"a");if(!f)return 1;fputs(" fixture\n",f);fclose(f);pc_debug_poll();pc_debug_snapshot(&snap);ok &= snap.errors==2;
        pc_debug_set(&on);pc_debug_sample(60,120);pc_debug_snapshot(&snap);
        ok &= fabs(snap.frame_ms-1000.0/60)<.001 && snap.fps==60 && snap.shown_fps==120;
        pc_debug_sample(NAN,10);pc_debug_snapshot(&snap);ok &= snap.fps==60;
        ok &= pc_debug_overlay(overlay,sizeof overlay,NULL) && strstr(overlay,"60.0/120.0") && strstr(overlay,"E:2 W:1");
        pc_debug_toggle_fps();pc_debug_get(&got);ok &= !got.show_fps && got.show_issues;pc_debug_set(&on);
        pc_debug_clear();pc_debug_poll();pc_debug_snapshot(&snap);ok &= !snap.errors && !snap.warnings;
        f=fopen(path,"w");if(!f)return 1;fputs("[WARN] rotated\n",f);fclose(f);pc_debug_poll();pc_debug_snapshot(&snap);ok &= snap.warnings==1;
        ok &= rename(path,moved)==0;f=fopen(path,"w");if(!f)return 1;fputs("[NATIVE-FAULT] fixture\n",f);fclose(f);
        pc_debug_poll();pc_debug_snapshot(&snap);ok &= snap.errors==1 && snap.warnings==1;
        /* Dialog Cancel leaves active values intact; Save persists only debug. */
        debug_test_ready=^(TSFPDebug *dialog,NSButton * __strong *checks){for(unsigned i=0;i<PC_DEBUG_OPTION_COUNT;i++)checks[i].state=NSControlStateValueOff;[dialog cancel:nil];};
        pc_debug_show(NULL);pc_debug_get(&got);ok &= !memcmp(&got,&on,sizeof got);
        __block int definitions_match=1;
        debug_test_ready=^(TSFPDebug *dialog,NSButton * __strong *checks){
            for(unsigned i=0;i<PC_DEBUG_OPTION_COUNT;i++) {
                definitions_match &= (checks[i]!=nil)==pc_debug_option_available(i,0);
                if(checks[i])definitions_match &= [checks[i].title isEqualToString:[NSString stringWithUTF8String:pc_debug_option(i)->label]];
                checks[i].state=NSControlStateValueOff;
            }
            checks[PC_DEBUG_OPTION_FPS].state=NSControlStateValueOn;[dialog save:nil];};
        pc_debug_show(NULL);debug_test_ready=nil;
        ok &= definitions_match;
        pc_debug_config_read(&got);ok &= got.show_fps && !got.show_frame_time && !got.show_issues && !got.log_performance && got.show_latency;
        const char *config=getenv("RECOMP_INPUT_CONFIG");struct stat st;
        if(!config || stat(config,&st) || chmod(config,0400))ok=0;
        else {ok &= !pc_debug_config_save(&on,error,sizeof error);if(chmod(config,st.st_mode&0777))ok=0;}
        pc_debug_get(&got);ok &= got.show_fps && !got.show_frame_time;
        pc_debug_shutdown();unlink(moved);
        fprintf(stderr,"[native-debug-test] shared stats/log rotation, Cocoa save/cancel, config preservation/failure: %s\n",ok?"PASS":"FAIL");
        return ok?0:1;
    }
}
