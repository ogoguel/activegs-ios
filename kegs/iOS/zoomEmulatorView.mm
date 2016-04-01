/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../Src/defc.h"
#include "../Src/sim65816.h"
#include "../Src/adb.h"
#import "zoomEmulatorView.h"
#ifdef ACTIVEGSKARATEKA
#import "../../ActiveGS_Karateka/KaratekaAppDelegate.h"
#elif defined(ACTIVEGS)
#import "../../common.iphone/activegsAppDelegate.h"

#endif






#undef debug_printf
#define debug_printf(...)
//#define debug_printf printf
//#define SHOW_COLOR

@implementation zoomEmulatorView
@synthesize ew = _ew;
@synthesize crt = _crt;
@synthesize useTouch = _useTouch;
@synthesize secondTouch = _secondTouch;

- (id)initWithFrame:(CGRect)frame  ratio:(float)ratio
{

    if (self = [super initWithFrame:frame]) 
	{
                
        self->dpiRatio = ratio;
        self->viewSize = frame.size;
        
        debug_printf("ratio:%f width:%f height:%f",self->dpiRatio,self->viewSize.width,self->viewSize.height);
        
		CGRect r = CGRectMake(0.0,0.0,X_A2_WINDOW_WIDTH/self->dpiRatio ,X_A2_WINDOW_HEIGHT/self->dpiRatio );
		
		self.contentSize = CGSizeMake(r.size.width,r.size.height); 
		
		self.ew = [[emulatorView alloc] initWithFrame:r];
		[self.ew setUserInteractionEnabled:TRUE];
				
		CGColorSpaceRef innerColorSpace = CGColorSpaceCreateDeviceRGB(); 
		CGContextRef icontext = CGBitmapContextCreate(nil, X_A2_WINDOW_WIDTH,X_A2_WINDOW_HEIGHT*2, 8, X_A2_WINDOW_WIDTH*4,innerColorSpace,kCGImageAlphaPremultipliedLast);
		
		CGContextSetRGBFillColor(icontext,0,0,0,0.0); // transparent
		CGContextFillRect(icontext,CGRectMake(0,0,X_A2_WINDOW_WIDTH,X_A2_WINDOW_HEIGHT*2));
		for(int h=0;h<X_A2_WINDOW_HEIGHT;h++)
		{
			CGRect rr = CGRectMake(0,h*2,X_A2_WINDOW_WIDTH,1);
			CGContextSetRGBFillColor(icontext,0,0,0,0.5); // transparent
			CGContextFillRect(icontext,rr);
		}
		CGImageRef innerImageRef = CGBitmapContextCreateImage(icontext);
		UIImage* img = [UIImage imageWithCGImage:innerImageRef];
		self.crt = [[UIImageView alloc]initWithImage:img];
		self.crt.transform = CGAffineTransformMakeScale(1,0.5);
		[self.crt setFrame:CGRectMake(0,0,r.size.width,r.size.height)];
		[self.crt setBounds:CGRectMake(0,0,r.size.width,r.size.height*2)];
		
         self.crt.hidden = (r_sim65816.get_video_fx()==VIDEOFX_CRT?0:1);
		
		[self.ew addSubview:self.crt];
        CFRelease(icontext);
        CFRelease(innerImageRef);
			
		self.maximumZoomScale = 3.0;
		self.minimumZoomScale = 0.25;
			
		self.delegate = self;

	self.scrollEnabled = FALSE;

			self.canCancelContentTouches = NO; // interdiction d'annuler une fois que la vue a commencé a récupéré les infs
		
		// Recherche les Gestures
		for (UIGestureRecognizer *gestureRecognizer in self.gestureRecognizers) {     
			if ([gestureRecognizer  isKindOfClass:[UIPanGestureRecognizer class]])
				panGR = (UIPanGestureRecognizer *) gestureRecognizer;
	    	else
			if ([gestureRecognizer  isKindOfClass:[UIPinchGestureRecognizer class]])
				pinchGR = (UIPinchGestureRecognizer *) gestureRecognizer;
	    	
		}
	
		panGR.minimumNumberOfTouches = 2;
		
		self.scrollEnabled = TRUE;


		self.pagingEnabled = NO;
		self.bounces = YES;
		self.clipsToBounds = YES;
		self.showsHorizontalScrollIndicator = NO;
		self.showsVerticalScrollIndicator = NO;	
	//	self.canCancelContentTouches = NO; // interdiction d'annuler une fois que la vue a commencé a récupéré les infs
		
#ifdef SHOW_COLOR
        [self setBackgroundColor:[UIColor redColor]];
        [self.ew setBackgroundColor:[UIColor greenColor]];
#else
        [self setBackgroundColor:[UIColor blackColor]];
        [self.ew setBackgroundColor:[UIColor blackColor]];
#endif
    [self addSubview:self.ew];
		
	}
		
    return self;
}



