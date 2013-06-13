#ifndef have_contacts_hpp
#define have_contacts_hpp
/** @file contacts.hpp
 * @brief Hlavickovy soubor pro posluchac kontaktu
 */
#include <Box2D.h>
#include "game.hpp"
#include "objects.hpp"

class Game;

/** Posluchac kontaktu */
class ContactListener: public b2ContactListener {
	Game *mGame; ///< Hra ktere tento posluchac prislusi
public:
	/** Nastavi hru */
	ContactListener(Game *game): mGame(game) { }

	/** Volano pri pridani noveho kontaktniho bodu.
	 * @param point Novy kontaktni bod
	 */
	void Add(const b2ContactPoint *point);

	/** Volano kdyz kontaktni bod pretrvava.
	 * @param point Kontaktni bod
	 */
	void Persist(const b2ContactPoint *point);

	/** Volano kdyz se kontaktni bod odstranuje.
	 * @param point Odstranovany bod
	 */
	void Remove(const b2ContactPoint *point);

	/** Vysledek kontaktu.
	 * @param result Objekt vysledku.
	 */
	void Result(const b2ContactResult *result);
};

#endif
