using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using System.Reflection;
[assembly: AssemblyTitle("Fullscreen Timer")]
[assembly: AssemblyProduct("Fullscreen Timer")]
[assembly: AssemblyVersion("2.0.0.0")]
class TimerWindow : Window {
 double seconds;
 bool paused = true, countdown, blink, fullscreen;
 string editing;
 TextBlock minutesText, secondsText, colon;
 StackPanel digits, tips;
 Button fullLabel, modeLabel, runLabel;
 WindowState previousState;
 Rect previousBounds;
 DispatcherTimer ticker;
 readonly Brush grey = new SolidColorBrush(Color.FromRgb(102,102,102));
 public TimerWindow() {
  Title="Fullscreen Timer"; Width=1280; Height=800; MinWidth=480; MinHeight=360;
  Background=Brushes.Black; Foreground=Brushes.White; WindowStartupLocation=WindowStartupLocation.CenterScreen;
  var grid=new Grid(); Content=grid;
  digits=new StackPanel {Orientation=Orientation.Horizontal, HorizontalAlignment=HorizontalAlignment.Center, VerticalAlignment=VerticalAlignment.Center};
  minutesText=Digit(); colon=Digit(); colon.Text=":"; secondsText=Digit();
  digits.Children.Add(minutesText); digits.Children.Add(colon); digits.Children.Add(secondsText); grid.Children.Add(digits);
  digits.MouseLeftButtonDown+=(s,e)=>{if(e.ClickCount==2)ToggleFullscreen();};
  tips=new StackPanel {HorizontalAlignment=HorizontalAlignment.Center,VerticalAlignment=VerticalAlignment.Bottom,Margin=new Thickness(0,0,0,20)};
  grid.Children.Add(tips);
  fullLabel=Tip(new string[]{"F"},new Action[]{ToggleFullscreen},"enter fullscreen");
  Tip(new string[]{"←","→","↑","↓"},new Action[]{()=>Move("left"),()=>Move("right"),()=>Move("up"),()=>Move("down")},"edit timer");
  Tip(new string[]{"R"},new Action[]{Reset},"reset timer");
  modeLabel=Tip(new string[]{"S"},new Action[]{SwitchMode},"countdown or stopwatch ✓"); modeLabel.Click+=(s,e)=>SwitchMode();
  runLabel=Tip(new string[]{"Space"},new Action[]{ToggleRun},"start timer");
  SizeChanged+=(s,e)=>ResizeDigits();
  PreviewKeyDown+=OnKey;
  ticker=new DispatcherTimer {Interval=TimeSpan.FromMilliseconds(500)};
  ticker.Tick+=(s,e)=>{if(editing!=null)blink=!blink; if(!paused){seconds+=countdown?-.5:.5;if(seconds<=0){seconds=0;paused=true;}} Render();};
  ticker.Start(); Render();
 }
 TextBlock Digit(){return new TextBlock {FontFamily=new FontFamily("Tahoma"),Foreground=Brushes.White,FontWeight=FontWeights.Normal,Focusable=false};}
 Button Tip(string[] keys,Action[] actions,string description) {
  var row=new StackPanel {Orientation=Orientation.Horizontal,Height=32}; tips.Children.Add(row);
  row.Children.Add(new TextBlock {Text="•",Foreground=grey,Width=20,VerticalAlignment=VerticalAlignment.Center,FontSize=16});
  for(int i=0;i<keys.Length;i++) {
   var button=new Button {Content=keys[i],FontSize=12,FontFamily=new FontFamily("Segoe UI"),Foreground=grey,Background=Brushes.Transparent,BorderBrush=new SolidColorBrush(Color.FromRgb(51,51,51)),BorderThickness=new Thickness(2),Padding=new Thickness(4,0,4,0),Margin=new Thickness(0,0,4,0),VerticalAlignment=VerticalAlignment.Center,Focusable=false,Cursor=Cursors.Hand};
   var template=new ControlTemplate(typeof(Button)); var border=new FrameworkElementFactory(typeof(Border));
   border.SetValue(Border.CornerRadiusProperty,new CornerRadius(4)); border.SetValue(Border.BorderBrushProperty,button.BorderBrush);border.SetValue(Border.BorderThicknessProperty,new Thickness(2));border.SetValue(Border.PaddingProperty,new Thickness(4,0,4,0));
   var content=new FrameworkElementFactory(typeof(ContentPresenter));border.AppendChild(content);template.VisualTree=border;button.Template=template;
   Action action=actions[i];button.Click+=(s,e)=>action();row.Children.Add(button);
  }
  row.Children.Add(new TextBlock {Text="-",Foreground=grey,VerticalAlignment=VerticalAlignment.Center,FontSize=16});
  var label=new Button {Content=description,Foreground=grey,Background=Brushes.Transparent,BorderThickness=new Thickness(0),Padding=new Thickness(0),Margin=new Thickness(4,0,0,0),FontSize=16,FontFamily=new FontFamily("Segoe UI"),VerticalAlignment=VerticalAlignment.Center,Focusable=false};
  var lt=new ControlTemplate(typeof(Button));lt.VisualTree=new FrameworkElementFactory(typeof(ContentPresenter));label.Template=lt;row.Children.Add(label);return label;
 }
 void ResizeDigits(){double size=((FrameworkElement)Content).ActualWidth*.30;minutesText.FontSize=colon.FontSize=secondsText.FontSize=Math.Max(1,size);}
 void Render(){
  minutesText.Text=((int)(seconds/60)).ToString("D2"); secondsText.Text=((int)(seconds%60)).ToString("D2");
  minutesText.Opacity=editing=="minute"&&blink?0:1;secondsText.Opacity=editing=="second"&&blink?0:1;
  tips.Visibility=paused?Visibility.Visible:Visibility.Hidden;
  fullLabel.Content=fullscreen?"exit fullscreen":"enter fullscreen";
  modeLabel.Content=countdown?"countdown ✓ or stopwatch":"countdown or stopwatch ✓";
  runLabel.Content=paused?"start timer":"pause timer";
 }
 void Reset(){seconds=0;paused=true;editing=null;blink=false;Render();}
 void SwitchMode(){countdown=!countdown;Render();}
 void ToggleRun(){paused=!paused;editing=null;blink=false;Render();}
 void Move(string direction){paused=true;blink=false;
  if(direction=="left")editing="minute";else if(direction=="right")editing="second";
  else {if(editing==null)editing="second";seconds=Math.Max(0,seconds+(direction=="up"?1:-1)*(editing=="minute"?60:1));}
  Render();
 }
 void ToggleFullscreen(){
  if(!fullscreen){previousState=WindowState;previousBounds=RestoreBounds;WindowState=WindowState.Normal;WindowStyle=WindowStyle.None;ResizeMode=ResizeMode.NoResize;WindowState=WindowState.Maximized;fullscreen=true;}
  else {WindowState=WindowState.Normal;WindowStyle=WindowStyle.SingleBorderWindow;ResizeMode=ResizeMode.CanResize;Left=previousBounds.Left;Top=previousBounds.Top;Width=previousBounds.Width;Height=previousBounds.Height;WindowState=previousState;fullscreen=false;}
  Render();
 }
 void OnKey(object sender,KeyEventArgs e){
  switch(e.Key){
   case Key.F:ToggleFullscreen();break;
   case Key.Escape:if(fullscreen)ToggleFullscreen();else return;break;
   case Key.R:Reset();break;
   case Key.S:SwitchMode();break;
   case Key.Space:ToggleRun();break;
   case Key.Left:Move("left");break;
   case Key.Right:Move("right");break;
   case Key.Up:Move("up");break;
   case Key.Down:Move("down");break;
   case Key.Enter:editing=editing==null?"second":null;blink=false;Render();break;
   default:return;
  }e.Handled=true;
 }
 [STAThread] static void Main(){new Application().Run(new TimerWindow());}
}
