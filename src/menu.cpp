#include <algorithm>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <guichan/opengl.hpp>
#include <guichan/opengl/openglsdlimageloader.hpp>
#include <SDL.h>
#include <sys/types.h>
#include <dirent.h>
//#include <errno.h>
#include <cerrno>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include "config.h"
#include "game.hpp"
#include "objects.hpp"
#include "menu.hpp"

Menu::LevelListModel::LevelListModel(const std::string &dir)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp = opendir(dir.c_str())) == NULL) {
		std::stringstream s;
		s << "Error(" << errno << ") opening " << dir << std::endl;
		throw std::runtime_error(s.str());
	}

	while ((dirp = readdir(dp)) != NULL) {
		std::string file = dirp->d_name;
		if(file[0] != '.') // preskoci se skryte soubory
			mLevels.push_back(file);
	}

	std::sort(mLevels.begin(), mLevels.end());

	closedir(dp);
}

void Menu::action(const gcn::ActionEvent &event)
{
	if(event.getId() == "play") {
		runGame();
	} else if(event.getId() == "exit") {
		mRunning = false;
	} else if(event.getId() == "openHelp") {
		mHelpWin->setVisible(true);
	} else if(event.getId() == "closeHelp") {
		mHelpWin->setVisible(false);
	}
}

void Menu::buildGui()
{
	gcn::Color buttonColor(157, 188, 229);

	/* hlavni kontejner */
	mTop = new gcn::Container();
	mTop->setDimension(gcn::Rectangle(0, 0, mScreenWidth, mScreenHeight));
	mTop->setBaseColor(gcn::Color(193, 209, 229));

	/* tlacitko hrat */
	mPlay = new gcn::Button("Play");
	mPlay->addActionListener(this);
	mPlay->setActionEventId("play");
	mPlay->setWidth(64);
	mPlay->setHeight(25);
	mPlay->setBaseColor(buttonColor);
	mTop->add(mPlay, 32, 32);

	/* tlacitko help */
	mHelp = new gcn::Button("Help");
	mHelp->addActionListener(this);
	mHelp->setActionEventId("openHelp");
	mHelp->setWidth(64);
	mHelp->setHeight(25);
	mHelp->setBaseColor(buttonColor);
	mTop->add(mHelp, 32, 72);

	/* tlacitko ukoncit */
	mExit = new gcn::Button("Exit");
	mExit->addActionListener(this);
	mExit->setActionEventId("exit");
	mExit->setWidth(64);
	mExit->setHeight(25);
	mExit->setBaseColor(buttonColor);
	mTop->add(mExit, 32, 112);

	/* popisek "Select level" */
	mSelect = new gcn::Label("Select level:");
	mSelect->setHeight(20);
	mTop->add(mSelect, 128, 32);

	/* vyber levelu */
	mLevelListModel = new LevelListModel(mLevelsDir);
	mLevelList = new gcn::ListBox(mLevelListModel);
	mLevelList->setWidth(256);
	mLevelList->setBackgroundColor(gcn::Color(157, 188, 229));
	mTop->add(mLevelList, 128, 54);

	/* okno s instrukcemi jak hrat */
	mHelpWin = new gcn::Window("Instructions");
	mHelpWin->setWidth(600);
	mHelpWin->setHeight(300);
	mHelpWin->setVisible(false);
	mHelpWin->setBaseColor(gcn::Color(125, 180, 255, 240));
	mTop->add(mHelpWin, 32, 32);

	/* text s instrukcemi */
	mHelpText = new gcn::TextBox();
	mHelpText->setEditable(false);
	mHelpText->setText(
	"Your goal is to retrieve the golden idol(s) from every \n"
	"totem. Idol can't touch the ground, otherwise they will break"
	);
	mHelpText->setOpaque(false);
	mHelpWin->add(mHelpText, 10, 10);

	/* popisek "Kinds of blocks" */
	mHelpKinds = new gcn::Label("Kinds of blocks");
	mHelpWin->add(mHelpKinds, 10, 50);

	int x = 10;
	int y = 70;
	const int padding = 5;
	int b;

	for(b = 0; b != Brick::BricksCount; b++) {
		gcn::Container *rect;
		rect = new gcn::Container();
		rect->setWidth(16);
		rect->setHeight(16);
		rect->setBaseColor(Brick::colors[b]);
		mHelpWin->add(rect, x, y);
		mHelpBricks[b] = rect;

		gcn::Label *label;
		label = new gcn::Label(Brick::labels[b]);
		mHelpWin->add(label, x + 20, y);
		mHelpBrickLabels[b] = label;

		y += 16 + padding;
	}

	/* text s popisem klaves */
	mHelpKeys = new gcn::TextBox();
	mHelpKeys->setEditable(false);
	mHelpKeys->setText(
	"Press 'r' to reset level, 'q' or 'esc' to quit level\n"
	"and 'p' to pause"
	);
	mHelpKeys->setOpaque(false);
	mHelpWin->add(mHelpKeys, x, y);

	/* tlacitko zavrit instrukce */
	mHelpClose = new gcn::Button("Close instructions");
	mHelpClose->addActionListener(this);
	mHelpClose->setActionEventId("closeHelp");
	mHelpClose->setHeight(25);
	mHelpClose->setBaseColor(buttonColor);
	mHelpWin->add(mHelpClose, 32, 250);
}

