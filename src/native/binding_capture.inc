/* Cocoa hardware codes -> SDL physical keys -> shared portable binding codes.
 * Keep capture and gameplay on the same mapping, including OEM punctuation. */
#import <Carbon/Carbon.h>
#include "input_sdl.h"
static int capture_keyboard(unsigned short code) {
    SDL_Scancode scancode;
    switch(code) {
    case kVK_ANSI_A:scancode=SDL_SCANCODE_A;break;
    case kVK_ANSI_B:scancode=SDL_SCANCODE_B;break;
    case kVK_ANSI_C:scancode=SDL_SCANCODE_C;break;
    case kVK_ANSI_D:scancode=SDL_SCANCODE_D;break;
    case kVK_ANSI_E:scancode=SDL_SCANCODE_E;break;
    case kVK_ANSI_F:scancode=SDL_SCANCODE_F;break;
    case kVK_ANSI_G:scancode=SDL_SCANCODE_G;break;
    case kVK_ANSI_H:scancode=SDL_SCANCODE_H;break;
    case kVK_ANSI_I:scancode=SDL_SCANCODE_I;break;
    case kVK_ANSI_J:scancode=SDL_SCANCODE_J;break;
    case kVK_ANSI_K:scancode=SDL_SCANCODE_K;break;
    case kVK_ANSI_L:scancode=SDL_SCANCODE_L;break;
    case kVK_ANSI_M:scancode=SDL_SCANCODE_M;break;
    case kVK_ANSI_N:scancode=SDL_SCANCODE_N;break;
    case kVK_ANSI_O:scancode=SDL_SCANCODE_O;break;
    case kVK_ANSI_P:scancode=SDL_SCANCODE_P;break;
    case kVK_ANSI_Q:scancode=SDL_SCANCODE_Q;break;
    case kVK_ANSI_R:scancode=SDL_SCANCODE_R;break;
    case kVK_ANSI_S:scancode=SDL_SCANCODE_S;break;
    case kVK_ANSI_T:scancode=SDL_SCANCODE_T;break;
    case kVK_ANSI_U:scancode=SDL_SCANCODE_U;break;
    case kVK_ANSI_V:scancode=SDL_SCANCODE_V;break;
    case kVK_ANSI_W:scancode=SDL_SCANCODE_W;break;
    case kVK_ANSI_X:scancode=SDL_SCANCODE_X;break;
    case kVK_ANSI_Y:scancode=SDL_SCANCODE_Y;break;
    case kVK_ANSI_Z:scancode=SDL_SCANCODE_Z;break;
    case kVK_ANSI_0:scancode=SDL_SCANCODE_0;break;
    case kVK_ANSI_1:scancode=SDL_SCANCODE_1;break;
    case kVK_ANSI_2:scancode=SDL_SCANCODE_2;break;
    case kVK_ANSI_3:scancode=SDL_SCANCODE_3;break;
    case kVK_ANSI_4:scancode=SDL_SCANCODE_4;break;
    case kVK_ANSI_5:scancode=SDL_SCANCODE_5;break;
    case kVK_ANSI_6:scancode=SDL_SCANCODE_6;break;
    case kVK_ANSI_7:scancode=SDL_SCANCODE_7;break;
    case kVK_ANSI_8:scancode=SDL_SCANCODE_8;break;
    case kVK_ANSI_9:scancode=SDL_SCANCODE_9;break;
    case kVK_ANSI_Equal:scancode=SDL_SCANCODE_EQUALS;break;
    case kVK_ANSI_Minus:scancode=SDL_SCANCODE_MINUS;break;
    case kVK_ANSI_RightBracket:scancode=SDL_SCANCODE_RIGHTBRACKET;break;
    case kVK_ANSI_LeftBracket:scancode=SDL_SCANCODE_LEFTBRACKET;break;
    case kVK_ANSI_Quote:scancode=SDL_SCANCODE_APOSTROPHE;break;
    case kVK_ANSI_Semicolon:scancode=SDL_SCANCODE_SEMICOLON;break;
    case kVK_ANSI_Backslash:scancode=SDL_SCANCODE_BACKSLASH;break;
    case kVK_ANSI_Comma:scancode=SDL_SCANCODE_COMMA;break;
    case kVK_ANSI_Slash:scancode=SDL_SCANCODE_SLASH;break;
    case kVK_ANSI_Period:scancode=SDL_SCANCODE_PERIOD;break;
    case kVK_ANSI_Grave:scancode=SDL_SCANCODE_GRAVE;break;
    case kVK_ANSI_KeypadDecimal:scancode=SDL_SCANCODE_KP_PERIOD;break;
    case kVK_ANSI_KeypadMultiply:scancode=SDL_SCANCODE_KP_MULTIPLY;break;
    case kVK_ANSI_KeypadPlus:scancode=SDL_SCANCODE_KP_PLUS;break;
    case kVK_ANSI_KeypadClear:scancode=SDL_SCANCODE_NUMLOCKCLEAR;break;
    case kVK_ANSI_KeypadDivide:scancode=SDL_SCANCODE_KP_DIVIDE;break;
    case kVK_ANSI_KeypadEnter:scancode=SDL_SCANCODE_KP_ENTER;break;
    case kVK_ANSI_KeypadMinus:scancode=SDL_SCANCODE_KP_MINUS;break;
    case kVK_ANSI_Keypad0:scancode=SDL_SCANCODE_KP_0;break;
    case kVK_ANSI_Keypad1:scancode=SDL_SCANCODE_KP_1;break;
    case kVK_ANSI_Keypad2:scancode=SDL_SCANCODE_KP_2;break;
    case kVK_ANSI_Keypad3:scancode=SDL_SCANCODE_KP_3;break;
    case kVK_ANSI_Keypad4:scancode=SDL_SCANCODE_KP_4;break;
    case kVK_ANSI_Keypad5:scancode=SDL_SCANCODE_KP_5;break;
    case kVK_ANSI_Keypad6:scancode=SDL_SCANCODE_KP_6;break;
    case kVK_ANSI_Keypad7:scancode=SDL_SCANCODE_KP_7;break;
    case kVK_ANSI_Keypad8:scancode=SDL_SCANCODE_KP_8;break;
    case kVK_ANSI_Keypad9:scancode=SDL_SCANCODE_KP_9;break;
    case kVK_F1:scancode=SDL_SCANCODE_F1;break;
    case kVK_F2:scancode=SDL_SCANCODE_F2;break;
    case kVK_F3:scancode=SDL_SCANCODE_F3;break;
    case kVK_F4:scancode=SDL_SCANCODE_F4;break;
    case kVK_F5:scancode=SDL_SCANCODE_F5;break;
    case kVK_F6:scancode=SDL_SCANCODE_F6;break;
    case kVK_F7:scancode=SDL_SCANCODE_F7;break;
    case kVK_F8:scancode=SDL_SCANCODE_F8;break;
    case kVK_F9:scancode=SDL_SCANCODE_F9;break;
    case kVK_F10:scancode=SDL_SCANCODE_F10;break;
    case kVK_F11:scancode=SDL_SCANCODE_F11;break;
    case kVK_F12:scancode=SDL_SCANCODE_F12;break;
    case kVK_F13:scancode=SDL_SCANCODE_F13;break;
    case kVK_F14:scancode=SDL_SCANCODE_F14;break;
    case kVK_F15:scancode=SDL_SCANCODE_F15;break;
    case kVK_F16:scancode=SDL_SCANCODE_F16;break;
    case kVK_F17:scancode=SDL_SCANCODE_F17;break;
    case kVK_F18:scancode=SDL_SCANCODE_F18;break;
    case kVK_F19:scancode=SDL_SCANCODE_F19;break;
    case kVK_F20:scancode=SDL_SCANCODE_F20;break;
    case kVK_Return:scancode=SDL_SCANCODE_RETURN;break;
    case kVK_Tab:scancode=SDL_SCANCODE_TAB;break;
    case kVK_Space:scancode=SDL_SCANCODE_SPACE;break;
    case kVK_Delete:scancode=SDL_SCANCODE_BACKSPACE;break;
    case kVK_Escape:scancode=SDL_SCANCODE_ESCAPE;break;
    case kVK_Command:scancode=SDL_SCANCODE_LGUI;break;
    case kVK_RightCommand:scancode=SDL_SCANCODE_RGUI;break;
    case kVK_Shift:scancode=SDL_SCANCODE_LSHIFT;break;
    case kVK_RightShift:scancode=SDL_SCANCODE_RSHIFT;break;
    case kVK_Option:scancode=SDL_SCANCODE_LALT;break;
    case kVK_RightOption:scancode=SDL_SCANCODE_RALT;break;
    case kVK_Control:scancode=SDL_SCANCODE_LCTRL;break;
    case kVK_RightControl:scancode=SDL_SCANCODE_RCTRL;break;
    case kVK_CapsLock:scancode=SDL_SCANCODE_CAPSLOCK;break;
    case kVK_Home:scancode=SDL_SCANCODE_HOME;break;
    case kVK_End:scancode=SDL_SCANCODE_END;break;
    case kVK_PageUp:scancode=SDL_SCANCODE_PAGEUP;break;
    case kVK_PageDown:scancode=SDL_SCANCODE_PAGEDOWN;break;
    case kVK_ForwardDelete:scancode=SDL_SCANCODE_DELETE;break;
    case kVK_Help:scancode=SDL_SCANCODE_INSERT;break;
    case kVK_LeftArrow:scancode=SDL_SCANCODE_LEFT;break;
    case kVK_RightArrow:scancode=SDL_SCANCODE_RIGHT;break;
    case kVK_DownArrow:scancode=SDL_SCANCODE_DOWN;break;
    case kVK_UpArrow:scancode=SDL_SCANCODE_UP;break;
    case kVK_ISO_Section:scancode=SDL_SCANCODE_NONUSBACKSLASH;break;
    default:return -1;
    }
    return tsfp_native_virtual_key(scancode);
}
static int capture_mouse(NSInteger button) {
    static const int keys[]={1,2,4,5,6};
    return button>=0 && button<5?keys[button]:-1;
}
static int capture_event(NSEvent *event) {
    if(event.type==NSEventTypeKeyDown)return event.isARepeat?0:capture_keyboard(event.keyCode);
    if(event.type==NSEventTypeFlagsChanged) {
        int key=capture_keyboard(event.keyCode);NSEventModifierFlags mask=0;
        if(key==0xa0||key==0xa1)mask=NSEventModifierFlagShift;
        if(key==0xa2||key==0xa3)mask=NSEventModifierFlagControl;
        if(key==0xa4||key==0xa5)mask=NSEventModifierFlagOption;
        if(key==0x5b||key==0x5c)mask=NSEventModifierFlagCommand;
        if(key==0x14)mask=NSEventModifierFlagCapsLock;
        return !mask?-1:(event.modifierFlags&mask?key:0);
    }
    if(event.type==NSEventTypeScrollWheel)
        return event.scrollingDeltaY>0?0x86:event.scrollingDeltaY<0?0x87:0;
    if(event.type==NSEventTypeLeftMouseDown || event.type==NSEventTypeRightMouseDown || event.type==NSEventTypeOtherMouseDown)
        return capture_mouse(event.buttonNumber);
    return 0;
}
@interface TSFPBindingCapture : NSView
@property(copy) void (^finish)(int);
@property(strong) NSTextField *status;
- (void)capture:(NSEvent *)event;
@end
@implementation TSFPBindingCapture
- (BOOL)acceptsFirstResponder {return YES;}
- (void)drawRect:(NSRect)dirty {
    [[NSColor controlBackgroundColor] setFill];NSRectFill(self.bounds);
    [[NSColor keyboardFocusIndicatorColor] setStroke];NSFrameRectWithWidth(self.bounds,2);
    [@"Press a key, or click / scroll inside this box" drawInRect:NSInsetRect(self.bounds,15,28)
        withAttributes:@{NSFontAttributeName:[NSFont systemFontOfSize:14],NSForegroundColorAttributeName:[NSColor labelColor]}];
}
- (void)capture:(NSEvent *)event {
    int key=capture_event(event);
    if(!key)return;
    if(key==0x1b){if(self.finish)self.finish(-1);return;}
    if(key<0){self.status.stringValue=@"This key or button is not supported. Choose another.";return;}
    if(!pc_settings_key_allowed(key)){self.status.stringValue=@"This shortcut is reserved. Choose another key.";return;}
    if(self.finish)self.finish(key);
}
- (void)keyDown:(NSEvent *)e {[self capture:e];}
- (void)flagsChanged:(NSEvent *)e {[self capture:e];}
- (void)mouseDown:(NSEvent *)e {[self capture:e];}
- (void)rightMouseDown:(NSEvent *)e {[self capture:e];}
- (void)otherMouseDown:(NSEvent *)e {[self capture:e];}
- (void)scrollWheel:(NSEvent *)e {[self capture:e];}
@end
static int capture_binding(NSString *action,void (^testReady)(NSAlert *,TSFPBindingCapture *)) {
    NSAlert *alert=[NSAlert new];alert.messageText=[@"Bind " stringByAppendingString:action];
    alert.informativeText=@"Escape cancels. Use the button below to bind Escape itself.";
    [alert addButtonWithTitle:@"Use Escape"];[alert addButtonWithTitle:@"Unbind"];[alert addButtonWithTitle:@"Cancel"];
    for(NSButton *b in alert.buttons)b.keyEquivalent=@"";
    NSView *accessory=[[NSView alloc] initWithFrame:NSMakeRect(0,0,460,125)];
    TSFPBindingCapture *capture=[[TSFPBindingCapture alloc] initWithFrame:NSMakeRect(0,40,460,85)];
    capture.status=[NSTextField labelWithString:@"Waiting for input…"];capture.status.frame=NSMakeRect(0,0,460,32);
    [accessory addSubview:capture];[accessory addSubview:capture.status];alert.accessoryView=accessory;
    __block int key=-1;
    capture.finish=^(int value){key=value;[NSApp stopModalWithCode:NSAlertFirstButtonReturn+3];};
    alert.window.initialFirstResponder=capture;[alert.window makeFirstResponder:capture];
    NSTimer *testTimer=nil;
    if(testReady){testTimer=[NSTimer timerWithTimeInterval:.05 repeats:NO block:^(NSTimer *timer){testReady(alert,capture);}];
        [[NSRunLoop currentRunLoop] addTimer:testTimer forMode:NSModalPanelRunLoopMode];}
    NSModalResponse response=[alert runModal];
    [testTimer invalidate];
    capture.finish=nil;
    if(response==NSAlertFirstButtonReturn)return 0x1b;
    if(response==NSAlertSecondButtonReturn)return 0;
    if(response==NSAlertThirdButtonReturn)return -1;
    return key;
}

