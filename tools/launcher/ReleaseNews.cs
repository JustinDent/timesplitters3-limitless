using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace TimeSplitters.Launcher
{
    // Release descriptions are display-only data, never HTML, XAML or executable links.
    public sealed class ReleaseNews
    {
        public Release Game, Launcher, InstalledGame, InstalledLauncher;
        public DateTime CheckedUtc;
        public bool Preview;

        public static Release NotesFor(IEnumerable<Dictionary<string, object>> records, string tag)
        {
            var record = records.FirstOrDefault(r => !Json.Flag(r, "draft") && Json.String(r, "tag_name") == tag);
            if (record == null) return null;
            string version = tag.StartsWith("launcher-v", StringComparison.Ordinal) ? tag.Substring(10) : tag;
            return new Release { Tag=version, Notes=Json.String(record,"body"),
                Url=ReleaseClient.ReleasesPage+"/tag/"+Uri.EscapeDataString(tag),
                Name=Json.String(record,"name"), Published=Json.String(record,"published_at") };
        }
        public static string Availability(string installed, Release latest, bool launcher)
        {
            if (latest == null) return "No compatible published release is available on this channel.";
            if (String.IsNullOrEmpty(installed)) return "Available to install. Open Home to install the game runtime.";
            try {
                int order=GameVersion.Parse(latest.Tag).CompareTo(GameVersion.Parse(installed));
                if(order<0)return "Your installed version is newer than this channel's latest release. No downgrade is offered.";
                if(order==0)return "You're up to date on this channel. These notes describe your installed release.";
                return launcher ? "A newer launcher is available. Restart the launcher to let the startup updater install it." :
                    "A newer game release is available. Read what changes below, then open Home to update.";
            } catch(System.IO.InvalidDataException) { return "The installed version cannot be compared. Review the published release below."; }
        }
    }

    public sealed class ReleaseNotesText
    {
        public readonly string Changes, KnownIssues;
        public readonly bool Shortened;
        public ReleaseNotesText(string notes)
        {
            notes=(notes??"").Replace("\r\n","\n").Replace('\r','\n');
            if(notes.Length>64000){notes=notes.Substring(0,64000);Shortened=true;}
            var changes=new StringBuilder();var issues=new StringBuilder();
            bool inIssues=false,fenced=false;int issueLevel=0,lineCount=0;
            foreach(string line in notes.Split('\n')) {
                if(++lineCount>512){Shortened=true;break;}
                bool fence=Regex.IsMatch(line,@"^\s*(```|~~~)");
                var heading=fenced?Match.Empty:Regex.Match(line,@"^\s{0,3}(#{1,6})\s+(.+?)\s*#*\s*$");
                if(heading.Success) {
                    int level=heading.Groups[1].Value.Length;
                    if(inIssues && level<=issueLevel)inIssues=false;
                    string title=Plain(heading.Groups[2].Value).Trim().TrimEnd(':').ToLowerInvariant();
                    if(Regex.IsMatch(title,@"^(known issues|known limitations|known bugs|limitations|known issues\s*[/&]\s*limitations)$")) {
                        inIssues=true;issueLevel=level;continue;
                    }
                }
                (inIssues?issues:changes).AppendLine(line);
                if(fence)fenced=!fenced;
            }
            Changes=changes.ToString().Trim();KnownIssues=issues.ToString().Trim();
        }
        public static string Plain(string text)
        {
            text=Regex.Replace(text,@"!\[[^\]]*\]\([^\r\n]*?\)","[Image: view on GitHub]");
            text=Regex.Replace(text,@"\[([^\]]+)\]\(([^\r\n]*?)\)","$1 ($2)");
            return text.Replace("**","").Replace("__","").Replace("`","");
        }
    }
}
