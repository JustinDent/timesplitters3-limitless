using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using Microsoft.Win32;

namespace TimeSplitters.Launcher
{
    internal sealed class WindowTheme
    {
        readonly Window window;
        readonly Func<bool> readDark;
        HwndSource source;
        bool? applied;

        [DllImport("dwmapi.dll")]
        static extern int DwmSetWindowAttribute(IntPtr window, int attribute, ref int value, int size);

        internal static bool SystemDark()
        {
            if (SystemParameters.HighContrast) return false;
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize"))
                    return key != null && Object.Equals(key.GetValue("AppsUseLightTheme"), 0);
            }
            catch (System.Security.SecurityException) { return false; }
            catch (UnauthorizedAccessException) { return false; }
            catch (System.IO.IOException) { return false; }
        }

        internal static void Attach(Window window) { Attach(window, SystemDark); }
        internal static void Attach(Window window, Func<bool> readDark)
        { new WindowTheme(window, readDark); }

        WindowTheme(Window window, Func<bool> readDark)
        {
            this.window = window;
            this.readDark = readDark;
            window.SourceInitialized += Initialized;
            window.Closed += Closed;
            if (new WindowInteropHelper(window).Handle != IntPtr.Zero) Initialized(null, EventArgs.Empty);
        }

        void Initialized(object sender, EventArgs args)
        {
            if (source != null) return;
            source = HwndSource.FromHwnd(new WindowInteropHelper(window).Handle);
            source.AddHook(Message);
            Refresh();
        }

        void Refresh()
        {
            bool dark = readDark();
            if (applied == dark) return;
            int value = dark ? 1 : 0;
            try
            {
                // Unsupported Windows versions retain the standard title bar.
                if (DwmSetWindowAttribute(source.Handle, 20, ref value, sizeof(int)) >= 0) applied = dark;
            }
            catch (DllNotFoundException) { }
            catch (EntryPointNotFoundException) { }
        }

        IntPtr Message(IntPtr hwnd, int message, IntPtr wp, IntPtr lp, ref bool handled)
        {
            if (message == 0x001A || message == 0x031A) Refresh(); // Settings/theme changed.
            return IntPtr.Zero;
        }

        void Closed(object sender, EventArgs args)
        {
            if (source != null && !source.IsDisposed) source.RemoveHook(Message);
            window.SourceInitialized -= Initialized;
            window.Closed -= Closed;
        }
    }
}
