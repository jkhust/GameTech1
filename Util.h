//
//  Util.h
//  Tech1
//
//  Created by Justin Hust on 12/15/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef GameTech1_Util_h
#define GameTech1__Util_h

#include "OgreVector3.h"

int random_int(int min, int max);
float random_float(float min, float max);
Ogre::Vector3 vec3_reflect(const Ogre::Vector3 &v1, const Ogre::Vector3 &norm);

#endif
