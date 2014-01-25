/*
-----------------------------------------------------------------------------
Filename:    GameTech1.h
-----------------------------------------------------------------------------
 
This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#ifndef __GameTech1_h_
#define __GameTech1_h_
 
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
 
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
 
#include <SdkTrays.h>
#include <SdkCameraMan.h>
 
class GameTech1 : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    GameTech1(void);
    virtual ~GameTech1(void);
    bool go(void);
protected:
	// DATA
		Ogre::Root *mRoot;
		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneMgr;
		Ogre::RenderWindow* mWindow;
		Ogre::String mResourcesCfg;
		Ogre::String mPluginsCfg;
 
		// OgreBites
		OgreBites::SdkTrayManager* mTrayMgr;
		OgreBites::SdkCameraMan* mCameraMan;      // basic camera controller
		OgreBites::ParamsPanel* mDetailsPanel;    // sample details panel
		bool mCursorWasVisible;                   // was cursor visible before dialog appeared
		bool mShutDown;
 
		// OIS Input devices
		OIS::InputManager* mInputManager;
		OIS::Mouse*    mMouse;
		OIS::Keyboard* mKeyboard;

	// game-specific data
		Ogre::Entity * _entSphere;
		Ogre::Vector3 _vPos;
		Ogre::Vector3 _vVel;
		Ogre::Real _fSpeed;
		Ogre::Real _fWorldSize;

	// MANIPULATORS
		void createScene(void);
		void updateScene(Ogre::Real fLastFrameTime);

	// OVERRIDES
		// Ogre::FrameListener
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
 
		// OIS::KeyListener
		virtual bool keyPressed( const OIS::KeyEvent &arg );
		virtual bool keyReleased( const OIS::KeyEvent &arg );
		// OIS::MouseListener
		virtual bool mouseMoved( const OIS::MouseEvent &arg );
		virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
		virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
 
		// Ogre::WindowEventListener
		virtual void windowResized(Ogre::RenderWindow* rw);
		virtual void windowClosed(Ogre::RenderWindow* rw);
};
 
#endif // #ifndef __GameTech1_h_