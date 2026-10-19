using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
namespace TimeSplitters.Launcher
{
    // Native controls edit the same transactional settings model as WPF.
    public sealed class NativeSettings
    {
        public readonly Grid View=new Grid();readonly GameSettings model;readonly TabControl tabs=new TabControl();
        readonly TextBlock status;readonly TextBox sensitivity,width,height,discordId;
        readonly CheckBox mouse,invert,fullscreen,vrr,overlay,skip,discord;
        readonly ComboBox device,scale,filter,interp,pacing,resolution,aspect;
        readonly Slider fov;readonly Button[,] bindings=new Button[24,4];
        readonly Action saved,cancelled;
        static TextBlock Text(string s,int size=13)=>NativeWindow.Text(s,size);
        static CheckBox Check(string title,bool value)=>new CheckBox{Content=title,IsChecked=value,Margin=new Thickness(0,4,0,6)};
        static ComboBox Choice(IEnumerable<string> values,string selected){var list=values.ToList();if(!list.Contains(selected))list.Add(selected);return new ComboBox{ItemsSource=list,SelectedItem=selected,HorizontalAlignment=HorizontalAlignment.Stretch};}
        static StackPanel[] Columns(StackPanel parent) {
            var grid=new Grid{ColumnDefinitions=new ColumnDefinitions("*,*")};var left=new StackPanel{Spacing=6,Margin=new Thickness(0,0,10,0)};var right=new StackPanel{Spacing=6,Margin=new Thickness(10,0,0,0)};grid.Children.Add(left);Grid.SetColumn(right,1);grid.Children.Add(right);parent.Children.Add(grid);return new[]{left,right};
        }
        static TextBox Entry(string value)=>new TextBox{Text=value,HorizontalAlignment=HorizontalAlignment.Stretch};
        static void Field(StackPanel panel,string title,Control control){panel.Children.Add(Text(title,12));panel.Children.Add(control);}
        static StackPanel Section(StackPanel parent,string title,string description){var body=new StackPanel{Spacing=8};body.Children.Add(Text(title,17));body.Children.Add(Text(description,12));parent.Children.Add(new Border{Child=body,Padding=new Thickness(18),Margin=new Thickness(0,0,0,16),CornerRadius=new CornerRadius(6),Background=Brush.Parse("#14283B"),BorderBrush=Brush.Parse("#2D455C"),BorderThickness=new Thickness(1)});return body;}
        public NativeSettings(string root,Action saved,Action cancelled)
        {
            this.saved=saved;this.cancelled=cancelled;model=GameSettings.Load(root,null);
            View.RowDefinitions=new RowDefinitions("Auto,*,Auto");View.Margin=new Thickness(30,24,30,18);
            var heading=new StackPanel();heading.Children.Add(Text("GAME PREFERENCES",11));heading.Children.Add(Text("Game settings",28));heading.Children.Add(Text("Adjust controls, display and performance before your next launch.",12));View.Children.Add(heading);
            Grid.SetRow(tabs,1);View.Children.Add(tabs);
            var controls=new StackPanel{Margin=new Thickness(0,14,12,0)};var input=Section(controls,"Input & mouse","Choose your input device and adjust how the camera responds.");
            var inputColumns=Columns(input);
            device=Choice(new[]{"keyboard","gamepad:0","gamepad:1","gamepad:2","gamepad:3","none"},model.Device);Field(inputColumns[0],"Player 1 input device",device);
            sensitivity=Entry(model.Sensitivity.ToString("G",CultureInfo.InvariantCulture));Field(inputColumns[1],"Mouse sensitivity · 0.001–10",sensitivity);
            mouse=Check("Mouse look",model.MouseEnabled);invert=Check("Invert vertical look",model.InvertY);input.Children.Add(mouse);input.Children.Add(invert);
            var keys=Section(controls,"Key bindings","Select a slot, then press a key or mouse button. Mouse wheel is supported.");
            var labels=new Grid{ColumnDefinitions=new ColumnDefinitions("200,*,*,*,*")};labels.Children.Add(Text("ACTION",11));for(int j=0;j<4;j++){var label=Text("SLOT "+(j+1),11);Grid.SetColumn(label,j+1);labels.Children.Add(label);}keys.Children.Add(labels);
            for(int i=0;i<24;i++){var row=new Grid{ColumnDefinitions=new ColumnDefinitions("200,*,*,*,*"),Background=i%2==0?Brush.Parse("#172B3F"):Brushes.Transparent};row.Children.Add(Text(GameSettings.Labels[i],12));
                for(int j=0;j<4;j++){int action=i,slot=j;var b=NativeWindow.Button("",()=>Capture(action,slot));b.FontSize=11;b.Margin=new Thickness(3);b.Padding=new Thickness(6);b.HorizontalAlignment=HorizontalAlignment.Stretch;Grid.SetColumn(b,j+1);row.Children.Add(b);bindings[i,j]=b;}keys.Children.Add(row);}
            keys.Children.Add(NativeWindow.Button("Restore default controls",()=>{model.ResetControls();sensitivity.Text="0.10";mouse.IsChecked=true;invert.IsChecked=false;device.SelectedItem="keyboard";RefreshBindings();}));
            var graphics=new StackPanel{Margin=new Thickness(0,14,12,0)};var display=Section(graphics,"Display","Choose the game’s output size. Your desktop resolution stays unchanged.");
            var modes=DisplayModes.Primary(model.Width,model.Height).Where(m=>model.SupportsResolution(m.Width,m.Height)).ToList();
            var displayColumns=Columns(display);
            aspect=new ComboBox{ItemsSource=DisplayModes.Aspects,SelectedIndex=0,HorizontalAlignment=HorizontalAlignment.Stretch};Field(displayColumns[0],"Aspect ratio",aspect);
            resolution=new ComboBox{ItemsSource=modes,SelectedItem=modes.FirstOrDefault(m=>m.Width==model.Width&&m.Height==model.Height),HorizontalAlignment=HorizontalAlignment.Stretch};Field(displayColumns[1],"Output resolution",resolution);
            var custom=new StackPanel();var customColumns=Columns(custom);width=Entry(model.Width.ToString());height=Entry(model.Height.ToString());Field(customColumns[0],"Width",width);Field(customColumns[1],"Height",height);display.Children.Add(new Expander{Header="Custom resolution",Content=custom});
            resolution.SelectionChanged+=(s,e)=>{if(resolution.SelectedItem is DisplayMode m){width.Text=m.Width.ToString();height.Text=m.Height.ToString();}};
            aspect.SelectionChanged+=(s,e)=>{int index=aspect.SelectedIndex;resolution.ItemsSource=modes.Where(m=>index<=0||DisplayModes.AspectIndex(m.Width,m.Height)==index).ToArray();};
            var camera=Section(graphics,"Camera & window","Adjust your view and choose how the game fills your display.");var cameraColumns=Columns(camera);fullscreen=Check("Borderless fullscreen",model.Fullscreen);Field(cameraColumns[1],"Window mode",fullscreen);
            fov=new Slider{Minimum=GameSettings.FovMin,Maximum=GameSettings.FovMax,Value=model.VerticalFov,TickFrequency=1,IsSnapToTickEnabled=true,IsEnabled=model.CanAdjustFov};Field(cameraColumns[0],"Vertical field of view · 40–100°",fov);var fovLabel=Text(model.VerticalFov+"°");fov.PropertyChanged+=(s,e)=>{if(e.Property==Slider.ValueProperty)fovLabel.Text=((int)fov.Value)+"°";};cameraColumns[0].Children.Add(fovLabel);cameraColumns[0].Children.Add(NativeWindow.Button("Reset FOV to 55°",()=>fov.Value=GameSettings.FovDefault));
            var quality=Section(graphics,"Rendering & performance","Render interpolation keeps the 60 Hz guest simulation unchanged.");
            var qualityColumns=Columns(quality);
            scale=Choice(Enumerable.Range(1,8).Select(n=>n.ToString()),model.Scale.ToString());Field(qualityColumns[0],"Render scale / supersampling",scale);
            filter=Choice(new[]{"1","2","4","8","16"},model.Anisotropy.ToString());Field(qualityColumns[1],"Anisotropic filtering",filter);
            interp=Choice(new[]{"1","2","3","4"},model.Interpolation.ToString());Field(qualityColumns[1],"Frame interpolation · 1 = off",interp);
            pacing=Choice(new[]{"adaptive","30","60","0"},model.FrameCap);Field(qualityColumns[0],"Frame pacing · 0 = uncapped",pacing);
            vrr=Check("Variable refresh rate",model.Vrr);overlay=Check("FPS overlay",model.FpsOverlay);quality.Children.Add(vrr);quality.Children.Add(overlay);
            var startup=Section(graphics,"Startup","Skip the publisher and developer splash movies. Story cutscenes remain enabled.");skip=Check("Skip startup splash screens",model.SkipStartupSplashes);startup.Children.Add(skip);
            var discordPanel=new StackPanel{Margin=new Thickness(0,14,12,0)};var presence=Section(discordPanel,"Discord Rich Presence","Saved preferences are shared with Windows. Native Mac Discord IPC support is pending.");discord=Check("Enable Rich Presence",model.DiscordEnabled);discordId=Entry(model.DiscordApplicationId);presence.Children.Add(discord);Field(presence,"Public Application ID",discordId);discord.IsEnabled=false;discordId.IsEnabled=false;
            tabs.Items.Add(new TabItem{Header="Keyboard & mouse",Content=NativeWindow.Scroll(controls)});tabs.Items.Add(new TabItem{Header="Graphics & resolution",Content=NativeWindow.Scroll(graphics)});tabs.Items.Add(new TabItem{Header="Discord",Content=NativeWindow.Scroll(discordPanel)});
            var footer=new DockPanel{Margin=new Thickness(0,12,0,0)};Grid.SetRow(footer,2);View.Children.Add(footer);var buttons=new StackPanel{Orientation=Orientation.Horizontal,Spacing=10};buttons.Children.Add(NativeWindow.Button("Cancel",()=>cancelled()));buttons.Children.Add(NativeWindow.Button("Save settings",Save));DockPanel.SetDock(buttons,Dock.Right);footer.Children.Add(buttons);
            status=Text("Home keeps your draft. Save when you’re ready.",12);footer.Children.Add(status);RefreshBindings();
        }
        public void SelectTab(int index){tabs.SelectedIndex=index;}
        void Save(){try{
            model.Sensitivity=Double.Parse(sensitivity.Text,CultureInfo.InvariantCulture);model.MouseEnabled=mouse.IsChecked==true;model.InvertY=invert.IsChecked==true;model.Device=(string)device.SelectedItem;
            model.Width=Int32.Parse(width.Text);model.Height=Int32.Parse(height.Text);model.VerticalFov=(int)fov.Value;model.Fullscreen=fullscreen.IsChecked==true;
            model.Scale=Int32.Parse((string)scale.SelectedItem);model.Anisotropy=Int32.Parse((string)filter.SelectedItem);model.Interpolation=Int32.Parse((string)interp.SelectedItem);model.FrameCap=(string)pacing.SelectedItem;model.Vrr=vrr.IsChecked==true;model.FpsOverlay=overlay.IsChecked==true;model.SkipStartupSplashes=skip.IsChecked==true;
            model.Save();saved();
        }catch(Exception e){status.Text=e.Message;}}
        void RefreshBindings(){for(int i=0;i<24;i++)for(int j=0;j<4;j++)bindings[i,j].Content=GameSettings.BindingLabel(model.Binding(i)[j]);}
        static int VirtualKey(Key key)
        {
            if(key>=Key.A&&key<=Key.Z)return 'A'+(key-Key.A);if(key>=Key.D0&&key<=Key.D9)return '0'+(key-Key.D0);if(key>=Key.F1&&key<=Key.F24)return 112+(key-Key.F1);
            switch(key){case Key.Up:return 38;case Key.Down:return 40;case Key.Left:return 37;case Key.Right:return 39;case Key.Enter:return 13;case Key.Back:return 8;case Key.Space:return 32;case Key.Tab:return 9;case Key.Escape:return 27;case Key.LeftShift:return 160;case Key.RightShift:return 161;case Key.LeftCtrl:return 162;case Key.RightCtrl:return 163;case Key.LeftAlt:return 164;case Key.RightAlt:return 165;case Key.Insert:return 45;case Key.Delete:return 46;case Key.CapsLock:return 20;case Key.OemMinus:return 189;case Key.OemPlus:return 187;case Key.OemComma:return 188;case Key.OemPeriod:return 190;case Key.OemQuestion:return 191;case Key.OemSemicolon:return 186;case Key.OemQuotes:return 222;case Key.OemOpenBrackets:return 219;case Key.OemCloseBrackets:return 221;case Key.OemPipe:return 220;case Key.OemTilde:return 192;case Key.Home:return 36;case Key.End:return 35;case Key.PageUp:return 33;case Key.PageDown:return 34;default:return -1;}
        }
        async void Capture(int action,int slot)
        {
            var owner=TopLevel.GetTopLevel(View) as NativeWindow;if(owner==null)return;
            var dialog=new Window{Title="Assign binding",Width=490,Height=320,CanResize=false,WindowStartupLocation=WindowStartupLocation.CenterOwner,Background=Brush.Parse("#102031")};
            var panel=new StackPanel{Margin=new Thickness(24),Spacing=12};panel.Children.Add(Text(GameSettings.Labels[action],20));var hint=Text("Press a key, or click / scroll inside the capture area.");panel.Children.Add(hint);
            var area=new Border{Focusable=true,Height=70,Background=Brush.Parse("#0B1420"),BorderBrush=Brush.Parse("#86CDFF"),BorderThickness=new Thickness(1),Child=Text("Capture keyboard or mouse input")};panel.Children.Add(area);
            Action<int> accept=key=>{key=GameSettings.CaptureKey(key);if(!GameSettings.KeyAllowed(key)){hint.Text="That key is reserved by the game. Choose another.";return;}dialog.Close(key);};
            var actions=new StackPanel{Orientation=Orientation.Horizontal,Spacing=8};actions.Children.Add(NativeWindow.Button("Bind Escape",()=>accept(27)));actions.Children.Add(NativeWindow.Button("Clear binding",()=>accept(0)));actions.Children.Add(NativeWindow.Button("Cancel",()=>dialog.Close()));panel.Children.Add(actions);dialog.Content=panel;
            area.KeyDown+=(s,e)=>{if(e.Key==Key.Escape){dialog.Close();e.Handled=true;return;}int key=VirtualKey(e.Key);if(key>=0){e.Handled=true;accept(key);}};
            area.PointerPressed+=(s,e)=>{var p=e.GetCurrentPoint(area).Properties;int key=p.IsRightButtonPressed?2:p.IsMiddleButtonPressed?4:p.IsXButton1Pressed?5:p.IsXButton2Pressed?6:p.IsLeftButtonPressed?1:-1;if(key>=0){e.Handled=true;accept(key);}};
            area.PointerWheelChanged+=(s,e)=>{e.Handled=true;accept(e.Delta.Y>0?GameSettings.WheelUpKey:GameSettings.WheelDownKey);};dialog.Opened+=(s,e)=>area.Focus();
            int? chosen=await dialog.ShowDialog<int?>(owner);if(!chosen.HasValue)return;
            try{int key=chosen.Value;bool move=model.Conflict(action,key)<0||await owner.Confirm("Move existing binding?","This input is already assigned to another action. Move it here?");if(!move)return;model.Assign(action,slot,key,true);RefreshBindings();}catch(Exception e){status.Text=e.Message;}
        }
    }
}
