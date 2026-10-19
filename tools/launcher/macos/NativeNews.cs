using System;
using System.Threading;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
namespace TimeSplitters.Launcher
{
    public sealed class NativeNews:IDisposable
    {
        public readonly Control View;readonly ReleaseClient client;readonly string root;readonly Func<bool> preview;
        readonly StackPanel body=new StackPanel(),issues=new StackPanel();readonly TextBlock status,issueStatus;
        bool launcher;readonly Button gameTab,launcherTab;readonly Button releaseLink;readonly ComboBox edition=new ComboBox{ItemsSource=new[]{"Available release","Installed release"},SelectedIndex=0};
        readonly CancellationTokenSource lifetime=new CancellationTokenSource();ReleaseNews news;KnownIssues known;bool loading,issuesLoading;
        public NativeNews(string root,ReleaseClient client,Func<bool> preview)
        {
            this.root=root;this.client=client;this.preview=preview;
            var layout=new Grid{RowDefinitions=new RowDefinitions("Auto,*,Auto"),Margin=new Thickness(30,24,30,20)};
            var heading=NativeWindow.Heading("RELEASE CENTER","What's new","Published changes for your game and launcher, all in one place.");heading.Margin=new Thickness(0);layout.Children.Add(heading);
            var tabs=new WrapPanel{Margin=new Thickness(0,10,0,6)};gameTab=NativeWindow.Button("Game releases",()=>Select(false));launcherTab=NativeWindow.Button("Launcher releases",()=>Select(true));tabs.Children.Add(gameTab);tabs.Children.Add(launcherTab);tabs.Children.Add(edition);heading.Children.Add(tabs);edition.SelectionChanged+=(s,e)=>Render();
            var panel=new StackPanel();panel.Children.Add(body);var knownPanel=Section(panel,"Known issues");knownPanel.Children.Add(NativeWindow.Text("Current open GitHub reports labeled bug, across all versions."));knownPanel.Children.Add(issues);
            issueStatus=NativeWindow.Text("Open this page to check the bug tracker.",12);knownPanel.Children.Add(issueStatus);var issueActions=new WrapPanel();issueActions.Children.Add(NativeWindow.Button("Refresh issues",async()=>await RefreshIssues()));issueActions.Children.Add(NativeWindow.Button("View bug tracker ↗",()=>NativeHost.Open(KnownIssues.TrackerUrl)));knownPanel.Children.Add(issueActions);
            var scroll=NativeWindow.Scroll(panel);Grid.SetRow(scroll,1);layout.Children.Add(scroll);
            var footer=new StackPanel{Margin=new Thickness(0,12,0,0)};var actions=new WrapPanel();actions.Children.Add(NativeWindow.Button("Refresh notes",async()=>await Refresh()));releaseLink=NativeWindow.Button("View release on GitHub ↗",()=>{var r=Selected();if(r!=null)NativeHost.Open(r.Url);});actions.Children.Add(releaseLink);footer.Children.Add(actions);status=NativeWindow.Text("Refresh to load published release notes.",12);footer.Children.Add(status);Grid.SetRow(footer,2);layout.Children.Add(footer);View=layout;Render();
        }
        static StackPanel Section(StackPanel parent,string title){var p=new StackPanel{Spacing=6};p.Children.Add(NativeWindow.Text(title,20));parent.Children.Add(new Border{Child=p,Padding=new Thickness(20),Margin=new Thickness(0,0,0,16),CornerRadius=new CornerRadius(6),Background=Brush.Parse("#14283B"),BorderBrush=Brush.Parse("#2D455C"),BorderThickness=new Thickness(1)});return p;}
        void Select(bool value){launcher=value;edition.SelectedIndex=news!=null&&(launcher?news.Launcher:news.Game)==null&&(launcher?news.InstalledLauncher:news.InstalledGame)!=null?1:0;Render();}
        Release Selected(){return news==null?null:edition.SelectedIndex==0?(launcher?news.Launcher:news.Game):(launcher?news.InstalledLauncher:news.InstalledGame);}

