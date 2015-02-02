#import <cassert>

#import <QuartzCore/QuartzCore.h>

#import "ColorCell.h"

@implementation ColorCell {
   double rgb[3];
}

//____________________________________________________________________________________________________
- (instancetype) initWithFrame : (CGRect) frame
{
   if (self = [super initWithFrame : frame]) {
      self.backgroundColor = [UIColor clearColor];
      self.layer.shadowColor = [UIColor darkGrayColor].CGColor;
      self.layer.shadowOffset = CGSizeMake(4.f, 4.f);
      self.layer.shadowOpacity = 0.4f;
      
      //Here important optimization.
      self.layer.shadowPath = [UIBezierPath bezierPathWithRect:CGRectMake(10.f, 10.f, frame.size.width - 20.f, frame.size.height - 20)].CGPath;
      //
      
      self.opaque = NO;
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) setRGB : (const double *) newRgb
{
   assert(newRgb != nullptr && "setRGN:, parameter 'newRgb' is null");

   rgb[0] = newRgb[0];
   rgb[1] = newRgb[1];
   rgb[2] = newRgb[2];
}

//____________________________________________________________________________________________________
- (void) drawRect : (CGRect) rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      //Log error: ctx is nil.
      return;
   }

   CGContextSetRGBFillColor(ctx, CGFloat(rgb[0]), CGFloat(rgb[1]), CGFloat(rgb[2]), 1.f);
   
   const CGRect colorCellRect = CGRectMake(10.f, 10.f, rect.size.width - 20.f, rect.size.height - 20.f);
   CGContextFillRect(ctx, colorCellRect);
}

@end