int inCran=0;
float refScale=0.5f;
float refScalePortrait;
float refScaleLandscape;

-(void)updateScreen:(CGRect)frame ratio:(float)ratio
{
 //   [self updateFrame:frame];
//    [self setBounds:frame];
    
    self->viewSize = frame.size;
    self->dpiRatio = ratio;
    
    CGRect r = CGRectMake(0.0,0.0,X_A2_WINDOW_WIDTH/self->dpiRatio ,X_A2_WINDOW_HEIGHT/self->dpiRatio );
    
    self.contentSize = CGSizeMake(r.size.width,r.size.height);
    [self.ew setFrame:r];
    
    [self.crt setFrame:CGRectMake(0,0,r.size.width,r.size.height)];
    [self.crt setBounds:CGRectMake(0,0,r.size.width,r.size.height*2)];
    
    [self setRatioToOne];
}

-(void)updateFrame:(CGRect)frame kbdRect:(CGRect)kbdRect
{
   
    // adapte la vue pour que le clavier ne cache pas l'écran
    CGRect scrollSize = self.bounds ;
    

    float ref = kbdRect.size.height;
    
    if ( (-scrollSize.origin.y) < ref)
    {
        // centre par rapport à la hauteur restante
        float hremaining = frame.size.height - ref;
        frame.origin.y = - kbdRect.size.height- (  hremaining - scrollSize.size.height*HEIGHT_MULTIPLY ) /2 ;
        // TODO : ajuster le zoom!
    }
    
 //   NSLog(@"Frame %f,%f %f,%f",frame.origin.x,frame.origin.y,frame.size.width,frame.size.height);

    [self setFrame:frame];
}