        public void Dispose(){lifetime.Cancel();lifetime.Dispose();}
        public async Task Refresh()
        {
            if(loading)return;loading=true;status.Text="Checking published release notes…";
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){timeout.CancelAfter(TimeSpan.FromSeconds(15));news=await client.ReadNews(preview(),Core.InstalledVersion(root),LauncherVersion.Value,timeout.Token);if(lifetime.IsCancellationRequested)return;status.Text="Checked "+news.CheckedUtc.ToLocalTime().ToString("g");if((launcher?news.Launcher:news.Game)==null&&(launcher?news.InstalledLauncher:news.InstalledGame)!=null)edition.SelectedIndex=1;Render();}}
            catch(Exception){if(!lifetime.IsCancellationRequested)status.Text=news==null?"Could not load release notes. Installed play remains available.":"Could not refresh. Showing the previous release notes.";}
            finally{loading=false;}await RefreshIssues();
        }
        public Task RefreshIssuesIfDue(){return known==null||DateTime.UtcNow-known.CheckedUtc.ToUniversalTime()>TimeSpan.FromMinutes(5)?RefreshIssues():Task.CompletedTask;}
        public async Task RefreshIssues()
        {
            if(issuesLoading||lifetime.IsCancellationRequested)return;issuesLoading=true;
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){timeout.CancelAfter(TimeSpan.FromSeconds(25));var next=await client.ReadKnownIssues(timeout.Token);if(lifetime.IsCancellationRequested)return;known=next;issues.Children.Clear();
                if(known.Items.Length==0)issues.Children.Add(NativeWindow.Text("No open issues labeled bug were returned. This does not mean every version is bug-free."));
                foreach(var issue in known.Items){var button=NativeWindow.Button("",()=>NativeHost.Open(issue.Url));var text=NativeWindow.Text("#"+issue.Number+"  "+issue.Title,13);text.Margin=new Thickness(0);button.Content=text;button.HorizontalAlignment=HorizontalAlignment.Stretch;button.HorizontalContentAlignment=HorizontalAlignment.Left;issues.Children.Add(button);}
                issueStatus.Text=known.Items.Length+" open bug reports · Checked "+known.CheckedUtc.ToLocalTime().ToString("g");}}
            catch(Exception){if(!lifetime.IsCancellationRequested)issueStatus.Text=known==null?"Could not check GitHub. Refresh or open the bug tracker.":"Could not refresh. Showing issues checked "+known.CheckedUtc.ToLocalTime().ToString("g")+"; reports may have changed.";}
            finally{issuesLoading=false;}
        }
        void Render()
        {
            string installed=launcher?LauncherVersion.Value:Core.InstalledVersion(root);Release latest=news==null?null:launcher?news.Launcher:news.Game;var release=Selected();body.Children.Clear();
            gameTab.Background=Brush.Parse(launcher?"#1C3349":"#294C69");launcherTab.Background=Brush.Parse(launcher?"#294C69":"#1C3349");releaseLink.IsEnabled=release!=null;
            var summary=Section(body,"Version overview");summary.Children.Add(NativeWindow.Text(preview()?"PREVIEW CHANNEL · includes published test builds":"STABLE CHANNEL · published stable releases",11));
            var grid=new Grid{ColumnDefinitions=new ColumnDefinitions("*,*")};var left=new StackPanel();left.Children.Add(NativeWindow.Text("INSTALLED",10));left.Children.Add(NativeWindow.Text(installed.Length==0?NativeHost.Version(root):installed,21));grid.Children.Add(left);var right=new StackPanel();right.Children.Add(NativeWindow.Text("AVAILABLE",10));right.Children.Add(NativeWindow.Text(news==null?"Not checked":latest?.Tag??"None on channel",21));Grid.SetColumn(right,1);grid.Children.Add(right);summary.Children.Add(grid);summary.Children.Add(NativeWindow.Text(news==null?"Refresh to check published releases.":ReleaseNews.Availability(installed,latest,launcher)));
            var changes=Section(body,"What this release changes");if(release==null){changes.Children.Add(NativeWindow.Text("No compatible native Mac release notes are available for this selection."));return;}
            changes.Children.Add(NativeWindow.Text((launcher?"Launcher ":"Game ")+release.Tag,18));DateTime date;if(DateTime.TryParse(release.Published,out date))changes.Children.Add(NativeWindow.Text("Published "+date.ToString("yyyy-MM-dd"),11));var notes=new ReleaseNotesText(release.Notes);NativeMarkdown.Render(changes,notes.Changes);
            if(notes.KnownIssues.Length>0){changes.Children.Add(NativeWindow.Text("Published limitations for this release",18));NativeMarkdown.Render(changes,notes.KnownIssues);}
            if(notes.Shortened)changes.Children.Add(NativeWindow.Text("Notes shortened. View the complete release on GitHub."));
        }
    }
}
