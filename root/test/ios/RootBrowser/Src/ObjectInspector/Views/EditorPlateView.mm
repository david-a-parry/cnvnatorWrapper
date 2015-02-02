#import <math.h>

#import <QuartzCore/QuartzCore.h>

#import "EditorPlateView.h"
#import "EditorView.h"


@implementation EditorPlateView {
   UIImage *plateImage;
   UILabel *editorLabel;
   
   UIImage *arrowImage;
   
   EditorView *topView;
}

@synthesize editorName;
@synthesize arrowImageView;

//____________________________________________________________________________________________________
+ (CGFloat) plateHeight
{
   return 50.f;
}

//____________________________________________________________________________________________________
- (instancetype) initWithFrame : (CGRect) frame editorName : (NSString *) name topView : (EditorView *) tv
{
   self = [super initWithFrame:frame];

   if (self) {
      editorLabel = [[UILabel alloc] initWithFrame : CGRectMake(frame.size.width / 2 - 60.f, 10.f, 120.f, 30.f)];
      editorLabel.backgroundColor = [UIColor clearColor];
      editorLabel.font = [UIFont systemFontOfSize : 14];
      editorLabel.textAlignment = NSTextAlignmentCenter;
      editorLabel.textColor = [UIColor whiteColor];
      [self addSubview : editorLabel];
      editorLabel.text = name;
      topView = tv;

      plateImage = [UIImage imageNamed:@"editor_plate.png"];
        
      arrowImage = [UIImage imageNamed:@"editor_state_arrow.png"];
      arrowImageView = [[UIImageView alloc] initWithImage : arrowImage];
      CGRect arrowFrame = arrowImageView.frame;
      arrowFrame.origin.x = frame.size.height / 2 - arrowFrame.size.width / 2 + 3;
      arrowFrame.origin.y = frame.size.height / 2 - arrowFrame.size.height / 2;
      arrowImageView.frame = arrowFrame;
      [self addSubview : arrowImageView];
      
      UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget : self action : @selector(handleTap:)];
      [self addGestureRecognizer:tap];
      self.editorName = name;
      self.opaque = NO;
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) drawRect : (CGRect) rect
{
   [plateImage drawAtPoint : CGPointZero];
   //Draw the triangle.
}

//____________________________________________________________________________________________________
- (void) handleTap : (UITapGestureRecognizer *) tap
{
   [topView plateTapped : self];
}

@end
