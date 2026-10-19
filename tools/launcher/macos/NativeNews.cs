using System;
using System.Threading;
using System.Threading.Tasks;
using Avalonia.Controls;
namespace TimeSplitters.Launcher
{
    // Host lifetime/network scheduling only; all presentation is shared with Windows.
    public sealed class NativeNews:IDisposable
    {
        readonly WhatsNewPage page=new WhatsNewPage(NativeHost.Open);
        public Control View { get { return page.View; } }
        readonly ReleaseClient client;readonly string root;readonly Func<bool> preview;
        readonly CancellationTokenSource lifetime=new CancellationTokenSource();
        KnownIssues known;bool loading,issuesLoading;
        public NativeNews(string root,ReleaseClient client,Func<bool> preview)
        {
            this.root=root;this.client=client;this.preview=preview;
            page.SetVersions(Core.InstalledVersion(root),LauncherVersion.Value,preview());
            page.Refresh.Click+=async(s,e)=>await Refresh();
            page.RefreshIssues.Click+=async(s,e)=>await RefreshIssues();
            page.OnGitHub.Click+=(s,e)=>{if(page.SelectedUrl!=null)NativeHost.Open(page.SelectedUrl);};
        }
        public void Dispose(){lifetime.Cancel();lifetime.Dispose();}
        public async Task Refresh()
        {
            if(loading||lifetime.IsCancellationRequested)return;
            loading=true;page.SetVersions(Core.InstalledVersion(root),LauncherVersion.Value,preview());page.SetLoading(true);
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){
                timeout.CancelAfter(TimeSpan.FromSeconds(15));var news=await client.ReadNews(preview(),Core.InstalledVersion(root),LauncherVersion.Value,timeout.Token);
                if(!lifetime.IsCancellationRequested)page.SetNews(news);
            }}catch(Exception){if(!lifetime.IsCancellationRequested)page.Failed();}
            finally{loading=false;}await RefreshIssues();
        }
        public Task RefreshIssuesIfDue(){return known==null||DateTime.UtcNow-known.CheckedUtc.ToUniversalTime()>TimeSpan.FromMinutes(5)?RefreshIssues():Task.CompletedTask;}
        public async Task RefreshIssues()
        {
            if(issuesLoading||lifetime.IsCancellationRequested)return;issuesLoading=true;page.SetIssuesLoading();
            try{using(var timeout=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)){
                timeout.CancelAfter(TimeSpan.FromSeconds(25));var next=await client.ReadKnownIssues(timeout.Token);
                if(!lifetime.IsCancellationRequested){known=next;page.SetIssues(next);}
            }}catch(Exception){if(!lifetime.IsCancellationRequested)page.IssuesFailed();}
            finally{issuesLoading=false;}
        }
    }
}
