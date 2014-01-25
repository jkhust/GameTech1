#include "Util.h"
#include <stdlib.h>

// -----------------------------------------------------------
int random_int(int min, int max) {
  return rand() % (max - min) + min;

}

// -----------------------------------------------------------
float random_float(float min, float max) {
  return(min + (float)rand() / ((float)RAND_MAX / ( max - min )));
  
}

// -----------------------------------------------------------
// reflection vector
// 2(NdotL)*N - L
Ogre::Vector3 vec3_reflect(const Ogre::Vector3 &v1, const Ogre::Vector3 &norm) {
	return (v1.reflect(norm) );

	/*
	Ogre::Real dot = v1.dotProduct(norm);
	if(dot < 0)
		dot = 0.0f;

	Ogre::Vector3 vReflect = (norm * 2.0f * dot);
	vReflect -= v1;

	vReflect.normalise();

	return vReflect;
  */

}