using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using TimeSplitters.Launcher;
#if MACOS
using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.LogicalTree;
using Element = Avalonia.Controls.Control;
#else
using System.Windows;
using System.Windows.Controls;
using Element = System.Windows.FrameworkElement;
#endif

// Exactly the same behavioral contracts run against the real WPF and Avalonia pages.
public static class LauncherSharedUiChecks
{
    static int passed;
    static void Need(bool value,string message){if(!value)throw new Exception(message);}
    static void Test(string name,Action action){action();passed++;Console.WriteLine("PASS shared UI: "+name);}
    static void Click(Button button){button.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));}
    static IEnumerable<Element> All(Element root)
    {
#if MACOS
        return root.GetLogicalDescendants().OfType<Element>();
#else
        return LogicalTreeHelper.GetChildren(root).OfType<Element>().SelectMany(c=>new[]{c}.Concat(All(c)));
#endif
    }
    static T Find<T>(Element root,string name) where T:Element { return All(root).OfType<T>().Single(c=>c.Name==name); }
    public static int Run()
    {
#if MACOS
        return RunCore();
#else
        Exception failure=null;var thread=new System.Threading.Thread(()=>{try{RunCore();}catch(Exception e){failure=e;}});
        thread.SetApartmentState(System.Threading.ApartmentState.STA);thread.IsBackground=true;thread.Start();
        Need(thread.Join(60000),"Shared UI tests timed out");if(failure!=null)throw failure;return 0;
#endif
    }
    static string Tip(Button button)
    {
#if MACOS
        return Convert.ToString(ToolTip.GetTip(button));
#else
        return Convert.ToString(button.ToolTip);
#endif
    }
    static string TestBase()
    {
#if MACOS
        // macOS's /var temp alias is a symlink; production intentionally rejects it.
        return Path.GetFullPath(".");
#else
        return Path.GetTempPath();
#endif
    }
    static int RunCore()
    {
        string root=Path.Combine(TestBase(),"tsfp-shared-ui-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
        try {
            Test("settings drafts, FOV reset, cancel and save",()=>{
                var page=new SettingsPage(new ResourceDictionary(),root,true);page.SelectTab(1);
                var fov=Find<Slider>(page.View,"VerticalFovSlider");var number=Find<TextBlock>(page.View,"VerticalFovValue");
                fov.Value=80;Need(number.Text=="80°","FOV label did not follow slider");
                bool cancelled=false;page.Cancelled+=()=>cancelled=true;Click(Find<Button>(page.View,"SettingsCancelButton"));
                Need(cancelled&&!Directory.Exists(Path.Combine(root,"Settings")),"Cancel saved a draft");
                Click(Find<Button>(page.View,"ResetFovButton"));Need(fov.Value==55,"FOV reset differs");
                fov.Value=75;Click(Find<Button>(page.View,"SettingsSaveButton"));Need(GameSettings.Load(root,null).VerticalFov==75,"Shared page did not save FOV");
            });
            Test("binding capture, conflict cancel, reassignment and clear",()=>{
                var page=new SettingsPage(new ResourceDictionary(),root,true);
                var binding=All(page.View).OfType<Button>().First(b=>Tip(b)==GameSettings.Labels[0]+", binding 1");
                Func<string,Button> action=label=>All(page.View).OfType<Button>().Single(b=>b.Name!="SettingsCancelButton"&&Convert.ToString(b.Content)==label);
                string original=Convert.ToString(binding.Content);Click(binding);Need(Find<Border>(page.View,"BindingCaptureArea")!=null,"No capture prompt");
                Click(action("Bind Escape"));Click(action("Cancel"));Need(Convert.ToString(binding.Content)==original,"Cancelled conflict changed binding");
                Click(binding);Click(action("Bind Escape"));Click(action("Move binding"));Need(Convert.ToString(binding.Content)=="Escape","Conflict move failed");
                Click(Find<Button>(page.View,"SettingsSaveButton"));var saved=GameSettings.Load(root,null);
                Need(GameSettings.KeyCode(saved.Binding(0)[0])==27&&saved.Binding(6).All(t=>GameSettings.KeyCode(t)!=27),"Conflict was not removed from previous action");
                Click(binding);Click(action("Clear binding"));Need(Convert.ToString(binding.Content)=="Unbound","Clear did not update draft");
            });
            Test("Discord uses the same settings model and preserves other settings",()=>{
                var page=new SettingsPage(new ResourceDictionary(),root,true);page.SelectTab(2);
                Find<CheckBox>(page.View,"DiscordPresenceEnabled").IsChecked=true;Find<TextBox>(page.View,"DiscordApplicationId").Text="123456789012345678";
                Click(Find<Button>(page.View,"SettingsSaveButton"));var saved=GameSettings.Load(root,null);
                Need(saved.DiscordEnabled&&saved.DiscordApplicationId=="123456789012345678"&&saved.VerticalFov==75,"Discord save changed other settings");
            });
            Test("news selection, stale notes and toolbar spacing",()=>{
                var page=new WhatsNewPage();page.SetVersions("v1.0.0","1.0.0",false);
                var game=new Release{Tag="v1.1.0",Url=ReleaseClient.ReleasesPage,Notes="## Changes\n- **Shared** release"};
                var launcher=new Release{Tag="1.1.0",Url=ReleaseClient.ReleasesPage+"/tag/launcher-v1.1.0"};
                page.SetNews(new ReleaseNews{Game=game,Launcher=launcher,CheckedUtc=DateTime.UtcNow});
                Need(page.SelectedUrl==game.Url&&page.OnGitHub.IsEnabled,"Game release selection failed");Click(page.LauncherTab);
                Need(page.SelectedUrl==launcher.Url,"Launcher tab did not switch notes");page.SetLoading(true);Need(!page.Refresh.IsEnabled,"Refresh was not locked");page.Failed();
                Need(page.Refresh.IsEnabled&&page.SelectedUrl==launcher.Url&&page.StatusText.Contains("Showing notes checked"),"Failed refresh lost previous notes");
                Need(page.GameTab.Margin.Right>=10&&page.LauncherTab.Margin.Right>=10&&page.Refresh.Margin.Right>=10,"Shared toolbar spacing regressed");
            });
            Test("installed-only notes remain selectable and issue errors retain their age",()=>{
                var page=new WhatsNewPage();page.SetVersions("v1.0.0","1.0.0",false);
                page.SetNews(new ReleaseNews{InstalledGame=new Release{Tag="v1.0.0",Url=ReleaseClient.ReleasesPage},CheckedUtc=DateTime.UtcNow});
                Need(page.Edition.SelectedIndex==1&&page.SelectedUrl!=null,"Installed-only notes were lost");
                page.SetIssues(new KnownIssues{Items=new[]{new KnownIssue{Number=12,Title="Literal <Button>"}},CheckedUtc=DateTime.UtcNow});page.IssuesFailed();
                Need(page.RefreshIssues.IsEnabled&&page.IssuesStatusText.Contains("Showing the list checked"),"Issue refresh failure lost stale status");
            });
            Test("version selection persists across pagination and locks while busy",()=>{
                var page=new GameVersionsPage();page.SetInstalled("v1.0.0");
                page.SetReleases(new GameReleaseList{Releases=new[]{new Release{Tag="v1.0.0",Url=ReleaseClient.ReleasesPage}},HasMore=true},true);
                page.SetReleases(new GameReleaseList{Releases=new[]{new Release{Tag="v1.1.0",Url=ReleaseClient.ReleasesPage}}},false);
                Need(page.Count==2&&page.Selected.Tag=="v1.0.0"&&page.Install.Content.ToString().StartsWith("Reinstall"),"Pagination changed selected version");
                page.SetBusy(true,false);Need(!page.Install.IsEnabled&&!page.Refresh.IsEnabled,"Busy version page accepted changes");page.SetBusy(false,true);Need(!page.Install.IsEnabled,"Running game could be replaced");
                page.SetBusy(false,false);page.SetLoading(true);page.Failed();Need(page.Count==2&&page.Install.IsEnabled,"Failure discarded loaded releases");
            });
            Test("maintenance locks navigation while retaining cancellation",()=>{
                var repair=new MaintenancePage(false,true);var diagnostics=new MaintenancePage(true,true);var area=new MaintenanceArea(repair,diagnostics);
                repair.SetBusy(true,false);area.SetBusy(true);area.Tabs.SelectedIndex=1;
                Need(area.Tabs.SelectedIndex==0&&!repair.VerifyGame.IsEnabled&&repair.Cancel.IsEnabled,"Maintenance did not keep cancel available and lock navigation");
                area.SetBusy(false);area.Tabs.SelectedIndex=1;Need(area.Tabs.SelectedIndex==1,"Maintenance stayed locked");
                repair.SetVerification(new VerificationResult{Version="v1.0.0"},new VerificationResult{Version=LauncherVersion.Value});repair.SetBusy(false,false);
                Need(repair.RepairGame.IsEnabled&&repair.RepairLauncher.IsEnabled,"Verified files cannot be repaired");repair.SetBusy(false,true);Need(!repair.RepairGame.IsEnabled&&!repair.RepairLauncher.IsEnabled,"Repair allowed during play");
            });
            Console.WriteLine(passed+" shared launcher UI groups passed.");return 0;
        }finally{Directory.Delete(root,true);}
    }
}
