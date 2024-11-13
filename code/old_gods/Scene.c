#include "Scene.h"
#include "ECS/Entities/AF_Entity.h"
#include "Assets.h"
#include "EntityFactory.h"
#include "Assets.h"
#include "AF_Input.h"
#include "PlayerController.h"
// Needed for game jam
#include "../../core.h"
#include <t3d/t3dmath.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

// ECS system
AF_Entity* camera = NULL;
//AF_Entity* playerEntities[PLAYER_COUNT] = {NULL};  // Initialize all elements to NULL

// God
AF_Entity* godEntity = NULL;
AF_Entity* godEye1 = NULL;
AF_Entity* godEye2 = NULL;
AF_Entity* godEye3 = NULL;
AF_Entity* godEye4 = NULL;
AF_Entity* godEyeInner1 = NULL;
AF_Entity* godEyeInner2 = NULL;
AF_Entity* godMouth = NULL;

// Environment
//AF_Entity* leftWall = NULL;
//AF_Entity* rightWall = NULL;
//AF_Entity* backWall = NULL;
//AF_Entity* frontWall = NULL;
//AF_Entity* groundPlaneEntity = NULL;
AF_Entity* levelMapEntity = NULL;

// Pickup
AF_Entity* bucket1 = NULL;
AF_Entity* bucket2 = NULL;
AF_Entity* bucket3 = NULL;
AF_Entity* bucket4 = NULL;

// Villagers
AF_Entity* villager1 = NULL;
AF_Entity* villager2 = NULL;
AF_Entity* villager3 = NULL;
AF_Entity* villager4 = NULL;

// Audio
AF_Entity* laserSoundEntity = NULL;
AF_Entity* cannonSoundEntity = NULL;
AF_Entity* musicSoundEntity = NULL;

// Gameplay Var
uint8_t g_currentBucket = 0;


#define VILLAGER_CARRY_HEIGHT 1

// Minigame vars
float countDownTimer;
bool isEnding;
float endTimer;
PlyNum winner;



#define FONT_TEXT           1
#define FONT_BILLBOARD      2
#define TEXT_COLOR          0x6CBB3CFF
#define TEXT_OUTLINE        0x30521AFF

#define HITBOX_RADIUS       10.f

#define ATTACK_OFFSET       10.f
#define ATTACK_RADIUS       5.f

#define ATTACK_TIME_START   0.333f
#define ATTACK_TIME_END     0.4f

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

#define BILLBOARD_YOFFSET   15.0f

typedef struct
{
  PlyNum plynum;
  T3DMat4FP* modelMatFP;
  rspq_block_t *dplSnake;
  T3DAnim animAttack;
  T3DAnim animWalk;
  T3DAnim animIdle;
  T3DSkeleton skelBlend;
  T3DSkeleton skel;
  T3DVec3 moveDir;
  T3DVec3 playerPos;
  float rotY;
  float currSpeed;
  float animBlend;
  bool isAttack;
  bool isAlive;
  float attackTimer;
  PlyNum ai_target;
  int ai_reactionspeed;
} player_data;

player_data players[MAXPLAYERS];

float countDownTimer;
bool isEnding;
float endTimer;
PlyNum winner;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;



void Scene_Awake(AppData* _appData){
    Scene_SetupEntities(_appData);
    Scene_CreateIU_Entities(&_appData->ecs);
}

void Scene_Start(AppData* _appData){
    // TODO: get rid of global state, current bucket
    debugf("Scene: Scene Start: TODO: get rid of global state, current bucket");
    Scene_SpawnBucket(&g_currentBucket);
}

