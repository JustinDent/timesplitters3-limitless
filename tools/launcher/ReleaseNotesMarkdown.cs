using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;

namespace TimeSplitters.Launcher
{
    // Native, selectable Markdown text. Remote HTML/XAML is never parsed, and
    // images are never fetched. Only explicit HTTP(S) links can open a browser.
    public sealed class ReleaseNotesMarkdown : RichTextBox
    {
        readonly Action<string> openLink;
        static readonly Regex Heading=new Regex(@"^ {0,3}(#{1,6})\s+(.+?)\s*#*\s*$");
        static readonly Regex Item=new Regex(@"^(\s*)([-+*]|\d{1,9}[.)])\s+(.+)$");
        static readonly Regex Rule=new Regex(@"^ {0,3}([-*_])(?:\s*\1){2,}\s*$");
        public ReleaseNotesMarkdown(Action<string> openLink=null)
        {
            this.openLink=openLink;IsReadOnly=true;IsDocumentEnabled=true;
            Background=Brushes.Transparent;Foreground=PageTheme.Brush("#CCDCE8");
            BorderThickness=new Thickness(0);Padding=new Thickness(0);FontSize=12;
            VerticalScrollBarVisibility=ScrollBarVisibility.Auto;
            HorizontalScrollBarVisibility=ScrollBarVisibility.Disabled;
            SetMarkdown("");
        }
        public static bool TryLink(string value,out Uri uri)
        {
            uri=null;
            return value!=null && value.Length<=4096 && value.IndexOfAny(new[]{'\r','\n','\t'})<0 &&
                Uri.TryCreate(value,UriKind.Absolute,out uri) &&
                (uri.Scheme==Uri.UriSchemeHttps || uri.Scheme==Uri.UriSchemeHttp) &&
                uri.Host.Length>0 && uri.UserInfo.Length==0;
        }
        public void SetMarkdown(string source)
        {
            source=(source??"").Replace("\r\n","\n").Replace('\r','\n');
            bool shortened=source.Length>64000;if(shortened)source=source.Substring(0,64000);
            var lines=source.Split('\n');int count=Math.Min(lines.Length,512);shortened|=lines.Length>count;
            var document=new FlowDocument {PagePadding=new Thickness(0),FontFamily=new FontFamily("Segoe UI"),FontSize=12,Foreground=Foreground};
            var paragraph=new StringBuilder();System.Windows.Documents.List list=null;bool ordered=false;int indent=0;
            Action flush=()=>{if(paragraph.Length>0){document.Blocks.Add(Paragraph(paragraph.ToString()));paragraph.Clear();}};
            for(int i=0;i<count;i++) {
                string line=lines[i],trim=line.TrimStart();
                if(trim.StartsWith("```") || trim.StartsWith("~~~")) {
                    flush();list=null;char marker=trim[0];int length=0;while(length<trim.Length && trim[length]==marker)length++;
                    var code=new StringBuilder();
                    while(++i<count) {string close=lines[i].Trim();if(close.Length>=length && close.Trim(marker).Length==0)break;code.AppendLine(lines[i]);}
                    var block=new Paragraph(new Run(code.ToString().TrimEnd('\r','\n'))) {FontFamily=new FontFamily("Consolas"),Background=PageTheme.Brush("#0B1826"),Padding=new Thickness(10),Margin=new Thickness(0,4,0,12)};
                    document.Blocks.Add(block);continue;
                }
                if(String.IsNullOrWhiteSpace(line)){flush();list=null;continue;}
                var heading=Heading.Match(line);
                if(heading.Success || (i+1<count && Regex.IsMatch(lines[i+1],@"^ {0,3}(=+|-+)\s*$"))) {
                    flush();list=null;int level=heading.Success?heading.Groups[1].Length:(lines[i+1].TrimStart()[0]=='='?1:2);
                    var block=Paragraph(heading.Success?heading.Groups[2].Value:line);block.FontSize=level==1?20:level==2?17:15;
                    block.FontWeight=FontWeights.SemiBold;block.Foreground=PageTheme.Brush("#CDE8FC");block.Margin=new Thickness(0,10,0,10);document.Blocks.Add(block);
                    if(!heading.Success)i++;continue;
                }
                if(Rule.IsMatch(line)){flush();list=null;document.Blocks.Add(new Paragraph {BorderBrush=PageTheme.Brush("#2D455C"),BorderThickness=new Thickness(0,0,0,1),Margin=new Thickness(0,8,0,12)});continue;}
                var item=Item.Match(line);
                if(item.Success) {
                    flush();bool number=Char.IsDigit(item.Groups[2].Value[0]);int space=Math.Min(item.Groups[1].Length,24);
                    if(list==null || number!=ordered || space!=indent) {
                        list=new System.Windows.Documents.List {MarkerStyle=number?TextMarkerStyle.Decimal:TextMarkerStyle.Disc,Padding=new Thickness(23+space*4,0,0,0),Margin=new Thickness(0,0,0,8)};
                        int start;if(number && Int32.TryParse(item.Groups[2].Value.TrimEnd('.',')'),out start) && start>0)list.StartIndex=start;
                        document.Blocks.Add(list);ordered=number;indent=space;
                    }
                    var block=Paragraph(item.Groups[3].Value);block.Margin=new Thickness(0,0,0,5);list.ListItems.Add(new ListItem(block));continue;
                }
                if(trim.StartsWith(">")) {
                    flush();list=null;var block=Paragraph(trim.Substring(1).TrimStart());block.BorderBrush=PageTheme.Brush("#86CDFF");block.BorderThickness=new Thickness(2,0,0,0);block.Padding=new Thickness(10,3,0,3);document.Blocks.Add(block);continue;
                }
                list=null;if(paragraph.Length>0)paragraph.Append(' ');paragraph.Append(line.Trim());
            }
            flush();if(shortened)document.Blocks.Add(Paragraph("Notes shortened for display. View the complete release on GitHub."));
            Document=document;ScrollToHome();
        }
        Paragraph Paragraph(string text)
        {
            var block=new Paragraph {Margin=new Thickness(0,0,0,10),LineHeight=20};
            Inline(block.Inlines,text,0);return block;
        }
        void Link(InlineCollection target,string label,string destination,int depth)
        {
            Uri uri;if(!TryLink(destination,out uri)){Inline(target,label,depth+1,false);return;}
            var link=new Hyperlink {NavigateUri=uri,ToolTip=uri.AbsoluteUri,Foreground=PageTheme.Brush("#86CDFF"),Focusable=true};
            Inline(link.Inlines,label,depth+1,false);
            link.RequestNavigate+=(s,e)=>{e.Handled=true;Uri checkedUri;if(TryLink(e.Uri.AbsoluteUri,out checkedUri) && openLink!=null)openLink(checkedUri.AbsoluteUri);};
            target.Add(link);
        }
        static bool ReadLink(string text,int start,out string label,out string destination,out int end)
        {
            label=destination="";end=start;int close=text.IndexOf("](",start+1,StringComparison.Ordinal);if(close<0)return false;
            int balance=1,index=close+2;
            for(;index<text.Length;index++) {
                if(text[index]=='\\'){index++;continue;}
                if(text[index]=='('){if(++balance>8)return false;}
                if(text[index]==')' && --balance==0)break;
            }
            if(index>=text.Length)return false;
            label=text.Substring(start+1,close-start-1);destination=text.Substring(close+2,index-close-2).Trim();
            if(destination.StartsWith("<") && destination.IndexOf('>')>0)destination=destination.Substring(1,destination.IndexOf('>')-1);
            else {int title=destination.IndexOf(" \"",StringComparison.Ordinal);if(title>=0)destination=destination.Substring(0,title);}
            end=index+1;return true;
        }
        void Inline(InlineCollection target,string text,int depth,bool allowLinks=true)
        {
            if(depth>=8){target.Add(new Run(text));return;}
            var plain=new StringBuilder();Action flush=()=>{if(plain.Length>0){target.Add(new Run(System.Net.WebUtility.HtmlDecode(plain.ToString())));plain.Clear();}};
            for(int i=0;i<text.Length;) {
                char c=text[i];
                if(c=='\\' && i+1<text.Length && "\\`*_{}[]()#+-.!>~".IndexOf(text[i+1])>=0){plain.Append(text[i+1]);i+=2;continue;}
                bool image=c=='!' && i+1<text.Length && text[i+1]=='[';string label,url;int end;
                if(allowLinks && (c=='[' || image) && ReadLink(text,i+(image?1:0),out label,out url,out end)) {
                    flush();if(image)target.Add(new Run("[Image: view on GitHub]"));else Link(target,label,url,depth);i=end;continue;
                }
                if(allowLinks && c=='<') {
                    int close=text.IndexOf('>',i+1);Uri uri;
                    if(close>i+1 && TryLink(text.Substring(i+1,close-i-1),out uri)){flush();Link(target,uri.AbsoluteUri,uri.AbsoluteUri,depth);i=close+1;continue;}
                }
                if(c=='`') {
                    int length=1;while(i+length<text.Length && text[i+length]=='`')length++;
                    string marker=new string('`',length);int close=text.IndexOf(marker,i+length,StringComparison.Ordinal);
                    if(close>=0){flush();target.Add(new Run(text.Substring(i+length,close-i-length)) {FontFamily=new FontFamily("Consolas"),Background=PageTheme.Brush("#0B1826")});i=close+length;continue;}
                }
                if(c=='*' || c=='_' || (c=='~' && i+1<text.Length && text[i+1]=='~')) {
                    int length=i+1<text.Length && text[i+1]==c?2:1;string marker=new string(c,length);
                    int close=text.IndexOf(marker,i+length,StringComparison.Ordinal);
                    bool word=c=='_' && i>0 && Char.IsLetterOrDigit(text[i-1]);
                    if(!word && close>i+length && !Char.IsWhiteSpace(text[i+length]) && !Char.IsWhiteSpace(text[close-1])) {
                        flush();Span span=c=='~'?new Span():length==2?(Span)new Bold():new Italic();
                        if(c=='~')span.TextDecorations=TextDecorations.Strikethrough;
                        Inline(span.Inlines,text.Substring(i+length,close-i-length),depth+1,allowLinks);target.Add(span);i=close+length;continue;
                    }
                }
                bool bare=allowLinks && (i==0 || Char.IsWhiteSpace(text[i-1]) || "(<".IndexOf(text[i-1])>=0) &&
                    ((text.Length-i>=8 && String.Compare(text,i,"https://",0,8,StringComparison.OrdinalIgnoreCase)==0) || (text.Length-i>=7 && String.Compare(text,i,"http://",0,7,StringComparison.OrdinalIgnoreCase)==0));
                if(bare) {
                    end=i;while(end<text.Length && !Char.IsWhiteSpace(text[end]) && "<>\"".IndexOf(text[end])<0)end++;
                    url=text.Substring(i,end-i).TrimEnd('.',',',';',':','!');
                    int balance=0;foreach(char part in url){if(part=='(')balance++;else if(part==')')balance--;}
                    while(balance<0 && url.EndsWith(")",StringComparison.Ordinal)){url=url.Substring(0,url.Length-1);balance++;}
                    Uri uri;if(TryLink(url,out uri)){flush();Link(target,url,url,depth);i+=url.Length;continue;}
                }
                plain.Append(c);i++;
            }
            flush();
        }
    }
}
