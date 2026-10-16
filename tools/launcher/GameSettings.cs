using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;

namespace TimeSplitters.Launcher
{
    public sealed class GameSettings
    {
        public static readonly string[] Actions = { "a", "b", "x", "y", "black", "white", "start", "back", "ltrigger", "rtrigger", "dpad_up", "dpad_down", "dpad_left", "dpad_right", "lstick_up", "lstick_down", "lstick_left", "lstick_right", "lthumb", "rstick_up", "rstick_down", "rstick_left", "rstick_right", "rthumb" };
        public static readonly string[] Labels = { "Interact / confirm (A)", "Melee / back (B)", "Reload (X)", "Y action", "Black button action", "White button action", "Pause / Start", "Objectives / Back", "Throw grenade", "Fire", "Weapon mode / zoom in", "Grenade type / zoom out", "Previous weapon", "Next weapon", "Move forward", "Move backward", "Strafe left", "Strafe right", "Crouch", "Look up", "Look down", "Look left", "Look right", "Aim" };
        readonly string root;
        readonly Dictionary<string, byte[]> originals = new Dictionary<string, byte[]>();
        Dictionary<string, object> document, controller, bindings;
        string runtime;
        public double Sensitivity = .10;
        public bool MouseEnabled = true, InvertY, Fullscreen, Vrr = true, FpsOverlay, SkipStartupSplashes;
        public const int FovDefault = 55, FovMin = 40, FovMax = 100;
        public int VerticalFov = FovDefault;
        public bool CanAdjustFov
        {
            get {
                // alpha.4 and earlier accept only the V1 display format.
                // Unpackaged development builds have no BUILD.json.
                try {
                    string version = Core.InstalledVersion(root);
                    return version.Length == 0 || GameVersion.Parse(version).CompareTo(GameVersion.Parse("v0.2.0-alpha.4")) > 0;
                } catch { return false; }
            }
        }
        public int Width, Height, Scale = 1, Anisotropy = 1, Interpolation = 1;
        public string FrameCap = "adaptive";
        public string Device { get { return Convert.ToString(controller["device"], CultureInfo.InvariantCulture); } set { controller["device"] = value; } }
        static readonly string[] Files = { "Settings/pc-controls.json", "Settings/pc-display.ini", "Settings/runtime.conf" };
        static Dictionary<string, object> Map(object value)
        { var map = value as Dictionary<string, object>; Core.Require(map != null, "Settings contain an invalid object."); return map; }
        static byte[] Read(string path)
        { if (!File.Exists(path)) return null; Core.Require(new FileInfo(path).Length <= 1024 * 1024, "Settings file exceeds 1 MiB."); return File.ReadAllBytes(path); }
        static string Text(byte[] bytes) { return bytes == null ? "" : Encoding.UTF8.GetString(bytes).TrimStart('\ufeff'); }
        static string DefaultControls()
        { using (var s = Assembly.GetExecutingAssembly().GetManifestResourceStream("launcher-controls-default.json")) using (var r = new StreamReader(s)) return r.ReadToEnd(); }
        public static GameSettings Load(string root, string inheritedRuntime)
        {
            var model = new GameSettings(root);
            foreach (string file in Files) model.originals[file] = Read(Core.PathIn(root, file));
            model.document = Json.Object(model.originals[Files[0]] == null ? DefaultControls() : Text(model.originals[Files[0]]));
            model.ReadController();
            object value;
            if (model.document.TryGetValue("launcher", out value))
            {
                var launcher = Map(value);
                if (launcher.TryGetValue("skip_startup_splashes", out value))
                {
                    Core.Require(value is bool, "Skip startup splash screens must be true or false. No settings were changed.");
                    model.SkipStartupSplashes = (bool)value;
                }
            }
            if (model.document.TryGetValue("mouse", out value))
            {
                var mouse = Map(value);
                if (mouse.TryGetValue("sensitivity", out value)) model.Sensitivity = Convert.ToDouble(value, CultureInfo.InvariantCulture);
                if (mouse.TryGetValue("enabled", out value)) model.MouseEnabled = Convert.ToBoolean(value, CultureInfo.InvariantCulture);
                if (mouse.TryGetValue("invert_y", out value)) model.InvertY = Convert.ToBoolean(value, CultureInfo.InvariantCulture);
            }
            string display = Text(model.originals[Files[1]]);
            if (display.Length > 0)
            {
                var match = Regex.Match(display, @"^\s*TSFP_DISPLAY_V(?<version>[12])\s+(\d+)\s+(\d+)(?:\s+(?<fov>\d+))?\s*$");
                Core.Require(match.Success && (match.Groups["version"].Value == "2") == match.Groups["fov"].Success, "The display settings file is invalid. No settings were changed.");
                if(match.Groups["fov"].Success) model.VerticalFov = Int32.Parse(match.Groups["fov"].Value, CultureInfo.InvariantCulture);
                model.Width = Int32.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
                model.Height = Int32.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture);
            }
            model.runtime = Text(model.originals[Files[2]]);
            if (model.originals[Files[2]] == null && !String.IsNullOrEmpty(inheritedRuntime)) model.runtime = Text(Read(inheritedRuntime));
            var settings = ParseRuntime(model.runtime);
            model.Scale = Number(settings, "resolution_scale", 1);
            model.Anisotropy = Number(settings, "anisotropy", 1);
            model.Interpolation = Number(settings, "frame_interp", 1);
            model.Fullscreen = Number(settings, "fullscreen", 0) != 0;
            model.Vrr = Number(settings, "vrr", 1) != 0;
            model.FpsOverlay = Number(settings, "fps_overlay", 0) != 0;
            if (settings.ContainsKey("frame_cap")) model.FrameCap = settings["frame_cap"];
            model.Validate();
            return model;
        }
        GameSettings(string path) { root = Path.GetFullPath(path); }
        void ReadController()
        {
            object value;
            Core.Require(document.TryGetValue("controllers", out value) && value is IEnumerable, "Controls file has no controller profiles.");
            int index = 0;
            foreach (object item in (IEnumerable)value)
            {
                var profile = Map(item); object port;
                if ((profile.TryGetValue("port", out port) && Convert.ToInt32(port) == 1) || (!profile.ContainsKey("port") && index == 0))
                { Core.Require(controller == null, "Controls file contains duplicate Player 1 profiles."); controller = profile; }
                index++;
            }
            Core.Require(controller != null, "Controls file has no Player 1 profile.");
            if (!controller.ContainsKey("device")) controller["device"] = "keyboard";
            if (!controller.TryGetValue("bindings", out value)) controller["bindings"] = value = new Dictionary<string, object>();
            bindings = Map(value);
        }
        public string[] Binding(int action)
        {
            object value; var list = new List<string>();
            if (bindings.TryGetValue(Actions[action], out value))
            {
                if (value is string) list.Add((string)value);
                else if (value is IEnumerable) foreach (object item in (IEnumerable)value) { Core.Require(item is string, "Invalid binding source."); list.Add((string)item); }
                else throw new InvalidDataException("Invalid binding value.");
            }
            Core.Require(list.Count <= 4, "This profile has more than four bindings on an action; it was left unchanged.");
            while (list.Count < 4) list.Add("none");
            return list.ToArray();
        }
        public static bool KeyAllowed(int key)
        { return key >= 0 && key <= 255 && key != 112 && key != 113 && !(key >= 120 && key <= 123) && key != 91 && key != 92; }
        public static int KeyCode(string token)
        {
            if (!token.StartsWith("key:", StringComparison.OrdinalIgnoreCase)) return 0;
            string name = token.Substring(4); int number;
            if (name.StartsWith("#") && Int32.TryParse(name.Substring(1), out number)) return number;
            if (name.Length == 1 && Char.IsLetterOrDigit(name[0])) return Char.ToUpperInvariant(name[0]);
            var names = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase) {
                {"LBUTTON",1},{"RBUTTON",2},{"MBUTTON",4},{"XBUTTON1",5},{"XBUTTON2",6},
                {"UP",38},{"DOWN",40},{"LEFT",37},{"RIGHT",39},{"RETURN",13},{"ENTER",13},{"BACK",8},{"BACKSPACE",8},
                {"SPACE",32},{"TAB",9},{"ESCAPE",27},{"SHIFT",16},{"CTRL",17},{"CONTROL",17},{"ALT",18},
                {"LSHIFT",160},{"RSHIFT",161},{"LCTRL",162},{"RCTRL",163},{"LALT",164},{"RALT",165},
                {"CAPSLOCK",20},{"INSERT",45},{"DELETE",46},{"HOME",36},{"END",35},{"PAGEUP",33},{"PAGEDOWN",34} };
            if (names.TryGetValue(name, out number)) return number;
            if (name.StartsWith("F") && Int32.TryParse(name.Substring(1), out number) && number >= 1 && number <= 24) return 111 + number;
            return 0;
        }
        public int Conflict(int action, int key)
        { for (int i = 0; i < Actions.Length; i++) if (i != action && Binding(i).Any(t => KeyCode(t) == key && key != 0)) return i; return -1; }
        public void Assign(int action, int slot, int key, bool move)
        {
            Core.Require(action >= 0 && action < 24 && slot >= 0 && slot < 4 && KeyAllowed(key), "That key is reserved by the game. Choose another key.");
            Core.Require(move || Conflict(action, key) < 0, "This key is already assigned to another action.");
            if (key != 0) for (int i = 0; i < 24; i++)
            {
                var keys = Binding(i); bool changed = false;
                for (int j = 0; j < 4; j++) if (KeyCode(keys[j]) == key) { keys[j] = "none"; changed = true; }
                if (changed) bindings[Actions[i]] = keys;
            }
            var assigned = Binding(action); assigned[slot] = key == 0 ? "none" : "key:#" + key;
            bindings[Actions[action]] = assigned;
        }
        public void ResetControls()
        {
            var defaults = Json.Object(DefaultControls());
            var profiles = (IEnumerable)defaults["controllers"];
            foreach (object profile in profiles) { bindings = Map(Map(profile)["bindings"]); break; }
            controller["bindings"] = bindings; Device = "keyboard";
            Sensitivity = .10; MouseEnabled = true; InvertY = false;
        }
        public static Dictionary<string, string> ParseRuntime(string text)
        {
            var values = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (string line in text.Split('\n'))
            { var m = Regex.Match(line, @"^\s*([a-zA-Z_]+)\s*=\s*([^#;\r]*?)\s*(?:[#;].*)?$"); if (m.Success) values[m.Groups[1].Value] = m.Groups[2].Value; }
            return values;
        }
        static int Number(Dictionary<string, string> values, string key, int fallback)
        { string v; int n; if (!values.TryGetValue(key, out v)) return fallback; if (Int32.TryParse(v, out n)) return n; if (v == "true" || v == "on" || v == "yes") return 1; if (v == "false" || v == "off" || v == "no") return 0; throw new InvalidDataException("Invalid runtime setting: " + key); }
        public void Validate()
        {
            Core.Require(!Double.IsNaN(Sensitivity) && !Double.IsInfinity(Sensitivity) && Sensitivity >= .001 && Sensitivity <= 10, "Mouse sensitivity must be between 0.001 and 10.");
            Core.Require(CanAdjustFov || VerticalFov == FovDefault, "Custom FOV requires a game release newer than v0.2.0-alpha.4. Reset FOV to 55 degrees before saving.");
            Core.Require(VerticalFov >= FovMin && VerticalFov <= FovMax, "Vertical FOV must be between 40 and 100 degrees.");
            Core.Require(DisplayMode.Valid(Width,Height), "Choose a resolution from 640×480 through 7680×4320, with an aspect ratio no wider than 4:1, or 0 × 0 for the console default.");
            Core.Require(Scale >= 1 && Scale <= 8 && Anisotropy >= 1 && Anisotropy <= 16 && Interpolation >= 1 && Interpolation <= 4, "A graphics setting is outside the supported range.");
            Core.Require(new[] { "adaptive", "30", "60", "0" }.Contains(FrameCap), "Unsupported frame pacing option.");
            Core.Require(new[] { "keyboard", "none", "gamepad:0", "gamepad:1", "gamepad:2", "gamepad:3", "xinput:0", "xinput:1", "xinput:2", "xinput:3" }.Contains(Device), "Unsupported Player 1 input device.");
            for (int i = 0; i < 24; i++) Binding(i);
        }
        public static bool Running(string root)
        {
            bool running = false; string exe = Core.PathIn(root, "Bin/futureperfect_recomp.exe");
            foreach (var p in Process.GetProcessesByName("futureperfect_recomp")) using (p)
                try { if (!p.HasExited && String.Equals(p.MainModule.FileName, exe, StringComparison.OrdinalIgnoreCase)) running = true; }
                catch (InvalidOperationException) { } catch (System.ComponentModel.Win32Exception) { running = true; }
            return running;
        }
        static string UpdateRuntime(string text, Dictionary<string, string> values)
        {
            var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            string changed = Regex.Replace(text, @"(?m)^\s*([a-zA-Z_]+)[ \t]*=[^\r\n]*", m => {
                string key = m.Groups[1].Value; if (!values.ContainsKey(key)) return m.Value;
                seen.Add(key); return key + " = " + values[key]; });
            foreach (var pair in values) if (!seen.Contains(pair.Key)) changed += "\n" + pair.Key + " = " + pair.Value;
            return changed.TrimEnd('\r','\n') + "\n";
        }
        static void Replace(string path, byte[] data, bool backup)
        {
            string temp = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(path));
                using (var f = new FileStream(temp, FileMode.CreateNew, FileAccess.Write, FileShare.None)) { f.Write(data, 0, data.Length); f.Flush(true); }
                if (File.Exists(path)) File.Replace(temp, path, backup ? path + ".bak" : null);
                else File.Move(temp, path);
            }
            finally { if (File.Exists(temp)) File.Delete(temp); }
        }
        public void Save()
        {
            Validate(); Core.Require(!Running(root), "Close the game before saving settings.");
            foreach (string file in Files)
            {
                string path = Core.PathIn(root, file); byte[] now = Read(path), old = originals[file];
                Core.PathIn(root, file + ".bak");
                Core.Require(!Directory.Exists(path), "A settings path is a directory. No settings were saved.");
                Core.Require((now == null && old == null) || (now != null && old != null && now.SequenceEqual(old)), "Settings changed outside the launcher. Close and reopen Settings before saving.");
                Core.Require(!File.Exists(path) || (File.GetAttributes(path) & FileAttributes.ReadOnly) == 0, "A settings file is read-only. No settings were saved.");
            }
            object value;
            if (!document.TryGetValue("mouse", out value)) document["mouse"] = value = new Dictionary<string, object>();
            var mouse = Map(value); mouse["sensitivity"] = Sensitivity; mouse["enabled"] = MouseEnabled; mouse["invert_y"] = InvertY;
            if (!document.TryGetValue("launcher", out value)) document["launcher"] = value = new Dictionary<string, object>();
            Map(value)["skip_startup_splashes"] = SkipStartupSplashes;
            var graphics = new Dictionary<string, string> { {"resolution_scale",Scale.ToString()}, {"anisotropy",Anisotropy.ToString()}, {"frame_interp",Interpolation.ToString()}, {"fullscreen",Fullscreen?"1":"0"}, {"vrr",Vrr?"1":"0"}, {"fps_overlay",FpsOverlay?"1":"0"}, {"frame_cap",FrameCap} };
            var payload = new[] { Json.Encode(document) + "\n", (VerticalFov == FovDefault ? "TSFP_DISPLAY_V1 " + Width + " " + Height : "TSFP_DISPLAY_V2 " + Width + " " + Height + " " + VerticalFov) + "\n", UpdateRuntime(runtime, graphics) };
            int committed = 0;
            try { for (int i = 0; i < Files.Length; i++) { Replace(Core.PathIn(root, Files[i]), Encoding.UTF8.GetBytes(payload[i]), true); committed++; } }
            catch
            {
                for (int i = committed - 1; i >= 0; i--) { string path = Core.PathIn(root, Files[i]); if (originals[Files[i]] == null) File.Delete(path); else Replace(path, originals[Files[i]], false); }
                throw;
            }
            foreach (string file in Files) originals[file] = Read(Core.PathIn(root, file));
            runtime = payload[2];
        }
        public static bool HasGraphics(string root) { return File.Exists(Core.PathIn(root, Files[2])); }
        public static ProcessStartInfo StartInfo(string root)
        {
            var model = Load(root, null); model.Validate();
            var info = new ProcessStartInfo(Core.PathIn(root, "Bin/futureperfect_recomp.exe")) { WorkingDirectory = Core.PathIn(root, "Bin"), UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true, RedirectStandardError = true };
            foreach (string key in info.EnvironmentVariables.Keys.Cast<string>().ToArray())
                if (key.StartsWith("RECOMP_", StringComparison.OrdinalIgnoreCase) || key.StartsWith("TSFP_", StringComparison.OrdinalIgnoreCase)) info.EnvironmentVariables.Remove(key);
            foreach (var pair in new Dictionary<string, string> {
                {"RECOMP_GAME_DIR","Data"}, {"RECOMP_SAVE_DIR","Saves"}, {"RECOMP_INPUT_CONFIG",Files[0]}, {"RECOMP_FFMPEG_DIR","Bin"}, {"RECOMP_DISPLAY_CONFIG",Files[2]}, {"TSFP_DISPLAY_CONFIG",Files[1]}, {"TSFP_TEXTURE_PACK_DIR","Textures"}, {"TSFP_ONLINE_SETTINGS","Settings/pc-online.json"} }) info.EnvironmentVariables[pair.Key] = Core.PathIn(root, pair.Value);
            info.EnvironmentVariables["RECOMP_TRACE_BUDGET"] = "0";
            // The existing XMV HLE completes skipped movies normally. Story
            // cutscenes are rendered by the game and do not use this player.
            info.EnvironmentVariables["RECOMP_XMV_PLAY"] = model.SkipStartupSplashes ? "0" : "1";
            info.EnvironmentVariables["RECOMP_PC_MOUSE"] = model.MouseEnabled ? "1" : "0";
            info.EnvironmentVariables["RECOMP_MOUSE_INVERT_Y"] = model.InvertY ? "1" : "0";
            info.EnvironmentVariables["RECOMP_MOUSE_SENSITIVITY"] = model.Sensitivity.ToString("R", CultureInfo.InvariantCulture);
            info.EnvironmentVariables["TSFP_TEXTURE_MODE"] = "classic";
            return info;
        }
    }
}
