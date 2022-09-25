// This project is based on:
// NeoPixelCylon
// Heavily modified, such that the WS2811 strip will all start at one color
// and one-by-one from both ends, each LED will change to another color
// Then it repeats for the opposite color
//
// This will demonstrate the use of the NeoEase animation ease methods; that provide
// simulated acceleration to the animations.
//
//

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 200; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 13;     // make sure to set this to the correct pin, ignored for Esp8266
const RgbColor PurpleColor(RgbColor(64, 0, 128));
const RgbColor OrangeColor(RgbColor(255, 55, 0));

NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// for esp8266 omit the pin
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);

NeoPixelAnimator animations(2); // only ever need 2 animations

uint16_t lastPixel = 0; // track the eye position
int8_t moveDir = 1;     // track the direction of movement

// uncomment one of the lines below to see the effects of
// changing the ease function on the movement animation
AnimEaseFunction moveEase =
    NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
// NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
//      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;

void FadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}

void DescendAnimUpdate(const AnimationParam &param)
{
    // apply the movement animation curve
    float progress = moveEase(param.progress);

    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    if (moveDir < 0)
    {
        nextPixel = progress * PixelCount;
    }
    else
    {
        nextPixel = (1.0f - progress) * PixelCount;
    }

    // if progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and
    // the last
    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + moveDir; i != nextPixel; i += moveDir)
        {
            strip.SetPixelColor(i, OrangeColor);
        }
    }
    strip.SetPixelColor(nextPixel, OrangeColor);

    lastPixel = nextPixel;

    if (param.state == AnimationState_Completed)
    {
        Serial.println("Descend Animation done........................................");

        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);
    }
}

void AscendAnimUpdate(const AnimationParam &param)
{
    // apply the movement animation curve
    float progress = moveEase(param.progress);

    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    if (moveDir > 0)
    {
        nextPixel = progress * PixelCount;
    }
    else
    {
        nextPixel = (1.0f - progress) * PixelCount;
    }

    // if progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and
    // the last
    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + moveDir; i != nextPixel; i += moveDir)
        {
            strip.SetPixelColor(i, PurpleColor);
        }
    }
    strip.SetPixelColor(nextPixel, PurpleColor);

    lastPixel = nextPixel;

    if (param.state == AnimationState_Completed)
    {
        Serial.println("Ascend Animation done........................................");
        // reverse direction of movement
        moveDir *= -1;

        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);
    }
}

void SetupAnimations()
{

    // take several seconds to move eye fron one side to the other
    animations.StartAnimation(0, 25000, AscendAnimUpdate);

    // fade all pixels providing a tail that is longer the faster
    // the pixel moves.
    animations.StartAnimation(1, 25000, DescendAnimUpdate);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ; // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    strip.Begin();
    strip.Show();

    SetupAnimations();
}

void loop()
{
    // this is all that is needed to keep it running
    // and avoiding using delay() is always a good thing for
    // any timing related routines
    animations.UpdateAnimations();
    strip.Show();
}