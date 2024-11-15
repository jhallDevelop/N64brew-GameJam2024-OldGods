/*
===============================================================================
AF_PHYSICS Implementation 
n64 implementation of AF_Physics
===============================================================================
*/
#include <libdragon.h>
#include "AF_Physics.h"
#include "ECS/Entities/AF_ECS.h"

float collisionColor[4] = {255,0, 0, 1};
// Physics Init
void AF_Physics_Init(AF_ECS* _ecs){
	assert(_ecs != NULL && "Physics: Physics_Init pass in a null reference\n");
	debugf("Physics_Init: \n");

	// Setup Broadphase physics
	/*
	AF_Physics_UpdateBroadphaseAABB
	||
	AF_Physics_BroadPhase
	||
	AF_Physics_NarrowPhase
	
	for(int i = 0; i < _ecs->entitiesCount; ++i){
		//AF_Physics_UpdateBroadphaseAABB(&_ecs->colliders[i]);
	}

	//AF_Physics_BroadPhase(_ecs);

	for(int i = 0; i < _ecs->entitiesCount; ++i){
		//AF_Physics_NarrowPhase(&_ecs->colliders[i].collision, _ecs->entitiesCount, 1);
	}*/
	

}


// Physics Update
void AF_Physics_Update(AF_ECS* _ecs, const float _dt){
	assert(_ecs != NULL && "Physics: AF_Physics_Update pass in a null reference\n");
	// loop through and update all transforms based on their velocities
	for(int i = 0; i < _ecs->entitiesCount; ++i){
	AF_C3DRigidbody* rigidbody = &_ecs->rigidbodies[i];
	if((AF_Component_GetHas(rigidbody->enabled) == TRUE) && (AF_Component_GetEnabled(rigidbody->enabled) == TRUE)){
		AF_CTransform3D* transform = &_ecs->transforms[i];

		// Negate the velocities before adding more
		//Vec3 zeroVelocity = {0,0,0};
		//rigidbody->velocity = zeroVelocity;
		
		
		//debgf("Physics: upate: velocity x: %f y: %f z: %f\n", rigidbody->velocity.x, rigidbody->velocity.y, rigidbody->velocity.z);
		// if the object isn't static
		if(rigidbody->inverseMass > 0 || rigidbody->isKinematic == TRUE){
				AF_Physics_IntegrateAccell(rigidbody, _dt);
				AF_Physics_IntegrateVelocity(transform, rigidbody, _dt);
		}
		//_ecs->transforms[i].pos = Vec3_ADD(_ecs->rigidbodies[i].velocity, _ecs->transforms[i].pos);

		}

		AF_CCollider* collider = &_ecs->colliders[i];
		// update the bounds position
		collider->pos = _ecs->transforms[i].pos;
		// clear all collsision except keep the callback
		AF_Collision clearedCollision = {FALSE, NULL, NULL, collider->collision.callback, {0,0,0}, 0.0f, {0,0,0}, 0};
		collider->collision = clearedCollision;
	}
}

void AF_Physics_LateUpdate(AF_ECS* _ecs){
	assert(_ecs != NULL && "Physics: AF_Physics_LateUpdate pass in a null reference\n");

	// Do collision tests
	AF_Physics_AABB_Test(_ecs);

	// call the collision pairs

	// Resolve collision between two objects
}

void AF_Physics_LateRenderUpdate(AF_ECS* _ecs){
	assert(_ecs != NULL && "Physics: AF_Physics_LateRenderUpdate pass in a null reference\n");
	for(int i = 0; i < _ecs->entitiesCount; ++i){
		AF_CCollider* collider = &_ecs->colliders[i];
		if(collider->showDebug != TRUE){
			//debugf("Physics: LateRenderUpate: not showing debug %i\n", i);
			continue;
		}

		if(collider->collision.collided != TRUE){
			//debugf("Physics: LateRenderUpate: not colided\n");
			continue;
		}
		//debugf("Physics: LateRenderUpate: draw debug\n");
		//AF_Physics_DrawBox(collider, collisionColor);	
	}
}



//Physics Shutdown
void AF_Physics_Shutdown(void){
	debugf("Physics: Shutdown\n");
}

