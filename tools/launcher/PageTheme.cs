
#if MACOS
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Automation;
using UIElement = Avalonia.Controls.Control;
using Brush = Avalonia.Media.IBrush;
using FontWeights = Avalonia.Media.FontWeight;
#else
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Markup;
#endif

namespace TimeSplitters.Launcher
{
    // Scoped to the embedded pages; native controls keep their keyboard behavior.
    public static class PageTheme
    {
        public static Brush Brush(string value) { return Ui.Brush(value); }
        public static TextBlock Text(string text,int size,string color="#E4EDF5")
        { return new TextBlock { Text=text,FontSize=size,Foreground=Brush(color),TextWrapping=TextWrapping.Wrap }; }
        public static Border Card(UIElement child)
        { return new Border { Background=Brush("#122233"),BorderBrush=Brush("#2D455C"),BorderThickness=new Thickness(1),CornerRadius=new CornerRadius(7),Padding=new Thickness(16),Child=child }; }
        public static StackPanel Section(Panel parent,string title,string description)
        {
            var body=new StackPanel();var heading=Text(title,17);heading.FontWeight=FontWeights.SemiBold;body.Children.Add(heading);
            if(description.Length>0){var hint=Text(description,13,"#B4C7D8");hint.Margin=new Thickness(0,4,0,10);body.Children.Add(hint);}
            var card=Card(body);card.Margin=new Thickness(0,0,0,14);parent.Children.Add(card);return body;
        }
        public static void Primary(Button button)
        { button.Background=Brush("#A7D9FA");button.Foreground=Brush("#0B2337");button.BorderBrush=Brush("#D6EBFA"); }
        public static void Secondary(Button button)
        { button.Background=Brush("#1C3349");button.Foreground=Brush("#DFEAF4");button.BorderBrush=Brush("#405D76"); }
#if !MACOS
        public static ResourceDictionary Tabs() { return Ui.LoadTheme("TabsTheme.xaml"); }
#endif
#if !MACOS
        public static ResourceDictionary Resources() { return Ui.LoadTheme("PageTheme.xaml"); }
#endif
    }
}
