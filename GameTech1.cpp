/*
-----------------------------------------------------------------------------
Filename:    GameTech1.cpp
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
#include "GameTech1.h"
#include "Util.h"
 
//-------------------------------------------------------------------------------------
GameTech1::GameTech1(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0)
{
}
//-------------------------------------------------------------------------------------
GameTech1::~GameTech1(void)
{
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
 
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}
 
bool GameTech1::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif
 
    // construct Ogre::Root
    mRoot = new Ogre::Root(mPluginsCfg);
 
//-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
 
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
//-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->restoreConfig() || mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "GameTech1 Render Window");
    }
    else
    {
        return false;
    }
//-------------------------------------------------------------------------------------
    // choose scenemanager
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
//-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
 
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);
 
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
//-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0.2f,0.2f,0.2f));
 
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
//-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

//-------------------------------------------------------------------------------------
    // Create any resource listeners (for loading screens)
    //createResourceListener();
//-------------------------------------------------------------------------------------
    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
//-------------------------------------------------------------------------------------
    // Create the scene
	this->createScene();

//-------------------------------------------------------------------------------------
    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
    mInputManager = OIS::InputManager::createInputSystem( pl );
 
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
 
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
 
    //Set initial mouse clipping size
    windowResized(mWindow);
 
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
 
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();
 
    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");
 
    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();
 
    mRoot->addFrameListener(this);
//-------------------------------------------------------------------------------------
    mRoot->startRendering();
 
    return true;
}

// ***********************************************************************************
void GameTech1::createScene(void) {
	// create player - a sphere with radius 10.
	_playerEnt = NULL;
	_playerEnt = mSceneMgr->createEntity("Player", "sphere.mesh");
	assert(_playerEnt);
	_playerEnt->setMaterialName("Examples/SphereMappedRustySteel");
	_playerEnt->setCastShadows(true);

	// attach player
	Ogre::SceneNode* playerNode = NULL; 
	playerNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerNode");
	// default sphere has radius 100, so scale it down to radius 5
	_fRadius = 3.0f;
	playerNode->scale( _fRadius / 100.0f, _fRadius / 100.0f, _fRadius / 100.0f );
	assert(playerNode);
    playerNode->attachObject(_playerEnt);


	// start at center with random velocity vector.
	_vPos = Ogre::Vector3(0.0f, 0.0f, 0.0f);
	_vVel = Ogre::Vector3( random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f) );
	_vVel.normalise();
	_fSpeed = random_float(80.0f, 100.0f);

	// ***** BUILD WORLD GEOMETRY: floor, ceiling, walls *******

	float fWorldSize = 100.0f;

	// floor
		Ogre::Plane floorPlane = Ogre::Plane(Ogre::Vector3::UNIT_Y, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("floorPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		floorPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

		Ogre::Entity* floorEnt = mSceneMgr->createEntity("floor", "floorPlane");
		floorEnt->setMaterialName("Examples/GrassFloor");
		floorEnt->setCastShadows(false);

	// ceiling
		Ogre::Plane ceilingPlane = Ogre::Plane( Ogre::Vector3::NEGATIVE_UNIT_Y, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("ceilingPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		ceilingPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);

		Ogre::Entity* ceilingEnt = mSceneMgr->createEntity("ceiling", "ceilingPlane");
		ceilingEnt->setMaterialName("WoodPallet");
		ceilingEnt->setCastShadows(false);

	// left wall
		Ogre::Plane lwallPlane = Ogre::Plane( Ogre::Vector3::UNIT_X, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("lwallPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		lwallPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);

		Ogre::Entity* lwallEnt = mSceneMgr->createEntity("lwallEnt", "lwallPlane");
		lwallEnt->setMaterialName("Examples/Rockwall");
		lwallEnt->setCastShadows(false);

	// right wall
		Ogre::Plane rwallPlane = Ogre::Plane( Ogre::Vector3::NEGATIVE_UNIT_X, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("rwallPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		rwallPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);

		Ogre::Entity* rwallEnt = mSceneMgr->createEntity("rwallEnt", "rwallPlane");
		rwallEnt->setMaterialName("Examples/Rockwall");
		rwallEnt->setCastShadows(false);

	// far wall
		Ogre::Plane fwallPlane = Ogre::Plane( Ogre::Vector3::UNIT_Z, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("fwallPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		fwallPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);

		Ogre::Entity* fwallEnt = mSceneMgr->createEntity("fwallEnt", "fwallPlane");
		fwallEnt->setMaterialName("Examples/Rockwall");
		fwallEnt->setCastShadows(false);

	// near wall
		Ogre::Plane nwallPlane = Ogre::Plane( Ogre::Vector3::NEGATIVE_UNIT_Z, fWorldSize / -2.0f);
		Ogre::MeshManager::getSingleton().createPlane("nwallPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		nwallPlane, fWorldSize, fWorldSize, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);

		Ogre::Entity* nwallEnt = mSceneMgr->createEntity("nwallEnt", "nwallPlane");
		nwallEnt->setMaterialName("Examples/Rockwall");
		nwallEnt->setCastShadows(false);

	// ATTACH ALL TO WORLDs
		Ogre::SceneNode * worldNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("WorldNode");
		worldNode->attachObject(floorEnt);
		worldNode->attachObject(ceilingEnt);
		worldNode->attachObject(lwallEnt);
		worldNode->attachObject(rwallEnt);
		worldNode->attachObject(fwallEnt);
		worldNode->attachObject(nwallEnt);

	// Keep record of all planes added.
		_walls[Wall_Floor] = floorPlane;
		_walls[Wall_Ceiling] = ceilingPlane;
		_walls[Wall_Left] = lwallPlane;
		_walls[Wall_Right] = rwallPlane;
		_walls[Wall_Far] = fwallPlane;
		_walls[Wall_Near] = nwallPlane;


	// **** lighting ****
    // ambient
	  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.01f, 0.01f, 0.01f));
	  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
 
    // diffuse point lights - one green, one blue
		Ogre::Light* l1 = mSceneMgr->createLight("PL0");
		l1->setType(Ogre::Light::LT_POINT);
		l1->setAttenuation(325.0f, 1.0f, 0.014f, 0.0007f);
		l1->setDiffuseColour( 1.0f, 1.0f, 1.0f );
		l1->setSpecularColour( 1.0f, 1.0f, 1.0f );
		l1->setPosition(30.0f,30.0f,-30.0f);
		
		Ogre::Light* l2 = mSceneMgr->createLight("PL1");
		l2->setType(Ogre::Light::LT_POINT);
		l2->setAttenuation(325.0f, 1.0f, 0.014f, 0.0007f);
		l2->setDiffuseColour( 0.0f, 0.0f, 0.8f );
		l2->setSpecularColour( 0.0f, 0.0f, 0.8f );
		l2->setPosition(-30.0f,-30.0f, 30.0f);
	

	// *** build items that can be collided with ***
	for(int i=0; i < 25; i++) {
		char name[80];
		sprintf(name, "Item%d", i);
		_items[i] = mSceneMgr->createEntity(name, "Barrel.mesh");
		_items[i]->setCastShadows(true);
		assert(_items[i] );

		// attack player
		Ogre::SceneNode* itemNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);
		itemNode->translate( random_float( -fWorldSize * 0.48f, fWorldSize * 0.48f),
							 random_float( -fWorldSize * 0.48f, fWorldSize * 0.48f),
							 random_float( -fWorldSize * 0.48f, fWorldSize * 0.48f));
		itemNode->scale(0.6f, 0.6f, 0.6f);
		itemNode->attachObject(_items[i]);

		_itemRotVel[i] = Ogre::Vector3(random_float(-60.0f, 60.0f),
			                           random_float(-60.0f, 60.0f), 
						               random_float(-60.0f, 60.0f));

	}

	// dynamic light for collisions
	_dynLight = mSceneMgr->createLight("DL0");
	_dynLight->setType(Ogre::Light::LightTypes::LT_POINT);
	_dynLight->setAttenuation(100.0f, 0.7, 0.045, 0.0075);
	_dynLight->setDiffuseColour( Ogre::ColourValue(0.0f, 0.0f, 0.0f) );
	_dynLight->setPosition(30.0f,30.0f,-30.0f);
	_dynLight->setVisible(false);

	// camera doesn't auto-track player by default
	_lastKeyT = false;
	_currKeyT = false;
	_bTrackPlayer = false;
	// move back a bit
	mCamera->setPosition( 0.0f, 75.0f, 150.0f);
	mCamera->lookAt(0.0f, 0.0f, 0.0f);
	
#ifdef SOUND_ENABLED
	_sndGrassHit = NULL;
	_sndGrassHit = Mix_LoadWAV("sounds/grass_hit.wav");
	assert(_sndGrassHit);

	_sndBrickHit = NULL;
	_sndBrickHit = Mix_LoadWAV("sounds/brick_hit.wav");
	assert(_sndBrickHit);

	_sndWoodHit = NULL;
	_sndWoodHit  = Mix_LoadWAV("sounds/wood_hit.wav");
	assert(_sndWoodHit);

	_sndExplode = NULL;
	_sndExplode  = Mix_LoadWAV("sounds/explode1.wav");
	assert(_sndExplode);

#endif

}

// ***********************************************************************************
void GameTech1::simulateScene(Ogre::Real fLastFrameTime) {
	// calculate slices based on diameter of ball compared against speed.
	// thus, the ball can never escape because we can never have bullet-through-paper.
	int iNumSlices = _fSpeed / 10.0f;

	// always have one slice at least.
	if(iNumSlices < 1)
		iNumSlices = 1;


	float fSliceTime = (float)(fLastFrameTime / ((float)iNumSlices));

	for(int i=0;i< iNumSlices; i++)
		GameTech1::simulateSlice(fSliceTime);

}

// ***********************************************************************************
void GameTech1::simulateSlice(Ogre::Real fSliceTime) {	
	assert(_playerEnt);

	// cal new pos by vel, speed, frametime.
	Ogre::Vector3 vNewPos = _vPos + (_vVel * _fSpeed * fSliceTime); 
	
	// make a temporary sphere for the player at new position, used for coldet.
	Ogre::Sphere playerBounds(vNewPos, 3.0f);

	// check intersections with visible items
	for(int j=0;j<25;j++) {
		if( _items[j]->isVisible() ) {
			Ogre::Sphere itemBounds(_items[j]->getParentSceneNode()->getPosition(), 2.0f);

			if( itemBounds.intersects(playerBounds)) {
				_vVel *= -1.0f;

				// add a little bit of randomness to the explosion
				if(_vVel.x < 0.0f)
					_vVel.x -= random_float(0.0f, 0.5f);
				else
					_vVel.x += random_float(0.0f, 0.5f);

				if(_vVel.y < 0.0f)
					_vVel.y -= random_float(0.0f, 0.5f);
				else
					_vVel.y += random_float(0.0f, 0.5f);

				if(_vVel.z < 0.0f)
					_vVel.z -= random_float(0.0f, 0.5f);
				else
					_vVel.z += random_float(0.0f, 0.5f);

				_vVel.normalise();

				_fSpeed *= 1.3f;
				_items[j]->setVisible(false);

				// red explosion light
				_dynLight->setPosition(_vPos);
				_dynLight->setDiffuseColour(1.0f, 0.0f, 0.0f);
				_dynLight->setSpecularColour(1.0f, 0.0f, 0.0f);
				_dynLight->setAttenuation(600.0f, 1.0f, 0.014f, 0.0007f);
				_dynLight->setVisible(true);
				_dynLightTime = 0.2f;

#ifdef SOUND_ENABLED
				Mix_PlayChannel(-1, _sndExplode, 0);
#endif

				return;

			}

		}

	}

	for(int i=0;i<Wall_Count; i++) {
		if( playerBounds.intersects( _walls[i] ) ) {
			Ogre::Vector3 vWallNormal = _walls[i].normal;
			_vVel = _vVel.reflect(vWallNormal);
			// slow down a bit.
			_fSpeed *= 0.98f;
			// TODO: play sound on reflect

			// move dyn light to player.
			_dynLight->setPosition(_vPos);
			_dynLight->setDiffuseColour(1.0f, 1.0f, 0.0f);
			_dynLight->setSpecularColour(1.0f, 1.0f, 0.0f);
			_dynLight->setAttenuation(200.0f, 0.7, 0.022, 0.0019);
			_dynLight->setVisible(true);
			_dynLightTime = 0.2f;

#ifdef SOUND_ENABLED
			if(i == Wall_Floor)
				Mix_PlayChannel(-1, _sndGrassHit, 0);
			else if(i == Wall_Ceiling)
				Mix_PlayChannel(-1, _sndWoodHit, 0);
			else
				Mix_PlayChannel(-1, _sndBrickHit, 0);

#endif
	
			return;

		}
	}


	// made it this far? move instead.
	_vPos = vNewPos;

	// move to whatever pos we generated.
	_playerEnt->getParentSceneNode()->setPosition(_vPos);

}

// ***********************************************************************************
void GameTech1::animateScene(float fTimeSinceLastFrame) {
	float degToRad = 3.14159265f / 180.0f;

	// spin barrels around randomly
	for(int j=0;j<25;j++) {
		if( _items[j]->isVisible() ) {
			Ogre::SceneNode * parent = _items[j]->getParentSceneNode();

			parent->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(_itemRotVel[j].x * degToRad * fTimeSinceLastFrame) );
			parent->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(_itemRotVel[j].x * degToRad * fTimeSinceLastFrame) );
			parent->rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(_itemRotVel[j].x * degToRad * fTimeSinceLastFrame) ) ;

		}

	}

	// show or hide the dynamic light
	_dynLightTime -= fTimeSinceLastFrame;
	_dynLight->setVisible(_dynLightTime > 0.0f);

}

// ***********************************************************************************
bool GameTech1::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;
 
    if(mShutDown)
        return false;
 
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();
 
    mTrayMgr->frameRenderingQueued(evt);
 
    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }

	// *** T key down? track player
	_lastKeyT = _currKeyT;
	_currKeyT = mKeyboard->isKeyDown(OIS::KeyCode::KC_T);
	if(!_lastKeyT && _currKeyT) {
		_bTrackPlayer = !_bTrackPlayer;

	}

	if(mKeyboard->isKeyDown(OIS::KeyCode::KC_P))
		_fSpeed *= 1.2f;

	if(_bTrackPlayer)
		mCamera->lookAt(_playerEnt->getParentSceneNode()->getPosition() );

	this->simulateScene(evt.timeSinceLastFrame);
	this->animateScene(evt.timeSinceLastFrame);

    return true;
}
//-------------------------------------------------------------------------------------
bool GameTech1::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up
 
    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;
 
        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }
 
        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;
 
        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }
 
        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
 
    mCameraMan->injectKeyDown(arg);
    return true;
}
 
bool GameTech1::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}
 
bool GameTech1::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
    return true;
}
 
bool GameTech1::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}
 
bool GameTech1::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}
 
//Adjust mouse clipping area
void GameTech1::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);
 
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
 
//Unattach OIS before window shutdown (very important under Linux)
void GameTech1::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );
 
            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
	
#ifdef SOUND_ENABLED
	Mix_FreeChunk(_sndGrassHit);
	Mix_FreeChunk(_sndBrickHit);
	Mix_FreeChunk(_sndWoodHit);
	Mix_FreeChunk(_sndExplode);

	Mix_CloseAudio();
#endif

	SDL_Quit();

}
 
 
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
		srand(time(NULL));

#ifdef SOUND_ENABLED
		/* Initialize SDL */
		if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
			fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		int audioInitResult = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
		assert(audioInitResult != -1);
#endif

        // Create application object
        GameTech1 app;
 
        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif
