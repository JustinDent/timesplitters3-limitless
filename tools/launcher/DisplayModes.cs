using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace TimeSplitters.Launcher
{
    public sealed class DisplayMode
    {
        public readonly int Width, Height;
        public bool SavedCustom;
        public DisplayMode(int width, int height) { Width=width; Height=height; }
        public override string ToString() { return Width==0 ? "Original console resolution" : Width+" × "+Height+(SavedCustom?" (saved custom)":""); }
        public static bool Valid(int w,int h)
        { return (w==0 && h==0) || (w>=640 && h>=480 && w<=7680 && h<=2160 && (long)w*h<=16588800 && w>=h && w<=4*h); }
    }
    public static class DisplayModes
    {
        // DEVMODEW's physical-pixel fields; fixed native layout is independent of DPI.
        [StructLayout(LayoutKind.Explicit, Size=220, CharSet=CharSet.Unicode)]
        struct Mode
        {
            [FieldOffset(68)] public ushort Size;
            [FieldOffset(70)] public ushort Extra;
            [FieldOffset(168)] public uint Bits;
            [FieldOffset(172)] public int Width;
            [FieldOffset(176)] public int Height;
        }
        [DllImport("user32.dll", CharSet=CharSet.Unicode, ExactSpelling=true)]
        [return:MarshalAs(UnmanagedType.Bool)]
        static extern bool EnumDisplaySettingsW(string device,uint index,ref Mode mode);
        public static List<DisplayMode> Choices(IEnumerable<DisplayMode> reported,int savedWidth,int savedHeight)
        {
            var list=new List<DisplayMode>{new DisplayMode(0,0)};
            list.AddRange(reported.Where(m=>m.Width!=0 && DisplayMode.Valid(m.Width,m.Height))
                .GroupBy(m=>new {m.Width,m.Height}).Select(g=>new DisplayMode(g.Key.Width,g.Key.Height))
                .OrderBy(m=>m.Width).ThenBy(m=>m.Height));
            if(DisplayMode.Valid(savedWidth,savedHeight) && !list.Any(m=>m.Width==savedWidth && m.Height==savedHeight))
                list.Add(new DisplayMode(savedWidth,savedHeight){SavedCustom=true});
            return list;
        }
        public static List<DisplayMode> Primary(int savedWidth,int savedHeight)
        {
            var reported=new List<DisplayMode>();
            var screen=System.Windows.Forms.Screen.PrimaryScreen;
            if(screen!=null) for(uint index=0;;index++)
            {
                var mode=new Mode{Size=220};
                if(!EnumDisplaySettingsW(screen.DeviceName,index,ref mode)) break;
                if(mode.Bits>=24) reported.Add(new DisplayMode(mode.Width,mode.Height));
            }
            return Choices(reported,savedWidth,savedHeight);
        }
    }
}
