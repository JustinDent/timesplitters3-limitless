using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Media;
namespace TimeSplitters.Launcher
{
    public sealed class NativeStartup:Window
    {
        readonly CancellationTokenSource stop=new CancellationTokenSource();readonly TextBlock status;
        readonly string root;readonly string[] args;bool handedOff;
        public NativeStartup(string root,string[] args)
        {
            this.root=root;this.args=args;Title="TimeSplitters · Future Perfect";Width=540;Height=300;CanResize=false;WindowStartupLocation=WindowStartupLocation.CenterScreen;Background=Brush.Parse("#0B1420");
            var panel=new StackPanel{Margin=new Thickness(28),Spacing=14};panel.Children.Add(NativeWindow.Text("TIMESPLITTERS",30));panel.Children.Add(NativeWindow.Text("FUTURE PERFECT",18));
            status=NativeWindow.Text("Checking for launcher updates…",13);panel.Children.Add(status);panel.Children.Add(NativeWindow.Button("Open installed launcher",()=>stop.Cancel()));Content=panel;
            Opened+=async(s,e)=>await Start();Closed+=(s,e)=>{if(!handedOff)stop.Cancel();};
        }
        async Task Start()
        {
            try{
                // Developer output and source checkouts never replace themselves.
                if(!LauncherPlatform.Checkout(root) && SelfUpdate.Complete(root) && !Array.Exists(args,a=>a=="--skip-self-update")){
                    using(var client=new ReleaseClient()){
                        var release=await SelfUpdate.CheckWithTimeout(client,SelfUpdate.PreviewChannel(root),TimeSpan.FromSeconds(8),stop.Token);
                        if(release!=null && GameVersion.Parse(release.Tag).CompareTo(GameVersion.Parse(LauncherVersion.Value))>0){
                            using(var deadline=CancellationTokenSource.CreateLinkedTokenSource(stop.Token)){
                                deadline.CancelAfter(TimeSpan.FromMinutes(2));status.Text="Downloading launcher "+release.Tag+"…";
                                if(await SelfUpdate.Prepare(root,release,client,new Progress<double>(n=>status.Text="Updating launcher · "+n.ToString("0")+"%"),deadline.Token)){
                                    string helper=Core.PathIn(root,".launcher/mac-self-helper");File.Copy(Environment.ProcessPath,helper,true);File.SetUnixFileMode(helper,UnixFileMode.UserRead|UnixFileMode.UserWrite|UnixFileMode.UserExecute);
                                    var info=new ProcessStartInfo(helper){UseShellExecute=false,WorkingDirectory=root};info.ArgumentList.Add("--apply-self");info.ArgumentList.Add(root);info.ArgumentList.Add(Environment.ProcessId.ToString());Process.Start(info);
                                    handedOff=true;((IClassicDesktopStyleApplicationLifetime)Application.Current.ApplicationLifetime).Shutdown();return;
                                }
                            }
                        }
                    }
                }
            }catch(Exception ex){LauncherLog.Write(root,"Startup update fallback: "+ex.Message);}
            if(!IsVisible)return;handedOff=true;var desktop=(IClassicDesktopStyleApplicationLifetime)Application.Current.ApplicationLifetime;
            var window=new NativeWindow(root,args);desktop.MainWindow=window;window.Show();Close();
        }
        public static int Apply(string root,int parent)
        {
            try{SelfUpdate.WaitForParent(parent);using(var guard=new FileStream(Core.PathIn(root,".launcher/mac.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.None)){
                Core.RecoverLauncher(root);SelfUpdate.InstallPending(root);
            }
            var info=new ProcessStartInfo("/usr/bin/open"){UseShellExecute=false};info.ArgumentList.Add("-n");info.ArgumentList.Add(Core.PathIn(root,"TimeSplitters Launcher.app"));info.ArgumentList.Add("--args");info.ArgumentList.Add("--skip-self-update");Process.Start(info);return 0;
            }catch(Exception e){LauncherLog.Write(root,e.ToString());return 1;}
        }
    }
}
