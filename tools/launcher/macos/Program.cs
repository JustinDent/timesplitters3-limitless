using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Reflection.Metadata;
using System.Reflection.PortableExecutable;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Themes.Fluent;
using Avalonia.Styling;
[assembly: System.Runtime.Versioning.SupportedOSPlatform("macos12.0")]
namespace TimeSplitters.Launcher
{
    public static class Program
    {
        public static string Root;
        public static string[] Arguments;
        [STAThread] public static int Main(string[] args)
        {
            Arguments=args;
            if(args.Length==3 && args[0]=="--test-self-helper")return NativeStartup.Apply(Path.GetFullPath(args[1]),Int32.Parse(args[2]),true);
            if(args.Length==3 && args[0]=="--apply-self")return NativeStartup.Apply(Path.GetFullPath(args[1]),Int32.Parse(args[2]));
            if(args.Length==3 && args[0]=="--extract-art") { ExtractArt(args[1],args[2]);return 0; }
            if(args.Length==1 && args[0]=="--test")return NativeTests.Run();
            if(args.Length==3 && args[0]=="--verify-package")return NativeTests.VerifyPackage(args[1],args[2]);
            if(args.Length==1 && args[0]=="--test-ui"){AppBuilder.Configure<NativeApplication>().UsePlatformDetect().SetupWithoutStarting();try{return LauncherSharedUiChecks.Run();}catch(Exception e){Console.Error.WriteLine(e);return 1;}}
            Root=FindRoot(args);
            if(args.Contains("--check")){try{NativeHost.CheckData(Root);NativeHost.CheckBuild(Root);GameSettings.Load(Root,null);Console.WriteLine("Native Mac launcher prerequisites passed.");return 0;}catch(Exception e){Console.Error.WriteLine(e.Message);return 1;}}
            return AppBuilder.Configure<NativeApplication>().UsePlatformDetect().StartWithClassicDesktopLifetime(args);
        }
        static string FindRoot(string[] args)
        {
            int option=Array.IndexOf(args,"--root");if(option>=0){if(option+1>=args.Length)throw new ArgumentException("--root needs a path");return Path.GetFullPath(args[option+1]);}
            for(var dir=new DirectoryInfo(AppContext.BaseDirectory);dir!=null;dir=dir.Parent)
                if(File.Exists(Path.Combine(dir.FullName,"tools/play-native-mac.py")))return dir.FullName;
            var bundle=new DirectoryInfo(AppContext.BaseDirectory).Parent?.Parent;
            if(bundle!=null && bundle.Name.EndsWith(".app"))return bundle.Parent.FullName;
            return AppContext.BaseDirectory;
        }
        public static void Log(string root,string text){LauncherLog.Write(root,text);}
        static void ExtractArt(string path,string destination)
        {
            Directory.CreateDirectory(destination);
            using(var pe=new PEReader(File.OpenRead(path))){var md=pe.GetMetadataReader();var resources=pe.GetSectionData(pe.PEHeaders.CorHeader.ResourcesDirectory.RelativeVirtualAddress);
                foreach(var handle in md.ManifestResources){var resource=md.GetManifestResource(handle);var name=md.GetString(resource.Name);
                    if(!new[]{"launcher-banner.png","launcher-backdrop.png","launcher-logo.png","launcher.ico"}.Contains(name))continue;
                    Core.Require(resource.Implementation.IsNil,"External art resource not supported.");var reader=resources.GetReader((int)resource.Offset,resources.Length-(int)resource.Offset);int size=reader.ReadInt32();Core.Require(size>0 && size<=8*1024*1024,"Invalid art size.");File.WriteAllBytes(Path.Combine(destination,name),reader.ReadBytes(size));Console.WriteLine(name+": "+size+" bytes");}}
        }
    }
    public sealed class NativeApplication:Application
    {
        public override void Initialize(){Name="TimeSplitters Launcher";RequestedThemeVariant=ThemeVariant.Dark;Styles.Add(new FluentTheme());}
        public override void OnFrameworkInitializationCompleted(){if(ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)desktop.MainWindow=Program.Arguments.Any(a=>a.StartsWith("--render-")||a=="--test-session")?(Avalonia.Controls.Window)new NativeWindow(Program.Root,Program.Arguments):new NativeStartup(Program.Root,Program.Arguments);base.OnFrameworkInitializationCompleted();}
    }
}
