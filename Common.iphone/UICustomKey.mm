/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "activegsAppDelegate.h"
#include"../ActiveGS_Karateka/KaratekaKBDController.h"
#import "UICustomKey.h"



@implementation UICustomKey

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}


@end

#define CUSTOM_KEY_WIDTH_DEFAULT 64.0f
#define CUSTOM_KEY_HEIGHT_DEFAULT 64.0f

float CUSTOM_KEY_WIDTH ;
float CUSTOM_KEY_HEIGHT ;

int nb_custom_keys = 0;
key_type custom_keys[MAX_CUSTOM_KEYS];


void init_custom_keys()
{
    custom_keys[0].mode = INPUTMODE_LEFTRIGHTKEY;
    custom_keys[0].str = @"←";
    custom_keys[0].code = 0x3B;
    custom_keys[0].halign = LEFT_BORDER ;
  //  custom_keys[0].valign = MIDDLE_BOTTOM+ 3 ;
    custom_keys[0].valign = SAME_LINE + 4 ;
    custom_keys[0].hfactor = 1 ;
    
    
    
    custom_keys[1].mode = INPUTMODE_LEFTRIGHTKEY;
    custom_keys[1].str = @"→";
     custom_keys[1].code = 0x3C;
    custom_keys[1].halign = LEFT_ALIGN + 0;
    custom_keys[1].valign = SAME_LINE + 0;
    custom_keys[1].hfactor = 1 ;
    
    custom_keys[8].mode = INPUTMODE_LEFTRIGHTKEY;
    custom_keys[8].str = @"SPC";
    custom_keys[8].code = 0x31;
    custom_keys[8].halign = LEFT_BORDER  /*LEFTCENTER_ALIGN*/ + 0;
    custom_keys[8].valign = TOP_ALIGN + 0;
  //  custom_keys[8].valign = BOTTOM_ALIGN + 0;
    custom_keys[8].hfactor = 2 ;

    
    
    custom_keys[2].mode = INPUTMODE_PUNCHKEY + INPUTMODE_PUNCHLEFTRIGHT;
    custom_keys[2].str = @"W"; //@"Q";
    custom_keys[2].halign = RIGHT_ALIGN + RIGHT_BORDER;
    custom_keys[2].valign = TOP_ALIGN + 3;
    custom_keys[2].code = 0x0D; //0x0C;
    
    custom_keys[3].mode = INPUTMODE_PUNCHKEY + INPUTMODE_PUNCHLEFTRIGHT;
    custom_keys[3].str = @"S"; //@"A";
    custom_keys[3].halign = RIGHT_ALIGN + RIGHT_BORDER;
    custom_keys[3].valign = VERTICAL_CENTER;
    custom_keys[3].code = 0x01; //0x00;
    
    custom_keys[4].mode = INPUTMODE_PUNCHKEY + INPUTMODE_PUNCHLEFTRIGHT;
    custom_keys[4].str = @"X"; //@"Z";
    custom_keys[4].halign = RIGHT_ALIGN + RIGHT_BORDER;
    custom_keys[4].valign = BOTTOM_ALIGN +3;
    custom_keys[4].code = 0x07; //0x06;
    
    custom_keys[5].mode = INPUTMODE_PUNCHKEY ;
    custom_keys[5].str = @"Q"; //@"W";
    custom_keys[5].code = 0x0C; //0x0D;
    custom_keys[5].halign = RIGHT_ALIGN + 2;
    custom_keys[5].valign = SAME_LINE + 2;
    
    
    custom_keys[6].mode = INPUTMODE_PUNCHKEY ;
    custom_keys[6].str = @"A"; //@"S";
    custom_keys[6].code = 0x00; //0x01;
    custom_keys[6].halign = SAME_COL + 5;
    custom_keys[6].valign = BOTTOM_ALIGN + 5;

    
    custom_keys[7].mode = INPUTMODE_PUNCHKEY ;
    custom_keys[7].str = @"Z"; //@"X";
    custom_keys[7].code = 0x06; //0x07;
    custom_keys[7].halign = SAME_COL + 6;
    custom_keys[7].valign = BOTTOM_ALIGN + 6;
    
    custom_keys[9].mode = INPUTMODE_SPACEKEY ;
    custom_keys[9].str = @"SPC";
    custom_keys[9].code = 0x31;
    custom_keys[9].halign = RIGHT_ALIGN + RIGHT_BORDER;
    custom_keys[9].valign = VERTICAL_CENTER;
    custom_keys[9].vfactor = 2;
    custom_keys[9].hfactor = 2;
    
    custom_keys[10].mode = INPUTMODE_PUNCHLEFTRIGHT ;
    custom_keys[10].str = @"W";
    custom_keys[10].code = 0x0D;
    custom_keys[10].halign = LEFT_ALIGN + LEFT_BORDER;
    custom_keys[10].valign = SAME_LINE + 2;
    
    
    custom_keys[11].mode = INPUTMODE_PUNCHLEFTRIGHT ;
    custom_keys[11].str = @"S";
    custom_keys[11].code = 0x01;
    custom_keys[11].halign = LEFT_ALIGN + LEFT_BORDER;
    custom_keys[11].valign = BOTTOM_ALIGN + 5;
    
    
    custom_keys[12].mode = INPUTMODE_PUNCHLEFTRIGHT ;
    custom_keys[12].str = @"X";
    custom_keys[12].code = 0x07;
    custom_keys[12].halign = LEFT_ALIGN + LEFT_BORDER;
    custom_keys[12].valign = BOTTOM_ALIGN + 6;
    
    nb_custom_keys = 13;
      
    float res = [pManager resolutionRatio];
    CUSTOM_KEY_HEIGHT = CUSTOM_KEY_HEIGHT_DEFAULT*res;
    CUSTOM_KEY_WIDTH = CUSTOM_KEY_WIDTH_DEFAULT*res;
    
    
    for (int i=0;i<nb_custom_keys;i++)
    {
        //if (custom_keys[i].enabled)
        {
            UILabel* v = [[UILabel alloc]initWithFrame:CGRectMake(-1,-1,
                                        CUSTOM_KEY_WIDTH*custom_keys[i].hfactor-2.0,
                                        CUSTOM_KEY_HEIGHT*custom_keys[i].vfactor-2.0)];
  //          v.text = custom_keys[i].str;
            v.backgroundColor = custom_keys[i].bgcolor_off;
            v.layer.borderColor =  custom_keys[i].bgcolor_on.CGColor;
            v.layer.borderWidth = 1.0;
        //    v.textColor = [UIColor redColor];
            KBDController *pKbd = [pManager getEmulatorView].kbdc;
            v.textAlignment = UITextAlignmentCenter;
            [v.layer setCornerRadius:6];
            [pKbd.view addSubview:v];
            custom_keys[i].view=v;
        }
    }
}