-(void)setRatioToOne
{
	   
    int w = (int)viewSize.width;
    int h = (int)viewSize.height;
    
    debug_printf("apprect %d x %d\n",w,h);
    
#define ZOOM_CLAMP  0.25  // calcul un zoom par multiple *.25

    
	for(int i=0;i<4;i++)
	{
		if (!(i&1))
		{
            

                
                // prend le plus petit des zooms possible
                
                float r = w*dpiRatio / (A2_WINDOW_WIDTH*WIDTH_MULTIPLY);
                r = r/ZOOM_CLAMP;
                if (r<1.0)
                    r = 1.0;
                int ir = (int)r;
                float zoomw = (float)ir*ZOOM_CLAMP;
                
                r = h*dpiRatio / (A2_WINDOW_HEIGHT*HEIGHT_MULTIPLY);
                r = r/ZOOM_CLAMP;
                if (r<1.0)
                    r = 1.0;
                ir = (int)r;
                float zoomh = (float)ir*ZOOM_CLAMP;
                
                float zoom = zoomh < zoomw ? zoomh : zoomw ;
                
                orientationVars[i].scale = zoom;
              
                
                float realew =(X_A2_WINDOW_WIDTH - A2_WINDOW_WIDTH)*WIDTH_MULTIPLY*zoom;
                float realw = w*dpiRatio;
                float posw = realw - A2_WINDOW_WIDTH*zoom;
                float x = (posw - realew)/2; // pour centrer
                
                
                float realeh =(X_A2_WINDOW_HEIGHT - A2_WINDOW_HEIGHT)*HEIGHT_MULTIPLY*zoom;
                float realh = h*dpiRatio;
                float posh = realh - A2_WINDOW_HEIGHT*HEIGHT_MULTIPLY*zoom;
                float y =  (posh - realeh)/2; // pour centrer
              
                orientationVars[i].p = CGPointMake(-x/dpiRatio/WIDTH_MULTIPLY,-y/dpiRatio/HEIGHT_MULTIPLY);
               
        	
		}
		else
		{

            
            float r = h*dpiRatio / A2_WINDOW_WIDTH;
            r = r/ZOOM_CLAMP;
            if (r<1.0)
                r = 1.0;
            int ir = (int)r;
            float zoomw = (float)ir*ZOOM_CLAMP;
            
            r = w*dpiRatio / ( A2_WINDOW_HEIGHT * HEIGHT_MULTIPLY );
            r = r/ZOOM_CLAMP;
            if (r<1.0)
                r = 1.0;
            ir = (int)r;
            float zoomh = (float)ir*ZOOM_CLAMP;
            
            float zoom = zoomh < zoomw ? zoomh : zoomw ;
            
            float realew =(X_A2_WINDOW_WIDTH - A2_WINDOW_WIDTH)*zoom;
            float realw = h*dpiRatio;
            float posw = realw - A2_WINDOW_WIDTH*zoom;
            float x = (posw - realew)/2;
            
                    
            float realeh =(X_A2_WINDOW_HEIGHT - A2_WINDOW_HEIGHT)*HEIGHT_MULTIPLY*zoom;
            float realh = w*dpiRatio;
            float posh = realh - A2_WINDOW_HEIGHT*HEIGHT_MULTIPLY*zoom;
            float y =  (posh - realeh)/2; // pour centrer
                     
            orientationVars[i].scale = zoom;
            orientationVars[i].p = CGPointMake(-x/dpiRatio,-y/dpiRatio/2);
      
                       
        }
        
        debug_printf("orientation:%d = %f , p = %f,%f\n",i, orientationVars[i].scale,orientationVars[i].p.x,orientationVars[i].p.y);
		
	}
	
	mouseDown = 0;
	allowInput = TRUE;
	zoomDisabled = FALSE;
	refScalePortrait = orientationVars[0].scale;
	refScaleLandscape = orientationVars[1].scale;
	
	
}


- (void)switchOrientation:(int)orientation 
{
    
    curOrientation = -1;
    float s = orientationVars[orientation].scale;
	CGPoint p = orientationVars[orientation].p;
    debug_printf("**** about to switchOrientation: (%d) scale:%f offset %f,%f\n",orientation,s,p.x,p.y);

    self.zoomScale = s ;
    [self setContentOffset:p];
    curOrientation = orientation;
    
    s = orientationVars[orientation].scale;
	p = orientationVars[orientation].p;
    debug_printf("**** done switching (%d) scale:%f offset %f,%f\n",orientation,s,p.x,p.y);

    inCran = false;
    
	if (orientation & 1)
		refScale = refScaleLandscape;
	else
		refScale = refScalePortrait;
	
}

