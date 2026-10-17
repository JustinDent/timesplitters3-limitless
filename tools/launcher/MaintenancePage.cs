using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TimeSplitters.Launcher
{
    public sealed class MaintenancePage
    {
        public readonly Grid View = new Grid();
        public readonly Button VerifyGame, VerifyLauncher, RepairGame, RepairLauncher, Cancel, Refresh, Copy, ReportBug, RequestEnhancement, OpenFiles;
        public readonly TextBox Report;
        readonly TextBlock status, gameStatus, launcherStatus;
        readonly ProgressBar progress;
        bool gameRepairable, launcherRepairable;
        static Brush Brush(string value) { return (Brush)new BrushConverter().ConvertFromString(value); }
        static TextBlock Text(string text,int size)
        { return new TextBlock { Text=text,FontSize=size,Foreground=Brush("#E4EDF5"),TextWrapping=TextWrapping.Wrap,Margin=new Thickness(0,0,0,10) }; }
        static Button Button(string text,string name)
        { return new Button { Content=text,Name=name,Padding=new Thickness(14,9,14,9),MinHeight=38,Margin=new Thickness(0,0,10,0) }; }
        static Border Card(UIElement child)
        { return new Border { Background=Brush("#152435"),BorderBrush=Brush("#36536F"),BorderThickness=new Thickness(1),CornerRadius=new CornerRadius(4),Padding=new Thickness(18),Child=child }; }
        public MaintenancePage(bool diagnostics)
        {
            View.Margin=new Thickness(30,24,30,20);
            View.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
            View.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
            View.RowDefinitions.Add(new RowDefinition { Height=new GridLength(1,GridUnitType.Star) });
            View.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
            var heading=new StackPanel();heading.Children.Add(Text(diagnostics?"Diagnostics":"Repair / Verify",26));
            var hint=Text(diagnostics?"Review the report below. Report buttons open a GitHub draft with diagnostics; you review and submit it there.":
                "Verify files without changing them. Repair downloads the exact installed release and replaces damaged release files.",12);
            hint.Foreground=Brush("#A4B8CA");heading.Children.Add(hint);View.Children.Add(heading);
            var actions=new Grid { Margin=new Thickness(0,10,0,16) };Grid.SetRow(actions,1);View.Children.Add(actions);
            if(diagnostics) {
                var buttons=new WrapPanel();Refresh=Button("Refresh diagnostics","RefreshDiagnostics");Copy=Button("Copy diagnostic report","CopyDiagnostics");
                ReportBug=Button("Report a bug ↗","ReportDiagnosticBug");
                RequestEnhancement=Button("Request enhancement ↗","RequestDiagnosticEnhancement");
                ReportBug.ToolTip="Copies this report and opens the GitHub bug form. You review and submit it there.";
                RequestEnhancement.ToolTip="Copies this report and opens the GitHub enhancement form. You review and submit it there.";
                buttons.Children.Add(Refresh);buttons.Children.Add(Copy);buttons.Children.Add(ReportBug);buttons.Children.Add(RequestEnhancement);actions.Children.Add(buttons);
            } else {
                actions.ColumnDefinitions.Add(new ColumnDefinition());actions.ColumnDefinitions.Add(new ColumnDefinition());
                actions.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
                actions.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
                for(int i=0;i<2;i++) {
                    var body=new StackPanel();body.Children.Add(Text(i==0?"Game runtime":"Launcher",18));
                    var summary=Text("Not checked this session.",12);summary.Foreground=Brush("#A4B8CA");body.Children.Add(summary);
                    var buttons=new WrapPanel();var verify=Button("Verify files",i==0?"VerifyGameFiles":"VerifyLauncherFiles");
                    var repair=Button("Repair files",i==0?"RepairGameFiles":"RepairLauncherFiles");repair.IsEnabled=false;
                    buttons.Children.Add(verify);buttons.Children.Add(repair);body.Children.Add(buttons);
                    var card=Card(body);card.Margin=i==0?new Thickness(0,0,8,0):new Thickness(8,0,0,0);Grid.SetColumn(card,i);actions.Children.Add(card);
                    if(i==0){VerifyGame=verify;RepairGame=repair;gameStatus=summary;}else{VerifyLauncher=verify;RepairLauncher=repair;launcherStatus=summary;}
                }
                OpenFiles=Button("Open game folder ↗","OpenGameFiles");
                OpenFiles.ToolTip="Open the installation folder to inspect game files.";
                OpenFiles.HorizontalAlignment=HorizontalAlignment.Left;OpenFiles.Margin=new Thickness(0,12,0,0);
                Grid.SetRow(OpenFiles,1);Grid.SetColumnSpan(OpenFiles,2);actions.Children.Add(OpenFiles);
            }
            Report=new TextBox { Name=diagnostics?"DiagnosticsReport":"VerificationReport",IsReadOnly=true,AcceptsReturn=true,TextWrapping=TextWrapping.Wrap,
                VerticalScrollBarVisibility=ScrollBarVisibility.Auto,HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled,
                Background=Brush("#0B1420"),Foreground=Brush("#DFE9F2"),BorderThickness=new Thickness(0),FontFamily=new FontFamily("Consolas"),FontSize=12,Padding=new Thickness(2) };
            Report.Text=diagnostics?"Collecting diagnostics…":"Choose Verify files to check installed SHA-256 checksums.\n\nRepair preserves saves, settings, extracted game data, ISOs, texture packs and untracked files. Modified release-owned runtime files may be replaced.\n\nA published copy of the exact installed version is required. Existing staged updates are retained. The launcher restarts to apply a repair.";
            var report=Card(Report);Grid.SetRow(report,2);View.Children.Add(report);
            var footer=new StackPanel { Margin=new Thickness(0,14,0,0) };Grid.SetRow(footer,3);View.Children.Add(footer);
            status=Text(diagnostics?"Report excludes raw logs, personal paths, saves and ISO information. Nothing is uploaded automatically.":"Verification is offline. Repairs require GitHub and a closed game.",11);
            status.Foreground=Brush("#A4B8CA");footer.Children.Add(status);
            var bottom=new DockPanel();Cancel=Button("Cancel operation","CancelMaintenance");Cancel.Visibility=Visibility.Collapsed;
            DockPanel.SetDock(Cancel,Dock.Right);bottom.Children.Add(Cancel);
            progress=new ProgressBar { Minimum=0,Maximum=100,Height=4,Foreground=Brush("#86CDFF"),Background=Brush("#253D54"),BorderThickness=new Thickness(0),VerticalAlignment=VerticalAlignment.Center };
            bottom.Children.Add(progress);footer.Children.Add(bottom);
        }
        public void SetVerification(VerificationResult game,VerificationResult launcher)
        {
            gameRepairable=game!=null && game.Version.Length>0;launcherRepairable=launcher!=null;
            if(gameStatus!=null)gameStatus.Text=game==null?"Not checked this session.":game.Summary+" · "+game.Checked+" files";
            if(launcherStatus!=null)launcherStatus.Text=launcher==null?"Not checked this session.":launcher.Summary+" · "+launcher.Checked+" files";
            Report.Text="Verification compares installed manifests. Repair replaces modified release-owned files, while preserving personal files and the previous game rollback.\n\n"+(game==null?"Game: not checked.":game.ToString())+"\n\n"+(launcher==null?"Launcher: not checked.":launcher.ToString());
        }
        public void SetBusy(bool busy,bool gameRunning)
        {
            foreach(var button in new[]{VerifyGame,VerifyLauncher,Refresh,Copy,ReportBug,RequestEnhancement,OpenFiles})if(button!=null)button.IsEnabled=!busy;
            if(RepairGame!=null)RepairGame.IsEnabled=!busy && !gameRunning && gameRepairable;
            if(RepairLauncher!=null)RepairLauncher.IsEnabled=!busy && !gameRunning && launcherRepairable;
            Cancel.Visibility=busy?Visibility.Visible:Visibility.Collapsed;
            progress.IsIndeterminate=busy; if(!busy)progress.Value=0;
        }
        public void Status(string text) { status.Text=text; }
        public void Progress(double value) { progress.IsIndeterminate=false;progress.Value=value; }
    }
}
