/* Fullscreen Timer, native GTK3/Cairo host. No network or browser runtime. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>

typedef struct { double seconds, base, anchor; int paused,countdown,editing,blink,fullscreen; } Model;
static Model m={.paused=1};
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void sync_time(double t){if(!m.paused){m.seconds=fmax(0,m.base+(t-m.anchor)*(m.countdown?-1:1));if(m.countdown&&m.seconds<=0)m.paused=1;}}
static void action(int key){sync_time(now());switch(key){
 case ' ':m.paused=!m.paused;m.editing=0;m.blink=0;if(!m.paused){m.base=m.seconds;m.anchor=now();}break;
 case 'r':m.seconds=0;m.paused=1;m.editing=0;m.blink=0;break;
 case 's':m.countdown=!m.countdown;if(!m.paused){m.base=m.seconds;m.anchor=now();}break;
 case 0xff0d:m.editing=m.editing?0:2;m.blink=0;break;
 case 0xff51:m.editing=1;m.paused=1;m.blink=0;break;
 case 0xff53:m.editing=2;m.paused=1;m.blink=0;break;
 case 0xff52:case 0xff54:m.paused=1;if(!m.editing)m.editing=2;m.seconds=fmax(0,floor(m.seconds)+(key==0xff52?1:-1)*(m.editing==1?60:1));m.blink=0;break;
}}
#define API(ret,name,args) static ret (*name)args
API(int,gtk_init_check,(int*,char***)); API(void*,gtk_window_new,(int)); API(void,gtk_window_set_title,(void*,const char*));
API(void,gtk_window_set_default_size,(void*,int,int)); API(void,gtk_window_set_position,(void*,int)); API(void,gtk_window_fullscreen,(void*)); API(void,gtk_window_unfullscreen,(void*));
API(void*,gtk_drawing_area_new,(void)); API(void,gtk_container_add,(void*,void*)); API(void,gtk_widget_show_all,(void*)); API(void,gtk_widget_queue_draw,(void*));
API(void,gtk_widget_add_events,(void*,int));API(void,gtk_widget_set_can_focus,(void*,int));API(void,gtk_widget_grab_focus,(void*));
API(int,gtk_widget_get_allocated_width,(void*));API(int,gtk_widget_get_allocated_height,(void*));
API(void,gtk_main,(void));API(void,gtk_main_quit,(void));API(unsigned long,g_signal_connect_data,(void*,const char*,void*,void*,void*,int));API(unsigned,g_timeout_add,(unsigned,void*,void*));
API(void,cairo_set_source_rgb,(void*,double,double,double));API(void,cairo_paint,(void*));API(void,cairo_select_font_face,(void*,const char*,int,int));
API(void,cairo_set_font_size,(void*,double));API(void,cairo_move_to,(void*,double,double));API(void,cairo_show_text,(void*,const char*));
typedef struct{double x_bearing,y_bearing,width,height,x_advance,y_advance;} Extents;
API(void,cairo_text_extents,(void*,const char*,Extents*));API(void,cairo_rectangle,(void*,double,double,double,double));API(void,cairo_set_line_width,(void*,double));API(void,cairo_stroke,(void*));
static void *win,*area; static int width=1280,height=760;
typedef struct{double x,y,w,h;int key;} Hit;
static Hit hits[20];static int nhits;
static void text(void*c,const char*s,double x,double y,double size,double color){cairo_select_font_face(c,"Tahoma",0,0);cairo_set_font_size(c,size);cairo_set_source_rgb(c,color,color,color);cairo_move_to(c,x,y);cairo_show_text(c,s);}
static double measure(void*c,const char*s,double size){Extents e;cairo_set_font_size(c,size);cairo_text_extents(c,s,&e);return e.x_advance;}
static double keybox(void*c,const char*s,double x,double y,int key){double w=measure(c,s,12)+10;text(c,s,x+5,y,12,.4);cairo_set_source_rgb(c,.2,.2,.2);cairo_set_line_width(c,2);cairo_rectangle(c,x,y-13,w,18);cairo_stroke(c);hits[nhits++]=(Hit){x,y-16,w,25,key};return x+w+4;}
static int draw(void*widget,void*c,void*data){
 if(widget){width=gtk_widget_get_allocated_width(widget);height=gtk_widget_get_allocated_height(widget);}
 cairo_set_source_rgb(c,0,0,0);cairo_paint(c);cairo_select_font_face(c,"Tahoma",0,0);
 char min[32],sec[8],all[64];snprintf(min,sizeof(min),"%02lld",(long long)(m.seconds/60));snprintf(sec,sizeof(sec),"%02d",(int)m.seconds%60);snprintf(all,sizeof(all),"%s:%s",min,sec);
 double size=width*.30;Extents e;cairo_set_font_size(c,size);cairo_text_extents(c,all,&e);
 double x=(width-e.x_advance)/2,y=(height-e.height)/2-e.y_bearing;
 if(!(m.editing==1&&m.blink))text(c,min,x,y,size,1);
 x+=measure(c,min,size);text(c,":",x,y,size,1);x+=measure(c,":",size);
 if(!(m.editing==2&&m.blink))text(c,sec,x,y,size,1);
 nhits=0;if(!m.paused)return 0;
 double left=width/2.0-145,top=height-20-4*32-7;
 for(int row=0;row<5;row++){
  double by=top+row*32,rx=left;text(c,"•",rx-18,by,16,.4);
  if(row==0)rx=keybox(c,"F",rx,by,'f');
  if(row==1){rx=keybox(c,"←",rx,by,0xff51);rx=keybox(c,"→",rx,by,0xff53);rx=keybox(c,"↑",rx,by,0xff52);rx=keybox(c,"↓",rx,by,0xff54);}
  if(row==2)rx=keybox(c,"R",rx,by,'r');if(row==3)rx=keybox(c,"S",rx,by,'s');if(row==4)rx=keybox(c,"Space",rx,by,' ');
  text(c,"-",rx,by,16,.4);rx+=10;
  const char*label=row==0?(m.fullscreen?"exit fullscreen":"enter fullscreen"):row==1?"edit timer":row==2?"reset timer":row==3?(m.countdown?"countdown ✓ or stopwatch":"countdown or stopwatch ✓"):"start timer";
  text(c,label,rx,by,16,.4);if(row==3)hits[nhits++]=(Hit){rx,by-16,250,25,'s'};
 }return 0;
}
static void dispatch(int key){if(key=='f'||(key==0xff1b&&m.fullscreen)){if(m.fullscreen)gtk_window_unfullscreen(win);else gtk_window_fullscreen(win);m.fullscreen=!m.fullscreen;}else action(key);gtk_widget_queue_draw(area);}
typedef struct{int type;void*window;char send_event;unsigned time,state,keyval;int length;char*string;unsigned short hardware_keycode;unsigned char group;unsigned is_modifier;} KeyEvent;
typedef struct{int type;void*window;char send_event;unsigned time;double x,y;void*axes;unsigned state,button;} MouseEvent;
typedef struct{int type;void*window;char send_event;unsigned changed_mask,new_state;} WindowEvent;
static int keypress(void*w,KeyEvent*e,void*d){unsigned k=e->keyval;if(k>='A'&&k<='Z')k+=32;dispatch(k);return k=='f'||k=='r'||k=='s'||k==' '||(k>=0xff51&&k<=0xff54)||k==0xff0d||k==0xff1b;}
static int mousepress(void*w,MouseEvent*e,void*d){if(e->button!=1)return 0;if(e->type==5&&e->y<height-190){dispatch('f');return 1;}if(m.paused)for(int i=0;i<nhits;i++){Hit h=hits[i];if(e->x>=h.x&&e->x<=h.x+h.w&&e->y>=h.y&&e->y<=h.y+h.h){dispatch(h.key);return 1;}}return 0;}
static int windowstate(void*w,WindowEvent*e,void*d){m.fullscreen=!!(e->new_state&16);gtk_widget_queue_draw(area);return 0;}
static int pulse(void*d){sync_time(now());static long last;long b=(long)(now()*2);if(b!=last){m.blink=!m.blink;last=b;}gtk_widget_queue_draw(area);return 1;}
static void destroy(void*w,void*d){gtk_main_quit();}
static void load(void*h,const char*name,void**target){*target=dlsym(h,name);if(!*target){fprintf(stderr,"Missing system function: %s\n",name);exit(1);}}
#define LOAD(h,n) load(h,#n,(void**)&n)
static int selftest(void){
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL %s\n",#x);return 1;}puts("PASS " #x);}while(0)
 action(0xff52);CHECK(m.seconds==1);action(0xff51);action(0xff52);CHECK(m.seconds==61&&m.editing==1);action(' ');CHECK(!m.paused&&!m.editing);action(' ');CHECK(m.paused);action('r');CHECK(m.seconds==0&&m.paused);action('s');CHECK(m.countdown);action(0xff52);action(' ');sync_time(m.anchor+2);CHECK(m.seconds==0&&m.paused);puts("PASS native timer model");return 0;
}
int main(int argc,char**argv){if(argc>1&&!strcmp(argv[1],"--self-test"))return selftest();
 void*gtk=dlopen("libgtk-3.so.0",RTLD_NOW|RTLD_GLOBAL);void*cairo=dlopen("libcairo.so.2",RTLD_NOW|RTLD_GLOBAL);
 if(!gtk||!cairo){fputs("Fullscreen Timer requires the system GTK 3 and Cairo libraries. No files are downloaded by this app.\n",stderr);return 1;}
 LOAD(gtk,gtk_init_check);LOAD(gtk,gtk_window_new);LOAD(gtk,gtk_window_set_title);LOAD(gtk,gtk_window_set_default_size);LOAD(gtk,gtk_window_set_position);LOAD(gtk,gtk_window_fullscreen);LOAD(gtk,gtk_window_unfullscreen);LOAD(gtk,gtk_drawing_area_new);LOAD(gtk,gtk_container_add);LOAD(gtk,gtk_widget_show_all);LOAD(gtk,gtk_widget_queue_draw);LOAD(gtk,gtk_widget_add_events);LOAD(gtk,gtk_widget_set_can_focus);LOAD(gtk,gtk_widget_grab_focus);LOAD(gtk,gtk_widget_get_allocated_width);LOAD(gtk,gtk_widget_get_allocated_height);LOAD(gtk,gtk_main);LOAD(gtk,gtk_main_quit);LOAD(gtk,g_signal_connect_data);LOAD(gtk,g_timeout_add);
 LOAD(cairo,cairo_set_source_rgb);LOAD(cairo,cairo_paint);LOAD(cairo,cairo_select_font_face);LOAD(cairo,cairo_set_font_size);LOAD(cairo,cairo_move_to);LOAD(cairo,cairo_show_text);LOAD(cairo,cairo_text_extents);LOAD(cairo,cairo_rectangle);LOAD(cairo,cairo_set_line_width);LOAD(cairo,cairo_stroke);
 if(argc>2&&!strcmp(argv[1],"--render-preview")){
  void*(*surface_new)(int,int,int)=dlsym(cairo,"cairo_image_surface_create");
  void*(*context_new)(void*)=dlsym(cairo,"cairo_create");
  int(*write_png)(void*,const char*)=dlsym(cairo,"cairo_surface_write_to_png");
  void*surface=surface_new(0,width,height);void*context=context_new(surface);
  draw(NULL,context,NULL);return write_png(surface,argv[2]);
 }
 if(!gtk_init_check(&argc,&argv)){fputs("No desktop display available.\n",stderr);return 1;}
 win=gtk_window_new(0);gtk_window_set_title(win,"Fullscreen Timer");gtk_window_set_default_size(win,1280,760);gtk_window_set_position(win,1);area=gtk_drawing_area_new();gtk_widget_set_can_focus(area,1);gtk_widget_add_events(area,(1<<8)|(1<<10));gtk_container_add(win,area);
 g_signal_connect_data(area,"draw",draw,0,0,0);g_signal_connect_data(win,"destroy",destroy,0,0,0);g_signal_connect_data(win,"key-press-event",keypress,0,0,0);g_signal_connect_data(area,"button-press-event",mousepress,0,0,0);g_signal_connect_data(win,"window-state-event",windowstate,0,0,0);g_timeout_add(33,pulse,0);gtk_widget_show_all(win);gtk_widget_grab_focus(area);gtk_main();return 0;
}
