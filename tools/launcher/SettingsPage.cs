using System;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace TimeSplitters.Launcher
{
    public sealed class SettingsPage
    {
        public readonly Grid View;
        readonly DockPanel layout;
        readonly Border overlay;
        readonly TabControl tabs;
        public event Action Saved, Cancelled;
        readonly GameSettings model;
        readonly TextBlock status;
        readonly Slider fov;
        readonly TextBox sensitivity, width, height, discordId;
        readonly CheckBox enabled, invert, fullscreen, vrr, fps, skipSplashes, discordEnabled;
        readonly ComboBox device, resolution, aspect, scale, filtering, pacing, interpolation;
        readonly Button[,] bindingButtons = new Button[24,4];
        static readonly string[] Devices = { "keyboard", "gamepad:0", "gamepad:1", "gamepad:2", "gamepad:3", "none" };
        static Brush ColorBrush(byte r,byte g,byte b) { var brush=new SolidColorBrush(Color.FromRgb(r,g,b));brush.Freeze();return brush; }
        static readonly Brush Ink = ColorBrush(228,237,245);
        static readonly Brush Muted = ColorBrush(158,175,191);
        static readonly Brush Accent = ColorBrush(134,205,255);
        static readonly Brush Panel = ColorBrush(21,36,53);
        static TextBlock Text(string text, int size) { var block=PageTheme.Text(text,size);block.Margin=new Thickness(0,0,0,8);return block; }
        static Button Button(string text) { return new Button { Content=text, Margin=new Thickness(0,0,8,0) }; }
        static CheckBox Check(string text, bool value) { return new CheckBox { Content=text, IsChecked=value, Foreground=Ink, Margin=new Thickness(0,7,20,7) }; }
        static TextBox Entry(string text) { return new TextBox { Text=text, MinWidth=90 }; }
        static ComboBox Choices(string[] choices, string selected)
        {
            var combo = new ComboBox();
            foreach (string choice in choices) combo.Items.Add(choice);
            if (!combo.Items.Contains(selected)) combo.Items.Add(selected);
            combo.SelectedItem=selected; return combo;
        }
        static void Field(Panel parent, string label, UIElement input)
        {
            var stack=new StackPanel { Margin=new Thickness(0,0,0,14) };var caption=Text(label,12);caption.Foreground=Muted;
            stack.Children.Add(caption);stack.Children.Add(input);parent.Children.Add(stack);
            System.Windows.Automation.AutomationProperties.SetLabeledBy(input,caption);
        }
        static Grid Columns(Panel parent)
        {
            var grid=new Grid();grid.ColumnDefinitions.Add(new ColumnDefinition());grid.ColumnDefinitions.Add(new ColumnDefinition());parent.Children.Add(grid);return grid;
        }
        static StackPanel Column(Grid grid,int index)
        { var panel=new StackPanel { Margin=index==0?new Thickness(0,0,10,0):new Thickness(10,0,0,0) };Grid.SetColumn(panel,index);grid.Children.Add(panel);return panel; }
        static ScrollViewer Scroll(UIElement content) { return new ScrollViewer { VerticalScrollBarVisibility=ScrollBarVisibility.Auto, HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled, Content=content, Padding=new Thickness(0,0,12,0) }; }
        public SettingsPage(ResourceDictionary resources, string root, bool preview)
        {
            string inherited = preview ? null : Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "xboxrecomp", "titles", "45410066.conf");
            model=GameSettings.Load(root, inherited);
            View=new Grid { Background=PageTheme.Brush("#0B1420"), Margin=new Thickness(30,24,30,18) };
            View.Resources.MergedDictionaries.Add(resources);
            View.Resources.MergedDictionaries.Add(PageTheme.Resources());
            View.Resources.MergedDictionaries.Add(PageTheme.Tabs());
            layout=new DockPanel(); View.Children.Add(layout);
            overlay=new Border { Visibility=Visibility.Collapsed, Background=new SolidColorBrush(Color.FromArgb(248,11,20,32)), BorderBrush=Accent, BorderThickness=new Thickness(1), CornerRadius=new CornerRadius(7), Padding=new Thickness(28) };
            View.Children.Add(overlay);
            var heading=new StackPanel(); DockPanel.SetDock(heading,Dock.Top); layout.Children.Add(heading);
            var eyebrow=PageTheme.Text("GAME PREFERENCES",10,"#86CDFF");eyebrow.FontWeight=FontWeights.SemiBold;heading.Children.Add(eyebrow);
            var title=Text("Game settings",28);title.Margin=new Thickness(0,5,0,7);heading.Children.Add(title);
            var subtitle=Text("Adjust controls, display and performance before your next launch.",12); subtitle.Foreground=Muted; heading.Children.Add(subtitle);
            var footer=new DockPanel { LastChildFill=true };
            var footerBorder=new Border { BorderBrush=PageTheme.Brush("#2D455C"),BorderThickness=new Thickness(0,1,0,0),Padding=new Thickness(0,16,0,0),Child=footer };
            DockPanel.SetDock(footerBorder,Dock.Bottom);layout.Children.Add(footerBorder);
            var actions=new StackPanel { Orientation=Orientation.Horizontal, VerticalAlignment=VerticalAlignment.Center };
            DockPanel.SetDock(actions,Dock.Right);footer.Children.Add(actions);
            var cancel=Button("Cancel"); cancel.Name="SettingsCancelButton"; cancel.Click+=(s,e)=> { if(Cancelled!=null)Cancelled(); }; actions.Children.Add(cancel);
            var save=Button("Save settings"); save.Name="SettingsSaveButton";save.Margin=new Thickness(0);PageTheme.Primary(save);save.Click+=(s,e)=>Save();actions.Children.Add(save);
            status=Text("Home keeps your draft. Save when you’re ready.",12);status.Foreground=Muted;status.Margin=new Thickness(0,0,16,0);status.VerticalAlignment=VerticalAlignment.Center;footer.Children.Add(status);
            tabs=new TabControl { Background=Brushes.Transparent,BorderThickness=new Thickness(0), Margin=new Thickness(0,10,0,18) };layout.Children.Add(tabs);
            var controls=new StackPanel();
            var input=PageTheme.Section(controls,"Input & mouse","Choose your input device and adjust how the camera responds.");
            var inputColumns=Columns(input);var inputLeft=Column(inputColumns,0);var inputRight=Column(inputColumns,1);
            var deviceLabels=new[]{"Keyboard & mouse","Controller 1","Controller 2","Controller 3","Controller 4","Disabled"};
            device=Choices(deviceLabels,deviceLabels[Array.IndexOf(Devices,model.Device.Replace("xinput:","gamepad:"))]);Field(inputLeft,"Player 1 input device",device);
            sensitivity=Entry(model.Sensitivity.ToString("G",CultureInfo.InvariantCulture));sensitivity.Name="MouseSensitivity";Field(inputRight,"Mouse sensitivity · 0.001–10",sensitivity);
            var mouse=new WrapPanel();enabled=Check("Mouse look",model.MouseEnabled);invert=Check("Invert vertical look",model.InvertY);mouse.Children.Add(enabled);mouse.Children.Add(invert);input.Children.Add(mouse);
            var bindings=PageTheme.Section(controls,"Key bindings","Select a slot, then press a key or mouse button. Mouse wheel is supported.");
            var bindingHead=new Grid { Margin=new Thickness(0,0,0,6) };bindingHead.ColumnDefinitions.Add(new ColumnDefinition {Width=new GridLength(200)});
            var actionHeading=PageTheme.Text("ACTION",10,"#8FA8BD");actionHeading.Margin=new Thickness(4,6,0,6);bindingHead.Children.Add(actionHeading);
            for(int slot=0;slot<4;slot++){bindingHead.ColumnDefinitions.Add(new ColumnDefinition());var label=PageTheme.Text("SLOT "+(slot+1),10,"#8FA8BD");label.HorizontalAlignment=HorizontalAlignment.Center;Grid.SetColumn(label,slot+1);bindingHead.Children.Add(label);}bindings.Children.Add(bindingHead);
            for (int i=0;i<24;i++)
            {
                var row=new Grid { Margin=new Thickness(0,0,0,3), Background=i%2==0?PageTheme.Brush("#172B3F"):Brushes.Transparent };
                row.ColumnDefinitions.Add(new ColumnDefinition { Width=new GridLength(200) });
                var label=Text(GameSettings.Labels[i],12);label.VerticalAlignment=VerticalAlignment.Center;label.Margin=new Thickness(8,7,8,7);row.Children.Add(label);
                for (int j=0;j<4;j++)
                {
                    row.ColumnDefinitions.Add(new ColumnDefinition());
                    int action=i,slot=j;var button=Button("");button.FontSize=11;button.MinHeight=32;button.Padding=new Thickness(5,7,5,7);button.Margin=new Thickness(3);button.Background=PageTheme.Brush("#122233");button.BorderBrush=PageTheme.Brush("#365069");button.ToolTip=GameSettings.Labels[i]+", binding "+(j+1);
                    button.Click+=(s,e)=>Capture(action,slot);Grid.SetColumn(button,j+1);row.Children.Add(button);bindingButtons[i,j]=button;
                }
                bindings.Children.Add(row);
            }
            var reset=Button("Restore default controls");reset.Margin=new Thickness(0,16,0,0);reset.HorizontalAlignment=HorizontalAlignment.Left;
            reset.Click+=(s,e)=>Confirm("Restore default controls?","Replace Player 1 bindings and mouse settings with the defaults? Changes apply only when you save.","Restore defaults",()=> { model.ResetControls(); sensitivity.Text="0.10"; enabled.IsChecked=true; invert.IsChecked=false; device.SelectedIndex=0; RefreshBindings(); });
            bindings.Children.Add(reset);tabs.Items.Add(new TabItem { Header="Keyboard & mouse",Content=Scroll(controls) });
            var graphics=new StackPanel();
            var display=PageTheme.Section(graphics,"Display","Choose the game’s output size. Your desktop resolution stays unchanged.");
            var displayColumns=Columns(display);var displayLeft=Column(displayColumns,0);var displayRight=Column(displayColumns,1);
            var sizes=DisplayModes.Primary(model.Width,model.Height).Where(m=>model.SupportsResolution(m.Width,m.Height)).ToList();
            aspect=new ComboBox { Name="AspectRatio" };
            foreach(var ratio in DisplayModes.Aspects)aspect.Items.Add(ratio);
            aspect.SelectedIndex=0;Field(displayLeft,"Aspect ratio",aspect);
            resolution=new ComboBox { Name="OutputResolution" };
            foreach(var mode in sizes) resolution.Items.Add(mode);
            resolution.SelectedItem=sizes.FirstOrDefault(m=>m.Width==model.Width && m.Height==model.Height);
            var selector=new DockPanel();
            var previous=Button("‹");previous.Width=32;previous.Padding=new Thickness(0);previous.Margin=new Thickness(0,0,6,0);previous.ToolTip="Previous resolution";
            System.Windows.Automation.AutomationProperties.SetName(previous,"Previous resolution");
            var next=Button("›");next.Width=32;next.Padding=new Thickness(0);next.Margin=new Thickness(6,0,0,0);next.ToolTip="Next resolution";
            System.Windows.Automation.AutomationProperties.SetName(next,"Next resolution");
            previous.Click+=(s,e)=>resolution.SelectedIndex=(resolution.SelectedIndex+resolution.Items.Count-1)%resolution.Items.Count;
            next.Click+=(s,e)=>resolution.SelectedIndex=(resolution.SelectedIndex+1)%resolution.Items.Count;
            DockPanel.SetDock(previous,Dock.Left); selector.Children.Add(previous);
            DockPanel.SetDock(next,Dock.Right); selector.Children.Add(next); selector.Children.Add(resolution);
            Field(displayRight,"Output resolution",selector);
            var custom=new StackPanel { Margin=new Thickness(0,12,0,0) };var dimensions=Columns(custom);
            width=Entry(model.Width.ToString());height=Entry(model.Height.ToString());Field(Column(dimensions,0),"Width",width);Field(Column(dimensions,1),"Height",height);
            var customHint=Text("Use a supported custom size, or 0 × 0 for the original console resolution.",12);customHint.Foreground=Muted;custom.Children.Add(customHint);
            var customSize=new Expander { Header="Custom resolution",Foreground=Ink,FontSize=12,Content=custom,Margin=new Thickness(0,2,0,12) };display.Children.Add(customSize);
            resolution.SelectionChanged+=(s,e)=> { var selected=resolution.SelectedItem as DisplayMode; if(selected!=null) { width.Text=selected.Width.ToString(); height.Text=selected.Height.ToString(); } };
            var ratioValue=Text("Current ratio: "+DisplayModes.RatioLabel(model.Width,model.Height),12);ratioValue.Name="CurrentAspectRatio";ratioValue.Foreground=Accent;display.Children.Add(ratioValue);
            TextChangedEventHandler updateRatio=(s,e)=> {
                int w,h;ratioValue.Text=Int32.TryParse(width.Text,out w)&&Int32.TryParse(height.Text,out h)?"Current ratio: "+DisplayModes.RatioLabel(w,h):"Enter a valid size";
            };
            width.TextChanged+=updateRatio;height.TextChanged+=updateRatio;
            aspect.SelectionChanged+=(s,e)=> {
                if(aspect.SelectedIndex<0)return;
                int w,h;if(!Int32.TryParse(width.Text,out w)||!Int32.TryParse(height.Text,out h)){w=model.Width;h=model.Height;}
                var available=sizes.Where(m=>aspect.SelectedIndex==0 || DisplayModes.AspectIndex(m.Width,m.Height)==aspect.SelectedIndex).ToList();
                if(available.Count==0)return;
                var nearest=available.OrderBy(m=>Math.Abs((long)(m.Height==0?480:m.Height)-(h==0?480:h))*10000+Math.Abs((long)(m.Width==0?640:m.Width)-(w==0?640:w))).First();
                resolution.Items.Clear();foreach(var mode in available)resolution.Items.Add(mode);resolution.SelectedItem=nearest;
            };
            var fieldOfView=new DockPanel { MaxWidth=460, HorizontalAlignment=HorizontalAlignment.Left };
            var fovValue=Text(model.VerticalFov.ToString(CultureInfo.InvariantCulture)+"°",14); fovValue.Name="VerticalFovValue";
            fovValue.MinWidth=48; fovValue.TextAlignment=TextAlignment.Right;
            DockPanel.SetDock(fovValue,Dock.Right); fieldOfView.Children.Add(fovValue);
            fov=new Slider { Name="VerticalFovSlider", Minimum=GameSettings.FovMin, Maximum=GameSettings.FovMax,
                Value=model.VerticalFov, TickFrequency=1, IsSnapToTickEnabled=true, SmallChange=1, LargeChange=5,
                MinWidth=180, MaxWidth=360, Margin=new Thickness(0,6,12,6), VerticalAlignment=VerticalAlignment.Center };
            System.Windows.Automation.AutomationProperties.SetName(fov,"Vertical field of view in degrees");
            fov.ValueChanged+=(s,e)=>fovValue.Text=((int)Math.Round(fov.Value)).ToString(CultureInfo.InvariantCulture)+"°";
            fov.IsEnabled=model.CanAdjustFov;
            fieldOfView.Children.Add(fov);
            var camera=PageTheme.Section(graphics,"Camera & window","Adjust your view and choose how the game fills your display.");
            Field(camera,"Vertical FOV · 40–100°",fieldOfView);
            var resetFov=Button("Reset FOV to 55°"); resetFov.Name="ResetFovButton";
            resetFov.IsEnabled=model.CanAdjustFov;
            resetFov.HorizontalAlignment=HorizontalAlignment.Left; resetFov.Click+=(s,e)=>fov.Value=GameSettings.FovDefault;
            camera.Children.Add(resetFov);
            if(!model.CanAdjustFov) camera.Children.Add(Text("Custom FOV requires a game release newer than v0.2.0-alpha.4. Resolution settings remain available.",12));
            var fovHint=Text("55° is the original view. Wider settings show more of the world; weapon zoom remains relative.",12);fovHint.Foreground=Muted;fovHint.Margin=new Thickness(0,10,0,4);camera.Children.Add(fovHint);
            fullscreen=Check("Borderless fullscreen",model.Fullscreen);camera.Children.Add(fullscreen);
            var quality=PageTheme.Section(graphics,"Quality & performance","Higher render scale and resolution increase GPU load.");
            var qualityColumns=Columns(quality);var qualityLeft=Column(qualityColumns,0);var qualityRight=Column(qualityColumns,1);
            scale=Choices(Enumerable.Range(1,8).Select(i=>i.ToString()).ToArray(),model.Scale.ToString());Field(qualityLeft,"Render scale / supersampling",scale);
            filtering=Choices(new[]{"1","2","4","8","16"},model.Anisotropy.ToString());Field(qualityRight,"Anisotropic filtering",filtering);
            pacing=Choices(new[]{"adaptive","60","30","0"},model.FrameCap);Field(qualityLeft,"Frame pacing · 0 = uncapped",pacing);
            interpolation=Choices(new[]{"1","2","3","4"},model.Interpolation.ToString());Field(qualityRight,"Frame interpolation · 1 = off",interpolation);
            vrr=Check("Variable refresh rate",model.Vrr);vrr.ToolTip="Requires display and driver support.";fps=Check("Show FPS overlay",model.FpsOverlay);
            var qualityToggles=new WrapPanel();qualityToggles.Children.Add(vrr);qualityToggles.Children.Add(fps);quality.Children.Add(qualityToggles);
            var interpolationHint=Text("Interpolation adds displayed frames while keeping the game clock unchanged. It can produce visual artifacts.",12);interpolationHint.Foreground=Muted;interpolationHint.Margin=new Thickness(0,8,0,0);quality.Children.Add(interpolationHint);
            var startup=PageTheme.Section(graphics,"Startup","Choose how the game opens.");
            skipSplashes=Check("Skip startup splash screens",model.SkipStartupSplashes);startup.Children.Add(skipSplashes);
            var startupHint=Text("Skips opening logo videos and the idle attract video. Story cutscenes still play.",12);startupHint.Foreground=Muted;startupHint.Margin=new Thickness(0,8,0,0);startup.Children.Add(startupHint);
            tabs.Items.Add(new TabItem { Header="Graphics & resolution", Content=Scroll(graphics) }); RefreshBindings();
            var discord=new StackPanel();
            var presence=PageTheme.Section(discord,"Discord Rich Presence","Share your current game mode and elapsed session time on your Discord profile.");
            discordEnabled=Check("Enable Discord Rich Presence",model.DiscordEnabled);discordEnabled.Name="DiscordPresenceEnabled";presence.Children.Add(discordEnabled);
            discordId=Entry(model.DiscordApplicationId);discordId.Name="DiscordApplicationId";discordId.MaxLength=20;
            Field(presence,"Application ID",discordId);
            presence.Children.Add(Text("Create an application in the Discord Developer Portal, then copy General Information → Application ID here. This ID is public; no bot token or client secret is needed.",12));
            var portal=Button("Open Discord Developer Portal");portal.HorizontalAlignment=HorizontalAlignment.Left;
            portal.Click+=(s,e)=> { try { System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo("https://discord.com/developers/applications") { UseShellExecute=true }); } catch(Exception ex) { status.Text=ex.Message; } };
            presence.Children.Add(portal);
            presence.Children.Add(Text("Keep the Discord desktop app open and allow activity sharing in Discord. Changes apply on your next game launch. Requires a game build with Rich Presence support.",12));
            tabs.Items.Add(new TabItem { Header="Discord",Content=Scroll(discord) });
        }
        static string KeyName(string token)
        {
            if (token=="none" || token=="") return "Unbound";
            int key=GameSettings.KeyCode(token);
            if(key>=48 && key<=57)return ((char)key).ToString();
            switch(key) { case 1:return "Mouse 1";case 2:return "Mouse 2";case 4:return "Mouse 3";case 5:return "Mouse 4";case 6:return "Mouse 5";case 8:return "Backspace";case 13:return "Enter";case 16:return "Shift";case 17:return "Ctrl";case 18:return "Alt";case 134:return "Wheel up";case 135:return "Wheel down"; }
            return key==0?token:((System.Windows.Forms.Keys)key).ToString();
        }
        void RefreshBindings() { for(int i=0;i<24;i++) { var keys=model.Binding(i); for(int j=0;j<4;j++) { bindingButtons[i,j].Content=KeyName(keys[j]);bindingButtons[i,j].Foreground=KeyName(keys[j])=="Unbound"?Muted:Ink; } } }
        public void SelectTab(int index) { tabs.SelectedIndex=index; }
        public void DismissPrompt()
        {
            overlay.Visibility=Visibility.Collapsed; overlay.Child=null; layout.IsEnabled=true;
        }
        StackPanel Prompt(string title, string description)
        {
            var content=new StackPanel { VerticalAlignment=VerticalAlignment.Center, MaxWidth=560 };
            content.Children.Add(Text(title,22)); content.Children.Add(Text(description,14));
            layout.IsEnabled=false; overlay.Child=content; overlay.Visibility=Visibility.Visible;
            return content;
        }
        void Confirm(string title,string description,string label,Action accepted)
        {
            var content=Prompt(title,description);
            var buttons=new WrapPanel { Margin=new Thickness(0,14,0,0) }; content.Children.Add(buttons);
            var confirm=Button(label); confirm.Click+=(s,e)=> { DismissPrompt(); accepted(); }; buttons.Children.Add(confirm);
            var cancel=Button("Cancel"); cancel.Click+=(s,e)=>DismissPrompt(); buttons.Children.Add(cancel);
            cancel.Focus();
        }
        void Capture(int action,int slot)
        {
            var content=Prompt("Bind · "+GameSettings.Labels[action],"Choose a key or mouse button for binding "+(slot+1)+".");
            var prompt=Text("Press a key, or click / scroll inside the capture area.",14); content.Children.Add(prompt);
            var area=new Border { Name="BindingCaptureArea", Background=new SolidColorBrush(Color.FromRgb(11,20,32)), BorderBrush=Accent, BorderThickness=new Thickness(1), Height=90, Focusable=true, Child=new TextBlock { Text="Capture keyboard or mouse input", Foreground=Accent, HorizontalAlignment=HorizontalAlignment.Center, VerticalAlignment=VerticalAlignment.Center } }; content.Children.Add(area);
            Action<int> accept=key=> {
                if(!GameSettings.KeyAllowed(key)) { prompt.Text="That key is reserved by the game. Choose another."; return; }
                DismissPrompt();
                Action assign=()=> { model.Assign(action,slot,key,true); RefreshBindings(); bindingButtons[action,slot].Focus(); };
                int conflict=model.Conflict(action,key);
                if(conflict>=0) Confirm("Move binding?","This key is assigned to "+GameSettings.Labels[conflict]+". Move it to "+GameSettings.Labels[action]+"?","Move binding",assign);
                else assign();
            };
            area.PreviewKeyDown+=(s,e)=> { if(e.Key==Key.Escape) { DismissPrompt(); bindingButtons[action,slot].Focus(); e.Handled=true; return; } Key key=e.Key==Key.System?e.SystemKey:e.Key; int vk=KeyInterop.VirtualKeyFromKey(key); if(vk==160||vk==161)vk=16; if(vk==162||vk==163)vk=17; if(vk==164||vk==165)vk=18; if(vk>0){e.Handled=true;accept(vk);} };
            area.PreviewMouseDown+=(s,e)=> { e.Handled=true; accept(e.ChangedButton==MouseButton.Left?1:e.ChangedButton==MouseButton.Right?2:e.ChangedButton==MouseButton.Middle?4:e.ChangedButton==MouseButton.XButton1?5:6); };
            area.PreviewMouseWheel+=(s,e)=> {e.Handled=true;accept(e.Delta>0?134:135);};
            var buttons=new WrapPanel { Margin=new Thickness(0,12,0,0) }; content.Children.Add(buttons);
            var escape=Button("Bind Escape"); escape.Click+=(s,e)=>accept(27); buttons.Children.Add(escape);
            var clear=Button("Clear binding"); clear.Click+=(s,e)=>accept(0); buttons.Children.Add(clear);
            var cancel=Button("Cancel"); cancel.Click+=(s,e)=> { DismissPrompt(); bindingButtons[action,slot].Focus(); }; buttons.Children.Add(cancel);
            area.Loaded+=(s,e)=>area.Focus(); area.Focus();
        }
        void Save()
        {
            try
            {
                double number; int w=0,h=0;
                Core.Require(Double.TryParse(sensitivity.Text,NumberStyles.Float,CultureInfo.InvariantCulture,out number),"Enter a numeric sensitivity, using a decimal point.");
                Core.Require(Int32.TryParse(width.Text,out w) && Int32.TryParse(height.Text,out h),"Enter a whole-number width and height (0 × 0 keeps the console resolution).");
                model.Sensitivity=number; model.MouseEnabled=enabled.IsChecked==true; model.InvertY=invert.IsChecked==true;
                string chosenDevice=Devices[device.SelectedIndex]; if(chosenDevice!=model.Device.Replace("xinput:","gamepad:")) model.Device=chosenDevice;
                model.VerticalFov=(int)Math.Round(fov.Value);
                model.Width=w; model.Height=h; model.Fullscreen=fullscreen.IsChecked==true; model.Vrr=vrr.IsChecked==true; model.FpsOverlay=fps.IsChecked==true;
                model.SkipStartupSplashes=skipSplashes.IsChecked==true;
                model.DiscordEnabled=discordEnabled.IsChecked==true;model.DiscordApplicationId=discordId.Text.Trim();
                model.Scale=Int32.Parse((string)scale.SelectedItem); model.Anisotropy=Int32.Parse((string)filtering.SelectedItem); model.FrameCap=(string)pacing.SelectedItem; model.Interpolation=Int32.Parse((string)interpolation.SelectedItem);
                model.Save(); status.Foreground=Accent; status.Text="Settings saved. Changes apply on your next launch.";
                if(Saved!=null)Saved();
            }
            catch(Exception ex) { status.Foreground=Accent; status.Text=ex.Message; }
        }
    }
}