void Scene_Update(AppData* _appData){
    // TODO
    if(_appData->gameplayData.gameState == GAME_STATE_PLAYING){
        
        PlayerController_UpdateAllPlayerMovements(&_appData->input, *_appData->gameplayData.playerEntities, PLAYER_COUNT);
    }

    AF_ECS* ecs = &_appData->ecs;
    // carry villages
    for(int i = 0; i < ecs->entitiesCount; ++i){
        AF_Entity* entity = &ecs->entities[i];
        AF_CPlayerData* playerData = entity->playerData;

        
        // TODO: move this out of this function
        //if((AF_Component_GetHas(playerData->enabled) == TRUE) && (AF_Component_GetEnabled(playerData->enabled) == TRUE)){
        if(playerData->isCarrying == TRUE){
            // make villager match player transform
            Vec3 villagerCarryPos = {entity->transform->pos.x, entity->transform->pos.y+VILLAGER_CARRY_HEIGHT, entity->transform->pos.z};
            GetVillager()->transform->pos = villagerCarryPos;
            
            //debugf("entity carrying villager: x: %f y: %f x: %f \n", villagerCarryPos.x, villagerCarryPos.y, villagerCarryPos.z);
        }
    }

    // Update the god count by summing the players scores
    
    int updatedTotalScore = 0;
    assert(_appData->gameplayData.playerEntities != NULL && "Scene_Update: player entities component is null\n");
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_Entity* playerEntity = _appData->gameplayData.playerEntities[i];
        assert(playerEntity!= NULL && "Scene_Update: player entity component is null\n");
        AF_CPlayerData* playerData = playerEntity->playerData;
        assert(playerData != NULL && "Scene_Update: playerdata component is null\n");
        updatedTotalScore += playerData->score;
    }
    _appData->gameplayData.godEatCount = updatedTotalScore;
    /**/
    
}

void Scene_Destroy(AF_ECS* _ecs){

}

