#ifndef have_menu_hpp
#define have_menu_hpp
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <guichan/opengl.hpp>
#include <guichan/opengl/openglsdlimageloader.hpp>
#include <SDL.h>
#include <string>
#include <vector>
#include "objects.hpp"

/** Trida menu */
class Menu: protected gcn::Gui, public gcn::ActionListener {
	/** Model reprezentujici soubory ve slozce */
	class LevelListModel: public gcn::ListModel {
		std::vector<std::string> mLevels; ///< Jmena urovni
	public:
		/** Nacte seznam urovni.
		 * @param dir Slozka s urovnema
		 */
		LevelListModel(const std::string &dir);

		/** Ziska jmeno urovne.
		 * @param i Index urovne
		 * @return Jmeno souboru urovne
		 */
		std::string getElementAt(int i) { return mLevels[i]; }

		/** Vrati pocet urovni */
		int getNumberOfElements() { return mLevels.size(); }
	};

	SDL_Surface *mScreen; ///< Obrazovka (OpenGL)
	gcn::SDLInput *mInput; ///< Vstup pro Guichan
	gcn::OpenGLGraphics *mGraphics; ///< Grafika pro Guichan
	gcn::ImageFont *mFont; ///< Font
	gcn::OpenGLSDLImageLoader *mImageLoader; ///< Nacitac obrazku
	int mScreenWidth; ///< Sirka obrazovky
	int mScreenHeight; ///< Vyska obrazovky
	std::string mDataDir; ///< Adresar s daty
	std::string mLevelsDir; ///< Adresar s mapami

	bool mRunning; ///< Bezi menu?
	Uint32 mFrametime; ///< Cas na jeden ramec v ms

	gcn::Container *mTop; ///< Hlavni kontejner
	gcn::Button *mPlay; ///< Tlacitko hrat
	gcn::Button *mHelp; ///< Tlacitko ktere zobrazi okno s instrukcemi
	gcn::Button *mExit; ///< Tlacitko vychod
	gcn::Label *mSelect; ///< Popisek "Select level"
	LevelListModel *mLevelListModel; ///< Model pro vyber urovni
	gcn::ListBox *mLevelList; ///< Seznam levelu

	gcn::Window *mHelpWin; ///< Okno s instrukcemi jak hrat
	gcn::TextBox *mHelpText; ///< Text v okne
	gcn::Label *mHelpKinds; ///< Popisek "Kinds of blocks"
	gcn::TextBox *mHelpKeys; ///< Text s klavesami
	gcn::Container *mHelpBricks[Brick::BricksCount]; ///< Obdelniky v barvach kosticek
	gcn::Label *mHelpBrickLabels[Brick::BricksCount]; ///< Popisky k obdelnikum
	gcn::Button *mHelpClose; ///< Zavre okno s instrukcemi

	/** "Vybuduje" prvky GUI */
	void buildGui();

protected:
	/** Akce */
	void action(const gcn::ActionEvent &event);

	/** Rozbehne hru */
	void runGame();
public:
	/** Vytvori a inicializuje menu.  */
	Menu();

	/** Destruktor */
	virtual ~Menu();

	/** Hlavni smycka.
	 * Spusti hlavni smycku menu, spousti hru kdyz ma a skonci kdyz si to uzivatel
	 * preje
	 * @throw gcn::Exception Pri chybe v Guichanu
	 * @throw std::runtime_error Pri jine chybe
	 */
	void run();

};

#endif
