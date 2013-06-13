/** @file game.cpp
 * @brief Implementace tridy Game
 * @see Game
 */
#include <cassert>
#include <Box2D.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <typeinfo>
#include "config.h"
#include "game.hpp"
#include "objects.hpp"
#include "json/parser.hpp"
#include "json/exceptions.hpp"

float Game::GameOverTime = 2.0;
float Game::SuccesTime = 2.0;
float Game::ChargingTime = 1.0;
float Game::ExplosionForce = 200.0;
float Game::ExplosionLimit = 0.5;

bool Game::setupGL()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0f);
	glPointSize(2.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glViewport(0, 0, mScreen->w, mScreen->h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* obrazovka se nastavi tak aby souhlasila s mCamera */
	glOrtho(mCamera.lowerBound.x, mCamera.upperBound.x,
					mCamera.lowerBound.y, mCamera.upperBound.y,
					-1, 1);
	glMatrixMode(GL_MODELVIEW);

	return true;
}

void Game::restoreGL()
{
	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

b2Vec2 Game::windowToWorld(int x, int y)
{
	const float xscale = (mCamera.upperBound.x - mCamera.lowerBound.x) / mScreen->w;
	const float yscale = (mCamera.upperBound.y - mCamera.lowerBound.y) / mScreen->h;
	b2Vec2 pos;
	pos.x = x*xscale + mCamera.lowerBound.x;
	pos.y = (mScreen->h - y)*yscale + mCamera.lowerBound.y;
	return pos;
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* kresleni teles ve svete */
	b2Body *body;
	for(body = mWorld->GetBodyList(); body != NULL; body = body->GetNext()) {
		GameObject *obj = static_cast<GameObject*>(body->GetUserData());
		if(obj)
			obj->draw();
	}

	/* kresleni pripadne zpravy */
	SDL_Surface *msg;
	if(mLost)
		msg = mGameOver;
	else if(mChecking)
		msg = mCheckingVictory;
	else if(mWin)
		msg = mSuccess;
	else
		msg = NULL;

	GLenum format;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	format = GL_RGBA;
#else
	format = GL_BGRA;
#endif
	glPixelZoom(1.0, -1.0); // surface je z pohledu OpenGL naopak

	if(msg) {
		glRasterPos2f(-(msg->w / 2)*mPixelToMeter, 3.0);
		/* TODO: ted to vsechno zavisi na formatu surface. neslo by to udelat nejak
		 * lepe? */
		glDrawPixels(msg->w, msg->h, format, GL_UNSIGNED_BYTE, msg->pixels);
	}

	/* kresleni jmena mapy */
	glRasterPos2f(-(mRenderedMapName->w / 2)*mPixelToMeter, 0.0);
	glDrawPixels(mRenderedMapName->w, mRenderedMapName->h, format, GL_UNSIGNED_BYTE, mRenderedMapName->pixels);

	/* kresleni poctu zbyvajicich kosticek */
	if(mToDestroy > 0) {
		SDL_Surface *toDestroy;
		SDL_Color c;
		c.r = 29; c.g = 77; c.b = 26;
		std::stringstream s;

		s << "Destroy: " << mToDestroy;
		toDestroy = TTF_RenderUTF8_Blended(mLittleFont, s.str().c_str(), c);
		glRasterPos2f(-(toDestroy->w / 2)*mPixelToMeter, -0.5);
		glDrawPixels(toDestroy->w, toDestroy->h, format, GL_UNSIGNED_BYTE, toDestroy->pixels);

		SDL_FreeSurface(toDestroy);
	}

	glFlush();
	SDL_GL_SwapBuffers();
}

void Game::processEvents()
{
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				mRunning = false;
				break;
			case SDL_KEYDOWN:
				keyPressed(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				keyReleased(event.key.keysym.sym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseClicked(event.button.button, event.button.x, event.button.y);
				break;
			default:
				break;
		}
	}
}

void Game::keyPressed(SDLKey key)
{
	switch(key) {
		case SDLK_PAUSE:
		case SDLK_p:
			mPaused = !mPaused;
			break;
		case SDLK_ESCAPE:
		case SDLK_q:
			mRunning = false;
			break;
		case SDLK_r:
			mWin = false;
			mLost = false;
			mChecking = false;
			mRunning = true;
			mLostTime = 0.0;
			mWinTime = 0.0;

			deleteMap();
			loadMap();
			break;
		default:
			break;
	}
}

void Game::keyReleased(SDLKey key)
{
	switch(key) {
		default:
			break;
	}
}

void Game::mouseClicked(Uint8 button, int x, int y)
{
	b2Vec2 pos = windowToWorld(x, y);

	/* kostky se nici levym tlacitkem a jen kdyz se nenabiji */
	if(button == SDL_BUTTON_LEFT and !mCharging) {
		/* tento kousek kodu je prevzat z prikladu k Box2D (TestBed) */
		/* hledani prvniho telesa na pozici pos */
		b2AABB aabb;
		b2Vec2 a;
		a.Set(b2_linearSlop, b2_linearSlop);
		aabb.lowerBound = pos - a;
		aabb.upperBound = pos + a;

		b2Body *body = NULL;
		const size_t maxCount = 10;
		b2Shape *shapes[maxCount];
		int32 count = mWorld->Query(aabb, shapes, maxCount);
		for (int32 i = 0; i < count; ++i) {
			b2Body *shapeBody = shapes[i]->GetBody();
			if(!shapeBody->IsStatic()) {
				if(shapes[i]->TestPoint(shapeBody->GetXForm(), pos)) {
					body = shapes[i]->GetBody();
					break;
				}
			}
		}

		if(body) {
			/* nici se jen kostky a jen ty u kterych muzeme */
			GameObject *object = static_cast<GameObject*>(body->GetUserData());
			Brick *brick = dynamic_cast<Brick*>(object);
			if(brick) {
				if(brick->canDestroy()) {
					destroyBrick(brick);
					mCharging = true;
				}
			}
		}
	}
}

void Game::loadMap()
{
	/* inicializace sveta */
	b2AABB worldAABB;
	worldAABB.lowerBound.Set(-100.0, -100.0);
	worldAABB.upperBound.Set(100.0, 100.0);
	b2Vec2 gravity(0.0f, -10.0f);
	mWorld = new b2World(worldAABB, gravity, true);
	mContactListener = new ContactListener(this);
	mWorld->SetContactListener(mContactListener);

	/* podlozka */
	new Ground(mWorld, b2Vec2(0.0, -2.5), 80.0, 5.0);

	/* nacteni dat z JSON */
	std::ifstream mapFile(mMapFile.c_str());
	if(!mapFile) {
		std::stringstream s;
		s << "Unable to open file " << mMapFile;
		throw std::runtime_error(s.str());
	}

	json::Value root = mParser.parse(mapFile);
	mMapName = root["name"].str();
	mToDestroy = root["destroy"].num();

	/* kostky */
	json::Array bricks = root["bricks"].ary();
	json::Array::const_iterator brick;
	/* pro kazdou kosticku v poli kosticek */
	for(brick = bricks.begin(); brick != bricks.end(); brick++) {
		json::Array b = brick->ary();

		/* pozice a rozmery kosticky */
		float x = b[1].num();
		float y = b[2].num();
		float w = b[3].num();
		float h = b[4].num();

		/* typ kosticky (pismenko) */
		json::String typeId = b[0].str();
		Brick::Type type;
		if(typeId == "n")
			type = Brick::Normal;
		else if(typeId == "d")
			type = Brick::Dark;
		else if(typeId == "c")
			type = Brick::Combo;
		else if(typeId == "s")
			type = Brick::Slippy;
		else if(typeId == "g")
			type = Brick::Gummy;
		else if(typeId == "x")
			type = Brick::TNT;
		else {
			std::stringstream s;
			s << "Bad brick type '" << typeId << "'";
			throw json::DataError(s.str());
		}

		new Brick(mWorld, b2Vec2(x, y), w, h, type);
	}

	/* buzci */
	json::Array idols = root["idols"].ary();
	json::Array::const_iterator idol;
	for(idol = idols.begin(); idol != idols.end(); idol++) {
		/* prvni prvek pole s buzkem je pozice */
		b2Vec2 pos;
		pos.x = (*idol)[0][0].num();
		pos.y = (*idol)[0][1].num();

		/* ostatni jsou vertexy */
		std::vector<b2Vec2> vertices;
		json::Array::size_type i;
		for(i = 1; i != idol->ary().size(); i++) {
			b2Vec2 vertex;
			vertex.x = (*idol)[i][0].num();
			vertex.y = (*idol)[i][1].num();
			vertices.push_back(vertex);
		}

		mIdols.push_back(new Idol(mWorld, pos, vertices));
	}
}

void Game::deleteMap()
{
	/* smazani vsech objektu */
	b2Body *body;
	b2Body *next;
	for(body = mWorld->GetBodyList(); body != NULL; body = next) {
		next = body->GetNext();
		delete static_cast<GameObject*>(body->GetUserData());
	}

	delete mWorld;
	delete mContactListener;
	mIdols.clear();

	mWorld = NULL;
	mContactListener = NULL;
}

void Game::deleteInvisible()
{
	static int maxShapes = 64;
	b2Shape *shapes[maxShapes];

	int shapeCount = mWorld->Query(mCamera, shapes, maxShapes);

	/* s velikou pravdepodobnosti se do pole nevesly vsechny tvary */
	if(shapeCount == maxShapes) {
		maxShapes *= 2; // proto se musi predpokladat ze jich je vic 
		deleteInvisible();
	} else {
		/* oznaceni vsech viditelnych */
		register int i;
		for(i=0; i!=shapeCount; i++) {
			GameObject *object = static_cast<GameObject*>(shapes[i]->GetBody()->GetUserData());
			object->isVisible = true;
		}

		/* smazani neviditelnych */
		b2Body *body;
		for(body = mWorld->GetBodyList(); body != NULL; ) {
			GameObject *object = static_cast<GameObject*>(body->GetUserData());
			body = body->GetNext(); // pozdeji by uz teleso mohlo byt smazane

			if(!object) {
				continue; // preskocime neznama telesa
			}

			if(!object->isVisible) {
				Brick *brick = dynamic_cast<Brick*>(object);
				if(brick) {
					/* znici se i kosticka kterou uzivatel znicit nesmi, ale nebude se
					 * pocitat (vyjimka je kombo, to na canDestroy() vraci false ale
					 * budeme ho nicit) */
					if(brick->canDestroy() or brick->type() == Brick::Combo) {
						destroyBrick(brick);
						continue;
					}
				} else if(dynamic_cast<Idol*>(object)) {
					/* buzek smazan - prohra */
					mLost = true;
				}

				delete object;
			} else {
				object->isVisible = false; // kvuli dalsi kontrole
			}
		}
	}
}

void Game::destroyBrick(Brick *brick)
{
	b2Vec2 pos = brick->body()->GetWorldCenter();
	float bombMass = brick->body()->GetMass();
	Brick::Type brickType = brick->type();

	delete brick;
	if(--mToDestroy <= 0) {
		mChecking = true;
	}

	if(brickType == Brick::TNT) {
		makeExplosion(pos, bombMass);
	}
}

void Game::makeExplosion(const b2Vec2 &position, float bombMass)
{
	const float size = ExplosionForce * bombMass;

	b2Body *body;
	for(body = mWorld->GetBodyList(); body != NULL; body = body->GetNext() ) {
		b2Vec2 force = body->GetWorldCenter() - position;
		float len = force.Length();
		force.Normalize();
		force *= size / len;

		body->ApplyForce(force, body->GetWorldCenter());
	}
}

Game::Game(SDL_Surface *screen, std::string dataDir, std::string font, std::string map):
	mScreen(screen),
	mLittleFont(NULL),
	mBigFont(NULL),
	mGameOver(NULL),
	mCheckingVictory(NULL),
	mSuccess(NULL),

	mWorld(NULL),
	mContactListener(NULL),
	mStepTime(1.0/60.0),
	mIterations(15),

	mLost(false),
	mChecking(false),
	mWin(false),
	mPaused(false),
	mRunning(false),
	mCharging(false),
	mLostTime(0.0),
	mWinTime(0.0),
	mChargingTime(0.0),

	mDataDir(dataDir),
	mMapFile(map),
	mToDestroy(0),

	mNormalCursor(NULL),
	mChargingCursor(NULL)
{
	/* nacitani fontu */
	mLittleFont = TTF_OpenFont(font.c_str(), 15);
	if(mLittleFont == NULL) {
		std::stringstream s;
		s << "Unable to open font " << font << ": " << TTF_GetError() << std::endl;
		throw std::runtime_error(s.str());
	}

	mBigFont = TTF_OpenFont(font.c_str(), 30);
	if(mBigFont == NULL) {
		std::stringstream s;
		s << "Unable to open font " << font << ": " << TTF_GetError() << std::endl;
		throw std::runtime_error(s.str());
	}

	try {
		loadMap();
	} catch(json::Exception e) {
		/* chyba v jsonu se jen vyhodi vys */
		std::stringstream s;
		s << "Error in file " << map << ": " << e.what() << std::endl;
		throw std::runtime_error(s.str());
	}

	/* vykreslovani hlasek */
	SDL_Color c;
	c.r = 255; c.g = 16; c.b = 16;
	mGameOver = TTF_RenderUTF8_Blended(mBigFont, "Game over", c);
	c.r = 68; c.g = 157; c.b = 50;
	mSuccess = TTF_RenderUTF8_Blended(mBigFont, "Success!", c);
	c.r = 128; c.g = 128; c.b = 128;
	mCheckingVictory = TTF_RenderUTF8_Blended(mBigFont, "Checking victory...", c);

	c.r = 29; c.g = 77; c.b = 26;
	mRenderedMapName = TTF_RenderUTF8_Blended(mLittleFont, mMapName.c_str(), c);

	/* nastaveni kamery */
	mCamera.lowerBound.Set(-16.0, -2.0);
	mCamera.upperBound.Set(16.0, 20.0);

	mPixelToMeter = (mCamera.upperBound.x - mCamera.lowerBound.x) / mScreen->w;

	/* kurzory */
	mNormalCursor = SDL_GetCursor(); // normalni kurzor je vychozi

	/* kolecko */
	Uint8 data[32] = {
		0x00, 0x00,
		0x03, 0xc0,
		0x0f, 0xf0,
		0x1f, 0xf8,
		0x3f, 0xfc,
		0x3f, 0xfc,
		0x7f, 0xfe,
		0x7f, 0xfe,

		0x7f, 0xfe,
		0x7f, 0xfe,
		0x3f, 0xfc,
		0x3f, 0xfc,
		0x1f, 0xf8,
		0x0f, 0xf0,
		0x03, 0xc0,
		0x00, 0x00
	};

	Uint8 mask[32] = {
		0x03, 0xc0,
		0x0f, 0xf0,
		0x1f, 0xf8,
		0x3f, 0xfc,
		0x7f, 0xfe,
		0x7f, 0xfe,
		0xff, 0xff,
		0xff, 0xff,

		0xff, 0xff,
		0xff, 0xff,
		0x7f, 0xfe,
		0x7f, 0xfe,
		0x3f, 0xfc,
		0x1f, 0xf8,
		0x0f, 0xf0,
		0x03, 0xc0
	};

	mChargingCursor = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
}

Game::~Game()
{
	SDL_SetCursor(mNormalCursor);

	SDL_FreeSurface(mGameOver);
	SDL_FreeSurface(mCheckingVictory);
	SDL_FreeSurface(mSuccess);
	TTF_CloseFont(mLittleFont);
	TTF_CloseFont(mBigFont);
	SDL_FreeCursor(mChargingCursor);

	glLineWidth(1.0f);
}

void Game::run()
{
	Uint32 ntime = SDL_GetTicks(); // cas dalsiho ramce v ms
	mRunning = true;

	setupGL();

	while(mRunning) {
		while(SDL_GetTicks() < ntime)
			SDL_Delay(1);
		ntime += mStepTime * 1000;

		if(mLost) {
			mLostTime += mStepTime;
			if(mLostTime > GameOverTime) 
				break;
		} else if(mChecking) {
			/* pokud jsou vsichni buzci v klidu, skonci se kontrola */
			std::vector<Idol*>::const_iterator id;
			mChecking = false;
			for(id = mIdols.begin(); id != mIdols.end(); id++) {
				if(!(*id)->body()->IsSleeping()) {
					mChecking = true;
					break;
				}
			}

			if(!mChecking) {
				mWin = true;
				mWinTime = 0.0;
			}
		} else if(mWin) {
			mWinTime += mStepTime;
			if(mWinTime > SuccesTime)
				break;
		}

		if(mCharging and not mPaused) {
			mChargingTime += mStepTime;
			if(mChargingTime > ChargingTime) {
				mCharging = false;
				mChargingTime = 0.0;
			}
		}

		/* nastaveni odpovidajiciho kurzoru */
		SDL_Cursor *actual = SDL_GetCursor();
		if(mCharging and actual != mChargingCursor) {
			SDL_SetCursor(mChargingCursor);
		} else if(!mCharging and actual != mNormalCursor) {
			SDL_SetCursor(mNormalCursor);
		}

		processEvents();
		if(!mPaused) {
			mWorld->Step(mStepTime, mIterations);

			/* znici se kombo kostky ktere se znicit maji */
			if(mCombosToDestroy.size() > 0) {
				std::vector<Brick*>::iterator combo;
				for(combo = mCombosToDestroy.begin();
						combo != mCombosToDestroy.end();
						combo++) {
					delete *combo;
					if(--mToDestroy <= 0) 
						mChecking = true;
				}
				mCombosToDestroy.clear();
			}

			deleteInvisible();
		}
		draw();
	}

	restoreGL();
}

