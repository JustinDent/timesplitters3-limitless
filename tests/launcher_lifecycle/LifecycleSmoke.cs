using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;
using TimeSplitters.Launcher;

public static class LifecycleSmoke
{
    static int result;
    static string root;
    [STAThread]
    public static int Main(string[] args)
    {
        root = args[0];
        var app = new Application { ShutdownMode = ShutdownMode.OnExplicitShutdown };
        app.Startup += async (s, e) => {
            try {
                await Check(WindowState.Normal, "");
                await Check(WindowState.Maximized, "");
                await Check(WindowState.Normal, "fail");
                await Check(WindowState.Normal, "no-window");
                await FailedStart();
            }
            catch (Exception error) { Console.WriteLine(error); result = 1; }
            finally { app.Shutdown(); }
        };
        app.Run();
        return result;
    }
    static void Require(bool condition, string message)
    { if (!condition) throw new Exception(message); }
    static async Task Check(WindowState initial, string mode)
    {
        var ui = new LauncherWindow(root, true);
        ui.Window.WindowState = initial;
        var probe = typeof(LauncherWindow).GetMethod("GameWindowReady", BindingFlags.Static | BindingFlags.NonPublic);
        Require(!(bool)probe.Invoke(null, new object[] { root }), "Another installation's process counted as ready.");
        // A valid build and pending-file sentinel ensure a clean exit without
        // a window does not accidentally authorize old-version cleanup.
        File.WriteAllText(Path.Combine(root, "Launch.ps1"), "synthetic launch");
        File.WriteAllText(Path.Combine(root, "BUILD.json"), Json.Encode(new {
            version = "v1.0.0", files = new System.Collections.Generic.Dictionary<string,string> {
                { "Bin/futureperfect_recomp.exe", Core.Hash(Path.Combine(root,"Bin/futureperfect_recomp.exe")) },
                { "Launch.ps1", Core.Hash(Path.Combine(root,"Launch.ps1")) }
            }
        }));
        string pending = Path.Combine(root, ".launcher/pending/payload");
        Directory.CreateDirectory(pending);
        string sentinel = Path.Combine(pending,"Launch.ps1"); File.WriteAllText(sentinel,"pending recovery sentinel");
        using (var game = Process.Start(new ProcessStartInfo(Path.Combine(root, "Bin/futureperfect_recomp.exe"), mode) {
            UseShellExecute = false, CreateNoWindow = true }))
        {
            var method = typeof(LauncherWindow).GetMethod("MonitorGame", BindingFlags.Instance | BindingFlags.NonPublic);
            var monitor = (Task)method.Invoke(ui, new object[] { game, null });
            Require(ui.Window.WindowState == WindowState.Minimized, "Launcher did not minimize immediately after process start.");
            Require(ui.Window.ShowInTaskbar, "Minimized launcher must remain available on the taskbar.");
            bool minimized = false;
            var deadline = Stopwatch.StartNew();
            while (!monitor.IsCompleted)
            {
                if (ui.Window.WindowState == WindowState.Minimized) minimized = true;
                Require(deadline.ElapsedMilliseconds < 8000, "Monitor did not finish.");
                await Task.Delay(40);
            }
            await monitor;
            Require(minimized, "Launcher never minimized.");
            Require(ui.Window.WindowState == initial, "Original window state was not restored.");
            Require(game.ExitCode == (mode == "fail" ? 7 : 0), "Wrong fixture result.");
            Require(File.Exists(sentinel) == (mode != ""), "Cleanup must require a real window and a successful exit.");
            Console.WriteLine("PASS " + initial + " " + mode + " minimizes on process start, stays in taskbar, restores and preserves cleanup rules");
        }
        ui.Window.Close();
    }
    static async Task FailedStart()
    {
        string missing = Path.Combine(root, "missing-game"); Directory.CreateDirectory(missing);
        var ui = new LauncherWindow(missing, true);
        try {
            var method = typeof(LauncherWindow).GetMethod("LaunchGame", BindingFlags.Instance | BindingFlags.NonPublic);
            bool failed = false;
            try { await (Task)method.Invoke(ui, new object[] { null }); }
            catch (System.ComponentModel.Win32Exception) { failed = true; }
            Require(failed && ui.Window.WindowState == WindowState.Normal, "A game that could not start must leave the launcher available.");
            Console.WriteLine("PASS missing game leaves launcher visible");
        } finally { ui.Window.Close(); }
    }
}
