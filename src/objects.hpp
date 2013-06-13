#ifndef have_objects_hpp
#define have_objects_hpp
/** @file objects.hpp
 * @brief Hlavickovy soubor pro objekty ve hre
 * @see GameObject
 */
#include <Box2D.h>
#include <vector>
#include <string>
#include "color.hpp"

/** Objekt ve hre.
 * Abstraktni trida ktera predstavuje kazdy objekt ve hre. Vsechna telesa maji
 * ve sve vlastnosti userData ukazatel na svoji instanci teto tridy */
class GameObject {
protected:
	b2Body *mBody; ///< prislusne teleso
	b2World *mWorld; ///< prislusny svet ve kterem teleso je

	/** Zavola glVertex2f pro vsechny vertexy v prvnim tvaru telesa mBody (uzitecne
	 * pri kresleni)
	 *
	 * @warning Je nutno nejdrive zavolat glBegin()!
	 */
	void drawVertices();

	/** Nakresli pekne prvni tvar v telese.
	 * @param c Barva kterou se ma tvar nakreslit
	 */
	void drawShape(Color c);
public:
	/** Umisteni telesa do sveta.
	 * Umisti teleso na urcenou pozici do sveta.
	 * 
	 * @param world Svet do ktereho se teleso prida
	 * @param position Pozice na kterou se teleso da
	 */
	GameObject(b2World *world, b2Vec2 position);

	/** Smaze teleso ze sveta.
	 * Smaze se vcetne sveho b2Body ze sveta
	 */
	virtual ~GameObject();

	/** Nakresli teleso.
	 * Pomoci OpenGL nakresli teleso. Predpoklada ze vzdalenost 1 v OpenGL je 1
	 * metr
	 */
	virtual void draw() = 0;

	/** Vrati teleso */
	const b2Body *body() { return mBody; }

	/** Pouzito v Game k mazani neviditelnych teles */
	bool isVisible; 
};

/** Kosticka.
 * Trida ktera reprezentuje obdelnikovou kosticku kterou uzivatel kliknutim
 * muze znicit (pokud to jeji typ povoluje). 
 *
 * @todo Vybusna kosticka
 */
class Brick: public GameObject {
public:
	/** Typ kosticky */
	enum Type {
		Normal = 0, ///< Normalni kosticka ktera se da znicit
		Dark, ///< Tmava neznicitelna kosticka
		Combo, ///< Kosticka ktera zmizi jen pri kontaktu s jinou kombo kostkou
		Slippy, ///< Velmi kluzka znicitelna kosticka
		Gummy, ///< "Gumova" kostka
		TNT, ///< Vybusna kostka
		BricksCount 
	};

	/* atributy pro jednotlive typy kosticek */
	static float friction[BricksCount]; ///< hodnoty treni pro kazdy typ kosticky
	static float density[BricksCount]; ///< hodnoty hustoty pro kazdy typ kosticky
	static float restitution[BricksCount]; ///< hodnoty odrazivosti (pruznosti)
	static bool canDestruct[BricksCount]; ///< znicitelnost 
	static Color colors[BricksCount]; ///< barva
	static std::string labels[BricksCount]; ///< popisky kosticek

private:
	Type mType; ///< Typ kosticky
public:
	/** Umisti kosticku prislusneho typu o danych rozmerech do sveta.
	 *
	 * @param world Svet
	 * @param position Pozice
	 * @param width Sirka kosticky 
	 * @param height Vyska kosticky 
	 * @param type Typ kosticky
	 */
	Brick(b2World *world, b2Vec2 position, float width, float height, Type type);

	/** Je tuto kosticku mozno znicit?
	 * @return true pokud je kosticka znicitelna, false pokud neni
	 */
	bool canDestroy() { return Brick::canDestruct[mType]; }

	/** Nakresli teleso */
	virtual void draw();

	/** Ziska typ kosticky */
	Type type() { return mType; }
};

/** Zeme.
 * Staticke zelene teleso ktere funguje jako podlozka */
class Ground: public GameObject {
public:
	/** Umisti obdelnik zeme o danych rozmerech na urcenou pozici.
	 *
	 * @param world Svet
	 * @param position Pozice
	 * @param width Sirka obdelniku
	 * @param height Vyska obdelniku
	 */
	Ground(b2World *world, b2Vec2 position, float width, float height);

	/** Nakresli zem */
	virtual void draw();
};

/** Buzek.
 * Jeho tvar neni pevne dany, ale vzdy to je polygon (konvexni). Vykresluje se
 * zlute */
class Idol: public GameObject {
public:
	/** Umisti buzka do sveta na danou pozici.
	 *
	 * @param world Svet 
	 * @param position Pozice
	 * @param vertices Specifikace tvaru
	 *
	 * @warning Na tvar jsou kladeny stejne naroky jako na vrcholy polygonu, tj.
	 * musi byt konvexni a pocet vrcholu nesmi presahnout b2_maxPolygonVertices
	 */
	Idol(b2World *world, b2Vec2 position, const std::vector<b2Vec2> &vertices);

	/** Nakresli buzka */
	virtual void draw();
};

#endif
