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
    public sealed class SettingsWindow
    {
        public readonly Window Window;
        readonly GameSettings model;
        readonly TextBlock status;
        readonly TextBox sensitivity, width, height;
        readonly CheckBox enabled, invert, fullscreen, vrr, fps;
        readonly ComboBox device, resolution, scale, filtering, pacing, interpolation;
        readonly Button[,] bindingButtons = new Button[24,4];
        static readonly string[] Devices = { "keyboard", "gamepad:0", "gamepad:1", "gamepad:2", "gamepad:3", "none" };
        static readonly Brush Ink = new SolidColorBrush(Color.FromRgb(228,237,245));
        static readonly Brush Muted = new SolidColorBrush(Color.FromRgb(158,175,191));
        static readonly Brush Accent = new SolidColorBrush(Color.FromRgb(134,205,255));
        static readonly Brush Panel = new SolidColorBrush(Color.FromRgb(21,36,53));
        static TextBlock Text(string text, int size) { return new TextBlock { Text=text, FontSize=size, Foreground=Ink, TextWrapping=TextWrapping.Wrap, Margin=new Thickness(0,4,0,8) }; }
        static Button Button(string text) { return new Button { Content=text, Margin=new Thickness(4), MinHeight=32, Padding=new Thickness(12,6,12,6) }; }
        static CheckBox Check(string text, bool value) { return new CheckBox { Content=text, IsChecked=value, Foreground=Ink, Margin=new Thickness(0,9,20,9) }; }
        static TextBox Entry(string text) { return new TextBox { Text=text, Background=new SolidColorBrush(Color.FromRgb(11,20,32)), Foreground=Ink, BorderBrush=Muted, Padding=new Thickness(8), Margin=new Thickness(0,2,12,8), MinWidth=100 }; }
        static ComboBox Choices(string[] choices, string selected)
        {
            var combo = new ComboBox { Margin=new Thickness(0,2,12,12), Padding=new Thickness(8), MinWidth=180, Foreground=Brushes.Black };
            foreach (string choice in choices) combo.Items.Add(choice);
            if (!combo.Items.Contains(selected)) combo.Items.Add(selected);
            combo.SelectedItem=selected; return combo;
        }
        static void Field(Panel parent, string label, UIElement input)
        { var stack=new StackPanel { Margin=new Thickness(0,0,18,8) }; stack.Children.Add(Text(label,12)); stack.Children.Add(input); parent.Children.Add(stack); }
        static ScrollViewer Scroll(UIElement content) { return new ScrollViewer { VerticalScrollBarVisibility=ScrollBarVisibility.Auto, Content=content, Margin=new Thickness(18) }; }
        static ResourceDictionary TabsTheme()
        {
            // Keep native TabControl navigation/accessibility with launcher-styled headers.
            return (ResourceDictionary)System.Windows.Markup.XamlReader.Parse(@"
<ResourceDictionary xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'
                    xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'>
  <Style TargetType='TabItem'>
    <Setter Property='Foreground' Value='#B4C7D8'/>
    <Setter Property='Background' Value='#152A3E'/>
    <Setter Property='BorderBrush' Value='#36536F'/>
    <Setter Property='BorderThickness' Value='1'/>
    <Setter Property='Padding' Value='22,12'/>
    <Setter Property='Margin' Value='0'/>
    <Setter Property='MinWidth' Value='196'/>
    <Setter Property='MinHeight' Value='48'/>
    <Setter Property='Cursor' Value='Hand'/>
    <Setter Property='Template'>
      <Setter.Value>
        <ControlTemplate TargetType='TabItem'>
          <Border x:Name='Face' Background='{TemplateBinding Background}'
                  BorderBrush='{TemplateBinding BorderBrush}' BorderThickness='{TemplateBinding BorderThickness}' CornerRadius='4' Margin='0,0,10,0'>
            <Grid>
              <ContentPresenter ContentSource='Header' RecognizesAccessKey='True'
                                TextElement.FontSize='13' TextElement.FontWeight='SemiBold'
                                HorizontalAlignment='Center' VerticalAlignment='Center' Margin='{TemplateBinding Padding}'/>
              <Border x:Name='SelectionLine' Height='2' Margin='18,0,18,4'
                      VerticalAlignment='Bottom' Background='#86CDFF' Visibility='Collapsed'/>
            </Grid>
          </Border>
          <ControlTemplate.Triggers>
            <Trigger Property='IsMouseOver' Value='True'>
              <Setter TargetName='Face' Property='Background' Value='#21415D'/>
              <Setter TargetName='Face' Property='BorderBrush' Value='#729CBE'/>
              <Setter Property='Foreground' Value='#F0F7FF'/>
            </Trigger>
            <Trigger Property='IsSelected' Value='True'>
              <Setter TargetName='Face' Property='Background'>
                <Setter.Value><LinearGradientBrush StartPoint='0,0' EndPoint='0,1'>
                  <GradientStop Color='#2B5679'/><GradientStop Color='#193C59' Offset='1'/>
                </LinearGradientBrush></Setter.Value>
              </Setter>
              <Setter TargetName='Face' Property='BorderBrush' Value='#86CDFF'/>
              <Setter Property='Foreground' Value='#DDEFFF'/>
              <Setter TargetName='SelectionLine' Property='Visibility' Value='Visible'/>
            </Trigger>
            <Trigger Property='IsKeyboardFocusWithin' Value='True'>
              <Setter TargetName='Face' Property='BorderBrush' Value='#F0F8FF'/>
              <Setter TargetName='Face' Property='BorderThickness' Value='2'/>
            </Trigger>
            <Trigger Property='IsEnabled' Value='False'>
              <Setter TargetName='Face' Property='Opacity' Value='0.4'/>
            </Trigger>
          </ControlTemplate.Triggers>
        </ControlTemplate>
      </Setter.Value>
    </Setter>
  </Style>
  <Style TargetType='TabControl'>
    <Setter Property='Template'>
      <Setter.Value>
        <ControlTemplate TargetType='TabControl'>
          <Grid KeyboardNavigation.TabNavigation='Local'>
            <Grid.RowDefinitions><RowDefinition Height='Auto'/><RowDefinition Height='*'/></Grid.RowDefinitions>
            <TabPanel IsItemsHost='True' KeyboardNavigation.TabIndex='1' Panel.ZIndex='1' Margin='0,0,0,12'/>
            <Border Grid.Row='1' Background='{TemplateBinding Background}' BorderBrush='{TemplateBinding BorderBrush}'
                    BorderThickness='1' CornerRadius='4' KeyboardNavigation.TabNavigation='Local'>
              <ContentPresenter x:Name='PART_SelectedContentHost' ContentSource='SelectedContent'/>
            </Border>
          </Grid>
        </ControlTemplate>
      </Setter.Value>
    </Setter>
  </Style>
</ResourceDictionary>");
        }
        public SettingsWindow(Window owner, string root, bool preview)
        {
            string inherited = preview ? null : Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "xboxrecomp", "titles", "45410066.conf");
            model=GameSettings.Load(root, inherited);
            Window=new Window { Owner=preview?null:owner, Title="TimeSplitters · Game settings", Width=960, Height=750, MinWidth=850, MinHeight=650, Background=new SolidColorBrush(Color.FromRgb(11,20,32)), Foreground=Ink, FontFamily=new FontFamily("Segoe UI"), WindowStartupLocation=WindowStartupLocation.CenterOwner, Icon=owner.Icon };
            Window.Resources.MergedDictionaries.Add(owner.Resources);
            Window.Resources.MergedDictionaries.Add(TabsTheme());
            var layout=new DockPanel { Margin=new Thickness(24) }; Window.Content=layout;
            var heading=new StackPanel(); DockPanel.SetDock(heading,Dock.Top); layout.Children.Add(heading);
            heading.Children.Add(Text("Prepare your next jump",25));
            var subtitle=Text("Set controls and graphics before starting the game. Changes apply on your next launch.",12); subtitle.Foreground=Muted; heading.Children.Add(subtitle);
            var footer=new StackPanel(); DockPanel.SetDock(footer,Dock.Bottom); layout.Children.Add(footer);
            status=Text("Save changes to keep them, or Cancel to leave your current settings untouched.",12); status.Foreground=Muted; footer.Children.Add(status);
            var actions=new StackPanel { Orientation=Orientation.Horizontal, HorizontalAlignment=HorizontalAlignment.Right };
            var cancel=Button("Cancel"); cancel.IsCancel=true; cancel.Click+=(s,e)=>Window.Close(); actions.Children.Add(cancel);
            var save=Button("Save settings"); save.Background=Accent; save.Foreground=new SolidColorBrush(Color.FromRgb(9,28,44)); save.Click+=(s,e)=>Save(); actions.Children.Add(save); footer.Children.Add(actions);
            var tabs=new TabControl { Background=Panel, BorderBrush=new SolidColorBrush(Color.FromRgb(48,72,95)), Margin=new Thickness(0,14,0,14) }; layout.Children.Add(tabs);
            var controls=new StackPanel();
            var mouse=new WrapPanel(); controls.Children.Add(mouse);
            sensitivity=Entry(model.Sensitivity.ToString("G",CultureInfo.InvariantCulture)); Field(mouse,"Mouse sensitivity (0.001–10)",sensitivity);
            enabled=Check("Mouse look",model.MouseEnabled); invert=Check("Invert vertical look",model.InvertY); mouse.Children.Add(enabled); mouse.Children.Add(invert);
            var deviceLabels=new[]{"Keyboard & mouse","Controller 1","Controller 2","Controller 3","Controller 4","Disabled"};
            device=Choices(deviceLabels,deviceLabels[Array.IndexOf(Devices,model.Device.Replace("xinput:","gamepad:"))]); Field(controls,"Player 1 input device",device);
            var hint=Text("Select a binding to capture a key or mouse button. Four slots per action. Mouse wheel is supported; game-reserved keys are unavailable.",12); hint.Foreground=Muted; controls.Children.Add(hint);
            for (int i=0;i<24;i++)
            {
                var row=new Grid { Margin=new Thickness(0,2,8,2), Background=i%2==0?Panel:Brushes.Transparent };
                row.ColumnDefinitions.Add(new ColumnDefinition { Width=new GridLength(235) });
                var label=Text(GameSettings.Labels[i],12); label.VerticalAlignment=VerticalAlignment.Center; label.Margin=new Thickness(8,4,8,4); row.Children.Add(label);
                for (int j=0;j<4;j++)
                {
                    row.ColumnDefinitions.Add(new ColumnDefinition());
                    int action=i,slot=j; var button=Button(""); button.FontSize=11; button.ToolTip=GameSettings.Labels[i]+", binding "+(j+1);
                    button.Click+=(s,e)=>Capture(action,slot); Grid.SetColumn(button,j+1); row.Children.Add(button); bindingButtons[i,j]=button;
                }
                controls.Children.Add(row);
            }
            var reset=Button("Restore default keyboard & mouse controls"); reset.HorizontalAlignment=HorizontalAlignment.Left;
            reset.Click+=(s,e)=> { if (MessageBox.Show(Window,"Replace Player 1 bindings and mouse settings with the defaults? Changes are applied only when you save.","Restore defaults",MessageBoxButton.YesNo,MessageBoxImage.Question)==MessageBoxResult.Yes) { model.ResetControls(); sensitivity.Text="0.10"; enabled.IsChecked=true; invert.IsChecked=false; device.SelectedIndex=0; RefreshBindings(); } };
            controls.Children.Add(reset); tabs.Items.Add(new TabItem { Header="Keyboard & mouse", Content=Scroll(controls) });
            var graphics=new StackPanel();
            graphics.Children.Add(Text("Display & graphics",20));
            graphics.Children.Add(Text("Higher render scale and resolution increase GPU load. These options use the game runtime’s supported settings.",12));
            var sizes=DisplayModes.Primary(model.Width,model.Height);
            resolution=new ComboBox { Margin=new Thickness(4,2,4,12), Padding=new Thickness(8), MinWidth=240, Foreground=Brushes.Black };
            foreach(var mode in sizes) resolution.Items.Add(mode);
            resolution.SelectedItem=sizes.FirstOrDefault(m=>m.Width==model.Width && m.Height==model.Height);
            var selector=new DockPanel { HorizontalAlignment=HorizontalAlignment.Left };
            var previous=Button("◀"); previous.ToolTip="Previous resolution";
            System.Windows.Automation.AutomationProperties.SetName(previous,"Previous resolution");
            var next=Button("▶"); next.ToolTip="Next resolution";
            System.Windows.Automation.AutomationProperties.SetName(next,"Next resolution");
            previous.Click+=(s,e)=>resolution.SelectedIndex=(resolution.SelectedIndex+resolution.Items.Count-1)%resolution.Items.Count;
            next.Click+=(s,e)=>resolution.SelectedIndex=(resolution.SelectedIndex+1)%resolution.Items.Count;
            DockPanel.SetDock(previous,Dock.Left); selector.Children.Add(previous);
            DockPanel.SetDock(next,Dock.Right); selector.Children.Add(next); selector.Children.Add(resolution);
            Field(graphics,"Output resolution",selector);
            graphics.Children.Add(Text("Modes reported by your primary display, plus the console default. A saved custom resolution is retained if it is no longer reported by the display.",12));
            var dimensions=new WrapPanel(); width=Entry(model.Width.ToString()); height=Entry(model.Height.ToString()); Field(dimensions,"Width",width); Field(dimensions,"Height",height); graphics.Children.Add(dimensions);
            resolution.SelectionChanged+=(s,e)=> { var selected=resolution.SelectedItem as DisplayMode; if(selected!=null) { width.Text=selected.Width.ToString(); height.Text=selected.Height.ToString(); } };
            fullscreen=Check("Borderless fullscreen",model.Fullscreen); graphics.Children.Add(fullscreen);
            var quality=new WrapPanel(); graphics.Children.Add(quality);
            scale=Choices(Enumerable.Range(1,8).Select(i=>i.ToString()).ToArray(),model.Scale.ToString()); Field(quality,"Render scale / supersampling",scale);
            filtering=Choices(new[]{"1","2","4","8","16"},model.Anisotropy.ToString()); Field(quality,"Anisotropic filtering",filtering);
            pacing=Choices(new[]{"adaptive","60","30","0"},model.FrameCap); Field(quality,"Frame pacing (0 = uncapped)",pacing);
            interpolation=Choices(new[]{"1","2","3","4"},model.Interpolation.ToString()); Field(quality,"Frame interpolation (1 = off)",interpolation);
            vrr=Check("Variable refresh rate (requires display/driver support)",model.Vrr); fps=Check("Show FPS overlay",model.FpsOverlay); graphics.Children.Add(vrr); graphics.Children.Add(fps);
            graphics.Children.Add(Text("Interpolation adds displayed frames while the game keeps its normal clock. It can produce visual artifacts. Your desktop refresh rate is unchanged.",12));
            tabs.Items.Add(new TabItem { Header="Graphics & resolution", Content=Scroll(graphics) }); RefreshBindings();
        }
        static string KeyName(string token)
        {
            if (token=="none" || token=="") return "Unbound";
            int key=GameSettings.KeyCode(token);
            if(key>=48 && key<=57)return ((char)key).ToString();
            switch(key) { case 1:return "Mouse 1";case 2:return "Mouse 2";case 4:return "Mouse 3";case 5:return "Mouse 4";case 6:return "Mouse 5";case 8:return "Backspace";case 13:return "Enter";case 16:return "Shift";case 17:return "Ctrl";case 18:return "Alt";case 134:return "Wheel up";case 135:return "Wheel down"; }
            return key==0?token:((System.Windows.Forms.Keys)key).ToString();
        }
        void RefreshBindings() { for(int i=0;i<24;i++) { var keys=model.Binding(i); for(int j=0;j<4;j++) bindingButtons[i,j].Content=KeyName(keys[j]); } }
        void Capture(int action,int slot)
        {
            var dialog=new Window { Owner=Window, Title="Bind · "+GameSettings.Labels[action], Width=470, Height=270, ResizeMode=ResizeMode.NoResize, WindowStartupLocation=WindowStartupLocation.CenterOwner, Background=Panel, Foreground=Ink, FontFamily=Window.FontFamily };
            dialog.Resources.MergedDictionaries.Add(Window.Resources);
            var content=new StackPanel { Margin=new Thickness(20) }; dialog.Content=content;
            var prompt=Text("Press a key, or click / scroll inside the capture area.",14); content.Children.Add(prompt);
            var area=new Border { Background=new SolidColorBrush(Color.FromRgb(11,20,32)), BorderBrush=Accent, BorderThickness=new Thickness(1), Height=70, Focusable=true, Child=new TextBlock { Text="Capture keyboard or mouse input", Foreground=Accent, HorizontalAlignment=HorizontalAlignment.Center, VerticalAlignment=VerticalAlignment.Center } }; content.Children.Add(area);
            int? captured=null;
            Action<int> accept=key=> { if(!GameSettings.KeyAllowed(key)) { prompt.Text="That key is reserved by the game. Choose another."; return; } captured=key; dialog.DialogResult=true; };
            dialog.PreviewKeyDown+=(s,e)=> { if(e.Key==Key.Escape) { dialog.DialogResult=false; e.Handled=true; return; } Key key=e.Key==Key.System?e.SystemKey:e.Key; int vk=KeyInterop.VirtualKeyFromKey(key); if(vk==160||vk==161)vk=16; if(vk==162||vk==163)vk=17; if(vk==164||vk==165)vk=18; if(vk>0){e.Handled=true;accept(vk);} };
            area.PreviewMouseDown+=(s,e)=> { e.Handled=true; accept(e.ChangedButton==MouseButton.Left?1:e.ChangedButton==MouseButton.Right?2:e.ChangedButton==MouseButton.Middle?4:e.ChangedButton==MouseButton.XButton1?5:6); };
            area.PreviewMouseWheel+=(s,e)=> {e.Handled=true;accept(e.Delta>0?134:135);};
            var buttons=new WrapPanel { Margin=new Thickness(0,12,0,0) }; content.Children.Add(buttons);
            var escape=Button("Bind Escape"); escape.Click+=(s,e)=>accept(27); buttons.Children.Add(escape);
            var clear=Button("Clear binding"); clear.Click+=(s,e)=>accept(0); buttons.Children.Add(clear);
            var cancel=Button("Cancel"); cancel.Click+=(s,e)=>dialog.DialogResult=false; buttons.Children.Add(cancel);
            dialog.Loaded+=(s,e)=>area.Focus();
            if(dialog.ShowDialog()!=true || !captured.HasValue)return;
            int conflict=model.Conflict(action,captured.Value);
            if(conflict>=0 && MessageBox.Show(Window,"This key is assigned to "+GameSettings.Labels[conflict]+". Move it to "+GameSettings.Labels[action]+"?","Move binding",MessageBoxButton.YesNo,MessageBoxImage.Question)!=MessageBoxResult.Yes)return;
            model.Assign(action,slot,captured.Value,true); RefreshBindings();
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
                model.Width=w; model.Height=h; model.Fullscreen=fullscreen.IsChecked==true; model.Vrr=vrr.IsChecked==true; model.FpsOverlay=fps.IsChecked==true;
                model.Scale=Int32.Parse((string)scale.SelectedItem); model.Anisotropy=Int32.Parse((string)filtering.SelectedItem); model.FrameCap=(string)pacing.SelectedItem; model.Interpolation=Int32.Parse((string)interpolation.SelectedItem);
                model.Save(); Window.DialogResult=true;
            }
            catch(Exception ex) { status.Foreground=Accent; status.Text=ex.Message; }
        }
    }
}
