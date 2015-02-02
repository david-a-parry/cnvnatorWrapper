#ifndef ROOT_HsimpleDemo
#define ROOT_HsimpleDemo

#include <memory>

#include "DemoBase.h"

class TH1F;

namespace ROOT {
namespace iOS {
namespace Demos {

class HsimpleDemo : public DemoBase {
public:
   HsimpleDemo();
   ~HsimpleDemo();
   
   //Overriders.
   void ResetDemo();
   bool IsAnimated() const;
   
   unsigned NumOfFrames() const;
   double AnimationTime() const;

   void StartAnimation();
   void NextStep();
   void StopAnimation();

   void AdjustPad(Pad *pad);

   void PresentDemo();
   
   bool Supports3DRotation() const {return false;}

private:
   std::unique_ptr<TH1F> fHist;
   
   HsimpleDemo(const HsimpleDemo &rhs) = delete;
   HsimpleDemo &operator = (const HsimpleDemo &rhs) = delete;
};

}
}
}

#endif
