using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
namespace TimeSplitters.Launcher
{
    public sealed class NativeVersions
    {
        public readonly Control View;readonly string root;readonly ReleaseClient client;readonly Func<bool> blocked;readonly Func<Release,Task> install;
        readonly Dictionary<string,Release> releases=new Dictionary<string,Release>();readonly ComboBox selector=new ComboBox{HorizontalAlignment=HorizontalAlignment.Stretch};readonly StackPanel notes=new StackPanel();readonly TextBlock status,details;readonly Button older,apply,refresh,link;int page=1;bool loading,more;
        Release Selected=>(selector.SelectedItem as ComboBoxItem)?.Tag as Release;
        public NativeVersions(string root,ReleaseClient client,Func<bool> blocked,Func<Release,Task> install){this.root=root;this.client=client;this.blocked=blocked;this.install=install;
            var layout=new Grid{RowDefinitions=new RowDefinitions("Auto,*,Auto"),Margin=new Thickness(30,24,30,20)};var head=NativeWindow.Heading("TESTING TOOLS","Game versions","Install a specific published release for comparisons and bug reproduction.");head.Margin=new Thickness(0);head.Children.Add(NativeWindow.Text("INSTALLED GAME  ·  "+NativeHost.Version(root),12));var row=new Grid{ColumnDefinitions=new ColumnDefinitions("*,Auto")};row.Children.Add(selector);older=NativeWindow.Button("Load older releases",()=>Run(()=>Load(false)));Grid.SetColumn(older,1);row.Children.Add(older);head.Children.Add(row);details=NativeWindow.Text("Refresh to load stable and preview releases with verified native Mac packages.",12);head.Children.Add(details);layout.Children.Add(head);
            var body=new StackPanel();body.Children.Add(NativeWindow.Text("RELEASE NOTES",10));body.Children.Add(notes);var card=new Border{Background=Brush.Parse("#14283B"),Padding=new Thickness(20),CornerRadius=new CornerRadius(6),Child=NativeWindow.Scroll(body)};Grid.SetRow(card,1);layout.Children.Add(card);
            var footer=new StackPanel{Margin=new Thickness(0,14,0,0)};footer.Children.Add(NativeWindow.Text("Switches the installed game only. Saves, settings, imported data and the launcher are kept. Back up saves before testing older builds: save compatibility can vary.",12));var actions=new WrapPanel();apply=NativeWindow.Button("Install selected version",()=>Run(async()=>{if(Selected!=null)await install(Selected);}));link=NativeWindow.Button("View release ↗",()=>{if(Selected!=null)NativeHost.Open(Selected.Url);});refresh=NativeWindow.Button("Refresh list",()=>Run(()=>Load(true)));actions.Children.Add(apply);actions.Children.Add(link);actions.Children.Add(refresh);footer.Children.Add(actions);status=NativeWindow.Text("Load the list to choose a release.",12);footer.Children.Add(status);Grid.SetRow(footer,2);layout.Children.Add(footer);selector.SelectionChanged+=(s,e)=>Render();View=layout;Render();
        }
        async void Run(Func<Task> action){try{await action();}catch(Exception e){status.Text=e.Message;}finally{Render();}}
        public async Task Load(bool reset){if(loading||blocked())return;loading=true;status.Text="Loading published game releases…";Render();
            try{using(var token=new CancellationTokenSource(TimeSpan.FromSeconds(15))){var result=await client.ReadGameVersions(reset?1:page,token.Token);string selected=Selected?.Tag??Core.InstalledVersion(root);if(reset)releases.Clear();foreach(var r in result.Releases)releases[r.Tag]=r;page=reset?2:page+1;more=result.HasMore;selector.Items.Clear();foreach(var r in releases.Values.OrderByDescending(r=>GameVersion.Parse(r.Tag))){var item=new ComboBoxItem{Content=r.Tag+"  ·  "+(r.IsPreview?"Preview":"Stable"),Tag=r};selector.Items.Add(item);if(r.Tag==selected)selector.SelectedItem=item;}if(selector.SelectedIndex<0&&selector.Items.Count>0)selector.SelectedIndex=0;status.Text=releases.Count==0?"No compatible native Mac game packages found on this page.":"Showing "+releases.Count+" published game versions. Choose one to install.";}}
            catch(Exception){status.Text="Could not load releases. Previously loaded versions and installed files were kept. Refresh to retry.";}
            finally{loading=false;Render();}
        }
        void Render(){bool enabled=!loading&&!blocked();selector.IsEnabled=enabled;refresh.IsEnabled=enabled;older.IsEnabled=enabled&&more;apply.IsEnabled=enabled&&Selected!=null&&!LauncherPlatform.Checkout(root);link.IsEnabled=Selected!=null;notes.Children.Clear();if(Selected==null)return;var r=Selected;apply.Content=r.Tag==Core.InstalledVersion(root)?"Reinstall selected version":"Install selected version";details.Text=r.Tag+"  ·  "+(r.IsPreview?"Preview":"Stable")+"  ·  "+(r.Size/1048576.0).ToString("0.0")+" MB";NativeMarkdown.Render(notes,String.IsNullOrWhiteSpace(r.Notes)?"No release notes were provided.":r.Notes);}
    }
}
