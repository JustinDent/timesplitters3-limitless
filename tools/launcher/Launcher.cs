using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace TimeSplitters.Launcher
{
    public static class Program
    {
        [STAThread]
        public static int Main(string[] args)
        {
            string root = Path.GetFullPath(AppDomain.CurrentDomain.BaseDirectory);
            if (Path.GetFileName(Assembly.GetExecutingAssembly().Location) == Core.AppName)
                root = Directory.GetParent(root.TrimEnd(Path.DirectorySeparatorChar)).FullName;
            try
            {
                if (args.Length == 3 && (args[0] == "--apply" || args[0] == "--rollback"))
                    return Helper(args[0], Path.GetFullPath(args[1]), Int32.Parse(args[2]));
                bool repairPreview = args.Length == 2 && args[0] == "--render-repair-preview";
                bool diagnosticsPreview = args.Length == 2 && args[0] == "--render-diagnostics-preview";
                bool graphicsPreview = args.Length == 2 && args[0] == "--render-settings-graphics";
                bool settingsPreview = graphicsPreview || (args.Length == 2 && args[0] == "--render-settings-preview");
                bool preview = repairPreview || diagnosticsPreview || settingsPreview || (args.Length == 2 && args[0] == "--render-preview");
                bool fromBootstrap = args.Length == 2 && args[0] == "--from-bootstrap";
                if (args.Length == 0)
                {
                    Process.Start(new ProcessStartInfo(Core.PathIn(root, Core.LauncherName)) { WorkingDirectory = root, UseShellExecute = true });
                    return 0;
                }
                if (!preview && !fromBootstrap) throw new ArgumentException("Unknown launcher arguments.");
                if (fromBootstrap) SelfUpdate.WaitForParent(Int32.Parse(args[1]));
                using (var mutex = new Mutex(false, Core.MutexName(root)))
                {
                    bool locked;
                    try { locked = mutex.WaitOne(0); } catch (AbandonedMutexException) { locked = true; }
                    if (!locked) { MessageBox.Show("The launcher is already open for this installation.", "TimeSplitters"); return 2; }
                    try
                    {
                        bool recovered = !preview && Core.Recover(root);
                        var app = new Application();
                        var ui = new LauncherWindow(root, preview);
                        if (preview)
                        {
                            if (settingsPreview) ui.ShowSettings(graphicsPreview?1:0);
                            if (repairPreview || diagnosticsPreview) ui.ShowMaintenance(diagnosticsPreview);
                            var window = ui.Window;
                            var visual = (Panel)window.Content;
                            visual.Background = window.Background;
                            int width = 1152, height = 772;
                            visual.Measure(new Size(width, height)); visual.Arrange(new Rect(0, 0, width, height)); visual.UpdateLayout();
                            var bitmap = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Pbgra32);
                            bitmap.Render(visual);
                            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
                            using (var stream = File.Create(args[1])) encoder.Save(stream);
                            return 0;
                        }
                        if (recovered) ui.SetStatus("Recovered an interrupted update. Your previous files have been restored.");
                        app.Run(ui.Window);
                    }
                    finally { mutex.ReleaseMutex(); }
                }
                return 0;
            }
            catch (Exception ex)
            {
                try { Log(root, ex.ToString()); } catch { }
                MessageBox.Show(ex.Message + "\n\nDetails are in .launcher\\launcher.log.", "TimeSplitters — Launcher", MessageBoxButton.OK, MessageBoxImage.Error);
                return 1;
            }
        }
        public static void Log(string root, string text)
        { LauncherLog.Write(root, text); }
        static int Helper(string action, string root, int parent)
        { return RunHelper(action, root, parent, true); }
        internal static int RunHelper(string action, string root, int parent, bool relaunch)
        {
            // This executable lives outside the replaceable payload. Parent exits first.
            try
            {
                using (var p = Process.GetProcessById(parent))
                    if (!p.WaitForExit(30000)) throw new IOException("The launcher did not close. No files were changed.");
            }
            catch (ArgumentException) { }
            bool success = false;
            using (var mutex = new Mutex(false, Core.MutexName(root)))
            {
                bool locked;
                try { locked = mutex.WaitOne(30000); } catch (AbandonedMutexException) { locked = true; }
                Core.Require(locked, "Another launcher is using this installation. No update was applied.");
                try
                {
                    Core.Recover(root);
                    if (action == "--rollback") Core.Rollback(root); else Core.InstallPending(root);
                    Core.Write(Core.PathIn(root, ".launcher/last-result.txt"), action == "--rollback" ? "Previous release restored." : "Release files installed and verified successfully.");
                    success = true;
                }
                catch (Exception ex)
                {
                    Log(root, ex.ToString());
                    Core.Write(Core.PathIn(root, ".launcher/last-result.txt"), "Update failed: " + ex.Message + " Check launcher.log before retrying.");
                    if (relaunch) MessageBox.Show(ex.Message + "\n\nKeep the .launcher folder for recovery. Details: .launcher\\launcher.log", "TimeSplitters — Update failed", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                finally { mutex.ReleaseMutex(); }
            }
            string launcher = Core.PathIn(root, Core.LauncherName);
            if (relaunch && File.Exists(launcher)) Process.Start(new ProcessStartInfo(launcher) { WorkingDirectory = root, UseShellExecute = true });
            return success ? 0 : 1;
        }
    }

    public sealed class LauncherWindow
    {
        public readonly Window Window;
        readonly string root;
        readonly bool previewMode;
        readonly ReleaseClient client = new ReleaseClient();
        readonly CancellationTokenSource lifetime = new CancellationTokenSource();
        Release available;
        SettingsPage settings;
        MaintenancePage repairPage, diagnosticsPage;
        VerificationResult gameVerification, launcherVerification;
        bool diagnosticsReady;
        CancellationTokenSource maintenanceStop;
        bool busy, checking, handingOff;
        bool validData, gameInstalled;
        string installed = "";
        T Find<T>(string name) where T : class { return Window.FindName(name) as T; }
        Button Button(string name) { return Find<Button>(name); }
        void Text(string name, string value) { Find<TextBlock>(name).Text = value; }
        ProgressBar Bar { get { return Find<ProgressBar>("OperationProgress"); } }
        ComboBox Channel { get { return Find<ComboBox>("ChannelPicker"); } }
        bool PreviewChannel { get { return Channel.SelectedIndex == 1; } }
        public void SetStatus(string value) { Text("StatusText", value); }
        public LauncherWindow(string path, bool preview)
        {
            root = path; previewMode = preview;
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Launcher.xaml"))
                Window = (Window)XamlReader.Load(stream);
            using (var icon = Assembly.GetExecutingAssembly().GetManifestResourceStream("launcher.ico"))
                Window.Icon = BitmapFrame.Create(icon, BitmapCreateOptions.None, BitmapCacheOption.OnLoad);
            Text("LauncherVersionLabel", "TSFP  /  COMMUNITY PC PROJECT  /  LAUNCHER v" + LauncherVersion.Value);
            using (var logo = Assembly.GetExecutingAssembly().GetManifestResourceStream("launcher-logo.png"))
            {
                if (logo != null)
                {
                    var bitmap = BitmapFrame.Create(logo, BitmapCreateOptions.None, BitmapCacheOption.OnLoad);
                    bitmap.Freeze();
                    Find<Image>("SidebarLogo").Source = bitmap;
                    // Display the original subtitle lettering at a readable sidebar size.
                    // Coordinates refer to the unchanged 500x187 source lockup.
                    var subtitle = new CroppedBitmap(bitmap, new Int32Rect(263, 150, 228, 22));
                    subtitle.Freeze();
                    Find<Image>("SidebarSubtitle").Source = subtitle;
                    Find<StackPanel>("SidebarBrand").Visibility = Visibility.Visible;
                    Find<StackPanel>("SidebarBrandFallback").Visibility = Visibility.Collapsed;
                }
            }
            using (var banner = Assembly.GetExecutingAssembly().GetManifestResourceStream("launcher-banner.png"))
            {
                if (banner != null)
                {
                    var bitmap = BitmapFrame.Create(banner, BitmapCreateOptions.None, BitmapCacheOption.OnLoad);
                    bitmap.Freeze();
                    var hero = Find<Border>("HeroBanner");
                    hero.Background = new ImageBrush(bitmap) { Stretch = Stretch.UniformToFill,
                        AlignmentX = AlignmentX.Center, AlignmentY = AlignmentY.Top };
                    hero.Visibility = Visibility.Visible;
                    Find<Grid>("HeroFallback").Visibility = Visibility.Collapsed;
                }
            }
            Refresh();
            Button("SettingsNav").Click += (s,e)=>ShowSettings();
            Button("HomeNav").Click += (s,e)=>ShowHome();
            Button("RepairNav").Click += (s,e)=>ShowMaintenance(false);
            Button("DiagnosticsNav").Click += (s,e)=>ShowMaintenance(true);
            Window.Closed += (s,e)=> { lifetime.Cancel(); client.Dispose(); };
            if (previewMode)
            {
                Text("InstalledLabel", "v0.1.0-alpha.1");
                Text("ReadyTitle", "Ready to play");
                Text("ReadyDescription", "Your game is installed and ready to launch.");
                Text("DataLabel", "●  Verified US Xbox data");
                Text("ReleaseTitle", "You're on the latest release.");
                Text("ReleaseDescription", "The installed game is up to date. New releases appear here when published.");
                SetStatus("Ready.");
                Button("PlayButton").IsEnabled = true;
                return;
            }
            var prefsPath = Core.PathIn(root, ".launcher/preferences.json");
            if (File.Exists(prefsPath))
            {
                try { Channel.SelectedIndex = Json.Flag(Json.Object(File.ReadAllText(prefsPath)), "preview") ? 1 : 0; }
                catch (Exception ex) { Program.Log(root, "Preferences: " + ex.Message); }
            }
            Channel.SelectionChanged += async (s, e) =>
            {
                try { Core.Write(prefsPath, Json.Encode(new { preview = PreviewChannel })); await Check(); }
                catch (Exception ex) { Fail(ex); }
            };
            Button("PlayButton").Click += async (s, e) => await Play();
            Button("ImportButton").Click += async (s, e) => await Import(false);
            Button("FolderButton").Click += async (s, e) => await Import(true);
            Button("CheckButton").Click += async (s, e) => await Check();
            Button("UpdateButton").Click += async (s, e) => await Update();
            Button("RollbackButton").Click += (s, e) =>
            {
                if (MessageBox.Show(Window, "Restore the previous installed release? Your game data, saves and settings will be kept.", "Restore previous release", MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes) return;
                try { HandOff(true); } catch (Exception ex) { Fail(ex); }
            };
            Button("ReleaseNotesButton").Click += (s, e) => Open(available == null ? ReleaseClient.ReleasesPage : available.Url);
            Button("SavesNav").Click += (s, e) => OpenFolder("Saves");
            Button("SupportNav").Click += (s, e) => Open("https://github.com/" + ReleaseClient.Repository + "/issues");
            Button("LogsButton").Click += (s, e) => OpenFolder(".launcher");
            Window.Loaded += async (s, e) =>
            {
                SelfUpdate.CleanupAfterStartup(root);
                busy = true; Refresh();
                try { await Task.Run(() => Core.CleanupGameAfterStartup(root)); }
                finally { busy = false; }
                await Check();
                string result = Core.PathIn(root, ".launcher/last-result.txt");
                if (File.Exists(result)) { SetStatus(File.ReadAllText(result)); File.Delete(result); }
                string selfResult = Core.PathIn(root, ".launcher/self-last-result.txt");
                if (File.Exists(selfResult)) { SetStatus(File.ReadAllText(selfResult)); File.Delete(selfResult); }
            };
            Window.Closing += (s, e) =>
            {
                if (busy && !handingOff) { e.Cancel = true; SetStatus("Please let the current operation finish before closing the launcher."); }
            };
        }
        public void ShowSettings(int tab=-1)
        {
            if(busy || (!previewMode && Core.GameRunning())) { SetStatus("Close the game before editing settings."); return; }
            try {
                if(settings==null) {
                    settings=new SettingsPage(Window.Resources,root,previewMode);
                    settings.Saved+=()=> { SetStatus("Settings saved."); Refresh(); };
                    settings.Cancelled+=()=> { ShowHome(); Find<ContentControl>("SettingsHost").Content=null; settings=null; SetStatus("Unsaved settings discarded."); };
                    Find<ContentControl>("SettingsHost").Content=settings.View;
                }
                if(tab>=0)settings.SelectTab(tab);
                Find<Grid>("CommandCenter").Visibility=Visibility.Collapsed;
                Find<ContentControl>("SettingsHost").Visibility=Visibility.Visible;
                Find<ContentControl>("MaintenanceHost").Visibility=Visibility.Collapsed;
                SelectNavigation("SettingsNav");
            } catch(Exception ex) { Fail(ex); }
        }
        void ShowHome()
        {
            if(settings!=null)settings.DismissPrompt();
            Find<ContentControl>("SettingsHost").Visibility=Visibility.Collapsed;
            Find<Grid>("CommandCenter").Visibility=Visibility.Visible;
            Find<ContentControl>("MaintenanceHost").Visibility=Visibility.Collapsed;
            SelectNavigation("HomeNav");
        }
        void SelectNavigation(string active)
        {
            foreach(string name in new[]{"HomeNav","SettingsNav","RepairNav","DiagnosticsNav"}) {
                bool selected=name==active;
                Button(name).Background=selected?new SolidColorBrush(Color.FromRgb(29,58,85)):Brushes.Transparent;
                Button(name).Foreground=new SolidColorBrush(selected?Color.FromRgb(134,205,255):Color.FromRgb(223,233,242));
            }
        }
        public async void ShowMaintenance(bool diagnostics)
        {
            if(busy)return;
            try {
                if(settings!=null)settings.DismissPrompt();
                var page=diagnostics?diagnosticsPage:repairPage;
                if(page==null) {
                    page=new MaintenancePage(diagnostics);
                    page.Cancel.Click+=(s,e)=> { if(maintenanceStop!=null)maintenanceStop.Cancel();page.Status("Cancellation requested; finishing the current file…"); };
                    if(diagnostics) {
                        diagnosticsPage=page;
                        page.Refresh.Click+=async(s,e)=>await RefreshDiagnostics();
                        page.ReportBug.Click+=(s,e)=>OpenDiagnosticIssue(false);
                        page.RequestEnhancement.Click+=(s,e)=>OpenDiagnosticIssue(true);
                        page.Copy.Click+=(s,e)=> { try { Clipboard.SetText(page.Report.Text);page.Status("Diagnostic report copied. Nothing was uploaded."); }catch(Exception ex){page.Status("Clipboard unavailable ("+ex.GetType().Name+"). Select and copy the report text instead.");} };
                    } else {
                        repairPage=page;
                        page.OpenFiles.Click+=(s,e)=> { if(previewMode)page.Status("Preview: opens the game installation folder.");else Open(root); };
                        page.VerifyGame.Click+=async(s,e)=>await VerifyFiles(false);
                        page.VerifyLauncher.Click+=async(s,e)=>await VerifyFiles(true);
                        page.RepairGame.Click+=async(s,e)=>await RepairFiles(false);
                        page.RepairLauncher.Click+=async(s,e)=>await RepairFiles(true);
                        page.SetVerification(gameVerification,launcherVerification);
                    }
                }
                Find<Grid>("CommandCenter").Visibility=Visibility.Collapsed;
                Find<ContentControl>("SettingsHost").Visibility=Visibility.Collapsed;
                var host=Find<ContentControl>("MaintenanceHost");host.Content=page.View;host.Visibility=Visibility.Visible;
                SelectNavigation(diagnostics?"DiagnosticsNav":"RepairNav");
                page.SetBusy(busy || checking,Core.GameRunning());
                if(diagnostics) {
                    if(previewMode)page.Report.Text="TimeSplitters launcher diagnostics — interface preview\n\nRunning launcher: "+LauncherVersion.Value+"\nInstalled game: v0.2.0-alpha.5 (sample)\nWindows: 64-bit\nGraphics: available in the installed launcher\nEmbedded artwork: banner present, logo present\n\nLast verification: not checked this session.\n\nRecent launcher errors: none in this preview.\n\nReport excludes raw logs, personal paths, saves and ISO information. Nothing is uploaded automatically.";
                    else await RefreshDiagnostics();
                }
            }catch(Exception ex){Fail(ex);}
        }
        async Task MaintenanceOperation(MaintenancePage page,Func<CancellationToken,Task> operation)
        {
            if(busy)return;
            if(previewMode){page.Status("Preview only. Verification and repairs use the installed launcher.");return;}
            busy=true;
            using(var cancel=CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token)) {
                maintenanceStop=cancel;Refresh();
                try { await operation(cancel.Token); }
                catch(OperationCanceledException){page.Status("Operation cancelled. Installed files were not changed.");}
                catch(Exception ex){page.Status(ex.Message);Fail(ex);}
                finally { maintenanceStop=null;busy=false;if(!handingOff)Refresh(); }
            }
        }
        async Task VerifyFiles(bool launcher)
        {
            await MaintenanceOperation(repairPage,async token=> {
                repairPage.Status("Verifying "+(launcher?"launcher":"game")+" files…");
                var progress=new Progress<double>(value=>repairPage.Progress(value));
                var result=await Task.Run(()=>InstallationMaintenance.Verify(root,launcher,progress,token),token);
                if(launcher)launcherVerification=result;else gameVerification=result;
                repairPage.SetVerification(gameVerification,launcherVerification);
                repairPage.Status(result.Summary+". Repairs use the exact published version; personal files are excluded.");
            });
        }
        async Task RefreshDiagnostics()
        {
            await MaintenanceOperation(diagnosticsPage,async token=> {
                diagnosticsPage.Status("Collecting system details…");
                string graphics=await InstallationMaintenance.GraphicsWithTimeout(token);
                string report=await Task.Run(()=>InstallationMaintenance.Report(root,gameVerification,launcherVerification,graphics),token);
                token.ThrowIfCancellationRequested();diagnosticsPage.Report.Text=report;diagnosticsReady=true;
                diagnosticsPage.Status("Report excludes raw logs, personal paths, saves and ISO information. Nothing is uploaded automatically.");
            });
        }
        void OpenDiagnosticIssue(bool enhancement)
        {
            if(busy || diagnosticsPage==null)return;
            if(previewMode){diagnosticsPage.Status("Preview only. GitHub drafts open from the installed launcher; no report was sent.");return;}
            if(!diagnosticsReady){diagnosticsPage.Status("Refresh diagnostics before preparing a GitHub report.");return;}
            try {
                var draft=DiagnosticIssueDraft.Create(enhancement,diagnosticsPage.Report.Text);
                bool copied=false;
                try {Clipboard.SetText(draft.ClipboardText);copied=true;}catch { /* The displayed report remains available for manual copying. */ }
                try {Process.Start(new ProcessStartInfo(draft.Url){UseShellExecute=true});}
                catch(Exception ex) {
                    diagnosticsPage.Status(copied?"Could not open the browser. The full report is copied; open GitHub Support and paste it into the report form.":
                        "Could not open the browser or clipboard. Copy the displayed report manually, then open GitHub Support.");
                    Program.Log(root,"GitHub draft browser handoff failed ("+ex.GetType().Name+").");return;
                }
                diagnosticsPage.Status(draft.RequiresPaste?
                    (copied?"GitHub draft opened. Paste the copied full report into Launcher diagnostics (optional), complete the form, and review before submitting.":
                        "GitHub draft opened. Clipboard unavailable: copy the report text here and paste it into Launcher diagnostics (optional)."):
                    "GitHub draft opened with diagnostics. Complete the form, review the details, and submit when ready."+(copied?" A full copy is on your clipboard.":" Clipboard copy was unavailable."));
            }catch(Exception ex){diagnosticsPage.Status("Could not prepare the GitHub draft. Refresh diagnostics and try again.");Program.Log(root,"GitHub draft preparation failed ("+ex.GetType().Name+").");}
        }
        async Task RepairFiles(bool launcher)
        {
            await MaintenanceOperation(repairPage,async token=> {
                Core.Require(!Core.GameRunning(),"Close the game before repairing files.");
                repairPage.Status("Finding the exact published release, then downloading and verifying its files…");
                var progress=new Progress<double>(value=> {repairPage.Progress(value);repairPage.Status("Downloading repair package · "+value.ToString("0")+"%. Installed files are unchanged until verification completes.");});
                bool prepared=false;
                try {
                    if(launcher) {
                        var release=await client.FindLauncher(LauncherVersion.Value,token);
                        prepared=await SelfUpdate.Prepare(root,release,client,progress,token,true);
                        Core.Require(prepared,"A newer launcher is installed. Restart it before repairing.");
                        token.ThrowIfCancellationRequested();
                        string helper=Core.PathIn(root,".launcher/self-helper.exe");
                        Core.Require(!Core.GameRunning(),"Close the game before repairing files.");
                        File.Copy(Core.PathIn(root,".launcher/self-pending/payload/"+Core.LauncherName),helper,true);
                        Process.Start(new ProcessStartInfo(helper,"--apply-launcher "+Quote(root)+" "+Process.GetCurrentProcess().Id){UseShellExecute=true,WorkingDirectory=root});
                        handingOff=true;Window.Close();
                    } else {
                        Core.Require(gameVerification!=null && gameVerification.Version.Length>0,"Verify the game before repairing it.");
                        await InstallationMaintenance.PrepareGameRepair(root,gameVerification.Version,client,progress,token);
                        prepared=true;token.ThrowIfCancellationRequested();HandOff(false);
                    }
                } catch {
                    // Only this operation's newly prepared staging can be discarded.
                    if(prepared && !handingOff)try {
                        Core.DiscardRepairStaging(root,launcher);
                    }catch { /* Retain locked or unsafe staging for recovery. */ }
                    throw;
                }
            });
        }
        void Open(string path)
        {
            try { Process.Start(new ProcessStartInfo(path) { UseShellExecute = true }); }
            catch (Exception ex) { Fail(ex); }
        }
        void OpenFolder(string name)
        {
            try { string path = Core.PathIn(root, name); Directory.CreateDirectory(path); Open(path); }
            catch (Exception ex) { Fail(ex); }
        }
        void Fail(Exception ex)
        {
            if (lifetime.IsCancellationRequested) return;
            SetStatus(ex.Message);
            try { Program.Log(root, ex.ToString()); } catch { }
        }
        void Refresh()
        {
            try { installed = Core.InstalledVersion(root); }
            catch (Exception ex) { installed = ""; SetStatus("Installed manifest could not be read: " + ex.Message); }
            gameInstalled = File.Exists(Core.PathIn(root, "Bin/futureperfect_recomp.exe")) && File.Exists(Core.PathIn(root, "Launch.ps1"));
            string xbe = Core.PathIn(root, "Data/default.xbe");
            validData = File.Exists(xbe) && Directory.Exists(Core.PathIn(root, "Data/pak")) && Directory.Exists(Core.PathIn(root, "Data/xmv")) && Core.Hash(xbe) == TsfpImport.ExpectedHash;
            Text("InstalledLabel", installed.Length > 0 ? installed : gameInstalled ? "UNVERSIONED" : "NOT INSTALLED");
            Text("ReadyTitle", gameInstalled && validData ? "Ready to play" : gameInstalled ? "Import game data" : "Install game");
            Text("ReadyDescription", gameInstalled && validData ? "Your game is installed and ready to launch." : gameInstalled ? "Import your US Xbox game once to start playing." : "Install a release, then import your US Xbox ISO.");
            bool dataExists = Directory.Exists(Core.PathIn(root, "Data"));
            if (dataExists && !validData)
            {
                Text("ReadyTitle", "Check your game data.");
                Text("ReadyDescription", "Existing Data is incomplete or unsupported. Choose Repair / Verify → Open game folder to inspect it; your files were kept.");
            }
            Text("DataLabel", validData ? "●  Verified US Xbox data" : dataExists ? "○  Data needs attention" : "○  ISO required");
            Button("ImportButton").IsEnabled = Button("FolderButton").IsEnabled = !busy && !dataExists;
            Button("PlayButton").IsEnabled = !busy && gameInstalled && validData && !Core.GameRunning();
            Button("SettingsNav").IsEnabled = !busy && (previewMode || !Core.GameRunning());
            Button("HomeNav").IsEnabled = Button("RepairNav").IsEnabled = Button("DiagnosticsNav").IsEnabled = !busy;
            if(repairPage!=null)repairPage.SetBusy(busy || checking,Core.GameRunning());
            if(diagnosticsPage!=null)diagnosticsPage.SetBusy(busy || checking,Core.GameRunning());
            Button("CheckButton").IsEnabled = !busy && !checking;
            Channel.IsEnabled = !busy && !checking;
            Button("UpdateButton").IsEnabled = !busy && !checking && CanUpdate();
            Button("UpdateButton").Content = gameInstalled ? "Update release" : "Install release";
            Button("RollbackButton").IsEnabled = !busy && !checking && Core.CanRollback(root) && !Core.GameRunning();
            Text("ChannelBadge", PreviewChannel ? "●  PREVIEW CHANNEL" : "●  STABLE CHANNEL");
        }
        bool CanUpdate()
        {
            if (available == null) return false;
            if (!gameInstalled || installed.Length == 0) return true;
            try { return GameVersion.Parse(available.Tag).CompareTo(GameVersion.Parse(installed)) > 0; }
            catch (InvalidDataException) { return true; }
        }
        async Task Check()
        {
            if (busy || checking) return;
            checking = true; available = null;
            Refresh();
            Text("ReleaseTitle", "Checking for updates…");
            Text("ReleaseDescription", "Looking for published " + (PreviewChannel ? "preview or stable" : "stable") + " releases on GitHub.");
            SetStatus("Checking GitHub Releases…");
            using (var timeout = CancellationTokenSource.CreateLinkedTokenSource(lifetime.Token))
            {
                timeout.CancelAfter(TimeSpan.FromSeconds(25));
                try
                {
                    available = await client.Check(PreviewChannel, timeout.Token);
                    if (available == null)
                    {
                        Text("ReleaseTitle", PreviewChannel ? "No compatible release yet." : "No stable release yet.");
                        Text("ReleaseDescription", PreviewChannel ? "A published Windows ZIP with a checksum will appear here." : "Stable excludes test builds. Choose Preview to check published prereleases.");
                        SetStatus(gameInstalled ? "Your installed game is available offline." : "No compatible published release was found for this channel.");
                    }
                    else
                    {
                        bool newer = CanUpdate();
                        Text("ReleaseTitle", newer ? available.Tag + " is ready." : "You're on the latest release.");
                        Text("ReleaseDescription", newer ? "Published on GitHub · " + (available.Size / 1048576.0).ToString("0.0") + " MB\n" + (gameInstalled ? "Update available." : "Install the game runtime, then import your game data.") : "The installed game is up to date. Check release notes for details.");
                        SetStatus(newer ? "Release available. Select " + (gameInstalled ? "Update" : "Install") + " to download and verify." : "Ready.");
                    }
                }
                catch (OperationCanceledException) { if (!lifetime.IsCancellationRequested) { Text("ReleaseTitle", "Release check timed out."); Text("ReleaseDescription", "Your installed game is available offline. Check again when connected."); SetStatus("GitHub did not respond in time. You can retry the release check."); } }
                catch (Exception ex) { Text("ReleaseTitle", "Update check failed."); Text("ReleaseDescription", "Your installed game is available offline. Try checking again later."); Fail(ex); }
            }
            checking = false; Refresh();
        }
        async Task Import(bool folder)
        {
            if (busy) return;
            string source;
            if (folder)
            {
                using (var picker = new System.Windows.Forms.FolderBrowserDialog { Description = "Choose your extracted US Xbox game folder (default.xbe, pak, xmv)." })
                {
                    if (picker.ShowDialog() != System.Windows.Forms.DialogResult.OK) return;
                    source = picker.SelectedPath;
                }
            }
            else
            {
                var picker = new Microsoft.Win32.OpenFileDialog { Title = "Locate your TimeSplitters: Future Perfect US Xbox ISO", Filter = "Xbox disc image (*.iso)|*.iso", CheckFileExists = true };
                if (picker.ShowDialog(Window) != true) return;
                source = picker.FileName;
            }
            busy = true; Refresh(); Bar.IsIndeterminate = true;
            SetStatus("Validating and importing game data. This can take a few minutes…");
            try
            {
                await Task.Run(() =>
                {
                    string dest = Core.PathIn(root, "Data");
                    if (folder) TsfpImport.ImportFolder(source, dest); else TsfpImport.ImportIso(source, dest);
                });
                SetStatus("Game data imported. Your ISO is no longer needed to play.");
                Program.Log(root, "Game data imported successfully.");
            }
            catch (Exception ex) { Fail(ex); }
            finally { busy = false; Bar.IsIndeterminate = false; Refresh(); }
        }
        async Task Update()
        {
            if (busy || !CanUpdate()) return;
            if (Core.GameRunning()) { SetStatus("Close the game before installing a release."); return; }
            busy = true; Refresh();
            try
            {
                Core.CheckSpace(root, available.Size * 4 + 256L * 1024 * 1024);
                Core.DiscardGameStaging(root);
                string pending = Core.PathIn(root, ".launcher/pending"); Directory.CreateDirectory(pending);
                string archive = Path.Combine(pending, "release.zip");
                SetStatus("Downloading " + available.Tag + "…");
                var progress = new Progress<double>(value => { Bar.Value = value; SetStatus("Downloading " + available.Tag + " · " + value.ToString("0") + "%"); });
                await client.Download(available, archive, progress, lifetime.Token);
                Bar.IsIndeterminate = true;
                SetStatus("Verifying and preparing the release…");
                string tag = available.Tag;
                await Task.Run(() => Core.Unpack(archive, Path.Combine(pending, "payload"), tag));
                Core.Write(Path.Combine(pending, "release.json"), Json.Encode(new { version = tag }));
                // Only payload and one previous release are retained, not the downloaded ZIP.
                File.Delete(archive);
                Program.Log(root, "Prepared " + tag + "; handing off to updater.");
                HandOff(false);
            }
            catch (Exception ex) { Fail(ex); }
            finally { if (!handingOff) { busy = false; Bar.IsIndeterminate = false; Bar.Value = 0; Refresh(); } }
        }
        void HandOff(bool rollback)
        {
            Core.Require(!Core.GameRunning(), "Close the game before changing the installed release.");
            string helper = Core.PathIn(root, ".launcher/update-helper.exe");
            Directory.CreateDirectory(Path.GetDirectoryName(helper));
            File.Copy(Assembly.GetExecutingAssembly().Location, helper, true);
            var info = new ProcessStartInfo(helper, (rollback ? "--rollback " : "--apply ") + Quote(root) + " " + Process.GetCurrentProcess().Id) { UseShellExecute = true, WorkingDirectory = root };
            Process.Start(info);
            handingOff = true; Window.Close();
        }
        public static string Quote(string value)
        { return SelfUpdate.Quote(value); }
        internal static bool GameWindowReady(string root)
        {
            string executable = Core.PathIn(root, "Bin/futureperfect_recomp.exe");
            bool ready = false;
            foreach (var game in Process.GetProcessesByName("futureperfect_recomp"))
            {
                using (game)
                {
                    try
                    {
                        if (!game.HasExited && game.MainWindowHandle != IntPtr.Zero &&
                            String.Equals(game.MainModule.FileName, executable, StringComparison.OrdinalIgnoreCase))
                            ready = true;
                    }
                    catch (InvalidOperationException) { } // Process exited during observation.
                    catch (System.ComponentModel.Win32Exception) { } // Unrelated/inaccessible process.
                }
            }
            return ready;
        }
        internal async Task MonitorGame(Process process)
        {
            bool minimizedForGame = false;
            string observedBuild = Core.GameBuildHash(root);
            WindowState previousState = Window.WindowState;
            try
            {
                SetStatus("Starting game…");
                while (!process.HasExited)
                {
                    if (!minimizedForGame && GameWindowReady(root))
                    {
                        Window.WindowState = WindowState.Minimized;
                        minimizedForGame = true;
                        SetStatus("Game running.");
                    }
                    await Task.Delay(200);
                }
                await Task.Run(() => process.WaitForExit());
            }
            finally
            {
                if (minimizedForGame) { Window.WindowState = previousState; Window.Activate(); }
            }
            await Task.Run(() => Core.FinishGameSession(root, observedBuild, minimizedForGame, process.ExitCode));
        }
        async Task Play()
        {
            if (busy) return;
            busy = true; Refresh();
            try
            {
                Core.Require(validData && gameInstalled, "Install the release and import the supported US Xbox data first.");
                Core.Require(!Core.GameRunning(), "The game is already running.");
                string shell = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "WindowsPowerShell", "v1.0", "powershell.exe");
                using (var process = new Process())
                {
                    if (GameSettings.HasGraphics(root))
                    {
                        process.StartInfo = GameSettings.StartInfo(root);
                        string logs = Core.PathIn(root, "Logs"); Directory.CreateDirectory(logs);
                        string tag = DateTime.Now.ToString("yyyyMMdd-HHmmss-fff", System.Globalization.CultureInfo.InvariantCulture);
                        string errorPath = Path.Combine(logs, tag + ".err");
                        process.StartInfo.EnvironmentVariables["RECOMP_LOG_FILE"] = errorPath;
                        using (var output = File.Create(Path.Combine(logs, tag + ".out")))
                        using (var errors = new FileStream(errorPath, FileMode.Create, FileAccess.Write, FileShare.ReadWrite))
                        {
                            process.Start();
                            var stdout = process.StandardOutput.BaseStream.CopyToAsync(output);
                            var stderr = process.StandardError.BaseStream.CopyToAsync(errors);
                            await MonitorGame(process); await Task.WhenAll(stdout, stderr);
                        }
                        SetStatus(process.ExitCode == 0 ? "Game closed." : "Game launch failed. View logs for details.");
                        return;
                    }
                    process.StartInfo = new ProcessStartInfo(shell, "-NoProfile -STA -ExecutionPolicy Bypass -File " + Quote(Core.PathIn(root, "Launch.ps1"))) {
                        WorkingDirectory = root, UseShellExecute = false, CreateNoWindow = true, RedirectStandardError = true, RedirectStandardOutput = true };
                    // Isolate inherited developer/test environment overrides.
                    var keys = new System.Collections.Generic.List<string>();
                    foreach (string key in process.StartInfo.EnvironmentVariables.Keys)
                        if (key.StartsWith("RECOMP_", StringComparison.OrdinalIgnoreCase) || key.StartsWith("TSFP_", StringComparison.OrdinalIgnoreCase)) keys.Add(key);
                    foreach (string key in keys) process.StartInfo.EnvironmentVariables.Remove(key);
                    process.OutputDataReceived += (s, e) => { if (e.Data != null) Program.Log(root, e.Data); };
                    process.ErrorDataReceived += (s, e) => { if (e.Data != null) Program.Log(root, e.Data); };
                    process.Start(); process.BeginOutputReadLine(); process.BeginErrorReadLine();
                    await MonitorGame(process);
                    SetStatus(process.ExitCode == 0 ? "Game closed." : "Game launch failed. View logs for details.");
                }
            }
            catch (Exception ex) { Fail(ex); }
            finally { busy = false; Refresh(); }
        }
    }
}
