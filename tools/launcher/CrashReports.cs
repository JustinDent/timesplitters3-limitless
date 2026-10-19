using System;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text.RegularExpressions;

namespace TimeSplitters.Launcher
{
    public static class CrashReports
    {
        static readonly string[] Names = { "report.txt", "report.json", "crash.dmp", "game-log-tail.txt" };
        public static string Latest(string root)
        {
            try {
            string area = Core.PathIn(root, "Logs/Crashes");
            if (!Directory.Exists(area)) return null;
            foreach (string directory in Directory.EnumerateDirectories(area).OrderByDescending(Path.GetFileName).Take(128)) {
                string name = Path.GetFileName(directory);
                if (!Regex.IsMatch(name, @"^crash-\d{8}-\d{9}-\d{10}-\d{6}$")) continue;
                try {
                    string report = Core.PathIn(root, "Logs/Crashes/" + name + "/report.json");
                    if (!File.Exists(report) || new FileInfo(report).Length > 8192) continue;
                    using (var locked = new FileStream(Core.PathIn(root,"Logs/Crashes/"+name+"/capture.lock"), FileMode.Open, FileAccess.Read, FileShare.None)) { }
                    var metadata = Json.Object(File.ReadAllText(report));
                    string kind=Json.String(metadata,"kind");
                    if (Convert.ToInt32(metadata["schema"]) == 1 && (kind == "unhandled_exception" || kind == "suspected_hang")) return name;
                } catch { /* A report may still be writing or inaccessible. */ }
            }
            } catch { /* Unavailable diagnostics must not prevent starting the game. */ }
            return null;
        }
        public static string Folder(string root, string name)
        {
            Core.Require(name != null && Regex.IsMatch(name,@"^crash-\d{8}-\d{9}-\d{10}-\d{6}$"), "Invalid crash report.");
            return Core.PathIn(root,"Logs/Crashes/"+name);
        }
        public static string Summary(string root)
        {
            try {
                string name = Latest(root);
                if (name == null) return "No completed crash report found.";
                var metadata = Json.Object(File.ReadAllText(Path.Combine(Folder(root,name),"report.json")));
                string code = Json.String(metadata,"exception_code"), hash = Json.String(metadata,"executable_sha256");
                if (!Regex.IsMatch(code,@"^0x[0-9A-Fa-f]{8}$")) code="unknown";
                if (!Regex.IsMatch(hash,@"^[0-9a-f]{64}$")) hash="unavailable";
                return name+(Json.String(metadata,"kind")=="suspected_hang"?"\nType: Suspected hang (the game was left running)":"\nException: "+code)+"\nMinidump: "+(metadata.ContainsKey("dump_saved") && Object.Equals(metadata["dump_saved"],true)?"saved":"unavailable; summary retained")+"\nExecutable SHA-256: "+hash;
            } catch { return "Crash reports unavailable; files retained."; }
        }
        public static bool IsHang(string root,string name)
        {
            if(name==null)return false;
            try {
                string path=Core.PathIn(root,"Logs/Crashes/"+Path.GetFileName(Folder(root,name))+"/report.json");
                if(new FileInfo(path).Length>8192)return false;
                return Json.String(Json.Object(File.ReadAllText(path)),"kind")=="suspected_hang";
            } catch {return false;}
        }
        public static void Export(string root, string name, string destination)
        {
            string folder = Folder(root,name);
            Core.Require(IsHang(root,name) || !Core.GameRunning(), "Close the game before exporting a crash report.");
            Core.Require(Latest(root) == name, "Refresh diagnostics before exporting this report.");
            string temporary = destination + "." + Guid.NewGuid().ToString("N") + ".tmp";
            // Only this report's known files; never sweep the game, settings or saves.
            try {
            using (var locked = new FileStream(Path.Combine(folder,"capture.lock"),FileMode.Open,FileAccess.Read,FileShare.Read))
            using (var output = new FileStream(temporary,FileMode.CreateNew,FileAccess.Write,FileShare.None))
            using (var zip = new ZipArchive(output,ZipArchiveMode.Create)) {
                foreach (string file in Names) {
                    string path=Core.PathIn(root,"Logs/Crashes/"+name+"/"+file);
                    if (!File.Exists(path)) continue;
                    long limit=file=="crash.dmp"?16*1024*1024:file=="game-log-tail.txt"?65536:8192;
                    using (var input=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read)) {
                        Core.Require(input.Length<=limit,"Crash report file exceeds its size limit.");
                        using (var entry=zip.CreateEntry(file,CompressionLevel.Fastest).Open()) input.CopyTo(entry);
                    }
                }
            }
            File.Move(temporary,destination);
            } finally { if(File.Exists(temporary))File.Delete(temporary); }
        }
    }
}
