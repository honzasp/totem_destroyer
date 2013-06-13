#ifndef have_main_hpp
#define have_main_hpp
/** @file game.hpp
 * @brief Hlavickovy soubor pro tridy Game
 * @see Game
 */
#include <Box2D.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "contacts.hpp"
#include "json/parser.hpp"

class ContactListener;

/** Hra (uroven).
 * Tato trida obaluje implementaci jedne urovne hry
 */
class Game {
	friend class ContactListener;

	b2AABB mCamera; ///< Box ktery udava co se zobrazuje
	float mPixelToMeter; ///< Kolik metru je jeden pixel?

	SDL_Surface *mScreen; ///< Obrazovka
	TTF_Font *mLittleFont; ///< Font pro male texty (napr. "level 134")
	TTF_Font *mBigFont; ///< Font pro velike texty (napr. "GAME OVER")
	SDL_Surface *mGameOver; ///< Vykreslena hlaska Game Over
	SDL_Surface *mCheckingVictory; ///< Vykreslena hlaska Checking victory...
	SDL_Surface *mSuccess; ///< Vykreslena hlaska Success
	SDL_Surface *mRenderedMapName; ///< Vykresleny nazev mapy

	b2World *mWorld; ///< Svet
	ContactListener *mContactListener; ///< Posluchac kontaktu
	float mStepTime; ///< Cas jednoho kroku
	int mIterations; ///< Pocet iteraci pro Box2D

	bool mLost; ///< Prohral hrac?
	bool mChecking; ///< Kontroluje se vyhra?
	bool mWin; ///< Vyhral hrac?
	bool mPaused; ///< Je hra pozastavena?
	bool mRunning; ///< Bezi hra?
	bool mCharging; ///< Nabiji se?
	float mLostTime; ///< Cas ktery uplynul od prohry hrace (v sekundach)
	float mWinTime; ///< Cas ktery uplynul od vyhry hrace
	float mChargingTime; ///< Jak dlouho se uz nabiji
	std::vector<Idol*> mIdols; ///< Vsichni buzci ve hre

	json::Parser mParser; ///< Parser na JSON
	std::string mDataDir; ///< Adresar s daty
	std::string mMapName; ///< Jmeno mapy
	std::string mMapFile; ///< Soubor s mapou
	int mToDestroy; ///< Pocet kosticek ktere se jeste musi znicit
	std::vector<Brick*> mCombosToDestroy; ///< Komba ktera se maji znicit (nemuzou se znicit v posluchaci)

	SDL_Cursor *mNormalCursor; ///< Normalni kurzor
	SDL_Cursor *mChargingCursor; ///< Kurzor pri nabijeni

	static float GameOverTime; ///< Cas po ktery se zobrazuje hlaska Game over
	static float SuccesTime; ///< Cas po ktery se zobrazuje Succes
	static float ChargingTime; ///< Cas po ktery se nabiji
	static float ExplosionForce; ///< Sila vybuchu TNT
	static float ExplosionLimit; ///< Pri jake sile vybuchu vybuchne dalsi krabice

	/** Nastavi OpenGL.
	* Nastavi OpenGL podle mScreen.
	*
	* @return true pokud se vse povedlo, false pokud neco selhalo
	*/
	bool setupGL();

	/** Obnovi OpenGL.
	 * Obnovi OpenGL do stavu v jakem byla pred volanim setupGL()
	 */
	void restoreGL();

	/** Prevede souradnice.
	* Prevede souradnice okna na souradnice ve svete
	* 
	* @param x X-ova souradnice
	* @param y Y-ova souradnice
	* @return Pozice ve svete
	*/
	b2Vec2 windowToWorld(int x, int y);

	/** Vykreslovani.
	* Vykresli vsechna telesa pomoci jejich metod draw()
	*/
	void draw();

	/** Zpracovani udalosti.
	* Zpracuje vsechny udalosti 
	*/
	void processEvents();

	/** Zpracovani stisku klavesy.
	 * Zpracuje stisk klavesy
	 *
	 * @param key Symbol stisknute klavesy (SDLK_*)
	 */
	void keyPressed(SDLKey key);

	/** Zpracovani uvolneni klavesy.
	 * Zpracuje uvolneni klavesy
	 *
	 * @param key Symbol uvolnene klavesy (SDLK_*)
	 */
	void keyReleased(SDLKey key);

	/** Zpracovani kliknuti mysi.
	 * Zpracuje kliknuti mysi, takze znici objekt na ktery se kliklo (pokud to
	 * jde).
	 *
	 * @param button Tlacitko (symbolicka konstanta SDL_BUTTON_*)
	 * @param x X-ova souradnice kliknuti (v px)
	 * @param y Y-ova souradnice kliknuti (v px)
	 * @warning Y-ova souradnice roste smerem dolu, ne nahoru!
	 */
	void mouseClicked(Uint8 button, int x, int y);

	/** Nacteni mapy.
	 * Nacte mapu z JSON souboru v mMapName a vytvori svet (mWorld)
	 */
	void loadMap();

	/** Smazani mapy.
	 * Smaze celou mapu
	 */
	void deleteMap();

	/** Znici kostky ktere nejdou videt.
	 * Znici vsechny kostky ktere nejdou videt
	 */
	void deleteInvisible();

	/** Znici kostku.
	 * Znici kostku, pokud je to TNT tak ji i odpali.
	 */
	void destroyBrick(Brick *brick);

	/** Udela vybuch.
	 * Udela vybuch predane kostky (mela by mit type() == Brick::TNT). Nemaze ji,
	 * o to se postara volajici.
	 *
	 * @param position Pozice vybuchu
	 * @param bombMass Hmotnost vybusniny (sila vybuchu)
	 * @warning Muzou vybouchnout i jine kostky!
	 */
	void makeExplosion(const b2Vec2 &position, float bombMass);

public:
	/** Inicializuje hru.
		* 
		* @param screen SDL surface okna s podporou OpenGL
		* @param dataDir Adresar s daty
		* @param font Cesta k souboru s fontem
		* @param map Cesta k JSON souboru s mapou
		* @throw std::runtime_error Chyba pri nacitani fontu nebo mapy
		*/
	Game(SDL_Surface *screen, std::string dataDir, std::string font, std::string map); 

	/** Deinicializuje hru */
	~Game();

	/** Rozbehne hru.
	* Rozbehne hlavni herni smycku
	*/
	void run();

	/** Frekvence herni smycky.
	 * @return Frekvenci herni smycky (hz)
	 */
	float fps() { return 1.0/mStepTime; }

	/** Nastavi frekvenci herni smycky.
	 * @param f Nova frekvence herni smycky (hz)
	 */ 
	void fps(float f) { mStepTime = 1.0/f; }

	/** Vrati pocet iteraci ktere se maji pouzit v Box2D.
	 * @return Pocet iteraci
	 */
	int iterations() { return mIterations; }

	/** Nastavi pocet iteraci ktere se maji pouzit v Box2D.
	 * @param i Novy pocet iteraci
	 */
	void iterations(int i) { mIterations = i; }

	/** Hrac prohral.
	 * Hra zobrazi upozorneni a skonci protoze hrac prohral 
	 */
	void lost() { mLost = true; }

	/** Pozastaveni hry.
	 * Hra se pozastavi - vykresluje se, ale nehybe
	 */
	void pause() { mPaused = true; }

	/** Rozbehnuti hry.
	 * Pokud je hra pozastavena, rozbehne se
	 */
	void play() { mPaused = false; }
};

#endif
