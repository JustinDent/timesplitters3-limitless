using System;
using System.Collections;
using System.IO;
using System.Diagnostics;
using System.Linq;
using TimeSplitters.Launcher;

public static class LauncherSettingsChecks
{
    static int count;
    static string temp;
    static void Require(bool condition) { if (!condition) throw new Exception("Settings assertion failed."); }
    static void Reject(Action action) { bool rejected=false; try { action(); } catch { rejected=true; } Require(rejected); }
    static string Root(string name) { string root=Path.Combine(temp,name);Directory.CreateDirectory(root);return root; }
    static void Put(string root,string file,string data) { string path=Path.Combine(root,file);Directory.CreateDirectory(Path.GetDirectoryName(path));File.WriteAllText(path,data); }
    static string Read(string root,string file) { return File.ReadAllText(Path.Combine(root,file)); }
    static void Test(string name,Action action)
    {
        Console.WriteLine("START "+name);
        var elapsed=Stopwatch.StartNew();
        try { action(); }
        catch(Exception failure) { throw new Exception("Launcher settings check failed: "+name,failure); }
        count++;Console.WriteLine("PASS "+name+" ("+elapsed.ElapsedMilliseconds+" ms)");
    }
    static void STA(Action action)
    {
        Exception failure=null;
        var thread=new System.Threading.Thread(()=> { try {action();} catch(Exception ex){failure=ex;} });
        thread.IsBackground=true;thread.SetApartmentState(System.Threading.ApartmentState.STA);thread.Start();
        // The first WPF control initializes fonts, themes and the graphics
        // stack. Cold hosted Windows runners can take more than ten seconds.
        // Keep the wait bounded, but distinguish startup timeouts from failed
        // assertions and preserve the original UI-thread stack for diagnosis.
        if(!thread.Join(60000)) throw new TimeoutException("Launcher settings UI thread did not finish within 60 seconds.");
        if(failure!=null) System.Runtime.ExceptionServices.ExceptionDispatchInfo.Capture(failure).Throw();
    }
    static void Click(System.Windows.Controls.Button button)
    { button.RaiseEvent(new System.Windows.RoutedEventArgs(System.Windows.Controls.Button.ClickEvent)); }
    static System.Collections.Generic.IEnumerable<System.Windows.DependencyObject> Descendants(System.Windows.DependencyObject parent)
    {
        foreach(var child in System.Windows.LogicalTreeHelper.GetChildren(parent).OfType<System.Windows.DependencyObject>())
        { yield return child; foreach(var descendant in Descendants(child)) yield return descendant; }
    }
    public static int Child()
    {
        string controls=Environment.GetEnvironmentVariable("RECOMP_INPUT_CONFIG");
        string display=Environment.GetEnvironmentVariable("TSFP_DISPLAY_CONFIG");
        string runtime=Environment.GetEnvironmentVariable("RECOMP_DISPLAY_CONFIG");
        var mouse=(System.Collections.Generic.Dictionary<string,object>)Json.Object(File.ReadAllText(controls))["mouse"];
        Console.WriteLine(Convert.ToString(mouse["sensitivity"],System.Globalization.CultureInfo.InvariantCulture));
        Console.WriteLine(File.ReadAllText(display).Trim());
        Console.WriteLine(GameSettings.ParseRuntime(File.ReadAllText(runtime))["anisotropy"]);
        Console.WriteLine("XMV_PLAY="+Environment.GetEnvironmentVariable("RECOMP_XMV_PLAY"));
        return 0;
    }
    public static int Run()
    {
        temp=Path.Combine(Path.GetDirectoryName(typeof(LauncherSettingsChecks).Assembly.Location),"settings-tests-"+Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(temp);
        try
        {
            Test("published alpha.4 retains V1 resolution settings and rejects custom FOV atomically",()=>{
                string root=Root("legacy-fov");Put(root,"BUILD.json","{\"version\":\"v0.2.0-alpha.4\"}");
                var model=GameSettings.Load(root,null);Require(!model.CanAdjustFov);
                model.Width=1920;model.Height=1080;model.Save();
                string original=Read(root,"Settings/pc-display.ini");
                Require(original=="TSFP_DISPLAY_V1 1920 1080\n");
                model.VerticalFov=75;Reject(()=>model.Save());Require(Read(root,"Settings/pc-display.ini")==original);
                Put(root,"BUILD.json","{\"version\":\"v0.2.0-alpha.5\"}");
                Require(model.CanAdjustFov);model.Save();Require(Read(root,"Settings/pc-display.ini")=="TSFP_DISPLAY_V2 1920 1080 75\n");
            });
            Test("alpha.4 FOV controls explain unavailable capability while resolution saves",()=>STA(()=>{
                string root=Root("legacy-fov-ui");Put(root,"BUILD.json","{\"version\":\"v0.2.0-alpha.4\"}");
                var owner=new System.Windows.Window();var editor=new SettingsPage(owner.Resources,root,true);editor.SelectTab(1);
                var nodes=Descendants(editor.View).ToList();
                Require(!nodes.OfType<System.Windows.Controls.Slider>().Single(c=>c.Name=="VerticalFovSlider").IsEnabled);
                Require(!nodes.OfType<System.Windows.Controls.Button>().Single(c=>c.Name=="ResetFovButton").IsEnabled);
                Require(nodes.OfType<System.Windows.Controls.TextBlock>().Any(c=>c.Text.Contains("newer than v0.2.0-alpha.4")));
                Click(nodes.OfType<System.Windows.Controls.Button>().Single(c=>c.Name=="SettingsSaveButton"));
                Require(Read(root,"Settings/pc-display.ini")=="TSFP_DISPLAY_V1 0 0\n");owner.Close();
            }));
            Test("FOV legacy defaults, range checks and shared V2 round trip",()=>{
                string root=Root("fov-settings");Put(root,"Settings/pc-display.ini","TSFP_DISPLAY_V1 1920 1080\n");
                var model=GameSettings.Load(root,null);Require(model.VerticalFov==55);
                model.VerticalFov=75;model.Save();
                Require(Read(root,"Settings/pc-display.ini")=="TSFP_DISPLAY_V2 1920 1080 75\n");
                var loaded=GameSettings.Load(root,null);Require(loaded.Width==1920 && loaded.Height==1080 && loaded.VerticalFov==75);
                loaded.VerticalFov=101;Reject(()=>loaded.Save());loaded.VerticalFov=39;Reject(()=>loaded.Save());
                Require(GameSettings.Load(root,null).VerticalFov==75);
                loaded.VerticalFov=55;loaded.Save();Require(Read(root,"Settings/pc-display.ini")=="TSFP_DISPLAY_V1 1920 1080\n");
                foreach(string invalid in new[]{"TSFP_DISPLAY_V2 1920 1080", "TSFP_DISPLAY_V1 1920 1080 75", "TSFP_DISPLAY_V2 1920 1080 999", "TSFP_DISPLAY_V2 1920 1080 NaN"}) {
                    Put(root,"Settings/pc-display.ini",invalid);Reject(()=>GameSettings.Load(root,null));
                }
            });
            Test("FOV slider number, reset, cancel and save",()=>STA(()=>{
                string root=Root("fov-slider");var owner=new System.Windows.Window();
                var editor=new SettingsPage(owner.Resources,root,true);editor.SelectTab(1);
                var nodes=Descendants(editor.View).ToList();
                var slider=nodes.OfType<System.Windows.Controls.Slider>().Single(c=>c.Name=="VerticalFovSlider");
                var value=nodes.OfType<System.Windows.Controls.TextBlock>().Single(c=>c.Name=="VerticalFovValue");
                Require(slider.Value==55 && slider.Minimum==40 && slider.Maximum==100 && slider.IsSnapToTickEnabled);
                slider.Value=80;Require(value.Text=="80°");
                bool cancelled=false;editor.Cancelled+=()=>cancelled=true;
                Click(nodes.OfType<System.Windows.Controls.Button>().Single(c=>c.Name=="SettingsCancelButton"));
                Require(cancelled && !File.Exists(Path.Combine(root,"Settings/pc-display.ini")));
                Click(nodes.OfType<System.Windows.Controls.Button>().Single(c=>c.Name=="ResetFovButton"));Require(slider.Value==55);
                slider.Value=75;Click(nodes.OfType<System.Windows.Controls.Button>().Single(c=>c.Name=="SettingsSaveButton"));
                Require(GameSettings.Load(root,null).VerticalFov==75);owner.Close();
            }));
            Test("primary mode list deduplicates, sorts, includes default and preserves saved custom",()=>{
                var modes=DisplayModes.Choices(new[]{new DisplayMode(1920,1080),new DisplayMode(640,480),new DisplayMode(1920,1080),new DisplayMode(3840,2160),new DisplayMode(1080,1920),new DisplayMode(0,720),new DisplayMode(7680,4320)},3440,1440);
                Require(modes.Count==5 && modes[0].Width==0 && modes[0].Height==0);
                Require(modes[1].Width==640 && modes[2].Width==1920 && modes[3].Width==3840);
                Require(modes[4].Width==3440 && modes[4].SavedCustom);
                var empty=DisplayModes.Choices(new DisplayMode[0],0,0);Require(empty.Count==1);
                Require(DisplayModes.Choices(new DisplayMode[0],0,1080).Count==1);
                var actual=DisplayModes.Primary(0,0);Require(actual.Count>=1 && actual[0].Width==0);
                Console.WriteLine("Primary display modes: "+String.Join(", ",actual));
            });
            Test("lower supported modes and original resolution persist without changing other settings",()=>{
                string root=Root("lower-resolution");var model=GameSettings.Load(root,null);model.Width=800;model.Height=600;model.Save();
                var loaded=GameSettings.Load(root,null);Require(loaded.Width==800 && loaded.Height==600);
                loaded.Width=0;loaded.Height=0;loaded.Save();Require(Read(root,"Settings/pc-display.ini")=="TSFP_DISPLAY_V1 0 0\n");
            });
            Test("launcher resolution arrows select both directions, wrap and leave files untouched",()=>{
                Exception failure=null;string root=Root("arrow-controls");
                var thread=new System.Threading.Thread(()=>{try {
                    var owner=new System.Windows.Window();var editor=new SettingsPage(owner.Resources,root,true);owner.Content=editor.View;
                    var tabs=Descendants(editor.View).OfType<System.Windows.Controls.TabControl>().Single();tabs.SelectedIndex=1;
                    var nodes=Descendants(editor.View).ToList();
                    var startup=nodes.OfType<System.Windows.Controls.CheckBox>().Single(c=>Convert.ToString(c.Content)=="Skip startup splash screens");
                    Require(startup.IsChecked==false);startup.IsChecked=true;
                    var picker=nodes.OfType<System.Windows.Controls.ComboBox>().Single(c=>c.SelectedItem is DisplayMode);
                    var buttons=nodes.OfType<System.Windows.Controls.Button>().ToList();
                    var previous=buttons.Single(b=>System.Windows.Automation.AutomationProperties.GetName(b)=="Previous resolution");
                    var next=buttons.Single(b=>System.Windows.Automation.AutomationProperties.GetName(b)=="Next resolution");
                    Require(picker.SelectedIndex==0);
                    previous.RaiseEvent(new System.Windows.RoutedEventArgs(System.Windows.Controls.Button.ClickEvent));Require(picker.SelectedIndex==picker.Items.Count-1);
                    next.RaiseEvent(new System.Windows.RoutedEventArgs(System.Windows.Controls.Button.ClickEvent));Require(picker.SelectedIndex==0);
                    next.RaiseEvent(new System.Windows.RoutedEventArgs(System.Windows.Controls.Button.ClickEvent));Require(picker.SelectedIndex==1%picker.Items.Count);
                    previous.RaiseEvent(new System.Windows.RoutedEventArgs(System.Windows.Controls.Button.ClickEvent));Require(picker.SelectedIndex==0);
                    owner.Close();
                } catch(Exception ex) {failure=ex;} });
                thread.SetApartmentState(System.Threading.ApartmentState.STA);thread.Start();thread.Join();
                if(failure!=null) throw failure;
                Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==0);
            });
            Test("settings stay in the main window, preserve the sidebar and retain drafts across navigation",()=>{
                string root=Root("inline-navigation");STA(()=>{
                    var ui=new LauncherWindow(root,true);
                    try {
                        int windows=System.Windows.Application.Current==null?0:System.Windows.Application.Current.Windows.Count;
                        var sidebar=(System.Windows.Controls.Border)ui.Window.FindName("SidebarPanel");
                        var host=(System.Windows.Controls.ContentControl)ui.Window.FindName("SettingsHost");
                        var home=(System.Windows.Controls.Grid)ui.Window.FindName("CommandCenter");
                        Click((System.Windows.Controls.Button)ui.Window.FindName("SettingsNav"));
                        Require(host.Visibility==System.Windows.Visibility.Visible && home.Visibility==System.Windows.Visibility.Collapsed && sidebar.Visibility==System.Windows.Visibility.Visible && sidebar.IsEnabled);
                        Require(System.Windows.Window.GetWindow((System.Windows.DependencyObject)host.Content)==ui.Window);
                        var shell=(System.Windows.Controls.Panel)ui.Window.Content;
                        foreach(var size in new[]{new System.Windows.Size(1000,720),new System.Windows.Size(1152,772)}) {
                            shell.Measure(size);shell.Arrange(new System.Windows.Rect(new System.Windows.Point(),size));shell.UpdateLayout();
                            Require(sidebar.ActualWidth==204 && host.ActualWidth==size.Width-204);
                            Require(((System.Windows.FrameworkElement)host.Content).ActualWidth>600);
                        }
                        var input=Descendants(host).OfType<System.Windows.Controls.TextBox>().Single(t=>t.Name=="MouseSensitivity");input.Text="0.42";
                        var draft=host.Content;
                        Click((System.Windows.Controls.Button)ui.Window.FindName("HomeNav"));Require(home.Visibility==System.Windows.Visibility.Visible && host.Visibility==System.Windows.Visibility.Collapsed);
                        Click((System.Windows.Controls.Button)ui.Window.FindName("SettingsNav"));Require(ReferenceEquals(host.Content,draft) && input.Text=="0.42");
                        Require(System.Windows.Application.Current==null || System.Windows.Application.Current.Windows.Count==windows);
                        Click(Descendants(host).OfType<System.Windows.Controls.Button>().Single(b=>b.Name=="SettingsCancelButton"));
                        Require(host.Content==null && home.Visibility==System.Windows.Visibility.Visible);
                        Click((System.Windows.Controls.Button)ui.Window.FindName("SettingsNav"));
                        Require(Descendants(host).OfType<System.Windows.Controls.TextBox>().Single(t=>t.Name=="MouseSensitivity").Text=="0.1");
                    } finally {ui.Window.Close();}
                });Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==0);
            });
            Test("inline binding capture, conflict cancellation and reassignment save without another window",()=>{
                string root=Root("inline-bindings");STA(()=>{
                    var ui=new LauncherWindow(root,true);
                    try {
                        ui.ShowSettings();var host=(System.Windows.Controls.ContentControl)ui.Window.FindName("SettingsHost");
                        Func<string,System.Windows.Controls.Button> button=label=>Descendants(host).OfType<System.Windows.Controls.Button>().Single(b=>b.IsEnabled && Convert.ToString(b.Content)==label);
                        var binding=Descendants(host).OfType<System.Windows.Controls.Button>().First(b=>Convert.ToString(b.ToolTip)==GameSettings.Labels[0]+", binding 1");
                        Click(binding);Require(Descendants(host).OfType<System.Windows.Controls.Border>().Any(b=>b.Name=="BindingCaptureArea"));
                        Click(button("Bind Escape"));Click(button("Cancel"));Require(Convert.ToString(binding.Content)=="E");
                        Click(binding);Click(button("Bind Escape"));Click(button("Move binding"));Require(Convert.ToString(binding.Content)=="Escape");
                        Click(Descendants(host).OfType<System.Windows.Controls.Button>().Single(b=>b.Name=="SettingsSaveButton"));
                        var saved=GameSettings.Load(root,null);Require(GameSettings.KeyCode(saved.Binding(0)[0])==27 && saved.Binding(6).All(t=>GameSettings.KeyCode(t)!=27));
                        Require(host.Visibility==System.Windows.Visibility.Visible);
                        Click(binding);Click((System.Windows.Controls.Button)ui.Window.FindName("HomeNav"));ui.ShowSettings();
                        Require(!Descendants(host).OfType<System.Windows.Controls.Border>().Any(b=>b.Name=="BindingCaptureArea"));
                    } finally {ui.Window.Close();}
                });
            });
            Test("inline save validates input and keeps the page open after a successful save",()=>{
                string root=Root("inline-save");STA(()=>{
                    var ui=new LauncherWindow(root,true);
                    try {
                        ui.ShowSettings(1);var host=(System.Windows.Controls.ContentControl)ui.Window.FindName("SettingsHost");
                        var nodes=Descendants(host).ToList();
                        var sensitivity=nodes.OfType<System.Windows.Controls.TextBox>().Single(t=>t.Name=="MouseSensitivity");
                        var save=nodes.OfType<System.Windows.Controls.Button>().Single(b=>b.Name=="SettingsSaveButton");
                        sensitivity.Text="invalid";Click(save);Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==0);
                        Require(Descendants(host).OfType<System.Windows.Controls.TextBlock>().Any(t=>t.Text.Contains("numeric sensitivity")));
                        sensitivity.Text="0.33";var skip=nodes.OfType<System.Windows.Controls.CheckBox>().Single(c=>Convert.ToString(c.Content)=="Skip startup splash screens");skip.IsChecked=true;Click(save);
                        var saved=GameSettings.Load(root,null);Require(saved.SkipStartupSplashes && saved.Sensitivity==.33 && host.Visibility==System.Windows.Visibility.Visible);
                    } finally {ui.Window.Close();}
                });
            });
            Test("opening settings does not create files; defaults have PC bindings",()=>{
                string root=Root("cancel");var model=GameSettings.Load(root,null);
                Require(model.Sensitivity==.10 && model.Binding(14)[0]=="key:W" && !model.SkipStartupSplashes);
                Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==0);
            });
            Test("startup preference round-trips, preserves other launcher keys and can be disabled",()=>{
                string root=Root("startup");var model=GameSettings.Load(root,null);model.Save();
                var json=Json.Object(Read(root,"Settings/pc-controls.json"));
                ((System.Collections.Generic.Dictionary<string,object>)json["launcher"])["future_option"]="keep";
                Put(root,"Settings/pc-controls.json",Json.Encode(json));
                model=GameSettings.Load(root,null);model.SkipStartupSplashes=true;model.Save();
                Require(GameSettings.Load(root,null).SkipStartupSplashes);
                Require(Read(root,"Settings/pc-controls.json").Contains("future_option"));
                string inherited=Environment.GetEnvironmentVariable("RECOMP_XMV_PLAY");
                try {
                    Environment.SetEnvironmentVariable("RECOMP_XMV_PLAY","1");
                    Require(GameSettings.StartInfo(root).EnvironmentVariables["RECOMP_XMV_PLAY"]=="0");
                    model=GameSettings.Load(root,null);model.SkipStartupSplashes=false;model.Save();
                    Environment.SetEnvironmentVariable("RECOMP_XMV_PLAY","0");
                    Require(!GameSettings.Load(root,null).SkipStartupSplashes);
                    Require(GameSettings.StartInfo(root).EnvironmentVariables["RECOMP_XMV_PLAY"]=="1");
                } finally {Environment.SetEnvironmentVariable("RECOMP_XMV_PLAY",inherited);}
            });
            Test("invalid startup preference is refused without altering settings",()=>{
                string root=Root("bad-startup");var model=GameSettings.Load(root,null);model.Save();
                var json=Json.Object(Read(root,"Settings/pc-controls.json"));
                ((System.Collections.Generic.Dictionary<string,object>)json["launcher"])["skip_startup_splashes"]="false";
                Put(root,"Settings/pc-controls.json",Json.Encode(json));string before=Read(root,"Settings/pc-controls.json");
                Reject(()=>GameSettings.Load(root,null));Require(Read(root,"Settings/pc-controls.json")==before);
            });
            Test("save round-trips controls and graphics; preserves other profiles and unknown fields",()=>{
                string root=Root("preserve"); var first=GameSettings.Load(root,null);first.Save();
                var json=Json.Object(Read(root,"Settings/pc-controls.json"));json["future_setting"]="keep";
                var profiles=(IList)json["controllers"];((System.Collections.Generic.Dictionary<string,object>)profiles[1])["future_pad"]="keep pad";
                Put(root,"Settings/pc-controls.json",Json.Encode(json));
                Put(root,"Settings/runtime.conf","# Keep this comment\nfuture_graphics = keep\nframe_interp = 2\n");
                var model=GameSettings.Load(root,null);model.Width=3440;model.Height=1440;model.Sensitivity=.35;model.Anisotropy=16;model.Fullscreen=true;
                model.Assign(14,1,90,false);model.Save();var saved=GameSettings.Load(root,null);
                Require(saved.Binding(14)[0]=="key:W" && GameSettings.KeyCode(saved.Binding(14)[1])==90);
                Require(saved.Width==3440 && saved.Height==1440 && saved.Fullscreen && saved.Interpolation==2 && saved.Anisotropy==16 && saved.Sensitivity==.35);
                Require(Read(root,"Settings/pc-controls.json").Contains("keep pad") && Read(root,"Settings/pc-controls.json").Contains("future_setting"));
                Require(Read(root,"Settings/runtime.conf").Contains("# Keep this comment") && Read(root,"Settings/runtime.conf").Contains("future_graphics = keep"));
                Require(File.Exists(Path.Combine(root,"Settings/pc-controls.json.bak")));
            });
            Test("conflicting key requires move and removes only that key",()=>{
                var model=GameSettings.Load(Root("conflict"),null);Reject(()=>model.Assign(14,1,69,false));
                model.Assign(14,1,69,true);Require(GameSettings.KeyCode(model.Binding(14)[1])==69);
                Require(model.Binding(0).All(t=>GameSettings.KeyCode(t)!=69));Require(model.Binding(0).Contains("key:SPACE"));
                Reject(()=>model.Assign(0,0,112,true));Reject(()=>model.Assign(0,0,122,true));
                model.Assign(14,1,0,true);Require(model.Binding(14)[1]=="none");
            });
            Test("invalid settings are rejected before any writes",()=>{
                string root=Root("invalid");var model=GameSettings.Load(root,null);model.Sensitivity=Double.NaN;Reject(()=>model.Save());
                model.Sensitivity=.1;model.Width=7680;model.Height=720;Reject(()=>model.Save());
                Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==0);
            });
            Test("malformed existing JSON is preserved",()=>{
                string root=Root("malformed");Put(root,"Settings/pc-controls.json","{broken");Reject(()=>GameSettings.Load(root,null));Require(Read(root,"Settings/pc-controls.json")=="{broken");
            });
            Test("external changes reject the entire stale save",()=>{
                string root=Root("external");var model=GameSettings.Load(root,null);Put(root,"Settings/pc-display.ini","TSFP_DISPLAY_V1 1920 1080\n");Reject(()=>model.Save());Require(!File.Exists(Path.Combine(root,"Settings/pc-controls.json")));Require(Read(root,"Settings/pc-display.ini").Contains("1920 1080"));
            });
            Test("read-only settings prevent partial changes",()=>{
                string root=Root("readonly");var model=GameSettings.Load(root,null);model.Save();model=GameSettings.Load(root,null);
                string path=Path.Combine(root,"Settings/runtime.conf");string before=Read(root,"Settings/pc-controls.json");File.SetAttributes(path,FileAttributes.ReadOnly);
                try {model.Sensitivity=.5;Reject(()=>model.Save());Require(Read(root,"Settings/pc-controls.json")==before);}finally{File.SetAttributes(path,FileAttributes.Normal);}
            });
            Test("late write failure rolls back already replaced files",()=>{
                string root=Root("rollback");var model=GameSettings.Load(root,null);model.Save();model=GameSettings.Load(root,null);
                string before=Read(root,"Settings/pc-controls.json"),display=Read(root,"Settings/pc-display.ini");
                Directory.CreateDirectory(Path.Combine(root,"Settings/runtime.conf.bak"));model.Sensitivity=.7;model.Width=1920;model.Height=1080;
                Reject(()=>model.Save());Require(Read(root,"Settings/pc-controls.json")==before && Read(root,"Settings/pc-display.ini")==display);
            });
            Test("inherited runtime settings are copied without modifying their source",()=>{
                string root=Root("inherit");Put(root,"old.conf","frame_interp = 2\nfuture = untouched\n");
                var model=GameSettings.Load(root,Path.Combine(root,"old.conf"));Require(model.Interpolation==2);model.Save();Require(Read(root,"old.conf")=="frame_interp = 2\nfuture = untouched\n");Require(Read(root,"Settings/runtime.conf").Contains("future = untouched"));
            });
            Test("configured launch passes portable settings directly to runtime",()=>{
                string root=Root("launch");var model=GameSettings.Load(root,null);model.Sensitivity=.25;model.Save();
                string old=Environment.GetEnvironmentVariable("RECOMP_SETTINGS_TEST");Environment.SetEnvironmentVariable("RECOMP_SETTINGS_TEST","bad");
                try {var info=GameSettings.StartInfo(root);Require(info.FileName==Path.Combine(root,"Bin","futureperfect_recomp.exe"));Require(info.EnvironmentVariables["RECOMP_DISPLAY_CONFIG"]==Path.Combine(root,"Settings","runtime.conf"));Require(info.EnvironmentVariables["TSFP_DISPLAY_CONFIG"]==Path.Combine(root,"Settings","pc-display.ini"));Require(info.EnvironmentVariables["RECOMP_MOUSE_SENSITIVITY"]=="0.25");Require(info.EnvironmentVariables["RECOMP_SETTINGS_TEST"]==null);}finally{Environment.SetEnvironmentVariable("RECOMP_SETTINGS_TEST",old);}
            });
            Test("child process receives and reads the selected portable settings",()=>{
                string root=Root("child");var model=GameSettings.Load(root,null);model.Sensitivity=.45;model.Width=2560;model.Height=1440;model.Anisotropy=8;model.SkipStartupSplashes=true;model.Save();
                Directory.CreateDirectory(Path.Combine(root,"Bin"));File.Copy(typeof(LauncherSettingsChecks).Assembly.Location,Path.Combine(root,"Bin/futureperfect_recomp.exe"));
                var info=GameSettings.StartInfo(root);info.Arguments="--settings-child";
                using(var child=Process.Start(info)) {
                    try {var output=child.StandardOutput.ReadToEndAsync();var errors=child.StandardError.ReadToEndAsync();Require(child.WaitForExit(5000));string text=output.GetAwaiter().GetResult();string error=errors.GetAwaiter().GetResult();Require(child.ExitCode==0 && text.Contains("0.45") && text.Contains("TSFP_DISPLAY_V1 2560 1440") && text.Contains("8") && text.Contains("XMV_PLAY=0"));Require(error.Length==0);}
                    finally{if(!child.HasExited)child.Kill();}
                }
            });
            Console.WriteLine("PASS: "+count+" isolated launcher settings checks.");return 0;
        }
        catch(Exception error){Console.WriteLine(error);return 1;}
        finally {Directory.Delete(temp,true);}
    }
}