/* Adapter fixtures call the same responder methods as AppKit, without injecting
 * events into another application or editing a personal configuration. */
@interface TSFPCaptureFixture : NSObject
@property NSEventType type;
@property unsigned short keyCode;
@property BOOL isARepeat;
@property NSEventModifierFlags modifierFlags;
@property NSInteger buttonNumber;
@property CGFloat scrollingDeltaY;
@end
@implementation TSFPCaptureFixture
@end
int tsfp_native_capture_test(void) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        TSFPBindingCapture *view=[[TSFPBindingCapture alloc] initWithFrame:NSMakeRect(0,0,460,85)];
        view.status=[NSTextField labelWithString:@""];
        __block int received=-999;view.finish=^(int key){received=key;};
        TSFPCaptureFixture *event=[TSFPCaptureFixture new];event.type=NSEventTypeKeyDown;event.keyCode=kVK_ANSI_Semicolon;
        [view keyDown:(NSEvent *)event];int ok=received==0xba;
        event.keyCode=kVK_ANSI_Keypad7;[view keyDown:(NSEvent *)event];ok &= received==0x67;
        event.keyCode=kVK_F2;received=-999;[view keyDown:(NSEvent *)event];ok &= received==-999 && [view.status.stringValue containsString:@"reserved"];
        event.keyCode=0xffff;[view keyDown:(NSEvent *)event];ok &= received==-999 && [view.status.stringValue containsString:@"not supported"];
        event.keyCode=kVK_ANSI_A;event.isARepeat=YES;[view keyDown:(NSEvent *)event];ok &= received==-999;
        event.isARepeat=NO;event.keyCode=kVK_Escape;[view keyDown:(NSEvent *)event];ok &= received==-1;
        event.type=NSEventTypeFlagsChanged;event.keyCode=kVK_RightShift;event.modifierFlags=NSEventModifierFlagShift;
        [view flagsChanged:(NSEvent *)event];ok &= received==0xa1;
        received=-999;event.modifierFlags=0;[view flagsChanged:(NSEvent *)event];ok &= received==-999;
        event.keyCode=kVK_Command;event.modifierFlags=NSEventModifierFlagCommand;[view flagsChanged:(NSEvent *)event];ok &= received==-999;
        const int mouse[]={1,2,4,5,6};
        for(int b=0;b<5;b++){event.type=b==0?NSEventTypeLeftMouseDown:b==1?NSEventTypeRightMouseDown:NSEventTypeOtherMouseDown;event.buttonNumber=b;
            [view capture:(NSEvent *)event];ok &= received==mouse[b];}
        event.type=NSEventTypeScrollWheel;event.scrollingDeltaY=2;[view scrollWheel:(NSEvent *)event];ok &= received==0x86;
        event.scrollingDeltaY=-2;[view scrollWheel:(NSEvent *)event];ok &= received==0x87;
        received=-999;event.scrollingDeltaY=0;[view scrollWheel:(NSEvent *)event];ok &= received==-999;
        view.finish=nil;
        __block BOOL focusOK=YES;
        int result=capture_binding(@"regression",^(NSAlert *alert,TSFPBindingCapture *box){
            focusOK &= alert.window.firstResponder==box;
            NSEvent *key=[NSEvent keyEventWithType:NSEventTypeKeyDown location:NSZeroPoint modifierFlags:0 timestamp:0
                windowNumber:alert.window.windowNumber context:nil characters:@"\r" charactersIgnoringModifiers:@"\r" isARepeat:NO keyCode:kVK_Return];
            [alert.window sendEvent:key];
        });
        ok &= focusOK && result==0x0d;
        result=capture_binding(@"regression",^(NSAlert *alert,TSFPBindingCapture *box){[alert.buttons[1] performClick:nil];});ok &= result==0;
        result=capture_binding(@"regression",^(NSAlert *alert,TSFPBindingCapture *box){[alert.buttons[0] performClick:nil];});ok &= result==0x1b;
        result=capture_binding(@"regression",^(NSAlert *alert,TSFPBindingCapture *box){[alert.buttons[2] performClick:nil];});ok &= result==-1;
        fprintf(stderr,"[native-capture-test] Cocoa keyboard/modifier/mouse/wheel responders, cancel/reserved/unsupported: %s\n",ok?"PASS":"FAIL");
        return ok;
    }
}
