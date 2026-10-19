"""Translate the shared Windows home layout into Avalonia's equivalent syntax.

The source of geometry, labels, artwork placement and colors stays Launcher.xaml.
Only UI framework syntax and host wording are adapted here; no game policy.
"""
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
W='http://schemas.microsoft.com/winfx/2006/xaml/presentation'
A='https://github.com/avaloniaui'
X='http://schemas.microsoft.com/winfx/2006/xaml'
ET.register_namespace('',A);ET.register_namespace('x',X)
def local(tag):return tag.rsplit('}',1)[-1]
def generate(source,destination):
 root=ET.parse(source).getroot();root.set('{'+X+'}Class','TimeSplitters.Launcher.NativeWindow')
 styles=ET.Element('{'+A+'}Window.Styles')
 for resource in list(root.find('{'+W+'}Window.Resources')):
  if local(resource.tag)!='Style':continue
  key=resource.attrib.get('{'+X+'}Key');target=resource.attrib['TargetType']
  style=ET.SubElement(styles,'{'+A+'}Style',{'Selector':target+('.'+key if key else '')})
  for setter in resource:
   if setter.attrib.get('Property') in ('Template','StrokeLineJoin','StrokeEndLineCap'):continue
   if 'Value' not in setter.attrib:continue
   if setter.attrib['Property'] in ('StrokeStartLineCap','StrokeEndLineCap'):setter.set('Property','StrokeLineCap')
   if setter.attrib['Property']=='Stroke':setter.set('Value','#B4C7D8')
   style.append(setter)
  root.find('{'+W+'}Window.Resources').remove(resource)
 # Avalonia's default button alignment and expander theme differ from WPF.
 for selector,values in [('TabItem',{'FontSize':'13','Padding':'16,12'}),('Button',{'HorizontalAlignment':'Stretch','CornerRadius':'3'}),('Expander',{'HorizontalAlignment':'Stretch','Background':'Transparent'}),('ComboBox',{'Background':'#102031','BorderBrush':'#416685','Foreground':'#E4EDF5'})]:
  style=ET.SubElement(styles,'{'+A+'}Style',{'Selector':selector})
  for key,value in values.items():ET.SubElement(style,'{'+A+'}Setter',{'Property':key,'Value':value})
 ET.SubElement(styles,'{'+A+'}StyleInclude',{'Source':'/NativeHomeStyles.axaml'})
 root.insert(1,styles)
 root.find('{'+W+'}Grid').set('Background','#0B1420')
 for element in root.iter():
  if element.tag.startswith('{'+W+'}'):element.tag='{'+A+'}'+local(element.tag)
  for key,value in list(element.attrib.items()):
   if key in ('SnapsToDevicePixels','CanContentScroll','ToolTipService.ShowOnDisabled','RenderOptions.BitmapScalingMode','StrokeLineJoin'):del element.attrib[key]
   elif key=='Visibility':del element.attrib[key];element.set('IsVisible','True' if value=='Visible' else 'False')
   elif key=='ToolTip':del element.attrib[key];element.set('ToolTip.Tip',value.replace('File Explorer','Finder'))
   elif key=='Style':del element.attrib[key];element.set('Classes',value.removeprefix('{StaticResource ').removesuffix('}'))
   elif key in ('StrokeStartLineCap','StrokeEndLineCap'):del element.attrib[key];element.set('StrokeLineCap',value)
   elif key in ('StartPoint','EndPoint'):element.set(key,','.join(str(float(n)*100)+'%' for n in value.split(',')))
   elif key=='MinHeight' and value.startswith('{Binding'):element.set(key,'360')
   elif key=='StrokeDashArray':element.set(key,value.replace(' ',','))
   elif key=='FontFamily':element.set(key,'Menlo' if value=='Consolas' else 'Arial')
   elif value=='WINDOWS x64':element.set(key,'macOS ARM64')
   elif value=='NATIVE PC PORT':element.set(key,'NATIVE MAC PORT')
 ET.indent(root)
 Path(destination).write_text(ET.tostring(root,encoding='unicode')+'\n')
if __name__=='__main__':generate(sys.argv[1],sys.argv[2])
