using System;
using System.Threading;
using System.Threading.Tasks;
using Avalonia.Controls;
namespace TimeSplitters.Launcher
{
    public sealed class NativeVersions
    {
        readonly GameVersionsPage page=new GameVersionsPage(NativeHost.Open);
        public Control View { get { return page.View; } }
        readonly string root;readonly ReleaseClient client;readonly Func<bool> blocked;
        int nextPage=1;bool loading;
        public NativeVersions(string root,ReleaseClient client,Func<bool> blocked,Func<Release,Task> install)
        {
            this.root=root;this.client=client;this.blocked=blocked;
            page.SetInstalled(Core.InstalledVersion(root));
            page.Older.Click+=(s,e)=>Run(()=>Load(false));page.Refresh.Click+=(s,e)=>Run(()=>Load(true));
            page.Install.Click+=(s,e)=>Run(async()=>{if(page.Selected!=null)await install(page.Selected);});
            page.OnGitHub.Click+=(s,e)=>{if(page.Selected!=null)NativeHost.Open(page.Selected.Url);};Update();
        }
        void Update(){page.SetInstalled(Core.InstalledVersion(root));page.SetBusy(blocked(),LauncherPlatform.Checkout(root));}
        async void Run(Func<Task> action){try{await action();}catch(Exception e){page.Status(e.Message);}finally{Update();}}
        public async Task Load(bool reset)
        {
            if(loading||blocked())return;loading=true;page.SetLoading(true);
            try{using(var token=new CancellationTokenSource(TimeSpan.FromSeconds(15))){
                var result=await client.ReadGameVersions(reset?1:nextPage,token.Token);page.SetReleases(result,reset);nextPage=reset?2:nextPage+1;
            }}catch(Exception){page.Failed();}finally{loading=false;Update();}
        }
    }
}
