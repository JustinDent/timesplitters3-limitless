using System;
using System.IO;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform.Storage;
namespace TimeSplitters.Launcher
{
    public sealed class NativeMaintenance
    {
        public readonly Control View;readonly NativeWindow owner;readonly string root;readonly TabControl tabs=new TabControl();
        readonly TextBox report=new TextBox{IsReadOnly=true,AcceptsReturn=true,TextWrapping=TextWrapping.Wrap,FontFamily=new FontFamily("Menlo"),MinHeight=260};
        readonly TextBlock status=NativeWindow.Text("Choose Verify files to check the game or launcher.");VerificationResult game,launcher;
        static StackPanel Card(string title,string text){var p=new StackPanel{Spacing=8,Margin=new Thickness(18)};p.Children.Add(NativeWindow.Text(title,20));p.Children.Add(NativeWindow.Text(text,12));return p;}
        public NativeMaintenance(string root,NativeWindow owner){this.root=root;this.owner=owner;
            var layout=new Grid{RowDefinitions=new RowDefinitions("Auto,*,Auto"),Margin=new Thickness(30,24,30,20)};var heading=NativeWindow.Heading("INSTALLATION CARE","Repair / Verify","Check your installation, restore release files and collect diagnostics for support.");heading.Margin=new Thickness(0,0,0,12);layout.Children.Add(heading);Grid.SetRow(tabs,1);layout.Children.Add(tabs);Grid.SetRow(status,2);layout.Children.Add(status);
            var repair=new StackPanel{Spacing=16,Margin=new Thickness(0,0,12,0)};var cards=new Grid{ColumnDefinitions=new ColumnDefinitions("*,*")};
            for(int i=0;i<2;i++){bool self=i==1;var p=Card(self?"Launcher":"Game",self?"Check the independent launcher installation.":"Verify the installed runtime against its recorded hashes.");p.Children.Add(NativeWindow.Button("Verify files",()=>Run(async()=>{await owner.Maintain(async token=>{var result=await Task.Run(()=>self?InstallationMaintenance.Verify(root,true,null,token):NativeHost.Verify(root,token));if(self)launcher=result;else game=result;report.Text=result.ToString();status.Text=result.Summary;});})));p.Children.Add(NativeWindow.Button("Repair files",()=>Run(async()=>{if(await owner.Confirm("Repair "+(self?"launcher":"game")+"?","Restore files from the exact installed release. Saves, settings and the previous game rollback are preserved.")){await owner.Maintain(token=>owner.Repair(self,token));status.Text="Repair completed.";}})));var card=new Border{Child=p,Background=Brush.Parse("#14283B"),BorderBrush=Brush.Parse("#2D455C"),BorderThickness=new Thickness(1),CornerRadius=new CornerRadius(6),Margin=new Thickness(i==0?0:8,10,i==0?8:0,0)};Grid.SetColumn(card,i);cards.Children.Add(card);}repair.Children.Add(cards);
            repair.Children.Add(NativeWindow.Text("Personal files stay yours",18));repair.Children.Add(NativeWindow.Text("Verification reads release files. Repair preserves game data, saves, settings and the previous game rollback."));var repairActions=new WrapPanel();repairActions.Children.Add(NativeWindow.Button("Open game folder",()=>NativeHost.Open(root)));repairActions.Children.Add(NativeWindow.Button("Cancel current operation",owner.CancelMaintenance));repairActions.Children.Add(NativeWindow.Button("View logs",()=>Open(".launcher")));repair.Children.Add(repairActions);
            var diagnostics=new StackPanel{Spacing=10,Margin=new Thickness(0,12,0,0)};var actions=new WrapPanel();
            actions.Children.Add(NativeWindow.Button("Refresh diagnostics",()=>Run(Refresh)));actions.Children.Add(NativeWindow.Button("Copy report",()=>Run(async()=>{if(owner.Clipboard!=null)await owner.Clipboard.SetTextAsync(report.Text);})));actions.Children.Add(NativeWindow.Button("Report a bug",()=>Run(async()=>{if(!report.Text.StartsWith("TimeSplitters launcher diagnostics"))await Refresh();var draft=DiagnosticIssueDraft.Create(false,report.Text);if(owner.Clipboard!=null)await owner.Clipboard.SetTextAsync(draft.ClipboardText);NativeHost.Open(draft.Url);})));actions.Children.Add(NativeWindow.Button("Request an improvement",()=>NativeHost.Open(DiagnosticIssueDraft.Create(true,"").Url)));diagnostics.Children.Add(actions);diagnostics.Children.Add(report);
            var crashActions=new WrapPanel();crashActions.Children.Add(NativeWindow.Button("Open latest game report",()=>Run(async()=>{string latest=CrashReports.Latest(root);Core.Require(latest!=null,"No completed game report found.");NativeHost.Open(CrashReports.Folder(root,latest));await Task.CompletedTask;})));crashActions.Children.Add(NativeWindow.Button("Export game report…",()=>Run(Export)));diagnostics.Children.Add(crashActions);diagnostics.Children.Add(NativeWindow.Text("Report exports include native context and a recent log excerpt. Review them before sharing. Nothing is uploaded automatically.",12));
            tabs.Items.Add(new TabItem{Header="Repair / Verify",Content=NativeWindow.Scroll(repair)});tabs.Items.Add(new TabItem{Header="Diagnostics",Content=NativeWindow.Scroll(diagnostics)});View=layout;
        }
        public void Select(bool diagnostics){tabs.SelectedIndex=diagnostics?1:0;if(diagnostics)Run(Refresh);}
        void Open(string relative){string path=Core.PathIn(root,relative);if(Directory.Exists(path))NativeHost.Open(path);else status.Text="This folder has not been created yet.";}
        async void Run(Func<Task> action){try{await action();}catch(OperationCanceledException){status.Text="Operation cancelled. Existing files were preserved.";}catch(Exception e){status.Text=e.Message;}}
        Task Refresh(){report.Text=InstallationMaintenance.Report(root,game,launcher,NativeHost.Graphics());status.Text="Diagnostics ready. Nothing was uploaded.";return Task.CompletedTask;}
        async Task Export(){string latest=CrashReports.Latest(root);Core.Require(latest!=null,"No completed game report found.");var file=await owner.StorageProvider.SaveFilePickerAsync(new FilePickerSaveOptions{Title="Save game report — review before sharing",SuggestedFileName=latest+".zip",FileTypeChoices=new[]{new FilePickerFileType("ZIP archive"){Patterns=new[]{"*.zip"}}}});string path=file?.TryGetLocalPath();if(path==null)return;CrashReports.Export(root,latest,path);status.Text="Game report exported locally. Nothing was uploaded.";}
    }
}
