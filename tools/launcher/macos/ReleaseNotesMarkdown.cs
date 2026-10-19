using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
namespace TimeSplitters.Launcher
{
    // Text-document adapter; parser, link policy and page interaction are shared.
    public sealed class ReleaseNotesMarkdown:ScrollViewer
    {
        readonly StackPanel body=new StackPanel();readonly Action<string> openLink;
        public ReleaseNotesMarkdown(Action<string> openLink=null) { this.openLink=openLink;Content=body;VerticalScrollBarVisibility=ScrollBarVisibility.Auto;HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled; }
        public static bool TryLink(string value,out Uri uri) { return ReleaseNotesDocument.TryLink(value,out uri); }
        public void SetMarkdown(string source) { body.Children.Clear();NativeMarkdown.Render(body,source,openLink);Ui.ScrollHome(this); }
    }
}