-(void)setContentOffset:(CGPoint)p /*animated:(BOOL)animated*/
{
	if (p.x == 0.0f || p.y == 0.0)
    {
        debug_printf("ignoring %f,%f\n",p.x,p.y);
        return ;
    }
	// appelé par [super initWithFrame:frame] alors que ew n'est pas encore initialisé
	if(self.ew != nil )	
	{
		
		CGSize vs = self.ew.frame.size;
		CGSize scrollSize = self.bounds.size;
		
		static float realScale=0.0f;
		
		
			if (!inCran)
			{
				if ( fabs(self.zoomScale - refScale) < 0.05 )
				{
					inCran = true;
					realScale = self.zoomScale;
					self.zoomScale = refScale;
				}
			}
			else
			{
				realScale += self.zoomScale - refScale;
				if (fabs(realScale - refScale) > 0.10 )
				{
					// à faire avant le sef.zoomScale= sinon on tombe dans une boucle infinie!
					inCran = false;
					self.zoomScale = realScale;
				}
				else
					self.zoomScale = refScale;
			}


		// recentre la fenetre si elle tient totalement en hauteur ou en largeur
    
        
		if (scrollSize.width >= vs.width || inCran )
			p.x = - (scrollSize.width - abs(vs.width)) / 2.0;
		if (scrollSize.height >= vs.height || inCran )
			p.y = - (scrollSize.height - abs(vs.height)) / 2.0;
        
    
		if (curOrientation!=-1)
		{
            debug_printf("saving scale (%d) %f %f,%f\n",curOrientation,self.zoomScale,p.x,p.y);
			orientationVars[curOrientation].scale = self.zoomScale ; // / kbdScale;
			orientationVars[curOrientation].p = p;
		}
     
     
	}
	debug_printf("********** offset  : %f %f\n",p.x,p.y);
	[super setContentOffset:p];
}



- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return self.ew;
	}

- (void)drawRect:(CGRect)rect {
	
    // Drawing code
	CGContextRef g = UIGraphicsGetCurrentContext();
	
#ifdef SHOW_COLOR
	CGContextSetFillColorWithColor(g, [UIColor blueColor].CGColor);
#else
    CGContextSetFillColorWithColor(g, [UIColor blackColor].CGColor);
#endif
    
	CGContextFillRect(g, rect);
	
	
}


-(CGPoint)rotateTouch:(UITouch*) _touch
{
	CGPoint pt;
	float scale = self->dpiRatio/self.zoomScale;
	
	if (g_adb.g_warp_pointer==WARP_TOUCHSCREEN)
	{
		pt = [_touch locationInView:self.ew];
		pt.x *= self->dpiRatio;
		pt.y *= self->dpiRatio;
		pt.y = (X_A2_WINDOW_HEIGHT - pt.y);
		pt.y *=2;
		pt.x  -= BASE_MARGIN_LEFT;
		pt.y -=  BASE_MARGIN_TOP*2;
		
		return pt;
	 }
	 
	 
	pt = [_touch locationInView:self];
	int height = self.contentSize.height;
		 
	if (r_sim65816.is_emulator_in_640mode())
	{
		// current scale for 640 mode
	}
	else
	{
		// otherwise reduce the speed of the mouse
		scale /= 2;
	}
		 
	pt.y = (height - pt.y)*scale;
	pt.x *= scale;
	
	return pt;

}


-(BOOL)touchesShouldCancelInContentView:(UIView *)view
{
	debug_printf("touchesShouldCancelInContentView\n");
	return !zoomDisabled;
}

-(void)enableInput
{
	allowInput = TRUE;
}

-(void)disableInput
{
	allowInput = FALSE;
	if (self.useTouch)
		self.useTouch = nil;
	
	if (self.secondTouch)
	{
        self.secondTouch = nil;
		printf("*** secondtouch set in disableinput --- should never appear");
	}
}

 
-(void)disableZoom
{
	
	debug_printf("disableZoom\n");
	
    // BUG!!!!
    // Sometimes, offset is being reset when gestures are disabled!
    // Workaround = save the former offset and restore it afterwards
    CGPoint offset = self.contentOffset;
    
    pinchGR.enabled = FALSE;
    panGR.enabled = FALSE;
    self.scrollEnabled = FALSE;
	zoomDisabled= TRUE;

    // REstoring the offset
    self.contentOffset = offset;
	
}

-(void)enableZoom
{

	debug_printf("enableZoom\n");

        pinchGR.enabled = TRUE;
        panGR.enabled = TRUE;
    
        self.scrollEnabled = TRUE;
  
	zoomDisabled = FALSE;
}


