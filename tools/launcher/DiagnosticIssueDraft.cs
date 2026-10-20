using System;
using System.Linq;

namespace TimeSplitters.Launcher
{
    // Generates an editable browser draft. This class never posts an issue or contacts GitHub.
    public sealed class DiagnosticIssueDraft
    {
        public const string DiagnosticsField = "diagnostics";
        public const int MaximumUrlLength = 1800;
        public string Url { get; private set; }
        public string ClipboardText { get; private set; }
        public bool RequiresPaste { get; private set; }
        static string ShortLine(string text)
        {
            if(text.Length<=100)return text;
            return text.Substring(0,Char.IsHighSurrogate(text[99])?99:100);
        }
        public static DiagnosticIssueDraft Create(bool enhancement,string report)
        {
            if(enhancement)return new DiagnosticIssueDraft {
                Url="https://github.com/"+ReleaseClient.Repository+"/issues/new?template=02-enhancement.yml",
                ClipboardText="",RequiresPaste=false};
            Core.Require(!String.IsNullOrWhiteSpace(report),"Refresh diagnostics before preparing a GitHub report.");
            string prefix="https://github.com/"+ReleaseClient.Repository+"/issues/new?template="+
                "01-bug-report.yml&"+DiagnosticsField+"=";
            string url=report.Length<=4096?prefix+Uri.EscapeDataString(report):"";
            bool paste=url.Length==0 || url.Length>MaximumUrlLength;
            if(paste) {
                // Preserve the complete report on the clipboard. Never silently cut off its findings.
                string[] headings={"Running launcher:","Installed launcher:","Installed game:","Windows:","Host:",".NET CLR:"};
                string[] summary=report.Split('\n').Select(line=>line.TrimEnd('\r'))
                    .Where(line=>headings.Any(heading=>line.StartsWith(heading,StringComparison.Ordinal))).Take(5).ToArray();
                string text="Summary only. The complete diagnostic report is copied in the launcher. Replace this field with that report before submitting.\n\n"+
                    String.Join("\n",summary.Select(ShortLine));
                url=prefix+Uri.EscapeDataString(text);
                // Keep even unusual Unicode summaries within the browser handoff budget.
                if(url.Length>MaximumUrlLength)url=prefix+Uri.EscapeDataString("Paste the full diagnostic report copied by the launcher here, then review it before submitting.");
            }
            return new DiagnosticIssueDraft {Url=url,ClipboardText=report,RequiresPaste=paste};
        }
    }
}
