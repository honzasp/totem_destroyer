/** @file objects.cpp
 * @brief Implementace trid definovanych v hlavickovem souboru objects.hpp
 * @see objects.hpp
 * @see GameObject
 */
#include <Box2D.h>
#include <cassert>
#include <vector>
#include <string>
#include "color.hpp"
#include "objects.hpp"
/* Pro komentare viz objects.hpp */


void GameObject::drawVertices()
{
	b2Shape *shape = mBody->GetShapeList();

	/* teleso bez tvaru se ignoruje */
	if(!shape)
		return;

	if(shape->GetType() == e_polygonShape) {
		/* polygon se zpracuje jednoduse */
		b2PolygonShape *poly;
		poly = dynamic_cast<b2PolygonShape*>(shape);
		assert(poly);

		for(int i=0; i!=poly->GetVertexCount(); i++)
			glVertex2f(poly->GetVertices()[i].x, poly->GetVertices()[i].y);
	} else {
		assert(shape->GetType() == e_circleShape);
		b2CircleShape *circle;
		circle = dynamic_cast<b2CircleShape*>(shape);
		assert(circle);
		/* kruh se musi spocitat */
		const int segs = 32; 
		const float step = 2*M_PI/segs;
		const float radius = circle->GetRadius();

		for(int i=0; i!=segs; i++)
			glVertex2f(std::cos(step*i)*radius, std::sin(step*i)*radius);
	}
}

void GameObject::drawShape(Color c)
{
	glPushMatrix();
	glTranslatef(mBody->GetPosition().x, mBody->GetPosition().y, 0.0f);
	glRotatef(mBody->GetAngle()/M_PI*180.0, 0.0f, 0.0f, 1.0f);

	/* nejprve se nakresli obrys */
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_LINE_LOOP);
		drawVertices();
	glEnd();

	/* a pak vypln */
	glColor4f(c.r, c.g, c.b, 0.7);
	glBegin(GL_POLYGON);
		drawVertices();
	glEnd();


	glPopMatrix();
}


GameObject::GameObject(b2World *world, b2Vec2 position):
	mWorld(world),
	isVisible(false)
{
	b2BodyDef bodyDef;
	bodyDef.position = position;
	bodyDef.userData = this;
	bodyDef.linearDamping = 0.1;
	bodyDef.angularDamping = 0.1;
	mBody = mWorld->CreateBody(&bodyDef);
}

GameObject::~GameObject()
{
	mWorld->DestroyBody(mBody);
}

float Brick::friction[BricksCount] = {
	0.3, 0.3, 0.1, 0.001, 0.9, 0.3
};

float Brick::density[BricksCount] = {
	1.0, 1.2, 0.5, 1.5, 0.8, 1.0
};

float Brick::restitution[BricksCount] = {
	0.2, 0.1, 0.7, 0.1, 0.8, 0.3
};

bool Brick::canDestruct[BricksCount] = { 
	true, false, false, true, true, true
};

std::string Brick::labels[BricksCount] = {
	"Normal brick is very stable and don't surprise you",
	"Solid brick is only brick which you can't destroy",
	"Combo bricks will destroy if two combos touch",
	"Slippy brick make totem totally unstable",
	"Gummy brick can also easily throw down totem",
	"TNT will explode when you destroy it"
};

Color Brick::colors[BricksCount] = {
	Color(212, 141, 76),
	Color(39, 30, 22),
	Color(238, 169, 50),
	Color(53, 161, 45),
	Color(255, 201, 201),
	Color(213, 66, 13)
};

Brick::Brick(b2World *world, b2Vec2 position, float width, float height, Type type):
	GameObject(world, position), mType(type)
{
	b2PolygonDef poly;
	poly.SetAsBox(width/2.0, height/2.0);
	poly.friction = friction[type];
	poly.density = density[type];
	poly.restitution = restitution[type];
	mBody->CreateShape(&poly);
	mBody->SetMassFromShapes();
}

void Brick::draw()
{
	drawShape(colors[mType]);
}

Ground::Ground(b2World *world, b2Vec2 position, float width, float height):
	GameObject(world, position)
{
	b2PolygonDef poly;
	poly.SetAsBox(width/2.0, height/2.0);
	poly.friction = 0.5;
	poly.restitution = 0.1;
	mBody->CreateShape(&poly);
}

void Ground::draw()
{
	drawShape(Color(59, 154, 52));
}

Idol::Idol(b2World *world, b2Vec2 position, const std::vector<b2Vec2> &vertices):
	GameObject(world, position)
{
	b2PolygonDef poly;
	poly.vertexCount = vertices.size();
	std::vector<b2Vec2>::size_type i;
	for(i=0; i!=vertices.size(); i++)
		poly.vertices[i] = vertices[i];
	poly.friction = 0.5;
	poly.restitution = 0.1;
	poly.density = 1.0;

	mBody->CreateShape(&poly);
	mBody->SetMassFromShapes();
}

void Idol::draw()
{
	drawShape(Color(244, 224, 0));
}