- (void)scrollViewWillBeginZooming:(UIScrollView *)scrollView withView:(UIView *)view 
{
	debug_printf("scrollViewWillBeginZooming\n");
#ifdef ACTIVEGS
	// le zoom a démarré : anule les timers pour que le zoom ne soit pas interrompu
	[[pManager getEmulatorView].contentView disableGestures:(MODE_ALL + MODE_EMULATOR & ~MODE_ZOOM)];
	[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:MODE_ZOOM color:[UIColor grayColor]];
#endif
}


- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(float)scale
{
	debug_printf("scrollViewDidEndZooming\n");
#ifdef ACTIVEGS
	// zoom terminé => on peut réactiver les gestures immédiatement
	[[pManager getEmulatorView].contentView reenableAllGestures];
#endif
}


- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
  //  NSLog(@" Offset = %@ ",NSStringFromCGPoint(scrollView.contentOffset));
}


- (void)scrollViewDidZoom:(UIScrollView *)scrollView
{
//	printf("scrollViewDidZoom");
//	[ew directDraw:nil];
}

- (void) mouseDown
{
	if (!mouseDown)
	{
		
		debug_printf("mouseDown  %f %f\n",lastMousePos.x,lastMousePos.y);
		mouseDown = 1;
		add_event_mouse(lastMousePos.x,lastMousePos.y,1,1);
#ifdef ACTIVEGS
		[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:MODE_MOUSEBUTTON color:[UIColor redColor]];
#endif
	}
}

