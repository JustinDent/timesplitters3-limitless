using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
namespace TimeSplitters.Launcher
{
    public sealed class NotesInline { public string Kind="text",Text="",Url; public readonly List<NotesInline> Children=new List<NotesInline>(); }
    public sealed class NotesBlock { public string Kind="paragraph",Text="",Marker;public int Level,Indent,Group;public readonly List<NotesInline> Inlines=new List<NotesInline>(); }
    // Shared bounded Markdown parser. Hosts only map this document to native controls.
    public static class ReleaseNotesDocument
    {
        static readonly Regex Heading=new Regex(@"^ {0,3}(#{1,6})\s+(.+?)\s*#*\s*$");
        static readonly Regex Item=new Regex(@"^(\s*)([-+*]|\d{1,9}[.)])\s+(.+)$");
        static readonly Regex Rule=new Regex(@"^ {0,3}([-*_])(?:\s*\1){2,}\s*$");
        public static bool TryLink(string value,out Uri uri)
        {
            uri=null;
            return value!=null && value.Length<=4096 && value.IndexOfAny(new[]{'\r','\n','\t'})<0 &&
                Uri.TryCreate(value,UriKind.Absolute,out uri) &&
                (uri.Scheme==Uri.UriSchemeHttps || uri.Scheme==Uri.UriSchemeHttp) &&
                uri.Host.Length>0 && uri.UserInfo.Length==0;
        }

        static NotesBlock Block(string text,string kind="paragraph",int level=0){var b=new NotesBlock{Kind=kind,Level=level,Text=text};Inline(b.Inlines,text,0);return b;}
        public static List<NotesBlock> Parse(string source)
        {
            source=(source??"").Replace("\r\n","\n").Replace('\r','\n');bool shortened=source.Length>64000;if(shortened)source=source.Substring(0,64000);
            var lines=source.Split('\n');int count=Math.Min(lines.Length,512);shortened|=lines.Length>count;
            var result=new List<NotesBlock>();var paragraph=new StringBuilder();int sequence=0,indent=-1,group=0;bool ordered=false,inList=false;
            Action flush=()=>{if(paragraph.Length>0){result.Add(Block(paragraph.ToString()));paragraph.Clear();}};
            for(int i=0;i<count;i++){
                string line=lines[i],trim=line.TrimStart();
                if(trim.StartsWith("```") || trim.StartsWith("~~~")){
                    flush();inList=false;char marker=trim[0];int length=0;while(length<trim.Length && trim[length]==marker)length++;
                    var code=new StringBuilder();while(++i<count){string close=lines[i].Trim();if(close.Length>=length && close.Trim(marker).Length==0)break;code.AppendLine(lines[i]);}
                    result.Add(new NotesBlock{Kind="code",Text=code.ToString().TrimEnd('\r','\n')});continue;
                }
                if(String.IsNullOrWhiteSpace(line)){flush();inList=false;continue;}
                var heading=Heading.Match(line);
                if(heading.Success || (i+1<count && Regex.IsMatch(lines[i+1],@"^ {0,3}(=+|-+)\s*$"))){
                    flush();inList=false;int level=heading.Success?heading.Groups[1].Length:(lines[i+1].TrimStart()[0]=='='?1:2);
                    result.Add(Block(heading.Success?heading.Groups[2].Value:line,"heading",level));if(!heading.Success)i++;continue;
                }
                if(Rule.IsMatch(line)){flush();inList=false;result.Add(new NotesBlock{Kind="rule"});continue;}
                var item=Item.Match(line);
                if(item.Success){
                    flush();bool number=Char.IsDigit(item.Groups[2].Value[0]);int space=Math.Min(item.Groups[1].Length,24);
                    if(!inList || number!=ordered || space!=indent){group++;sequence=1;int start;if(number && Int32.TryParse(item.Groups[2].Value.TrimEnd('.',')'),out start) && start>0)sequence=start;}
                    var b=Block(item.Groups[3].Value,"item");b.Indent=space;b.Group=group;b.Marker=number?(sequence++)+".":"•";result.Add(b);inList=true;ordered=number;indent=space;continue;
                }
                inList=false;
                if(trim.StartsWith(">")){flush();result.Add(Block(trim.Substring(1).TrimStart(),"quote"));continue;}
                if(paragraph.Length>0)paragraph.Append(' ');paragraph.Append(line.Trim());
            }
            flush();if(shortened)result.Add(Block("Notes shortened for display. View the complete release on GitHub."));return result;
        }
        static void Link(List<NotesInline> target,string label,string destination,int depth){
            Uri uri;if(!TryLink(destination,out uri)){Inline(target,label,depth+1,false);return;}
            var link=new NotesInline{Kind="link",Url=uri.AbsoluteUri};Inline(link.Children,label,depth+1,false);target.Add(link);
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
        static void Inline(List<NotesInline> target,string text,int depth,bool allowLinks=true)
        {
            if(depth>=8){target.Add(new NotesInline {Text=text});return;}
            var plain=new StringBuilder();Action flush=()=>{if(plain.Length>0){target.Add(new NotesInline {Text=System.Net.WebUtility.HtmlDecode(plain.ToString())});plain.Clear();}};
            for(int i=0;i<text.Length;) {
                char c=text[i];
                if(c=='\\' && i+1<text.Length && "\\`*_{}[]()#+-.!>~".IndexOf(text[i+1])>=0){plain.Append(text[i+1]);i+=2;continue;}
                bool image=c=='!' && i+1<text.Length && text[i+1]=='[';string label,url;int end;
                if(allowLinks && (c=='[' || image) && ReadLink(text,i+(image?1:0),out label,out url,out end)) {
                    flush();if(image)target.Add(new NotesInline {Text="[Image: view on GitHub]"});else Link(target,label,url,depth);i=end;continue;
                }
                if(allowLinks && c=='<') {
                    int close=text.IndexOf('>',i+1);Uri uri;
                    if(close>i+1 && TryLink(text.Substring(i+1,close-i-1),out uri)){flush();Link(target,uri.AbsoluteUri,uri.AbsoluteUri,depth);i=close+1;continue;}
                }
                if(c=='`') {
                    int length=1;while(i+length<text.Length && text[i+length]=='`')length++;
                    string marker=new string('`',length);int close=text.IndexOf(marker,i+length,StringComparison.Ordinal);
                    if(close>=0){flush();target.Add(new NotesInline {Text=text.Substring(i+length,close-i-length),Kind="code"});i=close+length;continue;}
                }
                if(c=='*' || c=='_' || (c=='~' && i+1<text.Length && text[i+1]=='~')) {
                    int length=i+1<text.Length && text[i+1]==c?2:1;string marker=new string(c,length);
                    int close=text.IndexOf(marker,i+length,StringComparison.Ordinal);
                    bool word=c=='_' && i>0 && Char.IsLetterOrDigit(text[i-1]);
                    if(!word && close>i+length && !Char.IsWhiteSpace(text[i+length]) && !Char.IsWhiteSpace(text[close-1])) {
                        flush();var span=new NotesInline {Kind=c=='~'?"strike":length==2?"bold":"italic"};
                        Inline(span.Children,text.Substring(i+length,close-i-length),depth+1,allowLinks);target.Add(span);i=close+length;continue;
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
