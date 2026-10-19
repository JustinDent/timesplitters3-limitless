using System;
using System.Threading;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
namespace TimeSplitters.Launcher
{
    public sealed class NativeNews:IDisposable
    {
        public readonly Control View;readonly ReleaseClient client;readonly string root;readonly Func<bool> preview;
        readonly StackPanel body=new StackPanel(),issues=new StackPanel();readonly TextBlock status,issueStatus;
        readonly ComboBox component=new ComboBox{ItemsSource=new[]{"Game releases","Launcher releases"},SelectedIndex=0},edition=new ComboBox{ItemsSource=new[]{"Available release","Installed release"},SelectedIndex=0};
        readonly CancellationTokenSource lifetime=new CancellationTokenSource();ReleaseNews news;KnownIssues known;bool loading,issuesLoading;
        public NativeNews(string root,ReleaseClient client,Func<bool> preview)
        {
            this.root=root;this.client=client;this.preview=preview;var panel=NativeWindow.Heading("RELEASE CENTER","What's new","Published changes for your game and launcher, all in one place.");
            var tabs=new StackPanel{Orientation=Orientation.Horizontal,Spacing=12};tabs.Children.Add(component);tabs.Children.Add(edition);panel.Children.Add(tabs);panel.Children.Add(body);
            component.SelectionChanged+=(s,e)=>Render();edition.SelectionChanged+=(s,e)=>Render();
            panel.Children.Add(NativeWindow.Text("Known issues",22));panel.Children.Add(NativeWindow.Text("Current open GitHub reports labeled bug, across all versions."));panel.Children.Add(issues);
            issueStatus=NativeWindow.Text("Open this page to check the bug tracker.",12);panel.Children.Add(issueStatus);
            panel.Children.Add(NativeWindow.Button("Refresh issues",async()=>await RefreshIssues()));panel.Children.Add(NativeWindow.Button("View bug tracker ↗",()=>NativeHost.Open(KnownIssues.TrackerUrl)));
            status=NativeWindow.Text("Refresh to load published release notes.",12);panel.Children.Add(status);panel.Children.Add(NativeWindow.Button("Refresh notes",async()=>await Refresh()));View=NativeWindow.Scroll(panel);Render();
        }
        public void Dispose(){lifetime.Cancel();lifetime.Dispose();}
        public async Task Refresh()
        {
            if(loading)return;loading=true;status.Text="Checking published release notes…";
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){timeout.CancelAfter(TimeSpan.FromSeconds(15));news=await client.ReadNews(preview(),Core.InstalledVersion(root),LauncherVersion.Value,timeout.Token);if(lifetime.IsCancellationRequested)return;status.Text="Checked "+news.CheckedUtc.ToLocalTime().ToString("g");Render();}}
            catch(Exception){if(!lifetime.IsCancellationRequested)status.Text=news==null?"Could not load release notes. Installed play remains available.":"Could not refresh. Showing the previous release notes.";}
            finally{loading=false;}await RefreshIssues();
        }
        public async Task RefreshIssues()
        {
            if(issuesLoading||lifetime.IsCancellationRequested)return;issuesLoading=true;
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){timeout.CancelAfter(TimeSpan.FromSeconds(25));var next=await client.ReadKnownIssues(timeout.Token);if(lifetime.IsCancellationRequested)return;known=next;issues.Children.Clear();
                if(known.Items.Length==0)issues.Children.Add(NativeWindow.Text("No open issues labeled bug were returned. This does not mean every version is bug-free."));
                foreach(var issue in known.Items)issues.Children.Add(NativeWindow.Button("#"+issue.Number+"  "+issue.Title,()=>NativeHost.Open(issue.Url)));
                issueStatus.Text=known.Items.Length+" open bug reports · Checked "+known.CheckedUtc.ToLocalTime().ToString("g");}}
            catch(Exception){if(!lifetime.IsCancellationRequested)issueStatus.Text=known==null?"Could not check GitHub. Refresh or open the bug tracker.":"Could not refresh. Showing issues checked "+known.CheckedUtc.ToLocalTime().ToString("g")+"; reports may have changed.";}
            finally{issuesLoading=false;}
        }
        void Render()
        {
            bool launcher=component.SelectedIndex==1;string installed=launcher?LauncherVersion.Value:Core.InstalledVersion(root);Release latest=news==null?null:launcher?news.Launcher:news.Game;
            var release=edition.SelectedIndex==0?latest:news==null?null:launcher?news.InstalledLauncher:news.InstalledGame;body.Children.Clear();
            body.Children.Add(NativeWindow.Text("Version overview",20));body.Children.Add(NativeWindow.Text(preview()?"PREVIEW CHANNEL":"STABLE CHANNEL",11));
            body.Children.Add(NativeWindow.Text("Installed: "+(installed.Length==0?NativeHost.Version(root):installed)+"     Available: "+(latest?.Tag??"None on channel"),15));
            body.Children.Add(NativeWindow.Text(ReleaseNews.Availability(installed,latest,launcher)));body.Children.Add(NativeWindow.Text("What this release changes",20));
            if(release==null){body.Children.Add(NativeWindow.Text("No compatible native Mac release notes are available for this selection."));return;}
            body.Children.Add(NativeWindow.Text((launcher?"Launcher ":"Game ")+release.Tag,18));var notes=new ReleaseNotesText(release.Notes);Markdown(body,notes.Changes);
            if(notes.KnownIssues.Length>0){body.Children.Add(NativeWindow.Text("Published limitations for this release",18));Markdown(body,notes.KnownIssues);}
            if(notes.Shortened)body.Children.Add(NativeWindow.Text("Notes shortened. View the complete release on GitHub."));body.Children.Add(NativeWindow.Button("View release on GitHub ↗",()=>NativeHost.Open(release.Url)));
        }
        static void Markdown(StackPanel panel,string markdown)
        {
            foreach(var raw in markdown.Split('\n')){var line=raw.Trim();if(line.Length==0)continue;int hashes=0;while(hashes<line.Length&&line[hashes]=='#')hashes++;bool heading=hashes>0&&hashes<7&&line.Length>hashes&&line[hashes]==' ';
                panel.Children.Add(NativeWindow.Text(ReleaseNotesText.Plain(heading?line.Substring(hashes+1):line),heading?18:13));}
        }
    }
}
