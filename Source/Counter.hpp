//
//  Counter.hpp
//  Metro
//
//  Created by Ryan Stables on 19/10/2017.
//
//

#ifndef Counter_hpp
#define Counter_hpp

#include <stdio.h>

//==============================================================================
//========= Counter ============================================================
//==============================================================================
class Counter
{
public:
    Counter(){};
    ~Counter(){};
    // do some other stuff to the object...
    void iterate(){counter++;};
    void reset(){counter = 0;};
    void set(int x){counter = x;};
    
    //return the counter in different formats...
    int      inSamples(){return counter;};
    double   inFrames(int frameLen){return (double)counter/(double)frameLen;};
    double   inSeconds(double fs){return (double)counter/fs;};
    double   inMilliseconds(double fs){return inSeconds(fs)*1000.0;};
    
private:
    int counter=0;
};



#endif /* Counter_hpp */
