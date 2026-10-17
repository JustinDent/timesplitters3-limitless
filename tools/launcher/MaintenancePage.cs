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
        readonly TextBlock status, gameStatus, launcherStatus, gameBadge, launcherBadge;
        readonly Expander details;
        readonly ProgressBar progress;
        bool gameRepairable, launcherRepairable;
        static Brush Brush(string value) { return PageTheme.Brush(value); }
        static TextBlock Text(string text,int size) { return PageTheme.Text(text,size); }
        static Button Button(string text,string name)
        { return new Button { Content=text,Name=name,Margin=new Thickness(0,0,10,0) }; }
        public MaintenancePage(bool diagnostics)
        {
            View.Margin=new Thickness(30,24,30,20);View.Resources.MergedDictionaries.Add(PageTheme.Resources());
            View.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
            View.RowDefinitions.Add(new RowDefinition { Height=new GridLength(1,GridUnitType.Star) });
            View.RowDefinitions.Add(new RowDefinition { Height=GridLength.Auto });
            var heading=new StackPanel { Margin=new Thickness(0,0,0,24) };
            var eyebrow=PageTheme.Text(diagnostics?"SUPPORT CENTER":"INSTALLATION CARE",10,"#86CDFF");eyebrow.FontWeight=FontWeights.SemiBold;heading.Children.Add(eyebrow);
            var title=Text(diagnostics?"Diagnostics":"Repair / Verify",28);title.Margin=new Thickness(0,5,0,7);heading.Children.Add(title);
            heading.Children.Add(PageTheme.Text(diagnostics?"Review your system report and prepare a GitHub issue.":"Check your installation. Restore release files when something needs attention.",12,"#9EB4C8"));View.Children.Add(heading);
            Report=new TextBox { Name=diagnostics?"DiagnosticsReport":"VerificationReport",IsReadOnly=true,AcceptsReturn=true,TextWrapping=TextWrapping.Wrap,
                VerticalScrollBarVisibility=ScrollBarVisibility.Auto,HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled,
                Background=Brush("#0B1826"),BorderThickness=new Thickness(0),FontFamily=new FontFamily("Consolas"),FontSize=12,Padding=new Thickness(12) };
            Report.Text=diagnostics?"Collecting diagnostics…":"No verification results yet. Choose Verify files to check the game or launcher.";
            if(diagnostics) {
                var body=new DockPanel();Grid.SetRow(body,1);View.Children.Add(body);
                var buttons=new WrapPanel { Margin=new Thickness(0,0,0,16) };DockPanel.SetDock(buttons,Dock.Top);body.Children.Add(buttons);
                Refresh=Button("Refresh diagnostics","RefreshDiagnostics");Copy=Button("Copy report","CopyDiagnostics");
                ReportBug=Button("Report a bug ↗","ReportDiagnosticBug");RequestEnhancement=Button("Request enhancement ↗","RequestDiagnosticEnhancement");
                ReportBug.ToolTip="Copies this report and opens the GitHub bug form. You review and submit it there.";
                RequestEnhancement.ToolTip="Copies this report and opens the GitHub enhancement form. You review and submit it there.";
                buttons.Children.Add(Refresh);buttons.Children.Add(Copy);buttons.Children.Add(ReportBug);buttons.Children.Add(RequestEnhancement);
                var report=PageTheme.Card(Report);report.Padding=new Thickness(8);body.Children.Add(report);
            } else {
                var body=new StackPanel();var scroll=new ScrollViewer { Content=body,VerticalScrollBarVisibility=ScrollBarVisibility.Auto,HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled };
                Grid.SetRow(scroll,1);View.Children.Add(scroll);
                var cards=new Grid { Margin=new Thickness(0,0,0,16) };cards.ColumnDefinitions.Add(new ColumnDefinition());cards.ColumnDefinitions.Add(new ColumnDefinition());body.Children.Add(cards);
                for(int i=0;i<2;i++) {
                    var content=new StackPanel();
                    var top=new DockPanel { Margin=new Thickness(0,0,0,20) };
                    var badge=PageTheme.Text("NOT CHECKED",10,"#A9BDD0");badge.FontWeight=FontWeights.SemiBold;
                    var pill=new Border { Child=badge,Background=Brush("#20374B"),CornerRadius=new CornerRadius(3),Padding=new Thickness(8,5,8,5),VerticalAlignment=VerticalAlignment.Center };
                    DockPanel.SetDock(pill,Dock.Right);top.Children.Add(pill);
                    var icon=new Border { Width=36,Height=36,Background=Brush("#1D3A54"),CornerRadius=new CornerRadius(6),HorizontalAlignment=HorizontalAlignment.Left,
                        Child=new TextBlock { Text=i==0?"◇":"◈",FontSize=21,Foreground=Brush("#A7D9FA"),HorizontalAlignment=HorizontalAlignment.Center,VerticalAlignment=VerticalAlignment.Center } };
                    top.Children.Add(icon);content.Children.Add(top);
                    var cardTitle=Text(i==0?"Game installation":"Launcher",19);cardTitle.FontWeight=FontWeights.SemiBold;content.Children.Add(cardTitle);
                    var summary=PageTheme.Text("Ready for a file check",12,"#A9BDD0");summary.Margin=new Thickness(0,8,0,0);summary.MinHeight=36;content.Children.Add(summary);
                    var help=PageTheme.Text(i==0?"Checks the installed game runtime. Repair restores its exact release.":"Checks the launcher and its artwork. Repair reinstalls this version.",12,"#9EB4C8");help.Margin=new Thickness(0,8,0,20);help.MinHeight=34;content.Children.Add(help);
                    var buttons=new WrapPanel();var verify=Button("Verify files",i==0?"VerifyGameFiles":"VerifyLauncherFiles");PageTheme.Primary(verify);
                    var repair=Button("Repair files",i==0?"RepairGameFiles":"RepairLauncherFiles");repair.IsEnabled=false;repair.ToolTip="Verify first. Repair requires a published copy of this version and a closed game.";
                    buttons.Children.Add(verify);buttons.Children.Add(repair);content.Children.Add(buttons);
                    var card=PageTheme.Card(content);card.Padding=new Thickness(22);card.Margin=i==0?new Thickness(0,0,8,0):new Thickness(8,0,0,0);Grid.SetColumn(card,i);cards.Children.Add(card);
                    if(i==0){VerifyGame=verify;RepairGame=repair;gameStatus=summary;gameBadge=badge;}else{VerifyLauncher=verify;RepairLauncher=repair;launcherStatus=summary;launcherBadge=badge;}
                }
                var safety=new DockPanel();OpenFiles=Button("Open game folder ↗","OpenGameFiles");OpenFiles.Margin=new Thickness(18,0,0,0);OpenFiles.VerticalAlignment=VerticalAlignment.Center;
                OpenFiles.ToolTip="Open the installation folder to inspect game files.";DockPanel.SetDock(OpenFiles,Dock.Right);safety.Children.Add(OpenFiles);
                var note=new StackPanel();var noteTitle=Text("Saves, settings and game data stay safe",13);noteTitle.FontWeight=FontWeights.SemiBold;note.Children.Add(noteTitle);
                var noteText=PageTheme.Text("Repair replaces release-owned files. Your ISO, texture packs and personal files are kept.",12,"#9EB4C8");noteText.Margin=new Thickness(0,5,0,0);note.Children.Add(noteText);safety.Children.Add(note);
                var safetyCard=PageTheme.Card(safety);safetyCard.Margin=new Thickness(0,0,0,16);body.Children.Add(safetyCard);
                Report.MaxHeight=200;Report.MinHeight=100;
                details=new Expander { Name="VerificationDetails",Header="Verification details",Foreground=Brush("#C7D9E8"),FontSize=13,Content=Report,IsExpanded=false };
                Report.Margin=new Thickness(0,14,0,0);var detailCard=PageTheme.Card(details);detailCard.Padding=new Thickness(18,14,18,14);body.Children.Add(detailCard);
                var hint=PageTheme.Text("Start with Verify files. If a check finds a problem, review the details before choosing Repair files.",12,"#8FA8BD");hint.Margin=new Thickness(2,16,2,16);body.Children.Add(hint);
            }
            var footer=new StackPanel { Margin=new Thickness(0,16,0,0) };Grid.SetRow(footer,2);View.Children.Add(footer);
            var bottom=new DockPanel();Cancel=Button("Cancel operation","CancelMaintenance");Cancel.Visibility=Visibility.Collapsed;Cancel.Margin=new Thickness(14,0,0,0);
            DockPanel.SetDock(Cancel,Dock.Right);bottom.Children.Add(Cancel);
            status=PageTheme.Text(diagnostics?"Filtered report only. Review the details before submitting on GitHub.":"Verification works offline · Repair requires internet and a closed game",11,"#9EB4C8");status.VerticalAlignment=VerticalAlignment.Center;bottom.Children.Add(status);footer.Children.Add(bottom);
            progress=new ProgressBar { Minimum=0,Maximum=100,Height=3,Margin=new Thickness(0,12,0,0),Foreground=Brush("#86CDFF"),Background=Brush("#253D54"),BorderThickness=new Thickness(0),Visibility=Visibility.Collapsed };
            footer.Children.Add(progress);
        }
        static void Result(TextBlock summary,TextBlock badge,VerificationResult result)
        {
            if(summary==null)return;
            summary.Text=result==null?"Ready for a file check":(result.Version.Length==0?"Version unavailable":result.Version)+" · "+result.Checked+" files checked";
            badge.Text=result==null?"NOT CHECKED":result.Healthy?"VERIFIED":"NEEDS ATTENTION";
            badge.Foreground=Brush(result==null?"#A9BDD0":result.Healthy?"#96DECF":"#FFD49A");
        }
        public void SetVerification(VerificationResult game,VerificationResult launcher)
        {
            gameRepairable=game!=null && game.Version.Length>0;launcherRepairable=launcher!=null;
            Result(gameStatus,gameBadge,game);Result(launcherStatus,launcherBadge,launcher);
            Report.Text="Repair replaces modified release-owned files and preserves personal files and the previous game rollback.\n\n"+(game==null?"Game: not checked.":game.ToString())+"\n\n"+(launcher==null?"Launcher: not checked.":launcher.ToString());
            if(details!=null && ((game!=null && !game.Healthy)||(launcher!=null && !launcher.Healthy)))details.IsExpanded=true;
        }
        public void SetBusy(bool busy,bool gameRunning)
        {
            foreach(var button in new[]{VerifyGame,VerifyLauncher,Refresh,Copy,ReportBug,RequestEnhancement,OpenFiles})if(button!=null)button.IsEnabled=!busy;
            if(RepairGame!=null)RepairGame.IsEnabled=!busy && !gameRunning && gameRepairable;
            if(RepairLauncher!=null)RepairLauncher.IsEnabled=!busy && !gameRunning && launcherRepairable;
            Cancel.Visibility=busy?Visibility.Visible:Visibility.Collapsed;
            progress.Visibility=busy?Visibility.Visible:Visibility.Collapsed;progress.IsIndeterminate=busy; if(!busy)progress.Value=0;
        }
        public void Status(string text) { status.Text=text; }
        public void Progress(double value) { progress.IsIndeterminate=false;progress.Value=value; }
    }
}