- (void) mouseUp
{
	if (mouseDown)
	{
		add_event_mouse(lastMousePos.x,lastMousePos.y,0,1);
		mouseDown = 0;
#ifdef ACTIVEGS
		[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:MODE_MOUSEBUTTON color:[UIColor greenColor]];
#endif
	}
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
#ifdef ACTIVEGS
	if ([[pManager getEmulatorView].kbdc myTouchesBegan:touches])
	{
		debug_printf("pad activated");

		[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:MODE_PAD color:[UIColor grayColor]];
		[[pManager getEmulatorView].contentView disableGestures:(MODE_ALL+MODE_EMULATOR & ~MODE_PAD)];
		return ;
	}
#endif
	
	// prend la première touche
	UITouch *touch = [[touches allObjects] objectAtIndex:0];
	

	
	
		
	if (!allowInput)
	{
		debug_printf("input disabled");
		return ;
	}
	
	if ([touches count] > 1)
	{
		debug_printf("multiple - disabling input");
#ifdef ACTIVEGS
		[[pManager getEmulatorView].contentView disableGestures:MODE_EMULATOR];
#endif
		return ;
	}
		  
		
	if (self.useTouch)
	{
		if (!self.secondTouch && zoomDisabled)
		{
			if (!mouseDown)
			{
				debug_printf("second touch mouse Down\n");
				[self mouseDown];
			}
			else
			{
				debug_printf("second touch\n");

			}

			self.secondTouch = touch;			
		}
		else
		{
			if (zoomDisabled)
			{
				debug_printf("third or more touch ignore");
			}
			else
			{
				debug_printf("ignoring second touch - too soon, must be a gestures");
#ifdef ACTIVEGS
				[[pManager getEmulatorView].contentView disableGestures:MODE_EMULATOR];
#endif
			}
		}
		return ;
		
	}
	

	
	self.useTouch = touch;
	
	lastMousePos = [self rotateTouch:touch];
	
	add_event_mouse(lastMousePos.x,lastMousePos.y,0,-1);
	
	if (self.secondTouch)
	{
		// mouseDown déjà enclenchée -> on a fini
		return ;
	}
		
	int nbtap = [touch tapCount];
	if (nbtap==1)
	{
		// prépare la désactivation des gestures
		//[[pManager getEmulatorView].contentView scheduleDisableAllGestures];
		startNewPhase=TRUE;
		
	}
	

	if (nbtap==2)
	{
		extern void simulate_space_event();
		simulate_space_event();
	}
	
	if (nbtap>=2 || g_adb.g_warp_pointer==WARP_TOUCHSCREEN)
	{
		
		debug_printf("*** multi-tap\n");
		
		[self mouseUp];

		[self mouseDown];
		
		// Désactive immédiatement les gestures
#ifdef ACTIVEGS
		if (g_adb.g_warp_pointer!=WARP_TOUCHSCREEN)
			[[pManager getEmulatorView].contentView disableGestures:MODE_ALL];
#endif
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	
	debug_printf("touchesEnded : %d",[touches count]);

#ifdef ACTIVEGS
	if (! [[pManager getEmulatorView].kbdc myTouchesEnded:touches])
	{
		// le pad n'est plus actif - reactive les input si elles étaient disabled
		if (!allowInput)
			[[pManager getEmulatorView].contentView reenableAllGestures];
	}
#endif
	
				
	if (!allowInput)
		return ;
	
	if (    [touches containsObject:self.useTouch] 
        &&  [touches containsObject:self.secondTouch])
	{
		debug_printf("both keys ended at the sametime");
		
		[self mouseUp];
    
		self.useTouch = nil;
		self.secondTouch = nil;
		
        #ifdef ACTIVEGS
		// vérifie les gestures
		customView* cv = [pManager getEmulatorView].contentView;
		[cv processGesturesOnTouchesEnded];
#endif
		return ;
	}
	
	if ([touches containsObject:self.useTouch])
	{
		debug_printf("contains useTouch");
		
		if (self.secondTouch)
		{
			// libère useTouch mais ne fait rien d'autre - maintien la touche enclenchée
		
			debug_printf("maintaining mouseDown - secondTouch ");
			self.useTouch = nil;
			return;
		}
		else 
		{
			debug_printf("touchesEnded\n");
			
			[self mouseUp];
			self.useTouch = nil;
#ifdef ACTIVEGS
			// vérifie les gestures
			customView* cv = [pManager getEmulatorView].contentView;
			[cv processGesturesOnTouchesEnded];
#endif
		}
	}
	
	if ([touches containsObject:self.secondTouch])
	{
		// correspond à un mouseUp
		[self mouseUp];
		self.secondTouch = nil;
		debug_printf("secondTouch ended");
		
		if (!self.useTouch)
		{
#ifdef ACTIVEGS
			// aucune touche n'est plus appuyée : réactive les gestures
			[[pManager getEmulatorView].contentView scheduleEnableAllGestures];
#endif
		}
	}
	
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	
	// on est ici car lez zoom a démarré => théoriquement, il ne doit y avoir aucun useTouch, secondTouch, ou mouseDown

	if ([touches containsObject:self.useTouch])
	{
		debug_printf("touchesCancelled useTouch\n");
		self.useTouch = nil;
	}
	if ([touches containsObject:self.secondTouch])
	{
		debug_printf("touchesCancelled secondTouch\n");
		self.secondTouch = nil;
	}
	
	// DEBUG INFO
	if (mouseDown)
	{
		printf("mouseDown in touchesCancelled***");
		[self mouseUp];
	}
	
	//[[pManager getEmulatorView].contentView reenableAllGestures];
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{

#ifdef ACTIVEGS
	if ([[pManager getEmulatorView].kbdc myTouchesMoved:touches])
		return ;
#endif
	if (!allowInput)
		return ;
	
	if (startNewPhase)
	{
		debug_printf("first finger moved");
		startNewPhase = FALSE;
#ifdef ACTIVEGS
		[[pManager getEmulatorView].contentView disableGestures:MODE_ZOOM]; // à faire avant le schedule
		[[pManager getEmulatorView].contentView scheduleDisableAllGestures];
#endif
	}
	
	if (!self.useTouch) return ;
	if ([touches containsObject:self.useTouch])
	{
		
		lastMousePos = [self rotateTouch:self.useTouch];
		add_event_mouse(lastMousePos.x,lastMousePos.y,mouseDown,1);
	}
}


- (void)dealloc {
	
    self.delegate = nil;
}


@end
