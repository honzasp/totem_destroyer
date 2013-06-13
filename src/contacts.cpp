/** @file contacts.cpp
 * @brief Implementace posluchace kontaktu
 * @see contacts.hpp
 */
#include <algorithm>
#include <Box2D.h>
#include <cassert>
#include <typeinfo>
#include <iostream>
#include "objects.hpp"
#include "contacts.hpp"

void ContactListener::Add(const b2ContactPoint *point)
{
	try {
		GameObject *obj1 = static_cast<GameObject*>(point->shape1->GetBody()->GetUserData());
		GameObject *obj2 = static_cast<GameObject*>(point->shape2->GetBody()->GetUserData());

		assert(obj1);
		assert(obj2);

		if((typeid(*obj1) == typeid(Idol) and typeid(*obj2) == typeid(Ground)) or
			(typeid(*obj2) == typeid(Idol) and typeid(*obj1) == typeid(Ground))) {
			/* kdyz se buzek dotkne zeme tak hra konci */
			mGame->lost();
		} else if(typeid(*obj1) == typeid(Brick) and typeid(*obj2) == typeid(Brick)) {
			/* dve kosticky */
			Brick *b1 = static_cast<Brick*>(obj1);
			Brick *b2 = static_cast<Brick*>(obj2);

			/* komba? */
			if(b1->type() == Brick::Combo and b2->type() == Brick::Combo) {
				/* nemuzou se znicit hned, protoze ted jsme v b2World.Step(), tak se
				 * pridaji do pole komb ktera se maji znici (pokud uz v nem nejsou) */

				if(std::find(mGame->mCombosToDestroy.begin(),
										 mGame->mCombosToDestroy.end(),
										 b1) == mGame->mCombosToDestroy.end())
					mGame->mCombosToDestroy.push_back(b1);

				if(std::find(mGame->mCombosToDestroy.begin(),
										 mGame->mCombosToDestroy.end(),
										 b2) == mGame->mCombosToDestroy.end())
					mGame->mCombosToDestroy.push_back(b2);
			}
		}
	} catch(std::bad_typeid& e) {
		std::cerr << "ContactListener::Add: " << e.what() << std::endl;
	}
}

void ContactListener::Persist(const b2ContactPoint *point) 
{

}

void ContactListener::Remove(const b2ContactPoint* point)
{

}

void ContactListener::Result(const b2ContactResult *result) 
{

}