void fade_custom_keys(float _alpha)
{
    for(int i=0;i<nb_custom_keys;i++)  
    {
        if (custom_keys[i].view)
            custom_keys[i].view.alpha = _alpha;
    }
            
}

void touch_begin_custom_key(CGPoint p, int nbtap)
{
    for(int i=0;i<nb_custom_keys;i++)  
    {
        UILabel* v = custom_keys[i].view;
        if (custom_keys[i].view.hidden == true )
            continue;
            
        CGRect r  =v.frame;
        if (CGRectContainsPoint(r,p))
        {
            // j'interromps le déplacement
            if (inputMode & INPUTMODE_PAD)
            {
                setLeft(UP);
                setRight(UP);
            }
            
            int code = custom_keys[i].code & 0xFF;
      //      printf("code %x down nbtap:%d\n",code,nbtap);
            
            
            if (nbtap==2 && inputMode & INPUTMODE_BOWONDOUBLETAP)
            {
                if ( is_player_standing() || is_player_about_to_stand() )
                {
             //       printf("about to bow\n");
                    setSpace(UP);
                    setB(DOWN); // Up is automatic
                    continue ;
                }
            }
            
            if (code==0x31)
                setSpace(DOWN);
            else
                add_event_key(code ,0);
         
            //  v.backgroundColor = custom_keys[i].bgcolor_over;
            v.layer.borderColor =  custom_keys[i].bgcolor_over.CGColor;
            custom_keys[i].status = 1;
           
          
        }
    }
}

