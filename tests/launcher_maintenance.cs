using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using TimeSplitters.Launcher;

public static class LauncherMaintenanceChecks
{
    static string temp;
    static int passed;
    static bool readOnly;
    static void TransactionTest(string name,Action body) { if(readOnly)Console.WriteLine("SKIP transaction check while preserving active game: "+name); else Test(name,body); }
    static void Require(bool value,string message="Maintenance assertion failed.") { if(!value)throw new Exception(message); }
    static void Reject(Action body) { bool failed=false;try{body();}catch{failed=true;}Require(failed,"Expected operation to be rejected."); }
    static void Test(string name,Action body) { try{body();}catch(Exception ex){throw new Exception("Maintenance check failed: "+name,ex);}passed++;Console.WriteLine("PASS "+name); }
    static void Put(string root,string name,string text) { string p=Core.PathIn(root,name);Directory.CreateDirectory(Path.GetDirectoryName(p));File.WriteAllText(p,text,new UTF8Encoding(false)); }
    static string Read(string root,string name) { return File.ReadAllText(Core.PathIn(root,name)); }
    static string Hash(byte[] bytes) { using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-","").ToLowerInvariant(); }
    static string Hash(string text) { return Hash(Encoding.UTF8.GetBytes(text)); }
    static string Root() { string root=Path.Combine(temp,Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);return root; }
    static Dictionary<string,string> GameFiles() { return new Dictionary<string,string> { {"Bin/futureperfect_recomp.exe","game"},{"Launch.ps1","launch"},{"Settings/pc-controls.json","defaults"},{"Bin/runtime.dll","library"} }; }
    static void Self(string root) {
        Put(root,Core.LauncherName,"themed bootstrap");Put(root,Core.AppPath,"themed app");
        Put(root,Core.SelfManifest,SelfManifest(LauncherVersion.Value,"themed bootstrap","themed app"));
    }
    static string SelfManifest(string version,string boot,string app) { return Json.Encode(new { schema=1,version=version,files=new Dictionary<string,string> { {Core.LauncherName,Hash(boot)},{Core.AppPath,Hash(app)} } }); }
    static string Game(string root) {
        var files=GameFiles();foreach(var pair in files)Put(root,pair.Key,pair.Value);
        // Deliberately stale bundled launcher hashes must not flag an independent launcher.
        files[Core.LauncherName]="bundled bootstrap";files[Core.AppPath]="bundled app";
        Put(root,"BUILD.json",Json.Encode(new {version="v1.0.0",files=files.ToDictionary(p=>p.Key,p=>Hash(p.Value))}));
        Self(root);Put(root,"Settings/pc-controls.json","personal controls");Put(root,"Settings/pc-display.ini","personal display");
        Put(root,"Saves/profile","personal save");Put(root,"Data/owned.iso","personal ISO");Put(root,"Textures/pack.dds","personal textures");Put(root,"custom.txt","untracked");return root;
    }
    static void Personal(string root) {
        foreach(var item in new[]{new[]{"Settings/pc-controls.json","personal controls"},new[]{"Settings/pc-display.ini","personal display"},new[]{"Saves/profile","personal save"},new[]{"Data/owned.iso","personal ISO"},new[]{"Textures/pack.dds","personal textures"},new[]{"custom.txt","untracked"}})Require(Read(root,item[0])==item[1]);
    }
    static byte[] Archive(bool launcher) {
        var files=launcher?new Dictionary<string,string> { {Core.LauncherName,"release bootstrap"},{Core.AppPath,"release app"},{Core.SelfManifest,SelfManifest(LauncherVersion.Value,"release bootstrap","release app")} }:GameFiles();
        if(!launcher) {
            files[Core.LauncherName]="newer bundled bootstrap";
            files["BUILD.json"]=Json.Encode(new {version="v1.0.0",launcher_version="99.0.0",files=files.ToDictionary(p=>p.Key,p=>Hash(p.Value))});
        }
        using(var buffer=new MemoryStream()) {
            using(var zip=new ZipArchive(buffer,ZipArchiveMode.Create,true))foreach(var pair in files)
                using(var writer=new StreamWriter(zip.CreateEntry(pair.Key).Open(),new UTF8Encoding(false)))writer.Write(pair.Value);
            return buffer.ToArray();
        }
    }
    sealed class Progress : IProgress<double> { public Action<double> Action;public void Report(double value){if(Action!=null)Action(value);} }
    sealed class Handler : HttpMessageHandler {
        public Func<HttpRequestMessage,HttpResponseMessage> Reply;
        protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request,CancellationToken token){var response=Reply(request);response.RequestMessage=request;return Task.FromResult(response);}
    }
    static string Url(string tag,string asset) { return "https://github.com/"+ReleaseClient.Repository+"/releases/download/"+tag+"/"+asset; }
    static object GameRelease(byte[] archive,string version="v1.0.0",bool draft=false) {
        string name="TimeSplitters-Future-Perfect-PC-"+version+"-windows-x64.zip";
        return new { tag_name=version,draft=draft,prerelease=true,assets=new[]{new { name=name,state="uploaded",size=archive.Length,digest="sha256:"+Hash(archive),browser_download_url=Url(version,name)} } };
    }
    static HttpResponseMessage Response(string text) { return new HttpResponseMessage(HttpStatusCode.OK){Content=new StringContent(text)}; }
    static ReleaseClient GameClient(byte[] archive,Action<HttpRequestMessage> request=null,byte[] actual=null) {
        return new ReleaseClient(new Handler {Reply=r=> {
            if(request!=null)request(r);
            return r.RequestUri.Host=="api.github.com"?Response(Json.Encode(GameRelease(archive))):new HttpResponseMessage(HttpStatusCode.OK){Content=new ByteArrayContent(actual??archive)};
        }});
    }
    static Dictionary<string,string> Snapshot(string root) { return Directory.GetFiles(root,"*",SearchOption.AllDirectories).ToDictionary(p=>p.Substring(root.Length),p=>Core.Hash(p)); }
    public static int Run(bool onlyReadOnly=false) {
        readOnly=onlyReadOnly;passed=0;temp=Path.Combine(Path.GetDirectoryName(typeof(LauncherMaintenanceChecks).Assembly.Location),"maintenance-tests-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(temp);
        try {
            Test("verification is read-only and excludes personal files and independent launcher hashes",()=> {
                string root=Game(Root());var before=Snapshot(root);
                var result=InstallationMaintenance.Verify(root,false,null,CancellationToken.None);
                Require(result.Healthy && result.Checked==3 && result.Skipped==3);
                File.Delete(Core.PathIn(root,"Bin/runtime.dll"));Put(root,"Launch.ps1","damaged");before=Snapshot(root);
                result=InstallationMaintenance.Verify(root,false,null,CancellationToken.None);
                Require(result.Issues.Count==2 && result.Issues.Any(s=>s.Contains("missing")) && result.Issues.Any(s=>s.Contains("mismatch")));
                Require(before.OrderBy(p=>p.Key).SequenceEqual(Snapshot(root).OrderBy(p=>p.Key)));Personal(root);
            });
            Test("missing and protected manifests report safely without creating files",()=> {
                string root=Root();Require(!InstallationMaintenance.Verify(root,false,null,CancellationToken.None).Healthy && Directory.GetFileSystemEntries(root).Length==0);
                Put(root,"BUILD.json",Json.Encode(new{version="v1.0.0",files=new Dictionary<string,string>{{"Data/owned.iso",Hash("secret")}}}));
                var before=Snapshot(root);var result=InstallationMaintenance.Verify(root,false,null,CancellationToken.None);
                Require(!result.Healthy && result.Checked==0 && before.SequenceEqual(Snapshot(root)));
                Put(root,Core.SelfManifest,"invalid manifest");before=Snapshot(root);
                Require(!InstallationMaintenance.Verify(root,true,null,CancellationToken.None).Healthy && before.SequenceEqual(Snapshot(root)));
            });
            Test("launcher verification identifies missing bootstrap and malformed manifests",()=> {
                string root=Game(Root());Require(InstallationMaintenance.Verify(root,true,null,CancellationToken.None).Healthy);
                File.Delete(Core.PathIn(root,Core.LauncherName));var result=InstallationMaintenance.Verify(root,true,null,CancellationToken.None);
                Require(!result.Healthy && result.Version==LauncherVersion.Value && result.Issues.Count==1);
            });
            Test("verification cancellation is observed between files without writes",()=> {
                string root=Game(Root());var before=Snapshot(root);
                using(var cancel=new CancellationTokenSource()) {
                    var progress=new Progress {Action=p=>cancel.Cancel()};
                    Reject(()=>InstallationMaintenance.Verify(root,false,progress,cancel.Token));
                }Require(before.SequenceEqual(Snapshot(root)));
            });
            Test("cancelled graphics query returns without starting system discovery",()=> {
                using(var cancel=new CancellationTokenSource()) {cancel.Cancel();Reject(()=>InstallationMaintenance.GraphicsWithTimeout(cancel.Token).GetAwaiter().GetResult());}
            });
            Test("diagnostic reports omit paths, raw logs, secrets, saves and ISO contents",()=> {
                string root=Game(Root());string secret="SECRET_MARKER_847593";
                Put(root,".launcher/launcher.log","2026-10-17 01:00:00Z System.IO.IOException: "+root+" "+secret+" user@example.com 10.2.3.4\n arbitrary secret: "+secret);
                var before=Snapshot(root);string report=InstallationMaintenance.Report(root,null,null,"Synthetic GPU / driver 1");
                Require(report.Contains("System.IO.IOException") && report.Contains("Synthetic GPU"));
                Require(!report.Contains(secret) && !report.Contains(root) && !report.Contains("user@example.com") && !report.Contains("10.2.3.4") && !report.Contains("personal save") && !report.Contains("owned.iso"));
                Require(before.SequenceEqual(Snapshot(root)));
            });
            Test("repair pins exact game version and refuses drafts and missing releases",()=> {
                byte[] archive=Archive(false);
                using(var client=GameClient(archive,r=> {if(r.RequestUri.Host=="api.github.com")Require(r.RequestUri.AbsolutePath.EndsWith("/releases/tags/v1.0.0"));}))
                    Require(client.FindGame("v1.0.0",CancellationToken.None).GetAwaiter().GetResult().Tag=="v1.0.0");
                foreach(var response in new[]{new HttpResponseMessage(HttpStatusCode.NotFound),Response(Json.Encode(GameRelease(archive,"v1.0.0",true))),Response(Json.Encode(GameRelease(archive,"v2.0.0")))})
                    using(var client=new ReleaseClient(new Handler {Reply=r=>response}))Reject(()=>client.FindGame("v1.0.0",CancellationToken.None).GetAwaiter().GetResult());
            });
            TransactionTest("game repair changes only damaged owned files and preserves game rollback",()=> {
                string root=Game(Root());Put(root,"Bin/runtime.dll","broken");File.Delete(Core.PathIn(root,"Launch.ps1"));
                Put(root,".launcher/rollback/files/BUILD.json","previous game marker");Put(root,".launcher/rollback/journal.json","[]");
                string self=Read(root,Core.SelfManifest);byte[] archive=Archive(false);
                using(var client=GameClient(archive))InstallationMaintenance.PrepareGameRepair(root,"v1.0.0",client,new Progress(),CancellationToken.None).GetAwaiter().GetResult();
                Require(Read(root,"Bin/runtime.dll")=="broken");Core.InstallPending(root);
                Require(Read(root,"Bin/runtime.dll")=="library" && Read(root,"Launch.ps1")=="launch" && Read(root,Core.SelfManifest)==self && Read(root,Core.LauncherName)=="themed bootstrap");
                Require(Read(root,".launcher/rollback/files/BUILD.json")=="previous game marker" && Directory.Exists(Core.PathIn(root,".launcher/repair-rollback")));
                Require(!File.Exists(Core.PathIn(root,".launcher/repair-rollback/files/Bin/futureperfect_recomp.exe")),"Healthy runtime was unnecessarily replaced.");Personal(root);
                string build=Core.GameBuildHash(root);
                Core.FinishGameSession(root,build,false,0);Require(Directory.Exists(Core.PathIn(root,".launcher/repair-rollback")));
                Core.FinishGameSession(root,build,true,1);Require(Directory.Exists(Core.PathIn(root,".launcher/repair-rollback")));
                Core.FinishGameSession(root,build,true,0);Require(!Directory.Exists(Core.PathIn(root,".launcher/repair-rollback")));Personal(root);
            });
            TransactionTest("repair cancels downloads and rejects corrupt archives without installed changes",()=> {
                foreach(bool cancelDownload in new[]{false,true}) {
                    string root=Game(Root());var before=Snapshot(root);byte[] archive=Archive(false);
                    using(var cancel=new CancellationTokenSource())using(var client=GameClient(archive,null,cancelDownload?null:Encoding.UTF8.GetBytes("bad"))) {
                        var progress=new Progress {Action=p=> {if(cancelDownload)cancel.Cancel();}};
                        Reject(()=>InstallationMaintenance.PrepareGameRepair(root,"v1.0.0",client,progress,cancel.Token).GetAwaiter().GetResult());
                    }Require(before.SequenceEqual(Snapshot(root)) && !Directory.Exists(Core.PathIn(root,".launcher/pending")));
                }
            });
            TransactionTest("repair preserves pre-existing staging and rejects changed installed version",()=> {
                string root=Game(Root());Put(root,".launcher/pending/unknown","keep");int requests=0;
                using(var client=GameClient(Archive(false),r=>requests++))Reject(()=>InstallationMaintenance.PrepareGameRepair(root,"v1.0.0",client,null,CancellationToken.None).GetAwaiter().GetResult());
                Require(requests==0 && Read(root,".launcher/pending/unknown")=="keep");
                root=Game(Root());using(var client=GameClient(Archive(false)))InstallationMaintenance.PrepareGameRepair(root,"v1.0.0",client,new Progress(),CancellationToken.None).GetAwaiter().GetResult();
                Put(root,"BUILD.json","{\"version\":\"v2.0.0\"}");var before=Snapshot(root);Reject(()=>Core.InstallPending(root));Require(before.SequenceEqual(Snapshot(root)));
            });
            TransactionTest("repair cleanup preserves an ISO placed inside managed staging",()=> {
                foreach(bool launcher in new[]{false,true}) {
                    string root=Game(Root());byte[] archive=Archive(launcher);
                    string staging=launcher?".launcher/self-pending":".launcher/pending";
                    using(var client=new ReleaseClient(new Handler {Reply=r=> {
                        if(r.RequestUri.Host=="api.github.com")return Response(Json.Encode(GameRelease(archive)));
                        Put(root,staging+"/owned.iso","preserve this ISO");
                        return new HttpResponseMessage(HttpStatusCode.OK){Content=new ByteArrayContent(new byte[0])};
                    }})) {
                        if(launcher) {
                            var release=new Release {Tag=LauncherVersion.Value,Size=archive.Length,Digest="sha256:"+Hash(archive),AssetUrl=Url("launcher-v"+LauncherVersion.Value,"launcher.zip")};
                            Reject(()=>SelfUpdate.Prepare(root,release,client,new Progress(),CancellationToken.None,true).GetAwaiter().GetResult());
                        } else Reject(()=>InstallationMaintenance.PrepareGameRepair(root,"v1.0.0",client,new Progress(),CancellationToken.None).GetAwaiter().GetResult());
                    }
                    Require(Read(root,staging+"/owned.iso")=="preserve this ISO");Personal(root);
                }
            });
            TransactionTest("interrupted repair recovers without replacing the previous game rollback",()=> {
                string root=Game(Root());Put(root,".launcher/rollback/preserve","previous game");
                Put(root,".launcher/repair-transaction/files/Launch.ps1","launch");
                Put(root,".launcher/repair-transaction/journal.json",Json.Encode(new[]{new Original {Name="Launch.ps1",Existed=true}}));
                Put(root,"Launch.ps1","partial repair");Require(Core.Recover(root));
                Require(Read(root,"Launch.ps1")=="launch" && Read(root,".launcher/rollback/preserve")=="previous game" && !Directory.Exists(Core.PathIn(root,".launcher/repair-transaction")));Personal(root);
            });
            TransactionTest("forced same-version launcher repair uses verified package and protects staging",()=> {
                byte[] archive=Archive(true);string version=LauncherVersion.Value;string root=Game(Root());
                var release=new Release {Tag=version,Size=archive.Length,Digest="sha256:"+Hash(archive),AssetUrl=Url("launcher-v"+version,"launcher.zip")};
                using(var client=new ReleaseClient(new Handler {Reply=r=>new HttpResponseMessage(HttpStatusCode.OK){Content=new ByteArrayContent(archive)}})) {
                    Require(!SelfUpdate.Prepare(root,release,client,new Progress(),CancellationToken.None).GetAwaiter().GetResult());
                    Require(SelfUpdate.Prepare(root,release,client,new Progress(),CancellationToken.None,true).GetAwaiter().GetResult());
                    Require(Read(root,Core.AppPath)=="themed app");SelfUpdate.InstallPending(root);
                    Require(SelfUpdate.Complete(root) && Read(root,Core.AppPath)=="release app");Personal(root);
                    Put(root,".launcher/self-pending/unknown","keep");Reject(()=>SelfUpdate.Prepare(root,release,client,new Progress(),CancellationToken.None,true).GetAwaiter().GetResult());
                    Require(Read(root,".launcher/self-pending/unknown")=="keep");
                }
            });
            Test("launcher exact-release lookup verifies metadata and never substitutes a different version",()=> {
                byte[] archive=Archive(true);string version=LauncherVersion.Value;string name="TimeSplitters-Launcher-v"+version+"-windows-x64.zip";
                object metadata=new {schema=1,version=version,asset_name=name,size=archive.Length,sha256=Hash(archive)};
                var record=new {tag_name="launcher-v"+version,draft=false,prerelease=false,assets=new object[]{
                    new {name=name,state="uploaded",size=archive.Length,digest="sha256:"+Hash(archive),browser_download_url=Url("launcher-v"+version,name)},
                    new {name="launcher-update.json",state="uploaded",browser_download_url=Url("launcher-v"+version,"launcher-update.json")} }};
                using(var client=new ReleaseClient(new Handler {Reply=r=>Response(Json.Encode(r.RequestUri.Host=="api.github.com"?(object)record:metadata))})) {
                    Require(client.FindLauncher(version,CancellationToken.None).GetAwaiter().GetResult().Tag==version);
                    Reject(()=>client.FindLauncher("99.0.0",CancellationToken.None).GetAwaiter().GetResult());
                }
            });
            Test("maintenance and diagnostics share the main window and preserve settings drafts",()=> {
                Exception failure=null;var thread=new Thread(()=> {try {
                    var ui=new LauncherWindow(Root(),true);
                    try {
                        ui.ShowSettings();var settings=(System.Windows.Controls.ContentControl)ui.Window.FindName("SettingsHost");var draft=settings.Content;
                        ui.ShowMaintenance(false);var host=(System.Windows.Controls.ContentControl)ui.Window.FindName("MaintenanceHost");
                        Require(host.Visibility==System.Windows.Visibility.Visible && settings.Visibility==System.Windows.Visibility.Collapsed && System.Windows.Window.GetWindow((System.Windows.DependencyObject)host.Content)==ui.Window);
                        Require(((System.Windows.Controls.Border)ui.Window.FindName("SidebarPanel")).Visibility==System.Windows.Visibility.Visible);
                        ui.ShowMaintenance(true);Require(host.Visibility==System.Windows.Visibility.Visible);
                        ui.ShowSettings();Require(ReferenceEquals(settings.Content,draft) && host.Visibility==System.Windows.Visibility.Collapsed);
                    }finally{ui.Window.Close();}
                }catch(Exception ex){failure=ex;}});
                thread.IsBackground=true;thread.SetApartmentState(ApartmentState.STA);thread.Start();Require(thread.Join(60000),"Maintenance UI timed out.");if(failure!=null)throw failure;
            });
            Console.WriteLine("PASS: "+passed+" isolated launcher maintenance checks.");return 0;
        } finally {
            string parent=Path.GetDirectoryName(typeof(LauncherMaintenanceChecks).Assembly.Location);
            Require(Path.GetDirectoryName(temp)==parent && (File.GetAttributes(temp)&FileAttributes.ReparsePoint)==0);
            Directory.Delete(temp,true);
        }
    }
}
