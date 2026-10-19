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
namespace TimeSplitters.Launcher
{
    public static class NativeTests
    {
        static int count;static string sandbox;
        static void Need(bool test,string message){if(!test)throw new Exception(message);}
        static void Reject(Action action){bool rejected=false;try{action();}catch(InvalidDataException){rejected=true;}catch(IOException){rejected=true;}catch(UnauthorizedAccessException){rejected=true;}Need(rejected,"Expected rejection");}
        static void Test(string name,Action action){action();count++;Console.WriteLine("PASS "+name);}
        static string NewRoot(string name){var root=Path.Combine(sandbox,name);Directory.CreateDirectory(root);return root;}
        static string Hash(byte[] b){return Convert.ToHexString(SHA256.HashData(b)).ToLowerInvariant();}
        static void Write(string root,string name,string value){var path=Core.PathIn(root,name);Directory.CreateDirectory(Path.GetDirectoryName(path));File.WriteAllText(path,value);}
        static string Archive(string root,string version,bool corrupt=false,bool launcher=false)
        {
            var files=launcher?new Dictionary<string,string>{{Core.LauncherName,"native launcher"},{Core.AppPath,"plist"}}:new Dictionary<string,string>{{LauncherPlatform.GameBinary,"native game "+version},{LauncherPlatform.LaunchScript,"#!/bin/sh"},{"Settings/pc-controls.json","defaults"},{"Licenses/example.txt","license"}};
            var hashes=files.ToDictionary(p=>p.Key,p=>(object)Hash(Encoding.UTF8.GetBytes(p.Value)));
            string manifest=Json.Encode(new {schema=1,version=version,files=hashes});var path=Path.Combine(root,Guid.NewGuid()+".zip");
            using(var zip=ZipFile.Open(path,ZipArchiveMode.Create)){foreach(var pair in files){using(var writer=new StreamWriter(zip.CreateEntry(pair.Key).Open()))writer.Write(pair.Value+(corrupt?"corrupt":""));}using(var writer=new StreamWriter(zip.CreateEntry(launcher?Core.SelfManifest:"BUILD.json").Open()))writer.Write(manifest);}
            return path;
        }
        static string Metadata(string suffix,string tag="v1.0.0",bool preview=false)
        {
            string name="TimeSplitters-Future-Perfect-PC-"+tag+"-"+suffix+".zip";
            return Json.Encode(new[]{new{tag_name=tag,prerelease=preview,draft=false,assets=new[]{new{name=name,state="uploaded",size=10,digest="sha256:"+new string('a',64),browser_download_url="https://github.com/"+ReleaseClient.Repository+"/releases/download/"+tag+"/"+name}}}});
        }
        sealed class Handler:HttpMessageHandler { public Func<HttpRequestMessage,HttpResponseMessage> Reply;protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage r,CancellationToken t){var response=Reply(r);response.RequestMessage=r;return Task.FromResult(response);} }
        public static int VerifyPackage(string archive,string version)
        {
            string test=Path.Combine(Path.GetDirectoryName(Path.GetFullPath(archive)),"package-check-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(test);
            try { var changes=SelfUpdate.Unpack(archive,Path.Combine(test,"payload"),version);string install=Path.Combine(test,"install");Directory.CreateDirectory(install);Write(install,"BUILD.json","preserved game metadata");Write(install,"Saves/keep","preserved save");Core.ApplyLauncher(install,changes,null);Need(SelfUpdate.Complete(install),"Packaged app did not verify after installation");Need(File.ReadAllText(Path.Combine(install,"Saves/keep"))=="preserved save","Package touched saves");Console.WriteLine("Packaged Mac app extracted, installed and verified; game metadata and saves preserved.");return 0; }
            catch(Exception e){Console.Error.WriteLine(e);return 1;}
            finally{Directory.Delete(test,true);}
        }
        public static int Run()
        {
            sandbox=Path.Combine(AppContext.BaseDirectory,"tsfp-mac-launcher-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(sandbox);
            try{
                Test("Mac release selection rejects Windows packages",()=>{Need(ReleaseClient.Select(Metadata("windows-x64"),true)==null,"Windows package selected");Need(ReleaseClient.Select(Metadata("macos-arm64"),false)!=null,"Native release missing");Need(ReleaseClient.Select(Metadata("macos-arm64","v1.1.0-alpha.1",true),false)==null,"Preview selected on stable");});
                Test("JSON adapter round-trips nested fields and journal records",()=>{var map=Json.Object("{\"nested\":{\"flag\":true},\"list\":[1,2],\"n\":9223372036854775807}");Need(map["nested"] is Dictionary<string,object> && map["list"] is object[] && (long)map["n"]==long.MaxValue,"JSON types lost");var journal=Json.Serializer().Deserialize<List<Original>>(Json.Encode(new[]{new Original{Name="Bin/test",Existed=true}}));Need(journal[0].Name=="Bin/test"&&journal[0].Existed,"Journal fields lost");});
                Test("settings drafts, backups, unknown fields and external edit protection",()=>{var r=NewRoot("settings");var model=GameSettings.Load(r,null);Need(!Directory.Exists(Path.Combine(r,"Settings")),"Load wrote settings");model.Width=1280;model.Height=720;model.Save();var file=Core.PathIn(r,"Settings/pc-controls.json");var map=Json.Object(File.ReadAllText(file));map["future_option"]="preserve";File.WriteAllText(file,Json.Encode(map));var before=File.ReadAllBytes(file);model=GameSettings.Load(r,null);model.Sensitivity=.2;model.Save();Need(File.ReadAllBytes(file+".bak").SequenceEqual(before),"Backup differs");Need(Json.String(Json.Object(File.ReadAllText(file)),"future_option")=="preserve","Unknown setting lost");var a=GameSettings.Load(r,null);File.AppendAllText(file,"\n");Reject(()=>a.Save());});
                Test("multi-file settings failure rolls back committed files",()=>{var r=NewRoot("settings-failure");var m=GameSettings.Load(r,null);m.Save();var files=LauncherPlatform.Settings(r).Select(p=>Core.PathIn(r,p)).ToArray();var old=files.Select(File.ReadAllBytes).ToArray();Directory.CreateDirectory(files[2]+".bak");m.Sensitivity=.3;Reject(()=>m.Save());for(int i=0;i<files.Length;i++)Need(File.ReadAllBytes(files[i]).SequenceEqual(old[i]),"Partial settings commit");});
                Test("checkout settings and launch routing preserve user defaults",()=>{var r=NewRoot("checkout");Write(r,"tools/play-native-mac.py","fixture");Write(r,"private/native-settings/runtime.conf","frame_interp = 1\nfuture_setting = retained\n");var m=GameSettings.Load(r,null);Need(m.Interpolation==1,"60 FPS preference lost");m.Save();Need(!Directory.Exists(Path.Combine(r,"Settings")),"Created package settings in checkout");Environment.SetEnvironmentVariable("RECOMP_TEST_STALE","bad");var info=GameSettings.StartInfo(r);Need(info.FileName==Path.Combine(r,"build/macos/futureperfect_recomp"),"Wrong native executable");Need(info.EnvironmentVariables["RECOMP_SAVE_DIR"]==Path.Combine(r,"private/saves"),"Wrong save route");Need(info.EnvironmentVariables["RECOMP_DISPLAY_CONFIG"]==Path.Combine(r,"private/native-settings/runtime.conf")&&info.EnvironmentVariables["TSFP_DISPLAY_CONFIG"]==Path.Combine(r,"config/pc-display.ini"),"Display routes wrong");Need(!info.EnvironmentVariables.ContainsKey("RECOMP_TEST_STALE"),"Inherited override leaked");Need(File.ReadAllText(Path.Combine(r,"private/native-settings/runtime.conf")).Contains("future_setting = retained"),"Unsupported setting lost");Environment.SetEnvironmentVariable("RECOMP_TEST_STALE",null);});
                Test("binding conflicts, reserved keys and clear are shared",()=>{Need(GameSettings.WheelUpKey==134 && GameSettings.WheelDownKey==135 && GameSettings.BindingLabel("key:F24")=="Wheel down" && GameSettings.CaptureKey(162)==17,"Native capture differs from shared bindings");var m=GameSettings.Load(NewRoot("bindings"),null);m.Assign(0,0,65,true);Need(m.Conflict(1,65)==0,"Conflict missing");Reject(()=>m.Assign(1,0,65,false));m.Assign(1,0,65,true);Need(m.Binding(0)[0]=="none","Move left old key");Reject(()=>m.Assign(1,0,113,true));m.Assign(1,0,0,true);Need(m.Binding(1)[0]=="none","Clear failed");});
                Test("native update, failed transaction recovery and rollback preserve personal files",()=>{var r=NewRoot("updates");Write(r,"Saves/keep","save");Write(r,"Data/keep","data");Write(r,"Settings/pc-controls.json","personal");var a=Core.Unpack(Archive(r,"v1.0.0"),Path.Combine(r,"one"),"v1.0.0");Core.Apply(r,a,null);Need(File.ReadAllText(Path.Combine(r,"Settings/pc-controls.json"))=="personal","Controls overwritten");Need((File.GetUnixFileMode(Path.Combine(r,LauncherPlatform.GameBinary))&UnixFileMode.UserExecute)!=0,"Execute bit absent");var b=Core.Unpack(Archive(r,"v1.1.0"),Path.Combine(r,"two"),"v1.1.0");Reject(()=>Core.Apply(r,b,i=>{if(i==1)throw new IOException("synthetic failure");}));Need(Core.InstalledVersion(r)=="v1.0.0","Failure did not recover");Core.Apply(r,b,null);Core.Rollback(r);Need(Core.InstalledVersion(r)=="v1.0.0","Rollback failed");Need(File.ReadAllText(Path.Combine(r,"Saves/keep"))=="save"&&File.ReadAllText(Path.Combine(r,"Data/keep"))=="data","Personal files changed");});
                Test("corrupt packages and unsafe paths are rejected",()=>{var r=NewRoot("invalid");Reject(()=>Core.Unpack(Archive(r,"v1.0.0",true),Path.Combine(r,"bad"),"v1.0.0"));foreach(var name in new[]{"../data","/outside","foo:bar","a\\b","CON","a/../b"})Reject(()=>Core.PathIn(r,name));});
                Test("linked installation directories cannot redirect writes",()=>{var r=NewRoot("links");var other=NewRoot("link-target");Directory.CreateSymbolicLink(Path.Combine(r,"Bin"),other);Reject(()=>Core.PathIn(r,"Bin/test"));Need(!Directory.EnumerateFileSystemEntries(other).Any(),"Link target changed");});
                Test("native launcher manifest verifies and installs independently",()=>{var r=NewRoot("self");Write(r,"BUILD.json","keep game");var changes=SelfUpdate.Unpack(Archive(r,"1.2.18",false,true),Path.Combine(r,"stage"),"1.2.18");Core.ApplyLauncher(r,changes,null);Need(SelfUpdate.Complete(r),"Self manifest incomplete");Need(File.ReadAllText(Path.Combine(r,"BUILD.json"))=="keep game","Self update changed game");});
                Test("wrong XBE and altered native executable fail read-only checks",()=>{var r=NewRoot("checks");Write(r,"Data/default.xbe","invalid");Reject(()=>NativeHost.CheckData(r));Write(r,LauncherPlatform.GameBinary,"invalid");Write(r,"BUILD.json",Json.Encode(new{version="v1.0.0",executable_sha256=new string('a',64)}));Reject(()=>NativeHost.CheckBuild(r));Need(!Directory.Exists(Path.Combine(r,"Saves")),"Check created saves");});
                Test("verified download and cached replay use shared integrity policy",()=>{var r=NewRoot("download");byte[] bytes=Encoding.UTF8.GetBytes("verified archive");var release=new Release{AssetUrl="https://github.com/"+ReleaseClient.Repository+"/releases/download/v1.0.0/a.zip",Digest="sha256:"+Hash(bytes),Size=bytes.Length};int calls=0;using(var c=new ReleaseClient(new Handler{Reply=q=>{calls++;return new HttpResponseMessage(HttpStatusCode.OK){Content=new ByteArrayContent(bytes)};}})){c.Download(release,Path.Combine(r,"one.zip"),null,CancellationToken.None,Path.Combine(r,"cache")).GetAwaiter().GetResult();c.Download(release,Path.Combine(r,"two.zip"),null,CancellationToken.None,Path.Combine(r,"cache")).GetAwaiter().GetResult();}Need(calls==1&&File.ReadAllBytes(Path.Combine(r,"two.zip")).SequenceEqual(bytes),"Cache replay failed");});
                Test("known issues exclude pull requests and closed bugs",()=>{bool more;var issues=KnownIssues.ParsePage("[{\"number\":1,\"title\":\"Live bug\",\"state\":\"open\",\"labels\":[{\"name\":\"bug\"}]},{\"number\":2,\"state\":\"closed\"},{\"pull_request\":{}}]",out more);Need(issues.Length==1&&issues[0].Number==1&&!more,"Wrong issue snapshot");});
                Console.WriteLine(count+" native launcher regression groups passed.");return 0;
            }catch(Exception ex){Console.Error.WriteLine(ex);return 1;}finally{if(Directory.Exists(sandbox))Directory.Delete(sandbox,true);}
        }
    }
}
