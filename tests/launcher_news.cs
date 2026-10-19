using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Navigation;
using TimeSplitters.Launcher;

public static class LauncherNewsChecks
{
    static int passed;
    static void Need(bool value){if(!value)throw new Exception("Release news assertion failed.");}
    static void Test(string name,Action body){body();passed++;Console.WriteLine("PASS "+name);}
    sealed class Handler : HttpMessageHandler {
        public Func<HttpRequestMessage,HttpResponseMessage> Reply;
        protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request,CancellationToken token){token.ThrowIfCancellationRequested();return Task.FromResult(Reply(request));}
    }
    sealed class StalledHandler : HttpMessageHandler {
        public readonly TaskCompletionSource<HttpResponseMessage> Pending=new TaskCompletionSource<HttpResponseMessage>();
        protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request,CancellationToken token){return Pending.Task;}
    }
    static HttpResponseMessage Response(object value){return new HttpResponseMessage(HttpStatusCode.OK){Content=new StringContent(Json.Encode(value))};}
    static object Record(string version,bool launcher=false,bool preview=false,bool draft=false)
    {
        string tag=launcher?"launcher-v"+version:version;
        string name=launcher?"TimeSplitters-Launcher-v"+version+"-windows-x64.zip":"TimeSplitters-Future-Perfect-PC-"+version+"-windows-x64.zip";
        string prefix="https://github.com/"+ReleaseClient.Repository+"/releases/download/"+tag+"/";
        var assets=new List<object>{new{name=name,state="uploaded",size=100,digest="sha256:"+new string('a',64),browser_download_url=prefix+name}};
        if(launcher)assets.Add(new{name="launcher-update.json",state="uploaded",browser_download_url=prefix+"launcher-update.json"});
        return new {tag_name=tag,name="Release "+version,body="## Changes\n- Release "+version+"\n## Known issues\n- Published limitation.",published_at="2026-10-17T12:00:00Z",prerelease=preview,draft=draft,assets=assets};
    }
    static object Metadata(string version){return new {schema=1,version=version,asset_name="TimeSplitters-Launcher-v"+version+"-windows-x64.zip",size=100,sha256=new string('a',64)};}
    static IEnumerable<T> All<T>(DependencyObject root) where T:DependencyObject
    {foreach(var child in LogicalTreeHelper.GetChildren(root).OfType<DependencyObject>()){if(child is T)yield return (T)child;foreach(var descendant in All<T>(child))yield return descendant;}}
    static ReleaseNews Read(ReleaseClient client,bool preview=false){return client.ReadNews(preview,"v1.0.0","1.0.0",CancellationToken.None).GetAwaiter().GetResult();}
    public static int Run()
    {
        Exception failure=null;var thread=new Thread(()=>{try{RunCore();}catch(Exception ex){failure=ex;}});
        thread.SetApartmentState(ApartmentState.STA);thread.IsBackground=true;thread.Start();Need(thread.Join(60000));if(failure!=null)throw failure;
        Console.WriteLine("PASS: "+passed+" isolated launcher release news checks.");return 0;
    }
    static void RunCore()
    {
        var records=new[]{Record("v1.0.0"),Record("v1.1.0"),Record("v1.2.0-beta.1",false,true),Record("v9.0.0",false,false,true),Record("1.0.0",true),Record("1.1.0",true)};
        ReleaseNews news=null;
        Test("release news uses update selection, verifies launcher metadata and retains exact installed notes",()=>{
            int requests=0;using(var client=new ReleaseClient(new Handler {Reply=r=>{requests++;Need(r.Method==HttpMethod.Get);return Response(r.RequestUri.Host=="api.github.com"?(object)records:Metadata("1.1.0"));}}))news=Read(client);
            Need(requests==2 && news.Game.Tag=="v1.1.0" && news.Launcher.Tag=="1.1.0" && news.InstalledGame.Tag=="v1.0.0" && news.InstalledLauncher.Tag=="1.0.0");
            Need(news.Launcher.Notes.Contains("Published limitation") && news.Launcher.Url.EndsWith("launcher-v1.1.0") && news.Game.Name=="Release v1.1.0" && news.CheckedUtc.Kind==DateTimeKind.Utc);
        });
        Test("preview news includes prereleases while stable and drafts remain filtered",()=>{
            using(var client=new ReleaseClient(new Handler {Reply=r=>Response(r.RequestUri.Host=="api.github.com"?(object)records:Metadata("1.1.0"))}))Need(Read(client,true).Game.Tag=="v1.2.0-beta.1");
        });
        Test("installed notes outside latest 100 are fetched by exact tag only",()=>{
            var paths=new List<string>();using(var client=new ReleaseClient(new Handler {Reply=r=>{
                paths.Add(r.RequestUri.AbsolutePath);return Response(r.RequestUri.AbsolutePath.EndsWith("/releases")?(object)new[]{Record("v1.1.0")}:
                    r.RequestUri.AbsolutePath.EndsWith("/v1.0.0")?Record("v1.0.0"):Record("1.0.0",true));
            }})) {var result=Read(client);Need(result.InstalledGame.Tag=="v1.0.0" && result.InstalledLauncher.Tag=="1.0.0");}
            Need(paths.Count==3 && paths[1].EndsWith("/tags/v1.0.0") && paths[2].EndsWith("/tags/launcher-v1.0.0"));
        });
        Test("unavailable or mismatched installed notes never substitute another release",()=>{
            using(var client=new ReleaseClient(new Handler {Reply=r=>r.RequestUri.AbsolutePath.EndsWith("/releases")?Response(new[]{Record("v1.1.0")}):new HttpResponseMessage(HttpStatusCode.NotFound)})){var result=Read(client);Need(result.InstalledGame==null && result.InstalledLauncher==null);}
            using(var client=new ReleaseClient(new Handler {Reply=r=>Response(r.RequestUri.AbsolutePath.EndsWith("/releases")?(object)new[]{Record("v1.1.0")}:Record("v99.0.0"))}))Need(Read(client).InstalledGame==null);
        });
        Test("invalid launcher metadata fails news refresh instead of advertising an unverified update",()=>{
            bool rejected=false;using(var client=new ReleaseClient(new Handler {Reply=r=>Response(r.RequestUri.Host=="api.github.com"?(object)records:Metadata("9.0.0"))}))try{Read(client);}catch(InvalidDataException){rejected=true;}Need(rejected);
        });
        Test("cancelled and rate limited news requests can be retried without installation",()=>{
            using(var client=new ReleaseClient(new Handler {Reply=r=>new HttpResponseMessage(HttpStatusCode.Forbidden)})){
                bool failed=false;try{Read(client);}catch(IOException){failed=true;}Need(failed);
                var stop=new CancellationTokenSource();stop.Cancel();failed=false;try{client.ReadNews(false,"","1.0.0",stop.Token).GetAwaiter().GetResult();}catch(OperationCanceledException){failed=true;}Need(failed);
            }
        });
        Test("release note deadline also bounds a request that ignores cancellation",()=>{
            var handler=new StalledHandler();using(var client=new ReleaseClient(handler)){
                var clock=System.Diagnostics.Stopwatch.StartNew();bool failed=false;
                try{client.ReadNewsWithTimeout(false,"","1.0.0",TimeSpan.FromMilliseconds(30),CancellationToken.None).GetAwaiter().GetResult();}catch(TimeoutException){failed=true;}
                Need(failed && clock.ElapsedMilliseconds<2000);handler.Pending.SetException(new IOException("Late synthetic failure"));
            }
        });
        Test("known issue sections end at a peer heading and preserve later installation notes",()=>{
            var text=new ReleaseNotesText("# Changes\nFixed a bug.\n## Known issues\nA limitation.\n### Workaround\nUse this setting.\n## Installation\nKeep your saves.");
            Need(text.KnownIssues.Contains("A limitation") && text.KnownIssues.Contains("Workaround") && !text.KnownIssues.Contains("Keep your saves") && text.Changes.Contains("Keep your saves"));
        });
        Test("code examples do not become published known issues and missing sections stay unknown",()=>{
            var text=new ReleaseNotesText("```text\n## Known issues\nexample\n```\n## Improvements\nChanges");Need(text.KnownIssues=="" && text.Changes.Contains("example"));Need(new ReleaseNotesText(null).KnownIssues=="");
        });
        Test("release text is bounded with an explicit shortened indicator",()=>{
            Need(new ReleaseNotesText(new string('x',65000)).Shortened);Need(new ReleaseNotesText(String.Join("\n",Enumerable.Repeat("line",600))).Shortened);
            Need(ReleaseNotesText.Plain("![picture](https://invalid.example/image.png)").Contains("Image: view on GitHub"));
        });
        Test("availability distinguishes newer, equal, older, missing and unversioned installs",()=>{
            Need(ReleaseNews.Availability("v1.0.0",news.Game,false).Contains("newer game"));Need(ReleaseNews.Availability("1.1.0",news.Launcher,true).Contains("up to date"));
            Need(ReleaseNews.Availability("v9.0.0",news.Game,false).Contains("No downgrade"));Need(ReleaseNews.Availability("",news.Game,false).Contains("Available to install"));Need(ReleaseNews.Availability("dev",news.Game,false).Contains("cannot be compared"));Need(ReleaseNews.Availability("v1.0.0",null,false).Contains("No compatible"));
        });
        Test("native Markdown formats headings, emphasis, lists, quotes and literal code",()=>{
            var view=new ReleaseNotesMarkdown();
            view.SetMarkdown("# Release **highlights**\n\nA **bold** fix and *italic* note with `file_name.exe`.\n\n- First\n- Second\n\n1. Install\n2. Play\n\n> Keep saves.\n\n```text\n[not a link](https://example.com) **literal**\n```\n");
            var doc=view.Document;string text=new TextRange(doc.ContentStart,doc.ContentEnd).Text;
            Need(doc.Blocks.FirstBlock.FontSize==20 && All<Bold>(doc).Any() && All<Italic>(doc).Any());
            Need(doc.Blocks.OfType<System.Windows.Documents.List>().Count()==2 && All<Hyperlink>(doc).Count()==0);
            Need(text.Contains("file_name.exe") && text.Contains("**literal**") && text.Contains("Keep saves.") && !text.Contains("# Release"));
            Need(view.IsReadOnly && view.IsDocumentEnabled);
        });
        Test("Markdown links show labels and navigate only after an explicit activation",()=>{
            string opened=null;var view=new ReleaseNotesMarkdown(url=>opened=url);
            view.SetMarkdown("Fix ([#106](https://github.com/TheDoffman/timesplitters-future-perfect-pc/pull/106)).\n\nhttps://example.com/notes\n\n[With parentheses](https://example.com/a_(b))");
            var links=All<Hyperlink>(view.Document).ToArray();Need(links.Length==3 && opened==null);
            Need(new TextRange(links[0].ContentStart,links[0].ContentEnd).Text=="#106");
            Need(links[2].NavigateUri.AbsoluteUri.EndsWith("/a_(b)"));
            links[0].RaiseEvent(new RequestNavigateEventArgs(links[0].NavigateUri,null){RoutedEvent=Hyperlink.RequestNavigateEvent});
            Need(opened=="https://github.com/TheDoffman/timesplitters-future-perfect-pc/pull/106");
        });
        Test("Markdown rejects executable links and keeps remote images and markup inert",()=>{
            foreach(string url in new[]{"file:///C:/Windows/notepad.exe","javascript:alert(1)","data:text/html,hello","ms-settings:display","https://user:password@example.com","https://example.com\nattack"}){Uri parsed;Need(!ReleaseNotesMarkdown.TryLink(url,out parsed));}
            var view=new ReleaseNotesMarkdown(url=>{throw new Exception("Unexpected navigation");});
            view.SetMarkdown("[File](file:///C:/test.exe) [JS](javascript:alert(1)) ![image](https://example.com/remote.png)\n\n<Window><Button>inert</Button></Window>\n\n"+new string('x',65000));
            string text=new TextRange(view.Document.ContentStart,view.Document.ContentEnd).Text;
            Need(!All<Hyperlink>(view.Document).Any() && !All<Image>(view.Document).Any() && text.Contains("Image: view on GitHub") && text.Contains("<Window>") && text.Contains("Notes shortened"));
        });
        Test("game versions and both news sections use the same selectable Markdown links",()=>{
            string body="## Changes\nFix ([#106](https://github.com/TheDoffman/timesplitters-future-perfect-pc/pull/106)).\n\n## Known issues\nSee [workaround](https://example.com/help).";
            var release=new Release {Tag="v1.0.0",Notes=body};
            var versions=new GameVersionsPage();versions.SetReleases(new GameReleaseList {Releases=new[]{release}},true);
            var notes=All<ReleaseNotesMarkdown>(versions.View).Single();Need(All<Hyperlink>(notes.Document).Count()==2);
            var page=new WhatsNewPage();page.SetNews(new ReleaseNews {Game=release,CheckedUtc=DateTime.UtcNow});
            var sections=All<ReleaseNotesMarkdown>(page.View).ToArray();Need(sections.Length==2 && sections.All(section=>All<Hyperlink>(section.Document).Count()==1));
            page.SelectComponent(true);page.SelectComponent(false);Need(All<ReleaseNotesMarkdown>(page.View).Count()==2);
        });
        Test("native news page switches versions and keeps previous notes on refresh failure",()=>{
            var page=new WhatsNewPage();page.SetVersions("v1.0.0","1.0.0",false);page.SetNews(news);Need(page.SelectedUrl.EndsWith("/v1.1.0"));
            page.Edition.SelectedIndex=1;Need(page.SelectedUrl.EndsWith("/v1.0.0"));page.SelectComponent(true);Need(page.SelectedUrl.EndsWith("launcher-v1.1.0"));
            page.SetLoading(true);Need(!page.Refresh.IsEnabled);page.Failed();Need(page.Refresh.IsEnabled && page.StatusText.Contains("Showing notes checked") && page.SelectedUrl.EndsWith("launcher-v1.1.0"));
            page.Reset();Need(page.SelectedUrl==null && !page.OnGitHub.IsEnabled);page.Failed();Need(page.StatusText.Contains("installed game remains available"));
        });
        Test("remote HTML and images stay inert native text without creating active content",()=>{
            var page=new WhatsNewPage();page.SetVersions("v1.0.0","1.0.0",false);
            page.SetNews(new ReleaseNews {CheckedUtc=DateTime.UtcNow,Game=new Release {Tag="v1.0.0",Url=ReleaseClient.ReleasesPage,
                Notes="<script>alert('test')</script>\n<Window><Button>Injected</Button></Window>\n![Remote](https://invalid.example/image.png)"}});
            string text=String.Join("\n",All<ReleaseNotesMarkdown>(page.View).Select(t=>new TextRange(t.Document.ContentStart,t.Document.ContentEnd).Text));
            Need(text.Contains("<script>") && text.Contains("<Window>") && text.Contains("Image: view on GitHub"));
            Need(!All<WebBrowser>(page.View).Any() && !All<Image>(page.View).Any() && All<Button>(page.View).Count()==4);
        });
        Test("installed preview notes remain readable when stable has no compatible game release",()=>{
            var page=new WhatsNewPage();page.SetVersions("v1.0.0-beta.1","1.0.0",false);page.SetNews(new ReleaseNews {InstalledGame=new Release {Tag="v1.0.0-beta.1",Notes="Installed preview",Url=ReleaseClient.ReleasesPage},CheckedUtc=DateTime.UtcNow});
            Need(page.Edition.SelectedIndex==1 && page.SelectedUrl!=null && page.AvailabilityText.Contains("No compatible"));
        });
        Test("release notes stay in the main window, preserve settings drafts and aligned sidebar",()=>{
            string root=Path.Combine(Path.GetDirectoryName(typeof(LauncherNewsChecks).Assembly.Location),"news-ui-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
            var ui=new LauncherWindow(root,true);
            try {
                ui.ShowSettings();var settings=(ContentControl)ui.Window.FindName("SettingsHost");var draft=settings.Content;
                ((Button)ui.Window.FindName("ReleaseNotesButton")).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                var host=(ContentControl)ui.Window.FindName("NewsHost");Need(host.Visibility==Visibility.Visible && settings.Visibility==Visibility.Collapsed && Window.GetWindow((DependencyObject)host.Content)==ui.Window);
                Need(((Border)ui.Window.FindName("SidebarPanel")).Visibility==Visibility.Visible);
                foreach(string name in new[]{"HomeNav","NewsNav","SettingsNav","RepairNav","SavesNav","SupportNav"})Need(((Grid)((Button)ui.Window.FindName(name)).Content).ColumnDefinitions[0].Width.Value==24);
                ui.ShowMaintenance(false);Need(host.Visibility==Visibility.Collapsed);ui.ShowNews();ui.ShowSettings();Need(ReferenceEquals(settings.Content,draft) && host.Visibility==Visibility.Collapsed);
                Need(!Directory.GetFiles(root,"*",SearchOption.AllDirectories).Any());
            }finally{ui.Window.Close();Need(Path.GetDirectoryName(root)==Path.GetDirectoryName(typeof(LauncherNewsChecks).Assembly.Location) && (File.GetAttributes(root)&FileAttributes.ReparsePoint)==0);Directory.Delete(root,true);}
        });
    }
}
