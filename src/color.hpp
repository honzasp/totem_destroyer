#ifndef have_color_hpp
#define have_color_hpp
/** @file color.hpp
 * @brief Specifikace tridy Color
 * @see Color
 */
#include <SDL_opengl.h>
#include <guichan.hpp>

/** Barva (pro OpenGL) */
struct Color {
	GLfloat r; ///< Cervena
	GLfloat g; ///< Zelena
	GLfloat b; ///< Modra

	/** Inicializace z jednotlivych slozek.
	 *
	 * @param r Cervena slozka v rozsahu [0..1]
	 * @param g Zelena slozka v rozsahu [0..1]
	 * @param b Modra slozka v rozsahu [0..1]
	 */
	Color(GLfloat r, GLfloat g, GLfloat b): r(r), g(g), b(b) { }

	/** Incializace z jednotlivych slozek.
	 *
	 * @param r Cervena slozka v rozsahu [0..155]
	 * @param g Zelena slozka v rozsahu [0..155]
	 * @param b Modra slozka v rozsahu [0..155]
	 */
	Color(int r, int g, int b): r(r/255.0), g(g/255.0), b(b/255.0) { }

	/** Vychozi barva je cerna */
	Color(): r(0.0), g(0.0), b(0.0) { }

	/** Konverze do Guichanovske barvy */
	operator gcn::Color() { return gcn::Color(r*255, g*255, b*255); }
};

#endif
