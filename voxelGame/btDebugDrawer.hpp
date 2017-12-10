// Taken from:
//	http://sio2interactive.forumotion.net/t599-enabling-bullet-debug-draw-code-included
// By Francescu in 2009
#pragma once
#include <LinearMath/btIDebugDraw.h>
#include "sre/RenderPass.hpp"


class btDebugDrawer : public btIDebugDraw {	
public:
	btDebugDrawer();

	virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	virtual void   reportErrorWarning(const char* warningString);
	virtual void   draw3dText(const btVector3& location, const char* textString);
	virtual void   setDebugMode(int debugMode);
	
	virtual int    getDebugMode() const { return m_debugMode; }

	sre::RenderPass* renderPass = nullptr;

private:
	int m_debugMode;	
};