// Setup the games entities
void Scene_SetupEntities(AppData* _appData){
    // TODO: store these entities into a special struct just to hold pointers to these elements specifically
    AF_ECS* _ecs = &_appData->ecs;
    GameplayData* gameplayData = &_appData->gameplayData;
    int zeroInverseMass = 0.0f;
	// initialise the ecs system
	// Create Camera
	camera = AF_ECS_CreateEntity(_ecs);
	
    // TODO: read this data from a csv or xml file for quick and efficient setup
    // also useful for mapping to a ui editor
    // Create God
    
	Vec3 godPos = {0, 0, 0};
	Vec3 godScale = {2,2,2};
    godEntity = Entity_Factory_CreatePrimative(_ecs, godPos, godScale, AF_MESH_TYPE_SPHERE, AABB);
    godEntity->mesh->material.textureID = TEXTURE_ID_9;
    godEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    godEntity->mesh->meshID = MODEL_SNAKE;
    godEntity->rigidbody->inverseMass = zeroInverseMass;
    godEntity->collider->collision.callback = Scene_OnGodTrigger;
    godEntity->collider->showDebug = TRUE;
    
    /*
    
    Vec3 godeye1Pos = {-2.5, 10, -12};
	Vec3 godeye1Scale = {1,1,1};
    godEye1 = Entity_Factory_CreatePrimative(_ecs, godeye1Pos, godeye1Scale, AF_MESH_TYPE_CUBE, AABB);
    godEye1->rigidbody->inverseMass = zeroInverseMass;
    godEye1->mesh->meshType = AF_MESH_TYPE_MESH;
    godEye1->mesh->meshID = MODEL_FOOD;
    godEye1->mesh->material.textureID = TEXTURE_ID_0;
    godEye1->mesh->isAnimating = TRUE;

    Vec3 godeye2Pos = {2.5, 10, -12};
	Vec3 godeye2Scale = {1,1,1};
    godEye2 = Entity_Factory_CreatePrimative(_ecs, godeye2Pos, godeye2Scale, AF_MESH_TYPE_CUBE, AABB);
    godEye2->rigidbody->inverseMass = zeroInverseMass;
    godEye2->mesh->material.textureID = TEXTURE_ID_1;
    godEye2->mesh->meshType = AF_MESH_TYPE_MESH;
    godEye2->mesh->meshID = MODEL_FOOD;
    godEye2->mesh->isAnimating = TRUE;

    Vec3 godeye3Pos = {2.5, 5, -12};
	Vec3 godeye3Scale = {1,1,1};
    godEye3 = Entity_Factory_CreatePrimative(_ecs, godeye3Pos, godeye3Scale, AF_MESH_TYPE_CUBE, AABB);
    godEye3->rigidbody->inverseMass = zeroInverseMass;
    godEye3->mesh->meshType = AF_MESH_TYPE_MESH;
    godEye3->mesh->meshID = MODEL_FOOD;
    godEye3->mesh->material.textureID = TEXTURE_ID_2;
    godEye3->mesh->isAnimating = TRUE;

    Vec3 godeye4Pos = {-2.5, 5, -12};
	Vec3 godeye4Scale = {1,1,1};
    godEye4 = Entity_Factory_CreatePrimative(_ecs, godeye4Pos, godeye4Scale, AF_MESH_TYPE_CUBE, AABB);
    godEye4->rigidbody->inverseMass = zeroInverseMass;
    godEye4->mesh->meshType = AF_MESH_TYPE_MESH;
    godEye4->mesh->meshID = MODEL_FOOD;
    godEye4->mesh->material.textureID = TEXTURE_ID_3;
    godEye4->mesh->isAnimating = TRUE;
    */
    
    
	// ---------Create Player1------------------
	Vec3 player1Pos = {2.0f, .5f, 1.0f};
	Vec3 player1Scale = {.5f,.5f,.5f};
    
    gameplayData->playerEntities[0] = Entity_Factory_CreatePrimative(_ecs, player1Pos, player1Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player1Entity = gameplayData->playerEntities[0];
    player1Entity->mesh->material.textureID = TEXTURE_ID_0;
    player1Entity->mesh->meshType = AF_MESH_TYPE_MESH;
    player1Entity->mesh->meshID = MODEL_SNAKE;
    player1Entity->rigidbody->inverseMass = zeroInverseMass;
	player1Entity->rigidbody->isKinematic = TRUE;
    *player1Entity->playerData = AF_CPlayerData_ADD();


    // Create Player2
	Vec3 player2Pos = {-2.0f, .5f, 1.0f};
	Vec3 player2Scale = {1,1,1};
    
    gameplayData->playerEntities[1] = Entity_Factory_CreatePrimative(_ecs, player2Pos, player2Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player2Entity = gameplayData->playerEntities[1];
    player2Entity->mesh->material.textureID = TEXTURE_ID_1;
    player2Entity->mesh->meshType = AF_MESH_TYPE_MESH;
    player2Entity->mesh->meshID = MODEL_SNAKE;
    player2Entity->rigidbody->inverseMass = zeroInverseMass;
	player2Entity->rigidbody->isKinematic = TRUE;
    *player2Entity->playerData = AF_CPlayerData_ADD();

    // Create Player3
	Vec3 player3Pos = {-2.0f, .5f, -1.0f};
	Vec3 player3Scale = {.75f,.75f,.75f};
    
    gameplayData->playerEntities[2] = Entity_Factory_CreatePrimative(_ecs, player3Pos, player3Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player3Entity = gameplayData->playerEntities[2];
    player3Entity->mesh->material.textureID = TEXTURE_ID_2;
    player3Entity->mesh->meshType = AF_MESH_TYPE_MESH;
    player3Entity->mesh->meshID = MODEL_SNAKE;
	player3Entity->rigidbody->isKinematic = TRUE;
    player3Entity->rigidbody->inverseMass = zeroInverseMass;
    *player3Entity->playerData = AF_CPlayerData_ADD();

    // Create Player4
	Vec3 player4Pos = {2.0f, .5f, -1.0f};
	Vec3 player4Scale = {1,1,1};
    
    gameplayData->playerEntities[3] = Entity_Factory_CreatePrimative(_ecs, player4Pos, player4Scale, AF_MESH_TYPE_MESH, AABB);
	AF_Entity* player4Entity = gameplayData->playerEntities[3];
    player4Entity->mesh->material.textureID = TEXTURE_ID_3;
    player4Entity->mesh->meshType = AF_MESH_TYPE_MESH;
    player4Entity->mesh->meshID = MODEL_SNAKE;
	player4Entity->rigidbody->isKinematic = TRUE;
    player4Entity->rigidbody->inverseMass = zeroInverseMass;
    *player4Entity->playerData = AF_CPlayerData_ADD();

	//=========ENVIRONMENT========
    Vec3 levelMapPos = {0, 0, 2};
	Vec3 levelMapScale = {2,1,1};
    levelMapEntity = Entity_Factory_CreatePrimative(_ecs, levelMapPos, levelMapScale, AF_MESH_TYPE_CUBE, AABB);
    //levelMapEntity->mesh->material.textureID = TEXTURE_ID_7;
    levelMapEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    levelMapEntity->mesh->meshID = MODEL_MAP;
	levelMapEntity->rigidbody->inverseMass = zeroInverseMass;

    /*
	// Create Plane
	Vec3 planePos = {0, -2, 0};
	Vec3 planeScale = {40,1,40};
    groundPlaneEntity = Entity_Factory_CreatePrimative(_ecs, planePos, planeScale, AF_MESH_TYPE_CUBE, AABB);
    groundPlaneEntity->mesh->material.textureID = TEXTURE_ID_7;
	groundPlaneEntity->rigidbody->inverseMass = zeroInverseMass;

    // Create Left Wall
    float wallHeight = 3;
	Vec3 leftWallPos = {-40, 0, 0};
	Vec3 leftWallScale = {1,wallHeight,40};
	leftWall = Entity_Factory_CreatePrimative(_ecs, leftWallPos, leftWallScale, AF_MESH_TYPE_CUBE, AABB);
    leftWall->mesh->material.textureID = TEXTURE_ID_8;
	leftWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Right Wall
	Vec3 rightWallPos = {40, 0, 0};
	Vec3 rightWallScale = {1,wallHeight,40};    
    rightWall = Entity_Factory_CreatePrimative(_ecs, rightWallPos, rightWallScale, AF_MESH_TYPE_CUBE, AABB);
    rightWall->mesh->material.textureID = TEXTURE_ID_8;
	rightWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Back Wall
	Vec3 backWallPos = {0, 0, -30};
	Vec3 backWallScale = {40,wallHeight,1};
    backWall = Entity_Factory_CreatePrimative(_ecs, backWallPos, backWallScale, AF_MESH_TYPE_CUBE, AABB);
    backWall->mesh->material.textureID = TEXTURE_ID_8;
	backWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Front Wall
	Vec3 frontWallPos = {0, 0, 20};
	Vec3 frontWallScale = {40,wallHeight,1};
    frontWall = Entity_Factory_CreatePrimative(_ecs, frontWallPos, frontWallScale, AF_MESH_TYPE_CUBE, AABB);
    frontWall->mesh->material.textureID = TEXTURE_ID_8;
	frontWall->rigidbody->inverseMass = zeroInverseMass;
    */

    // ============Buckets=============
    // Bucket 1
    // World pos and scale for bucket
	Vec3 bucket1Pos = {-5.0f, .5f, 0.0f};
	Vec3 bucket1Scale = {1,1,1};
    bucket1 = Entity_Factory_CreatePrimative(_ecs, bucket1Pos, bucket1Scale,AF_MESH_TYPE_CUBE, AABB);
    bucket1->mesh->material.textureID = TEXTURE_ID_0;
    bucket1->mesh->meshType = AF_MESH_TYPE_MESH;
    bucket1->mesh->meshID = MODEL_BOX;
    bucket1->rigidbody->inverseMass = zeroInverseMass;

    // TODO: add details to scene_onBucketTrigger callback
    bucket1->collider->collision.callback = Scene_OnBucket1Trigger;
    // Bucket 2
    // World pos and scale for bucket
	Vec3 bucket2Pos = {5.0f, .5f, 0.0f};
	Vec3 bucket2Scale = {1,1,1};
	bucket2 = Entity_Factory_CreatePrimative(_ecs, bucket2Pos, bucket2Scale,AF_MESH_TYPE_CUBE, AABB);
    bucket2->mesh->material.textureID = TEXTURE_ID_1;
    bucket2->mesh->meshType = AF_MESH_TYPE_MESH;
    bucket2->mesh->meshID = MODEL_BOX;
    bucket2->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket2->collider->collision.callback = Scene_OnBucket2Trigger;

    // Bucket 3
    // World pos and scale for bucket
	Vec3 bucket3Pos = {5.0f, .5f, 5.0f};
	Vec3 bucket3Scale = {1,1,1};
	bucket3 = Entity_Factory_CreatePrimative(_ecs, bucket3Pos, bucket3Scale,AF_MESH_TYPE_CUBE, AABB);
    bucket3->mesh->material.textureID = TEXTURE_ID_2;
    bucket3->mesh->meshType = AF_MESH_TYPE_MESH;
    bucket3->mesh->meshID = MODEL_BOX;
    bucket3->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket3->collider->collision.callback = Scene_OnBucket3Trigger;
    // Bucket 4
    // World pos and scale for bucket
	Vec3 bucket4Pos = {-5.0f, .5f, 5.0f};
	Vec3 bucket4Scale = {1,1,1};
	bucket4 = Entity_Factory_CreatePrimative(_ecs, bucket4Pos, bucket4Scale,AF_MESH_TYPE_CUBE, AABB);
    bucket4->mesh->material.textureID = TEXTURE_ID_3;
    bucket4->mesh->meshType = AF_MESH_TYPE_MESH;
    bucket4->mesh->meshID = MODEL_BOX;
    bucket4->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket4->collider->collision.callback = Scene_OnBucket4Trigger;

    /// Villages
	Vec3 villager1Pos = {-1000.0f, 0, 0};
	Vec3 villager1Scale = {1,1,1};
    villager1 = Entity_Factory_CreatePrimative(_ecs, villager1Pos, villager1Scale, AF_MESH_TYPE_CUBE, AABB);
    villager1->mesh->material.textureID = TEXTURE_ID_8;
    villager1->mesh->meshType = AF_MESH_TYPE_MESH;
    villager1->mesh->meshID = MODEL_FOOD;
	villager1->rigidbody->inverseMass = zeroInverseMass;
	villager1->rigidbody->isKinematic = TRUE;
    villager1->collider->collision.callback = Scene_OnCollision;


	// Setup Audio
	// TODO: put into audio function
    // Already init from brew game jam structure
	//audio_init(44100, 4);
	//mixer_init(32);  // Initialize up to 16 channels

/*
	AF_AudioClip musicAudioClip = {0, musicFXPath, 12800};
	laserSoundEntity = Entity_Factory_CreateAudio(_ecs, musicAudioClip, CHANNEL_MUSIC, (void*)&sfx_music, TRUE);

	AF_AudioClip sfx1AudioClip = {0, laserFXPath, 128000};
	laserSoundEntity = Entity_Factory_CreateAudio(_ecs, sfx1AudioClip, CHANNEL_SFX1, (void*)&sfx_laser, FALSE);

	AF_AudioClip sfx2AudioClip = {0, cannonFXPath, 128000};
	cannonSoundEntity = Entity_Factory_CreateAudio(_ecs, sfx2AudioClip, CHANNEL_SFX2, (void*)&sfx_cannon, FALSE);

	

	bool music = false;
	//int music_frequency = sfx_music.wave.frequency;
	music = !music;
	if (music) {
		//wav64_play(&sfx_music, CHANNEL_MUSIC);
		//music_frequency = sfx_music.wave.frequency;
	}

*/	

    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // scale everything
        _ecs->transforms[i].scale = Vec3_MULT_SCALAR(_ecs->transforms[i].scale, .0075f);//0.075f);
    }
	
}



// ======== SPAWNERS ==========
// TODO: add better control flow
void Scene_SpawnBucket(uint8_t* _currentBucket){
    int upper = 4;
    int lower = 0;
    int randomNum = (rand() % (upper + - lower) + lower);
    // don't let the random number go above 4, as we count from 0
    if(randomNum >=4){
        randomNum = 3;
    }

    if(godEntity == NULL || bucket1 == NULL || bucket2 == NULL || bucket3 == NULL || bucket4 == NULL)
    {   debugf("Game: SpawnBucket: god or bucket entity is null \n");
        return;
    }
    if(randomNum == 0){
        AF_Entity* bucket1 = GetBucket1();
        AF_Entity* bucket2 = GetBucket2();
        AF_Entity* bucket3 = GetBucket3();
        AF_Entity* bucket4 = GetBucket4();

        *_currentBucket = 0;
        GetGodEntity()->mesh->material.textureID = TEXTURE_ID_0;

        /*
        bucket1->mesh->material.textureID = TEXTURE_ID_0;
        bucket2->mesh->material.textureID = TEXTURE_ID_5;
        bucket3->mesh->material.textureID = TEXTURE_ID_5;
        bucket4->mesh->material.textureID = TEXTURE_ID_5;
        */
        bucket1->mesh->meshID = MODEL_FOOD;
        bucket2->mesh->meshID = MODEL_BOX;
        bucket3->mesh->meshID = MODEL_BOX;
        bucket4->mesh->meshID = MODEL_BOX;
    }else if( randomNum == 1){
        *_currentBucket = 1;
        
        godEntity->mesh->material.textureID = TEXTURE_ID_1;
        /*
        bucket1->mesh->material.textureID = TEXTURE_ID_5;
        bucket2->mesh->material.textureID = TEXTURE_ID_1;
        bucket3->mesh->material.textureID = TEXTURE_ID_5;
        bucket4->mesh->material.textureID = TEXTURE_ID_5;
        */
        bucket1->mesh->meshID = MODEL_BOX;
        bucket2->mesh->meshID = MODEL_FOOD;
        bucket3->mesh->meshID = MODEL_BOX;
        bucket4->mesh->meshID = MODEL_BOX;
    }else if( randomNum == 2){
        *_currentBucket = 2;
        
        godEntity->mesh->material.textureID = TEXTURE_ID_2;
        /*
        bucket1->mesh->material.textureID = TEXTURE_ID_5;
        bucket2->mesh->material.textureID = TEXTURE_ID_5;
        bucket3->mesh->material.textureID = TEXTURE_ID_2;
        bucket4->mesh->material.textureID = TEXTURE_ID_5;
        */
        bucket1->mesh->meshID = MODEL_BOX;
        bucket2->mesh->meshID = MODEL_BOX;
        bucket3->mesh->meshID = MODEL_FOOD;
        bucket4->mesh->meshID = MODEL_BOX;
    }else if( randomNum == 3){
        *_currentBucket = 3;
        godEntity->mesh->material.textureID = TEXTURE_ID_3;
        /*
        bucket1->mesh->material.textureID = TEXTURE_ID_5;
        bucket2->mesh->material.textureID = TEXTURE_ID_5;
        bucket3->mesh->material.textureID = TEXTURE_ID_5;
        bucket4->mesh->material.textureID = TEXTURE_ID_3;
        */
        bucket1->mesh->meshID = MODEL_BOX;
        bucket2->mesh->meshID = MODEL_BOX;
        bucket3->mesh->meshID = MODEL_BOX;
        bucket4->mesh->meshID = MODEL_FOOD;
    }
}

// ======== TRIGGERS =========

/*
Scene_OnTrigger
Default collision callback to be used by game entities
*/
void Scene_OnTrigger(AF_Collision* _collision){
    
}

/*
Scene_OnGodTrigger
Callback Behaviour triggered when the player dropps off a sacrafice to the gods
*/
void Scene_OnGodTrigger(AF_Collision* _collision){

    
	//AF_Entity* entity1 =  _collision->entity1;
	AF_Entity* entity2 =  _collision->entity2;
	//uint32_t entity1ID = AF_ECS_GetID(entity1->id_tag);
	//uint32_t entity2ID = AF_ECS_GetID(entity2->id_tag);
	//PACKED_UINT32 entity1Tag = AF_ECS_GetTag(entity1->id_tag);
	//PACKED_UINT32 entity2Tag = AF_ECS_GetTag(entity2->id_tag);
	

    // if entity is carrying, eat and shift the villager into the distance
    if(entity2->playerData->isCarrying == TRUE){
        //debugf("Scene_GodTrigger: eat count %i \n", godEatCount);
        // TODO: update godEatCount. observer pattern would be nice right now
        //godEatCount++;
        
        // update the player who collided with gods score.
        entity2->playerData->score ++;
        

        entity2->playerData->isCarrying = FALSE;
        entity2->playerData->carryingEntity = 0;
        Vec3 poolLocation = {100, 0,0};
        villager1->transform->pos = poolLocation;
        // randomly call for a colour bucket
        Scene_SpawnBucket(&g_currentBucket);
        // play sound
		//AF_Audio_Play(cannonSoundEntity->audioSource, 0.5f, FALSE);

        // clear the players from carrying
        debugf("Scene_OnGoTrigger: tell all the players they are not carrying");
        //playerEntities[0]->playerData->isCarrying = FALSE;
        //playerEntities[1]->playerData->isCarrying = FALSE;
        //playerEntities[2]->playerData->isCarrying = FALSE;
        //playerEntities[3]->playerData->isCarrying = FALSE;
    }
    
	
	// Play sound
	//AF_Audio_Play(cannonSoundEntity->audioSource, 1.0f, FALSE);
	//wav64_play(&sfx_cannon, CHANNEL_SFX1);
    
}


/*
Scene_BucketCollisionBehaviour
Perform gameplay logic if bucket has been collided with by a player character
*/
void Scene_BucketCollisionBehaviour(int _currentBucket, int _bucketID, AF_Collision* _collision, AF_Entity* _villager, AF_Entity* _godEntity){
      
    // Don't react if this bucket isn't activated
    if(_currentBucket != _bucketID){
        return;
    }
	AF_Entity* entity1 =  _collision->entity1;
	AF_Entity* entity2 =  _collision->entity2;
	//uint32_t entity1ID = AF_ECS_GetID(entity1->id_tag);
	//uint32_t entity2ID = AF_ECS_GetID(entity2->id_tag);
	//PACKED_UINT32 entity1Tag = AF_ECS_GetTag(entity1->id_tag);
	//PACKED_UINT32 entity2Tag = AF_ECS_GetTag(entity2->id_tag);
	//debugf("Scene_OnBucketTrigger:  \n");
    // attatch next villager
    //AF_CPlayerData* playerData1 = entity1->playerData;

    // Second collision is the playable character
    AF_CPlayerData* playerData2 = entity2->playerData;
    if(entity2->mesh->meshID == MODEL_FOOD){
        debugf("entity 1 %f %i Entity 2 %i\n", entity2->transform->pos.x, entity1->mesh->meshID, entity2->mesh->meshID);
    }
    
    //if((AF_Component_GetHas(playerData1->enabled) == TRUE) && (AF_Component_GetEnabled(playerData1->enabled) == TRUE)){
        // attatch the villager to this player
        if(_villager->playerData->isCarried == FALSE){
            //debugf("OnBucketTrigger: carry villager \n");
            playerData2->carryingEntity = _villager->id_tag;
            _villager->mesh->material.textureID = _godEntity->mesh->material.textureID;
            playerData2->isCarrying = TRUE;
            // play sound
		    //AF_Audio_Play(laserSoundEntity->audioSource, 0.5f, FALSE);
        }
}

/*
Scene_OnBucketTrigger
Trigger callback assigned to buckets in the game world
*/
void Scene_OnBucket1Trigger(AF_Collision* _collision){
    int bucketID = 0;
    if(g_currentBucket != bucketID){
        return;
    }
    
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket2Trigger(AF_Collision* _collision){
    int bucketID = 1;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket3Trigger(AF_Collision* _collision){
    int bucketID = 2;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket4Trigger(AF_Collision* _collision){
    int bucketID = 3;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}




/*===============
Scene_OnCollision
Default collision call.
================*/
void Scene_OnCollision(AF_Collision* _collision){
	if(_collision == NULL){
		debugf("Game: Scene_OnCollision: passed null collision object\n");
		return;
	}
	if(_collision->entity1 == NULL){
		debugf("Game: Scene_OnCollision: entity 1 is null\n");
		return;
	}
	AF_Entity* entity = (AF_Entity*)_collision->entity1;
	if(entity == NULL){
		debugf("Game: Scene_OnCollision: entity is null\n");
		return;
	}
	AF_CCollider* collider = entity->collider;
	if(collider == NULL){
		debugf("Game: Scene_OnCollision: collider is null\n");
		return;
	}
	// do collision things
}


// ======== GETTERS & SETTERS ==========

AF_Entity* GetVillager(){
    return villager1;
}

AF_Entity* GetGodEntity(){
    return godEntity;
}

/*
AF_Entity* GetPlayerEntity(uint8_t _index){
    if(_index >= PLAYER_COUNT)
    {
        debugf("Scene: GetPlayerEntity: passed index out of range %i \n", _index);
        return NULL;
    }
    return playerEntities[_index];
}*/


AF_Entity* GetBucket1(){
    return bucket1;
}

AF_Entity* GetBucket2(){
    return bucket2;
}

AF_Entity* GetBucket3(){
    return bucket3;
}

AF_Entity* GetBucket4(){
    return bucket4;
}