void touch_end_custom_key()
{
    for(int i=0;i<nb_custom_keys;i++)  
    {
        if (!custom_keys[i].status )
            continue;
            
        [UIView beginAnimations:nil context:NULL];
        [UIView setAnimationDuration:0.2];
      //  custom_keys[i].view.backgroundColor=custom_keys[i].bgcolor_off;
         custom_keys[i].view.layer.borderColor =  custom_keys[i].bgcolor_on.CGColor;
        [UIView commitAnimations];
        
        int code = custom_keys[i].code & 0xFF;
        
        if (code==0x31)
            setSpace(UP);
        else
            add_event_key(code,1);
        
        custom_keys[i].status = 0;
     

        
    }
    
}

void toggle_custom_keys(int _mode, int _hide)
{
    for(int i=0;i<nb_custom_keys;i++)
    {
        if (custom_keys[i].mode & _mode)
            custom_keys[i].view.hidden = _hide;
    }
}

void show_custom_keys(CGRect _rv, int _mode, int _centery)
{
    
   // KBDController *pKbd = [pManager getEmulatorView].kbdc;
    CGSize screen = _rv.size;
    
    // reset la position
    
    for(int i=0;i<nb_custom_keys;i++)  
    {
        if (!(custom_keys[i].mode & _mode))
            custom_keys[i].view.hidden = true;
        else
            custom_keys[i].view.hidden = false;
        
    
        UILabel* v = custom_keys[i].view;
        CGRect r = v.frame;
        r.origin.x = -1;
        r.origin.y = -1;
        [v setFrame:r];
    }

    
    int done;
    do
    {
        done = 1;
        
        for(int i=0;i<nb_custom_keys;i++)  
        {
            /*
             if (!(custom_keys[i].mode & _mode))
                continue;
            */
            
            UILabel* v = custom_keys[i].view;
            CGRect r = v.frame;
            if (r.origin.x != -1 && r.origin.y != -1 )
                continue;
         
            float refx=-1;
            float refy=-1;
            
            int ihref = -1;
            if (custom_keys[i].halign & LEFT_BORDER)
                refx = 0;
            else
            if (custom_keys[i].halign & RIGHT_BORDER)
                refx = screen.width;
            else
            {
                ihref= custom_keys[i].halign&0xFF;
                refx = custom_keys[ihref].view.frame.origin.x; 
            }
            
              int ivref = -1;
            if (custom_keys[i].valign & BOTTOM_BORDER)
                refy = screen.height;
            else
            if (custom_keys[i].valign & VERTICAL_CENTER)
            {
                refy = _centery - CUSTOM_KEY_HEIGHT*custom_keys[i].vfactor/2;
                /*
                if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
                    refy = screen.height ;
                else
                    refy = (screen.height - CUSTOM_KEY_HEIGHT*custom_keys[i].vfactor)/2;
                */
            }
            else
            {
                ivref= custom_keys[i].valign&0xFF;
                refy = custom_keys[ivref].view.frame.origin.y; 
            }     
            
            if (custom_keys[i].halign & LEFT_ALIGN && refx!=-1)
                refx += CUSTOM_KEY_WIDTH*custom_keys[ihref].hfactor;
            
            if (custom_keys[i].halign & LEFTCENTER_ALIGN && refx!=-1)
                refx += CUSTOM_KEY_WIDTH*custom_keys[ihref].hfactor*0.5;
            
            if (custom_keys[i].halign & RIGHT_ALIGN && refx!=-1)
                refx -= CUSTOM_KEY_WIDTH*custom_keys[i].hfactor;
            
            if (custom_keys[i].valign & TOP_ALIGN && refy!=-1)
                refy -= CUSTOM_KEY_HEIGHT*custom_keys[i].vfactor;
            if (custom_keys[i].valign & BOTTOM_ALIGN && refy!=-1)
                refy += CUSTOM_KEY_HEIGHT*custom_keys[ivref].vfactor;
            if (custom_keys[i].valign & MIDDLE_BOTTOM && refy!=-1)
                refy += CUSTOM_KEY_HEIGHT*custom_keys[ivref].vfactor/2;
            
            int mod=0;
            if (refx!=-1 && r.origin.x ==-1)
            {
                mod = 1;
                r.origin.x = refx;
            }
            
            if (refy!=-1 && r.origin.y ==-1)
            {
                mod = 1;
                r.origin.y = refy;
            }
            
            if ( mod )
            {
           //    NSLog(@"key:%d ->x:%f, y:%f",i,refx,refy);
                [v setFrame:r];
            }
            
            done = 0;    
        }   
    }
    while (!done);
        
}