void Menu::runGame()
{
	if(mLevelList->getSelected() >= 0) {
		Game game(mScreen, mDataDir, mDataDir + "/DejaVuSans.ttf", 
							mLevelsDir + "/" + mLevelListModel->getElementAt(mLevelList->getSelected()));
		game.run();
	}
}

Menu::Menu():
	mScreen(NULL),
	mInput(NULL),
	mGraphics(NULL),
	mFont(NULL),
	mScreenWidth(800),
	mScreenHeight(600),

	mRunning(false),
	mFrametime(1000/30),

	mTop(NULL),
	mPlay(NULL),
	mLevelListModel(NULL),
	mLevelList(NULL),

	mHelpWin(NULL),
	mHelpText(NULL),
	mHelpKinds(NULL),
	mHelpKeys(NULL),
	mHelpClose(NULL)
{
	/* nastaveni poli mHelpBricks a mHelpBrickLabels na NULLy */
	register int i;
	for(i=0; i!=Brick::BricksCount; i++) {
		mHelpBricks[i] = NULL;
		mHelpBrickLabels[i] = NULL;
	}

	/* ziskani slozky s daty */
	if(std::getenv("TOTEM_DESTROYER_DATADIR"))
		mDataDir = std::getenv("TOTEM_DESTROYER_DATADIR");
	else
		mDataDir = INSTALL_DATADIR;

	/* a mapy */
	if(std::getenv("TOTEM_DESTROYER_MAPS"))
		mLevelsDir = std::getenv("TOTEM_DESTROYER_MAPS");
	else
		mLevelsDir = mDataDir + "/maps";

	/* nacitac obrazku */
	mImageLoader = new gcn::OpenGLSDLImageLoader();
	gcn::Image::setImageLoader(mImageLoader);

	/* inicializace sdl */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
		throw std::runtime_error(std::string("Cannot initialize SDL: ") + SDL_GetError());

	if(TTF_Init() < 0) 
		throw std::runtime_error(std::string("Cannot initialize SDL_ttf: ") + TTF_GetError());

	/* okno */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	mScreen = SDL_SetVideoMode(mScreenWidth, mScreenHeight, 0, SDL_OPENGL | SDL_HWSURFACE | SDL_HWACCEL);
	if(mScreen == NULL) 
		throw std::runtime_error(std::string("Unable to set video mode: ") + SDL_GetError());
	SDL_WM_SetCaption(PACKAGE_STRING, NULL);

	mGraphics = new gcn::OpenGLGraphics(mScreenWidth, mScreenHeight);
	setGraphics(mGraphics);

	mFont = new gcn::ImageFont(mDataDir + "/font.png", " abcdefghijklmnopqrstuvwxyz" \
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"");
	gcn::Widget::setGlobalFont(mFont);

	mInput = new gcn::SDLInput();
	setInput(mInput);

	buildGui();
	setTop(mTop);
}

Menu::~Menu()
{
	SDL_FreeSurface(mScreen);
	delete mInput;
	delete mGraphics;
	delete mFont;
	delete mImageLoader;

	delete mTop;
	delete mPlay;
	delete mHelp;
	delete mExit;
	delete mLevelListModel;
	delete mLevelList;

	delete mHelpWin;
	delete mHelpText;
	delete mHelpKinds;
	delete mHelpKeys;
	delete mHelpClose;

	/* pokud je vytvorene okynko s instrukcemi, je mozno predpokladat ze jsou
	 * inicializovany i kosticky */
	if(mHelpWin) {
		/* mazani obsahu mHelpBricks a mHelpBrickLabels */
		register int i; ///< niceni by nemelo ubirat pamet :-)
		for(i=0; i!=Brick::BricksCount; i++) {
			delete mHelpBricks[i];
			delete mHelpBrickLabels[i];
		}
	}

	SDL_Quit();
	TTF_Quit();
}

void Menu::run()
{
	mRunning = true;

	Uint32 nextTime = SDL_GetTicks();
	while(mRunning) {
		while(SDL_GetTicks() < nextTime)
			SDL_Delay(1);
		nextTime += mFrametime;

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT)
				mRunning = false;
			else
				mInput->pushInput(event);
		}

		draw();
		logic();
		SDL_GL_SwapBuffers();
	}
}
