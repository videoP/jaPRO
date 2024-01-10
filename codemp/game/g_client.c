// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "ghoul2/G2.h"
#include "bg_saga.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
static vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

extern int g_siegeRespawnCheck;

void WP_SaberAddG2Model( gentity_t *saberent, const char *saberModel, qhandle_t saberSkin );
void WP_SaberRemoveG2Model( gentity_t *saberent );
extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
void G_AddPlayerLog(char *name, char *ip, char *guid);

forcedata_t Client_Force[MAX_CLIENTS];

/*QUAKED info_player_duel (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for duelists in duel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_duel1 (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for lone duelists in powerduel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel1( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_duel2 (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for paired duelists in powerduel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel2( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
Targets will be fired when someone spawns in on them.
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_start_red (1 0 0) (-16 -16 -24) (16 16 32) INITIAL
For Red Team DM starts

Targets will be fired when someone spawns in on them.
equivalent to info_player_deathmatch

INITIAL - The first time a player enters the game, they will be at an 'initial' spot.

"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_start_red(gentity_t *ent) {
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_start_blue (1 0 0) (-16 -16 -24) (16 16 32) INITIAL
For Blue Team DM starts

Targets will be fired when someone spawns in on them.
equivalent to info_player_deathmatch

INITIAL - The first time a player enters the game, they will be at an 'initial' spot.

"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_start_blue(gentity_t *ent) {
	SP_info_player_deathmatch( ent );
}

void SiegePointUse( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	//Toggle the point on/off
	if (self->genericValue1)
	{
		self->genericValue1 = 0;
	}
	else
	{
		self->genericValue1 = 1;
	}
}

/*QUAKED info_player_siegeteam1 (1 0 0) (-16 -16 -24) (16 16 32)
siege start point - team1
name and behavior of team1 depends on what is defined in the
.siege file for this level

startoff - if non-0 spawn point will be disabled until used
idealclass - if specified, this spawn point will be considered
"ideal" for players of this class name. Corresponds to the name
entry in the .scl (siege class) file.
Targets will be fired when someone spawns in on them.
*/
void SP_info_player_siegeteam1(gentity_t *ent) {
	int soff = 0;

	if (level.gametype != GT_SIEGE)
	{ //turn into a DM spawn if not in siege game mode
		ent->classname = "info_player_deathmatch";
		SP_info_player_deathmatch( ent );

		return;
	}

	G_SpawnInt("startoff", "0", &soff);

	if (soff)
	{ //start disabled
		ent->genericValue1 = 0;
	}
	else
	{
		ent->genericValue1 = 1;
	}

	ent->use = SiegePointUse;
}

/*QUAKED info_player_siegeteam2 (0 0 1) (-16 -16 -24) (16 16 32)
siege start point - team2
name and behavior of team2 depends on what is defined in the
.siege file for this level

startoff - if non-0 spawn point will be disabled until used
idealclass - if specified, this spawn point will be considered
"ideal" for players of this class name. Corresponds to the name
entry in the .scl (siege class) file.
Targets will be fired when someone spawns in on them.
*/
void SP_info_player_siegeteam2(gentity_t *ent) {
	int soff = 0;

	if (level.gametype != GT_SIEGE)
	{ //turn into a DM spawn if not in siege game mode
		ent->classname = "info_player_deathmatch";
		SP_info_player_deathmatch( ent );

		return;
	}

	G_SpawnInt("startoff", "0", &soff);

	if (soff)
	{ //start disabled
		ent->genericValue1 = 0;
	}
	else
	{
		ent->genericValue1 = 1;
	}

	ent->use = SiegePointUse;
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32) RED BLUE
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
RED - In a Siege game, the intermission will happen here if the Red (attacking) team wins
BLUE - In a Siege game, the intermission will happen here if the Blue (defending) team wins
*/
void SP_info_player_intermission( gentity_t *ent ) {

}

/*QUAKED info_player_intermission_red (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.

In a Siege game, the intermission will happen here if the Red (attacking) team wins
target - ent to look at
target2 - ents to use when this intermission point is chosen
*/
void SP_info_player_intermission_red( gentity_t *ent ) {

}

/*QUAKED info_player_intermission_blue (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.

In a Siege game, the intermission will happen here if the Blue (defending) team wins
target - ent to look at
target2 - ents to use when this intermission point is chosen
*/
void SP_info_player_intermission_blue( gentity_t *ent ) {

}

#define JMSABER_RESPAWN_TIME 20000 //in case it gets stuck somewhere no one can reach

void ThrowSaberToAttacker(gentity_t *self, gentity_t *attacker)
{
	gentity_t *ent = &g_entities[self->client->ps.saberIndex];
	vec3_t a;
	int altVelocity = 0;

	if (!ent || ent->enemy != self)
	{ //something has gone very wrong (this should never happen)
		//but in case it does.. find the saber manually
#ifdef _DEBUG
		Com_Printf("Lost the saber! Attempting to use global pointer..\n");
#endif
		ent = gJMSaberEnt;

		if (!ent)
		{
#ifdef _DEBUG
			Com_Printf("The global pointer was NULL. This is a bad thing.\n");
#endif
			return;
		}

#ifdef _DEBUG
		Com_Printf("Got it (%i). Setting enemy to client %i.\n", ent->s.number, self->s.number);
#endif

		ent->enemy = self;
		self->client->ps.saberIndex = ent->s.number;
	}

	trap->SetConfigstring ( CS_CLIENT_JEDIMASTER, "-1" );

	if (attacker && attacker->client && self->client->ps.saberInFlight)
	{ //someone killed us and we had the saber thrown, so actually move this saber to the saber location
	  //if we killed ourselves with saber thrown, however, same suicide rules of respawning at spawn spot still
	  //apply.
		gentity_t *flyingsaber = &g_entities[self->client->ps.saberEntityNum];

		if (flyingsaber && flyingsaber->inuse)
		{
			VectorCopy(flyingsaber->s.pos.trBase, ent->s.pos.trBase);
			VectorCopy(flyingsaber->s.pos.trDelta, ent->s.pos.trDelta);
			VectorCopy(flyingsaber->s.apos.trBase, ent->s.apos.trBase);
			VectorCopy(flyingsaber->s.apos.trDelta, ent->s.apos.trDelta);

			VectorCopy(flyingsaber->r.currentOrigin, ent->r.currentOrigin);
			VectorCopy(flyingsaber->r.currentAngles, ent->r.currentAngles);
			altVelocity = 1;
		}
	}

	self->client->ps.saberInFlight = qtrue; //say he threw it anyway in order to properly remove from dead body

	WP_SaberAddG2Model( ent, self->client->saber[0].model, self->client->saber[0].skin );

	ent->s.eFlags &= ~(EF_NODRAW);
	ent->s.modelGhoul2 = 1;
	ent->s.eType = ET_MISSILE;
	ent->enemy = NULL;

	if (!attacker || !attacker->client)
	{
		VectorCopy(ent->s.origin2, ent->s.pos.trBase);
		VectorCopy(ent->s.origin2, ent->s.origin);
		VectorCopy(ent->s.origin2, ent->r.currentOrigin);
		ent->pos2[0] = 0;
		trap->LinkEntity((sharedEntity_t *)ent);
		return;
	}

	if (!altVelocity)
	{
		VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
		VectorCopy(self->s.pos.trBase, ent->s.origin);
		VectorCopy(self->s.pos.trBase, ent->r.currentOrigin);

		VectorSubtract(attacker->client->ps.origin, ent->s.pos.trBase, a);

		VectorNormalize(a);

		ent->s.pos.trDelta[0] = a[0]*256;
		ent->s.pos.trDelta[1] = a[1]*256;
		ent->s.pos.trDelta[2] = 256;
	}

	trap->LinkEntity((sharedEntity_t *)ent);
}

void JMSaberThink(gentity_t *ent)
{
	gJMSaberEnt = ent;

	if (ent->enemy)
	{
		if (!ent->enemy->client || !ent->enemy->inuse)
		{ //disconnected?
			VectorCopy(ent->enemy->s.pos.trBase, ent->s.pos.trBase);
			VectorCopy(ent->enemy->s.pos.trBase, ent->s.origin);
			VectorCopy(ent->enemy->s.pos.trBase, ent->r.currentOrigin);
			ent->s.modelindex = G_ModelIndex("models/weapons2/saber/saber_w.glm");
			ent->s.eFlags &= ~(EF_NODRAW);
			ent->s.modelGhoul2 = 1;
			ent->s.eType = ET_MISSILE;
			ent->enemy = NULL;

			ent->pos2[0] = 1;
			ent->pos2[1] = 0; //respawn next think
			trap->LinkEntity((sharedEntity_t *)ent);
		}
		else
		{
			ent->pos2[1] = level.time + JMSABER_RESPAWN_TIME;
		}
	}
	else if (ent->pos2[0] && ent->pos2[1] < level.time)
	{
		VectorCopy(ent->s.origin2, ent->s.pos.trBase);
		VectorCopy(ent->s.origin2, ent->s.origin);
		VectorCopy(ent->s.origin2, ent->r.currentOrigin);
		ent->pos2[0] = 0;
		trap->LinkEntity((sharedEntity_t *)ent);
	}

	ent->nextthink = level.time + 50;
	G_RunObject(ent);
}

void JMSaberTouch(gentity_t *self, gentity_t *other, trace_t *trace)
{
	int i = 0;
//	gentity_t *te;

	if (!other || !other->client || other->health < 1)
	{
		return;
	}

	if (self->enemy)
	{
		return;
	}

	if (!self->s.modelindex)
	{
		return;
	}

	if (other->client->ps.stats[STAT_WEAPONS] & (1 << WP_SABER))
	{
		return;
	}

	if (other->client->ps.isJediMaster)
	{
		return;
	}

	self->enemy = other;
	other->client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
	other->client->ps.weapon = WP_SABER;
	other->s.weapon = WP_SABER;
	other->client->ps.zoomMode = 0;
	G_AddEvent(other, EV_BECOME_JEDIMASTER, 0);

	// Track the jedi master 
	trap->SetConfigstring ( CS_CLIENT_JEDIMASTER, va("%i", other->s.number ) );

	if (g_spawnInvulnerability.integer)
	{
		other->client->ps.eFlags |= EF_INVULNERABLE;
		other->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
	}

	trap->SendServerCommand( -1, va("cp \"%s %s\n\"", other->client->pers.netname, G_GetStringEdString("MP_SVGAME", "BECOMEJM")) );

	other->client->ps.isJediMaster = qtrue;
	other->client->ps.saberIndex = self->s.number;

	if (other->health < 200 && other->health > 0)
	{ //full health when you become the Jedi Master
		other->client->ps.stats[STAT_HEALTH] = other->health = 200;
	}

	if (other->client->ps.fd.forcePower < 100)
	{
		other->client->ps.fd.forcePower = 100;
	}

	while (i < NUM_FORCE_POWERS)
	{
		other->client->ps.fd.forcePowersKnown |= (1 << i);
		other->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;

		i++;
	}

	self->pos2[0] = 1;
	self->pos2[1] = level.time + JMSABER_RESPAWN_TIME;

	self->s.modelindex = 0;
	self->s.eFlags |= EF_NODRAW;
	self->s.modelGhoul2 = 0;
	self->s.eType = ET_GENERAL;

	/*
	te = G_TempEntity( vec3_origin, EV_DESTROY_GHOUL2_INSTANCE );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = self->s.number;
	*/
	G_KillG2Queue(self->s.number);

	return;
}

gentity_t *gJMSaberEnt = NULL;

/*QUAKED info_jedimaster_start (1 0 0) (-16 -16 -24) (16 16 32)
"jedi master" saber spawn point
*/
void SP_info_jedimaster_start(gentity_t *ent)
{
	if (level.gametype != GT_JEDIMASTER)
	{
		gJMSaberEnt = NULL;
		G_FreeEntity(ent);
		return;
	}

	ent->enemy = NULL;

	ent->flags = FL_BOUNCE_HALF;

	ent->s.modelindex = G_ModelIndex("models/weapons2/saber/saber_w.glm");
	ent->s.modelGhoul2 = 1;
	ent->s.g2radius = 20;
	//ent->s.eType = ET_GENERAL;
	ent->s.eType = ET_MISSILE;
	ent->s.weapon = WP_SABER;
	ent->s.pos.trType = TR_GRAVITY;
	ent->s.pos.trTime = level.time;
	VectorSet( ent->r.maxs, 3, 3, 3 );
	VectorSet( ent->r.mins, -3, -3, -3 );
	ent->r.contents = CONTENTS_TRIGGER;
	ent->clipmask = MASK_SOLID;

	ent->isSaberEntity = qtrue;

	ent->bounceCount = -5;

	ent->physicsObject = qtrue;

	VectorCopy(ent->s.pos.trBase, ent->s.origin2); //remember the spawn spot

	ent->touch = JMSaberTouch;

	trap->LinkEntity((sharedEntity_t *)ent);

	ent->think = JMSaberThink;
	ent->nextthink = level.time + 50;
}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client) {
			return qtrue;
		}

	}

	return qfalse;
}

qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest ) 
{
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( dest, mover->r.mins, mins );
	VectorAdd( dest, mover->r.maxs, maxs );
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) 
	{
		hit = &g_entities[touch[i]];
		if ( hit == mover )
		{
			continue;
		}

		if ( hit->r.contents & mover->r.contents )
		{
			return qtrue;
		}
	}

	return qfalse;
}

qboolean SpotWouldTelefrag3( vec3_t spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot, playerMins, mins );
	VectorAdd( spot, playerMaxs, maxs );
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
			return qtrue;
		}
		if (hit->r.contents & CONTENTS_LAVA || hit->r.contents & CONTENTS_NODROP) { //assume only pits are contents_nodrop ?
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( qboolean isbot ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL && count < MAX_SPAWN_POINTS) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	//in Team DM, look for a team start spot first, if any
	if ( level.gametype == GT_TEAM 
		&& team != TEAM_FREE 
		&& team != TEAM_SPECTATOR )
	{
		char *classname = NULL;
		if ( team == TEAM_RED )
		{
			classname = "info_player_start_red";
		}
		else
		{
			classname = "info_player_start_blue";
		}
		while ((spot = G_Find (spot, FOFS(classname), classname)) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}

			if(((spot->flags & FL_NO_BOTS) && isbot) ||
			   ((spot->flags & FL_NO_HUMANS) && !isbot))
			{
				// spot is not for this human/bot player
				continue;
			}

			VectorSubtract( spot->s.origin, avoidPoint, delta );
			dist = VectorLength( delta );
			for (i = 0; i < numSpots; i++) {
				if ( dist > list_dist[i] ) {
					if ( numSpots >= MAX_SPAWN_POINTS )
						numSpots = MAX_SPAWN_POINTS-1;
					for (j = numSpots; j > i; j--) {
						list_dist[j] = list_dist[j-1];
						list_spot[j] = list_spot[j-1];
					}
					list_dist[i] = dist;
					list_spot[i] = spot;
					numSpots++;
					break;
				}
			}
			if (i >= numSpots && numSpots < MAX_SPAWN_POINTS) {
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
	}

	if ( !numSpots )
	{//couldn't find any of the above
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}

			if(((spot->flags & FL_NO_BOTS) && isbot) ||
			   ((spot->flags & FL_NO_HUMANS) && !isbot))
			{
				// spot is not for this human/bot player
				continue;
			}

			VectorSubtract( spot->s.origin, avoidPoint, delta );
			dist = VectorLength( delta );
			for (i = 0; i < numSpots; i++) {
				if ( dist > list_dist[i] ) {
					if ( numSpots >= MAX_SPAWN_POINTS )
						numSpots = MAX_SPAWN_POINTS-1;
					for (j = numSpots; j > i; j--) {
						list_dist[j] = list_dist[j-1];
						list_spot[j] = list_spot[j-1];
					}
					list_dist[i] = dist;
					list_spot[i] = spot;
					numSpots++;
					break;
				}
			}
			if (i >= numSpots && numSpots < MAX_SPAWN_POINTS) {
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
		if (!numSpots) {
			spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
			if (!spot)
				trap->Error( ERR_DROP, "Couldn't find a spawn point" );
			VectorCopy (spot->s.origin, origin);
			origin[2] += 9;
			VectorCopy (spot->s.angles, angles);
			return spot;
		}
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

gentity_t *SelectDuelSpawnPoint( int team, vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot )
{
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
	int			numSpots, rnd, i, j;
	char		*spotName;

	if (team == DUELTEAM_LONE)
	{
		spotName = "info_player_duel1";
	}
	else if (team == DUELTEAM_DOUBLE)
	{
		spotName = "info_player_duel2";
	}
	else if (team == DUELTEAM_SINGLE)
	{
		spotName = "info_player_duel";
	}
	else
	{
		spotName = "info_player_deathmatch";
	}
tryAgain:

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), spotName)) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
			((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );
		for (i = 0; i < numSpots; i++) {
			if ( dist > list_dist[i] ) {
				if ( numSpots >= MAX_SPAWN_POINTS )
					numSpots = MAX_SPAWN_POINTS-1;
				for (j = numSpots; j > i; j--) {
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				break;
			}
		}
		if (i >= numSpots && numSpots < MAX_SPAWN_POINTS) {
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if (!numSpots)
	{
		if (Q_stricmp(spotName, "info_player_deathmatch"))
		{ //try the loop again with info_player_deathmatch as the target if we couldn't find a duel spot
			spotName = "info_player_deathmatch";
			goto tryAgain;
		}

		//If we got here we found no free duel or DM spots, just try the first DM spot
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			trap->Error( ERR_DROP, "Couldn't find a spawn point" );
		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, team, isbot );

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}		
	}

	// find a single player start spot
	if (!spot) {
		trap->Error( ERR_DROP, "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	gentity_t	*spot;

	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			continue;
		}

		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles, team, isbot );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
=======================================================================

BODYQUE

=======================================================================
*/

//[JAPRO - Serverside - All - Corpse Removal Time]
#define BODY_SINK_TIME		1000 * g_corpseRemovalTime.integer

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn(qtrue);
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and disappear
=============
*/
extern void BodyRid(gentity_t *ent);
void BodySink( gentity_t *ent ) {
	/*
	if ( level.time - ent->timestamp > BODY_SINK_TIME + 2500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap->UnlinkEntity( (sharedEntity_t *)ent );
		ent->physicsObject = qfalse;
		return;	
	}
//	ent->nextthink = level.time + 100;
//	ent->s.pos.trBase[2] -= 1;

	G_AddEvent(ent, EV_BODYFADE, 0);
	ent->nextthink = level.time + 18000;
	*/

	G_AddEvent(ent, EV_BODYFADE, 69);
	ent->think = BodyRid;
	ent->nextthink = level.time + 4000;
	ent->takedamage = qfalse;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
static qboolean CopyToBodyQue( gentity_t *ent ) {
	gentity_t		*body;
	int			contents;
	int			islight = 0;

	if (level.intermissiontime)
	{
		return qfalse;
	}

	trap->UnlinkEntity ((sharedEntity_t *)ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap->PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return qfalse;
	}

	if (ent->client && ((ent->client->ps.eFlags & EF_DISINTEGRATION) || (ent->client->ps.eFlags & EF_NODRAW)))
	{ //for now, just don't spawn a body if you got disint'd
		return qfalse;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap->UnlinkEntity ((sharedEntity_t *)body);
	body->s = ent->s;

	//avoid oddly angled corpses floating around
	body->s.angles[PITCH] = body->s.angles[ROLL] = body->s.apos.trBase[PITCH] = body->s.apos.trBase[ROLL] = 0;

	body->s.g2radius = 100;

	body->s.eType = ET_BODY;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc

	if (ent->client && (ent->client->ps.eFlags & EF_DISINTEGRATION))
	{
		body->s.eFlags |= EF_DISINTEGRATION;
	}

	VectorCopy(ent->client->ps.lastHitLoc, body->s.origin2);

	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.loopIsSoundset = qfalse;
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	body->s.weapon = ent->s.bolt2;

	if (body->s.weapon == WP_SABER && ent->client->ps.saberInFlight)
	{
		body->s.weapon = WP_BLASTER; //lie to keep from putting a saber on the corpse, because it was thrown at death
	}

	//G_AddEvent(body, EV_BODY_QUEUE_COPY, ent->s.clientNum);
	//Now doing this through a modified version of the rcg reliable command.
	if (ent->client && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
	{
		islight = 1;
	}
	trap->SendServerCommand(-1, va("ircg %i %i %i %i", ent->s.number, body->s.number, body->s.weapon, islight));

	body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->s.torsoAnim = body->s.legsAnim = ent->client->ps.legsAnim;

	body->s.customRGBA[0] = ent->client->ps.customRGBA[0];
	body->s.customRGBA[1] = ent->client->ps.customRGBA[1];
	body->s.customRGBA[2] = ent->client->ps.customRGBA[2];
	body->s.customRGBA[3] = ent->client->ps.customRGBA[3];

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + BODY_SINK_TIME;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}

	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap->LinkEntity ((sharedEntity_t *)body);

	return qtrue;
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

void MaintainBodyQueue(gentity_t *ent)
{ //do whatever should be done taking ragdoll and dismemberment states into account.
	qboolean doRCG = qfalse;

	assert(ent && ent->client);
	if (ent->client->tempSpectate > level.time ||
		(ent->client->ps.eFlags2 & EF2_SHIP_DEATH))
	{
		ent->client->noCorpse = qtrue;
	}

	if (!ent->client->noCorpse && !ent->client->ps.fallingToDeath)
	{
		if (!CopyToBodyQue (ent))
		{
			doRCG = qtrue;
		}
	}
	else
	{
		ent->client->noCorpse = qfalse; //clear it for next time
		ent->client->ps.fallingToDeath = qfalse;
		doRCG = qtrue;
	}

	if (doRCG)
	{ //bodyque func didn't manage to call ircg so call this to assure our limbs and ragdoll states are proper on the client.
		trap->SendServerCommand(-1, va("rcg %i", ent->s.clientNum));
	}
}

/*
================
ClientRespawn
================
*/
void SiegeRespawn(gentity_t *ent);
void ClientRespawn( gentity_t *ent ) {

	MaintainBodyQueue(ent);

	if (gEscaping || level.gametype == GT_POWERDUEL)
	{
		ent->client->sess.sessionTeam = TEAM_SPECTATOR;
		ent->client->sess.spectatorState = SPECTATOR_FREE;
		ent->client->sess.spectatorClient = 0;

		ent->client->pers.teamState.state = TEAM_BEGIN;
		AddTournamentQueue(ent->client);
		ClientSpawn(ent);
		ent->client->iAmALoser = qtrue;
		return;
	}

	trap->UnlinkEntity ((sharedEntity_t *)ent);

	if (level.gametype == GT_SIEGE)
	{
		if (g_siegeRespawn.integer)
		{
			if (ent->client->tempSpectate <= level.time)
			{
				int minDel = g_siegeRespawn.integer* 2000;
				if (minDel < 20000)
				{
					minDel = 20000;
				}
				ent->client->tempSpectate = level.time + minDel;
				ent->health = ent->client->ps.stats[STAT_HEALTH] = 1;
				ent->waterlevel = ent->watertype = 0;
				ent->client->ps.weapon = WP_NONE;
				ent->client->ps.stats[STAT_WEAPONS] = 0;
				ent->client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
				ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
				ent->takedamage = qfalse;
				trap->LinkEntity((sharedEntity_t *)ent);

				// Respawn time.
				if ( ent->s.number < MAX_CLIENTS )
				{
					gentity_t *te = G_TempEntity( ent->client->ps.origin, EV_SIEGESPEC );
					te->s.time = g_siegeRespawnCheck;
					te->s.owner = ent->s.number;
				}

				return;
			}
		}
		SiegeRespawn(ent);
	}
	else
	{
		ClientSpawn(ent);
	}
}

/*
================
TeamCount

Returns number of players on a team
================
*/
int TeamCount( int ignoreClientNum, team_t team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
		else if (level.gametype == GT_SIEGE &&
            level.clients[i].sess.siegeDesiredTeam == team)
		{
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			if ( level.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize )
{
	int outpos = 0, colorlessLen = 0, spaces = 0, ats = 0;

	// discard leading spaces
	for ( ; *in == ' '; in++);

	// discard leading asterisk's (fail raven for using * as a skipnotify)
	// apparently .* causes the issue too so... derp
	for(; *in == '*'; in++);
	
	for(; *in && outpos < outSize - 1; in++)
	{
		out[outpos] = *in;

		if ( *in == ' ' )
		{// don't allow too many consecutive spaces
			if ( spaces > 2 )
				continue;

			spaces++;
		}
		else if ( *in == '@' )
		{// don't allow too many consecutive at signs
			if ( ++ats > 2 ) {
				outpos -= 2;
				ats = 0;
				continue;
			}
		}
		else if ( (byte)*in < 0x20
				|| (byte)*in == 0x81 || (byte)*in == 0x8D || (byte)*in == 0x8F || (byte)*in == 0x90 || (byte)*in == 0x9D
				|| (byte)*in == 0xA0 || (byte)*in == 0xAD )
		{
			continue;
		}
		else if ( outpos > 0 && out[outpos-1] == Q_COLOR_ESCAPE )
		{
			if ( Q_IsColorStringExt( &out[outpos-1] ) )
			{
				colorlessLen--;
				
#if 0
				if ( ColorIndex( *in ) == 0 )
				{// Disallow color black in names to prevent players from getting advantage playing in front of black backgrounds
					outpos--;
					continue;
				}
#endif
			}
			else
			{
				spaces = ats = 0;
				colorlessLen++;
			}
		}
		else
		{
			spaces = ats = 0;
			colorlessLen++;
		}
		
		outpos++;
	}

	out[outpos] = '\0';

	// don't allow empty names
	if ( *out == '\0' || colorlessLen == 0 )
		Q_strncpyz( out, "Padawan", outSize );
}

#ifdef _DEBUG
void G_DebugWrite(const char *path, const char *text)
{
	fileHandle_t f;

	trap->FS_Open( path, &f, FS_APPEND );
	trap->FS_Write(text, strlen(text), f);
	trap->FS_Close(f);
}
#endif

qboolean G_SaberModelSetup(gentity_t *ent)
{
	int i = 0;
	qboolean fallbackForSaber = qtrue;

	while (i < MAX_SABERS)
	{
		if (ent->client->saber[i].model[0])
		{
			//first kill it off if we've already got it
			if (ent->client->weaponGhoul2[i])
			{
				trap->G2API_CleanGhoul2Models(&(ent->client->weaponGhoul2[i]));
			}
			trap->G2API_InitGhoul2Model(&ent->client->weaponGhoul2[i], ent->client->saber[i].model, 0, 0, -20, 0, 0);

			if (ent->client->weaponGhoul2[i])
			{
				int j = 0;
				char *tagName;
				int tagBolt;

				if (ent->client->saber[i].skin)
				{
					trap->G2API_SetSkin(ent->client->weaponGhoul2[i], 0, ent->client->saber[i].skin, ent->client->saber[i].skin);
				}

				if (ent->client->saber[i].saberFlags & SFL_BOLT_TO_WRIST)
				{
					trap->G2API_SetBoltInfo(ent->client->weaponGhoul2[i], 0, 3+i);
				}
				else
				{ // bolt to right hand for 0, or left hand for 1
					trap->G2API_SetBoltInfo(ent->client->weaponGhoul2[i], 0, i);
				}

				//Add all the bolt points
				while (j < ent->client->saber[i].numBlades)
				{
					tagName = va("*blade%i", j+1);
					tagBolt = trap->G2API_AddBolt(ent->client->weaponGhoul2[i], 0, tagName);

					if (tagBolt == -1)
					{
						if (j == 0)
						{ //guess this is an 0ldsk3wl saber
							tagBolt = trap->G2API_AddBolt(ent->client->weaponGhoul2[i], 0, "*flash");
							fallbackForSaber = qfalse;
							break;
						}

						if (tagBolt == -1)
						{
							assert(0);
							break;

						}
					}
					j++;

					fallbackForSaber = qfalse; //got at least one custom saber so don't need default
				}

				//Copy it into the main instance
				trap->G2API_CopySpecificGhoul2Model(ent->client->weaponGhoul2[i], 0, ent->ghoul2, i+1); 
			}
		}
		else
		{
			break;
		}

		i++;
	}

	return fallbackForSaber;
}

/*
===========
SetupGameGhoul2Model

There are two ghoul2 model instances per player (actually three).  One is on the clientinfo (the base for the client side 
player, and copied for player spawns and for corpses).  One is attached to the centity itself, which is the model acutally 
animated and rendered by the system.  The final is the game ghoul2 model.  This is animated by pmove on the server, and
is used for determining where the lightsaber should be, and for per-poly collision tests.
===========
*/
void *g2SaberInstance = NULL;

qboolean BG_IsValidCharacterModel(const char *modelName, const char *skinName);
qboolean BG_ValidateSkinForTeam( const char *modelName, char *skinName, int team, float *colors );
void BG_GetVehicleModelName(char *modelname, int len);

void SetupGameGhoul2Model(gentity_t *ent, char *modelname, char *skinName)
{
	int handle;
	char		afilename[MAX_QPATH];
#if 0
	char		/**GLAName,*/ *slash;
#endif
	char		GLAName[MAX_QPATH];
	vec3_t	tempVec = {0,0,0};

#if 0 //HELLO?
	if (strlen(modelname) >= MAX_QPATH )
	{
		Com_Error( ERR_FATAL, "SetupGameGhoul2Model(%s): modelname exceeds MAX_QPATH.\n", modelname );
	}
	if (skinName && strlen(skinName) >= MAX_QPATH )
	{
		Com_Error( ERR_FATAL, "SetupGameGhoul2Model(%s): skinName exceeds MAX_QPATH.\n", skinName );
	}
#endif

	// First things first.  If this is a ghoul2 model, then let's make sure we demolish this first.
	if (ent->ghoul2 && trap->G2API_HaveWeGhoul2Models(ent->ghoul2))
	{
		trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
	}

	//rww - just load the "standard" model for the server"
	if (!precachedKyle)
	{
		int defSkin;

		Com_sprintf( afilename, sizeof( afilename ), "models/players/kyle/model.glm" );
		handle = trap->G2API_InitGhoul2Model(&precachedKyle, afilename, 0, 0, -20, 0, 0);

		if (handle<0)
		{
			return;
		}

		defSkin = trap->R_RegisterSkin("models/players/kyle/model_default.skin");
		trap->G2API_SetSkin(precachedKyle, 0, defSkin, defSkin);
	}

	if (precachedKyle && trap->G2API_HaveWeGhoul2Models(precachedKyle))
	{
		if (d_perPlayerGhoul2.integer || ent->s.number >= MAX_CLIENTS ||
			G_PlayerHasCustomSkeleton(ent))
		{ //rww - allow option for perplayer models on server for collision and bolt stuff.
			char modelFullPath[MAX_QPATH];
			char truncModelName[MAX_QPATH];
			char skin[MAX_QPATH];
			char vehicleName[MAX_QPATH];
			int skinHandle = 0;
			int i = 0;
			char *p;

			// If this is a vehicle, get it's model name.
			if ( ent->client->NPC_class == CLASS_VEHICLE )
			{
				Q_strncpyz( vehicleName, modelname, sizeof( vehicleName ) );
				BG_GetVehicleModelName(modelname, strlen( modelname ));
				strcpy(truncModelName, modelname);
				skin[0] = 0;
				if ( ent->m_pVehicle
					&& ent->m_pVehicle->m_pVehicleInfo
					&& ent->m_pVehicle->m_pVehicleInfo->skin
					&& ent->m_pVehicle->m_pVehicleInfo->skin[0] )
				{
					skinHandle = trap->R_RegisterSkin(va("models/players/%s/model_%s.skin", modelname, ent->m_pVehicle->m_pVehicleInfo->skin));
				}
				else
				{
					skinHandle = trap->R_RegisterSkin(va("models/players/%s/model_default.skin", modelname));
				}
			}
			else
			{
				if (skinName && skinName[0])
				{
					strcpy(skin, skinName);
					strcpy(truncModelName, modelname);
				}
				else
				{
					strcpy(skin, "default");

					strcpy(truncModelName, modelname);
					p = Q_strrchr(truncModelName, '/');

					if (p)
					{
						*p = 0;
						p++;

						while (p && *p)
						{
							skin[i] = *p;
							i++;
							p++;
						}
						skin[i] = 0;
						i = 0;
					}

					if (!BG_IsValidCharacterModel(truncModelName, skin))
					{
						strcpy(truncModelName, DEFAULT_MODEL);
						strcpy(skin, "default");
					}

					if ( level.gametype >= GT_TEAM && level.gametype != GT_SIEGE && !g_jediVmerc.integer )
					{
						float colorOverride[3];

						colorOverride[0] = colorOverride[1] = colorOverride[2] = 0.0f;

						BG_ValidateSkinForTeam( truncModelName, skin, ent->client->sess.sessionTeam, colorOverride);
						if (colorOverride[0] != 0.0f ||
							colorOverride[1] != 0.0f ||
							colorOverride[2] != 0.0f)
						{
							ent->client->ps.customRGBA[0] = colorOverride[0]*255.0f;
							ent->client->ps.customRGBA[1] = colorOverride[1]*255.0f;
							ent->client->ps.customRGBA[2] = colorOverride[2]*255.0f;
						}

						//BG_ValidateSkinForTeam( truncModelName, skin, ent->client->sess.sessionTeam, NULL );
					}
					else if (level.gametype == GT_SIEGE)
					{ //force skin for class if appropriate
						if (ent->client->siegeClass != -1)
						{
							siegeClass_t *scl = &bgSiegeClasses[ent->client->siegeClass];
							if (scl->forcedSkin[0])
							{
								Q_strncpyz( skin, scl->forcedSkin, sizeof( skin ) );
							}
						}
					}
				}
			}

			if (skin[0])
			{
				char *useSkinName;

				if (strchr(skin, '|'))
				{//three part skin
					useSkinName = va("models/players/%s/|%s", truncModelName, skin);
				}
				else
				{
					useSkinName = va("models/players/%s/model_%s.skin", truncModelName, skin);
				}

				skinHandle = trap->R_RegisterSkin(useSkinName);
			}

			strcpy(modelFullPath, va("models/players/%s/model.glm", truncModelName));
			handle = trap->G2API_InitGhoul2Model(&ent->ghoul2, modelFullPath, 0, skinHandle, -20, 0, 0);

			if (handle<0)
			{ //Huh. Guess we don't have this model. Use the default.

				if (ent->ghoul2 && trap->G2API_HaveWeGhoul2Models(ent->ghoul2))
				{
					trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
				}
				ent->ghoul2 = NULL;
				trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
			}
			else
			{
				trap->G2API_SetSkin(ent->ghoul2, 0, skinHandle, skinHandle);

				GLAName[0] = 0;
				trap->G2API_GetGLAName( ent->ghoul2, 0, GLAName);

				if (!GLAName[0] || (!strstr(GLAName, "players/_humanoid/") && ent->s.number < MAX_CLIENTS && !G_PlayerHasCustomSkeleton(ent)))
				{ //a bad model
					trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
					ent->ghoul2 = NULL;
					trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
				}

				if (ent->s.number >= MAX_CLIENTS)
				{
					ent->s.modelGhoul2 = 1; //so we know to free it on the client when we're removed.

					if (skin[0])
					{ //append it after a *
						strcat( modelFullPath, va("*%s", skin) );
					}

					if ( ent->client->NPC_class == CLASS_VEHICLE )
					{ //vehicles are tricky and send over their vehicle names as the model (the model is then retrieved based on the vehicle name)
						ent->s.modelindex = G_ModelIndex(vehicleName);
					}
					else
					{
						ent->s.modelindex = G_ModelIndex(modelFullPath);
					}
				}
			}
		}
		else
		{
			trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
		}
	}
	else
	{
		return;
	}

	//Attach the instance to this entity num so we can make use of client-server
	//shared operations if possible.
	trap->G2API_AttachInstanceToEntNum(ent->ghoul2, ent->s.number, qtrue);

	// The model is now loaded.

	GLAName[0] = 0;

	if (!BGPAFtextLoaded)
	{
		if (BG_ParseAnimationFile("models/players/_humanoid/animation.cfg", bgHumanoidAnimations, qtrue) == -1)
		{
			Com_Printf( "Failed to load humanoid animation file\n");
			return;
		}
	}

	if (ent->s.number >= MAX_CLIENTS || G_PlayerHasCustomSkeleton(ent))
	{
		ent->localAnimIndex = -1;

		GLAName[0] = 0;
		trap->G2API_GetGLAName(ent->ghoul2, 0, GLAName);

		if (GLAName[0] &&
			!strstr(GLAName, "players/_humanoid/") /*&&
			!strstr(GLAName, "players/rockettrooper/")*/)
		{ //it doesn't use humanoid anims.
			char *slash = Q_strrchr( GLAName, '/' );
			if ( slash )
			{
				strcpy(slash, "/animation.cfg");

				ent->localAnimIndex = BG_ParseAnimationFile(GLAName, NULL, qfalse);
			}
		}
		else
		{ //humanoid index.
			if (strstr(GLAName, "players/rockettrooper/"))
			{
				ent->localAnimIndex = 1;
			}
			else
			{
				ent->localAnimIndex = 0;
			}
		}

		if (ent->localAnimIndex == -1)
		{
			Com_Error(ERR_DROP, "NPC had an invalid GLA\n");
		}
	}
	else
	{
		GLAName[0] = 0;
		trap->G2API_GetGLAName(ent->ghoul2, 0, GLAName);

		if (strstr(GLAName, "players/rockettrooper/"))
		{
			//assert(!"Should not have gotten in here with rockettrooper skel");
			ent->localAnimIndex = 1;
		}
		else
		{
			ent->localAnimIndex = 0;
		}
	}

	if (ent->s.NPC_class == CLASS_VEHICLE &&
		ent->m_pVehicle)
	{ //do special vehicle stuff
		char strTemp[128];
		int i;

		// Setup the default first bolt
		i = trap->G2API_AddBolt( ent->ghoul2, 0, "model_root" );

		// Setup the droid unit.
		ent->m_pVehicle->m_iDroidUnitTag = trap->G2API_AddBolt( ent->ghoul2, 0, "*droidunit" );

		// Setup the Exhausts.
		for ( i = 0; i < MAX_VEHICLE_EXHAUSTS; i++ )
		{
			Com_sprintf( strTemp, 128, "*exhaust%i", i + 1 );
			ent->m_pVehicle->m_iExhaustTag[i] = trap->G2API_AddBolt( ent->ghoul2, 0, strTemp );
		}

		// Setup the Muzzles.
		for ( i = 0; i < MAX_VEHICLE_MUZZLES; i++ )
		{
			Com_sprintf( strTemp, 128, "*muzzle%i", i + 1 );
			ent->m_pVehicle->m_iMuzzleTag[i] = trap->G2API_AddBolt( ent->ghoul2, 0, strTemp );
			if ( ent->m_pVehicle->m_iMuzzleTag[i] == -1 )
			{//ergh, try *flash?
				Com_sprintf( strTemp, 128, "*flash%i", i + 1 );
				ent->m_pVehicle->m_iMuzzleTag[i] = trap->G2API_AddBolt( ent->ghoul2, 0, strTemp );
			}
		}

		// Setup the Turrets.
		for ( i = 0; i < MAX_VEHICLE_TURRET_MUZZLES; i++ )
		{
			if ( ent->m_pVehicle->m_pVehicleInfo->turret[i].gunnerViewTag )
			{
				ent->m_pVehicle->m_iGunnerViewTag[i] = trap->G2API_AddBolt( ent->ghoul2, 0, ent->m_pVehicle->m_pVehicleInfo->turret[i].gunnerViewTag );
			}
			else
			{
				ent->m_pVehicle->m_iGunnerViewTag[i] = -1;
			}
		}
	}
	
	if (ent->client->ps.weapon == WP_SABER || ent->s.number < MAX_CLIENTS)
	{ //a player or NPC saber user
		trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand");
		trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand");

		//rhand must always be first bolt. lhand always second. Whichever you want the
		//jetpack bolted to must always be third.
		trap->G2API_AddBolt(ent->ghoul2, 0, "*chestg");

		//claw bolts
		trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand_cap_r_arm");
		trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand_cap_l_arm");

		trap->G2API_SetBoneAnim(ent->ghoul2, 0, "model_root", 0, 12, BONE_ANIM_OVERRIDE_LOOP, 1.0f, level.time, -1, -1);
		trap->G2API_SetBoneAngles(ent->ghoul2, 0, "upper_lumbar", tempVec, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, NULL, 0, level.time);
		trap->G2API_SetBoneAngles(ent->ghoul2, 0, "cranium", tempVec, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, NULL, 0, level.time);

		if (!g2SaberInstance)
		{
			trap->G2API_InitGhoul2Model(&g2SaberInstance, "models/weapons2/saber/saber_w.glm", 0, 0, -20, 0, 0);

			if (g2SaberInstance)
			{
				// indicate we will be bolted to model 0 (ie the player) on bolt 0 (always the right hand) when we get copied
				trap->G2API_SetBoltInfo(g2SaberInstance, 0, 0);
				// now set up the gun bolt on it
				trap->G2API_AddBolt(g2SaberInstance, 0, "*blade1");
			}
		}

		if (G_SaberModelSetup(ent))
		{
			if (g2SaberInstance)
			{
				trap->G2API_CopySpecificGhoul2Model(g2SaberInstance, 0, ent->ghoul2, 1); 
			}
		}
	}

	if (ent->s.number >= MAX_CLIENTS)
	{ //some extra NPC stuff
		if (trap->G2API_AddBolt(ent->ghoul2, 0, "lower_lumbar") == -1)
		{ //check now to see if we have this bone for setting anims and such
			ent->noLumbar = qtrue;
		}
	}
}

//JAPRO - Serverside - Same player names fix - Start
/*
static QINLINE qboolean IsColorCode(const char *s) {
	return (qboolean) (*s == '^' && '0' <= *(s+1) && *(s+1) <= '9');
}
*/

/*
static void StripColors(const char *s, char *out) {
	while (*s) {
		if (IsColorCode(s)) s += 2;
		else {
			*out = *s;
			out += 1;
			s += 1;
		}
	}
	*out = 0;
}
*/

void StripWhitespace(char *s)
{
  char* i = s;
  char* j = s;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}

static qboolean ClientNameEquals(const char *n1, const char *n2) {
	char o1[MAX_NETNAME];
	char o2[MAX_NETNAME];
	//StripColors(n1, o1);
	//StripColors(n2, o2);
	Q_strncpyz(o1, n1, MAX_NETNAME);
	Q_strncpyz(o2, n2, MAX_NETNAME);

	Q_CleanStr(o1);
	Q_CleanStr(o2);
	StripWhitespace(o1);
	StripWhitespace(o2);

	return (qboolean) (Q_stricmp(o1, o2) == 0);
}

static qboolean ClientNameInUse(const char *netname, int ignore) {
	int i;
	for (i = 0; i < level.maxclients; ++i) {
		if (i != ignore
			&& level.clients[i].pers.connected != CON_DISCONNECTED
			&& ClientNameEquals(level.clients[i].pers.netname, netname)) {
				return qtrue;
		}
	}
	return qfalse;
}

static void CheckDuplicateName(int clientNum) {
	char *netname = level.clients[clientNum].pers.netname;
	//char userinfo[MAX_INFO_STRING]; 
	if (ClientNameInUse(netname, clientNum)) {
		char bufname[MAX_NETNAME];
		int num = 0;
		int len = strlen(netname);
		// make room for a number
		// assuming a number will be 2 digits in length
		// format will be like "%s ^7[%d]"

		if (len > 28) netname[28] = 0;
		do {
			// move the new name to the buffer, since we shouldn't read and write in same location
			Com_sprintf(bufname, MAX_NETNAME, "%s^7[%d]", netname, num);
		} 
		while (num++ < level.maxclients && ClientNameInUse(bufname, clientNum));
		{
			char	userinfo[MAX_INFO_STRING]; 

			Q_strncpyz(netname, bufname, MAX_NETNAME);// move bufname back to netname
			trap->GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
			Info_SetValueForKey(userinfo, "name", bufname);
			trap->SetUserinfo(clientNum, userinfo); 
			ClientUserinfoChanged(clientNum); 
		}


	}
}
//JAPRO - Serverside - Same player names fix - End




/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap->SetUserinfo
if desired.
============
*/

qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride);
void G_ValidateSiegeClassForTeam(gentity_t *ent, int team);

typedef struct userinfoValidate_s {
	const char		*field, *fieldClean;
	unsigned int	minCount, maxCount;
} userinfoValidate_t;

#define UIF( x, _min, _max ) { STRING(\\) #x STRING(\\), STRING( x ), _min, _max }
static userinfoValidate_t userinfoFields[] = {
	UIF( cl_guid,			0, 0 ), // not allowed, q3fill protection
	UIF( cl_punkbuster,		0, 0 ), // not allowed, q3fill protection
	UIF( ip,				0, 1 ), // engine adds this at the end
	UIF( name,				1, 1 ),
	UIF( rate,				1, 1 ),
	UIF( snaps,				1, 1 ),
	UIF( model,				1, 1 ),
	UIF( forcepowers,		1, 1 ),
	UIF( color1,			1, 1 ),
	UIF( color2,			1, 1 ),
	UIF( handicap,			1, 1 ),
	UIF( sex,				0, 1 ),
	UIF( cg_predictItems,	1, 1 ),
	UIF( saber1,			1, 1 ),
	UIF( saber2,			1, 1 ),
	UIF( char_color_red,	1, 1 ),
	UIF( char_color_green,	1, 1 ),
	UIF( char_color_blue,	1, 1 ),
	UIF( teamtask,			1, 1 ),
	UIF( password,			0, 1 ), // optional
	UIF( teamoverlay,		0, 1 ), // only registered in cgame, not sent when connecting
};
static const size_t numUserinfoFields = ARRAY_LEN( userinfoFields );

static const char *userinfoValidateExtra[USERINFO_VALIDATION_MAX] = {
	"Size",					// USERINFO_VALIDATION_SIZE
	"# of slashes",			// USERINFO_VALIDATION_SLASH
	"Extended ascii",		// USERINFO_VALIDATION_EXTASCII
	"Control characters",	// USERINFO_VALIDATION_CONTROLCHARS
};

void Svcmd_ToggleUserinfoValidation_f( void ) {
	if ( trap->Argc() == 1 ) {
		int i=0;
		for ( i=0; i<numUserinfoFields; i++ ) {
			if ( (g_userinfoValidate.integer & (1<<i)) )	trap->Print( "%2d [X] %s\n", i, userinfoFields[i].fieldClean );
			else											trap->Print( "%2d [ ] %s\n", i, userinfoFields[i].fieldClean );
		}
		for ( ; i<numUserinfoFields+USERINFO_VALIDATION_MAX; i++ ) {
			if ( (g_userinfoValidate.integer & (1<<i)) )	trap->Print( "%2d [X] %s\n", i, userinfoValidateExtra[i-numUserinfoFields] );
			else											trap->Print( "%2d [ ] %s\n", i, userinfoValidateExtra[i-numUserinfoFields] );
		}
		return;
	}
	else {
		char arg[8]={0};
		int index;

		trap->Argv( 1, arg, sizeof( arg ) );
		index = atoi( arg );

		if ( index < 0 || index > numUserinfoFields+USERINFO_VALIDATION_MAX-1 ) {
			Com_Printf( "ToggleUserinfoValidation: Invalid range: %i [0, %i]\n", index, numUserinfoFields+USERINFO_VALIDATION_MAX-1 );
			return;
		}

		trap->Cvar_Set( "g_userinfoValidate", va( "%i", (1<<index) ^ g_userinfoValidate.integer ) );
		trap->Cvar_Update( &g_userinfoValidate );

		if ( index < numUserinfoFields )	Com_Printf( "%s %s\n", userinfoFields[index].fieldClean,				((g_userinfoValidate.integer & (1<<index)) ? "Validated" : "Ignored") );
		else								Com_Printf( "%s %s\n", userinfoValidateExtra[index-numUserinfoFields],	((g_userinfoValidate.integer & (1<<index)) ? "Validated" : "Ignored") );
	}
}

char *G_ValidateUserinfo( const char *userinfo )
{
	unsigned int		i=0, count=0, currentInfo=0;
	size_t				length = strlen( userinfo ), fieldLen = 0;
	userinfoValidate_t	*info = NULL;

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_SIZE)) )
	{// size checks
		if ( length < 1 )
			return "Userinfo too short";
		else if ( length >= MAX_INFO_STRING )
			return "Userinfo too long";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_SLASH)) )
	{// slash checks
		// there must be a leading slash
		if ( userinfo[0] != '\\' )
			return "Missing leading slash";

		// no trailing slashes allowed, engine will append ip\\ip:port
		if ( userinfo[length-1] == '\\' )
			return "Trailing slash";

		// format for userinfo field is: \\key\\value
		// so there must be an even amount of slashes
		for ( i=0, count=0; i<length; i++ ) {
			if ( userinfo[i] == '\\' )
				count++;
		}
		if ( (count&1) ) // odd
			return "Bad number of slashes";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_EXTASCII)) )
	{// extended characters are impossible to type, may want to disable
		for ( i=0, count=0; i<length; i++ ) {
			if ( userinfo[i] < 0 )
				count++;
		}
		if ( count )
			return "Extended ASCII characters found";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_CONTROLCHARS)) )
	{// disallow \n \r ; and \"
		if ( Q_strchrs( userinfo, "\n\r;\"" ) )
			return "Invalid characters found";
	}

	//this shit seems broken.. VVV
	for ( currentInfo=0; currentInfo<numUserinfoFields; currentInfo++ )
	{
		info = &userinfoFields[currentInfo];
		if ( g_userinfoValidate.integer & (1<<currentInfo) )
		{
			fieldLen = strlen( info->field );
			for ( i=0, count=0; i<length; i++ ) {
				if ( !Q_stricmpn( &userinfo[i], info->field, fieldLen ) ) {
					count++;
					i += fieldLen-1;
				}
			}
			if ( info->minCount && !count )
				return va( "%s field not found", info->fieldClean );
			else if ( count > info->maxCount )
				return va( "Too many %s fields (%i)", info->fieldClean, info->maxCount );
		}
	}

	return NULL;
}

void G_ValidateCosmetics(gclient_t *client, char *cosmeticString, size_t cosmeticStringSize) {
	int cosmetics = atoi(cosmeticString);

	if (client->sess.accountFlags & JAPRO_ACCOUNTFLAG_ALLCOSMETICS)
		return; //debug testing

	if (cosmetics) {//Optimized
		int i;

		for (i=0; i<MAX_COSMETIC_UNLOCKS; i++) { //For each bit, check if its allowed, if not, remove.
			if (!cosmeticUnlocks[i].active)
				break;
			if ((cosmetics & (1 << cosmeticUnlocks[i].bitvalue))) { //Use .bitvalue instead of i, since some of these are "public/free" cosmetics
				if (!(client->pers.unlocks & 1 << cosmeticUnlocks[i].bitvalue)) { //Check to see if its unlocked, if not disable.
					cosmetics &= ~(1 << cosmeticUnlocks[i].bitvalue);
				}
			}
		}
	}

	Q_strncpyz(cosmeticString, va("%i", cosmetics), cosmeticStringSize);
}


void G_Kill(gentity_t *ent);
void DetectTribesClass(gentity_t *ent, const char *model) {
	//WT_TRIBES
	if (!ent || !ent->client)
		return;
	if (g_tribesClass.integer && (!ent->client->sess.raceMode || (level.gametype >= GT_TEAM && ent->client->sess.sessionTeam > TEAM_FREE))) {
		if (!Q_strncmp("tribesheavy", model, 16) || !Q_strncmp("reborn_twin", model, 11) || !Q_strncmp("reelo", model, 5) || !Q_strncmp("noghri", model, 6) || !Q_strncmp("rax_joris", model, 9)) {
			//Com_Printf("Detetcting heavy\n");
			if (ent->client->pers.tribesClass != 3) {
				if (ent->health > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
					G_Kill(ent);
				ent->client->pers.tribesClass = 3;
				trap->SendServerCommand(ent - g_entities, va("print \"Spawning as Tribes heavy class\n\""));
			}
		}
		else if (!Q_strncmp("tavion_new", model, 10) || !Q_strncmp("tavion", model, 6) || !Q_strncmp("jan", model, 3) || !Q_strncmp("alora", model, 5) || !Q_strncmp("alora2", model, 6) || !Q_strncmp("jedi_tf", model, 7) || !Q_strncmp("jedi_zf", model, 7) || !Q_strncmp("jedi_hf", model, 7)) {
			//Com_Printf("Detetcting heavy\n");
			if (ent->client->pers.tribesClass != 1) {
				if (ent->health > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
					G_Kill(ent);
				ent->client->pers.tribesClass = 1;
				trap->SendServerCommand(ent - g_entities, va("print \"Spawning as Tribes light class\n\""));
			}
		}
		else {
			//Com_Printf("Detetcting medium \n");
			if (ent->client->pers.tribesClass != 2) {
				if (ent->health > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
					G_Kill(ent);
				ent->client->pers.tribesClass = 2;
				trap->SendServerCommand(ent - g_entities, va("print \"Spawning as Tribes medium class\n\""));
			}
		}
	}
}

qboolean ClientUserinfoChanged( int clientNum ) { //I think anything treated as an INT can just be max_qpath instead of max_info_string and help performance  a bit..?
	gentity_t	*ent = g_entities + clientNum;
	gclient_t	*client = ent->client;
	int			teamLeader, team=TEAM_FREE, health=100, maxHealth=100;
	char		*s=NULL,						*value=NULL,
				userinfo[MAX_INFO_STRING]={0},	buf[MAX_INFO_STRING]={0},		oldClientinfo[MAX_INFO_STRING]={0},
				model[MAX_QPATH]={0},			forcePowers[MAX_QPATH]={0},		oldname[MAX_NETNAME]={0},
				className[MAX_QPATH]={0},		color1[MAX_QPATH]={0},	color2[MAX_QPATH]={0}, cp_sbRGB1[MAX_QPATH]={0}, cp_sbRGB2[MAX_QPATH]={0}, cp_cosmetics[MAX_QPATH] = { 0 };
	qboolean	modelChanged = qfalse, female = qfalse;

	trap->GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	s = G_ValidateUserinfo( userinfo );
	if ( s && *s )
	{
		G_SecurityLogPrintf( "Client %d (%s) failed userinfo validation: %s [IP: %s]\n", clientNum, ent->client->pers.netname, s, client->sess.IP );
		trap->DropClient( clientNum, va( "Failed userinfo validation: %s", s ) );
		return qfalse;
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) && !(ent->r.svFlags & SVF_BOT) )
		client->pers.localClient = qtrue;

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) )	client->pers.predictItemPickup = qfalse;
	else				client->pers.predictItemPickup = qtrue;

//JAPRO - Serverside - Get Clients Mod version, if any - Start
	s = Info_ValueForKey( userinfo, "cjp_client" );
	if ( !strcmp( s, "1.4JAPRO" ) ) {
		client->pers.isJAPRO = qtrue;
	} else {
		client->pers.isJAPRO = qfalse;
	}

	s = Info_ValueForKey( userinfo, "cp_pluginDisable" );
	if (atoi(s) & JAPRO_PLUGIN_JAWARUN)
		client->pers.JAWARUN = qtrue;
	else
		client->pers.JAWARUN = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_BHOP)
		client->pers.onlyBhop = qtrue;
	else
		client->pers.onlyBhop = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_NOROLL)
		client->pers.noRoll = qtrue;
	else
		client->pers.noRoll = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_NOCART)
		client->pers.noCartwheel = qtrue;
	else
		client->pers.noCartwheel = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_CENTERMUZZLE)
		client->pers.centerMuzzle = qtrue;
	else
		client->pers.centerMuzzle = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_NOCENTERCP)
		client->pers.showCenterCP = qfalse;
	else
		client->pers.showCenterCP = qtrue;

	if (atoi(s) & JAPRO_PLUGIN_CHATBOXCP)
		client->pers.showChatCP = qtrue;
	else
		client->pers.showChatCP = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_CONSOLECP)
		client->pers.showConsoleCP = qtrue;
	else
		client->pers.showConsoleCP = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_NODMGNUMBERS)
		client->pers.noDamageNumbers = qtrue;
	else
		client->pers.noDamageNumbers = qfalse;

	if (atoi(s) & JAPRO_PLUGIN_NODUELTELE)
		client->pers.noDuelTele = qtrue;
	else
		client->pers.noDuelTele = qfalse;

	s = Info_ValueForKey( userinfo, "rate" );
	client->pers.rate = atoi(s);
	if (client->pers.rate > 90000)
		client->pers.rate = 90000;
	else if (client->pers.rate < 0)
		client->pers.rate = 0;

	s = Info_ValueForKey( userinfo, "snaps" );
	client->pers.snaps = atoi(s);
	if (client->pers.snaps > 1000)
		client->pers.snaps = 1000;
	else if (client->pers.snaps < 0)
		client->pers.snaps = 0;

	s = Info_ValueForKey( userinfo, "cg_displayNetSettings" );
	if (Q_stricmp(s, "")) { //if s is set
		char * pch;
		int i = 0;
		pch = strtok (s, " ");
		while (pch != NULL) {
			if (i == 0)
				client->pers.maxPackets = atoi(pch);
			else if (i == 1)
				client->pers.timenudge = atoi(pch);
			else if (i == 2)
				client->pers.maxFPS = atoi(pch); //This should have been max msec i guess. w/e
			else 
				break;
			pch = strtok (NULL, " ");
			i++;
		}
	}
	else {
		client->pers.timenudge = Q3_INFINITE;//Not set..
	}

	s = Info_ValueForKey( userinfo, "cg_displayCameraPosition" );
	if (Q_stricmp(s, "")) { //if s is set
		char strTemp[64] = {0};
		int encodedRange;
		int encodedOffset;

		char * pch;
		int i = 0;
		pch = strtok (s, " ");
		while (pch != NULL) {
			if (i == 0)
				client->pers.thirdPerson = (qboolean)atoi(pch);
			else if (i == 1)
				client->pers.thirdPersonRange = atoi(pch);
			else if (i == 2)
				client->pers.thirdPersonVertOffset = atoi(pch);
			else 
				break;
			pch = strtok (NULL, " ");
			i++;
		}

		encodedRange = client->pers.thirdPersonRange;
		encodedOffset = client->pers.thirdPersonVertOffset;
		//Use negative if 1st person.  Clamp to -32765, 32765 range.
		if (encodedRange > 325) //max 1625 - abs max 327
			encodedRange = 325;
		if (encodedOffset > 64) //max 256 - abs max 67..?
			encodedOffset = 64;
		if (!client->pers.thirdPerson)
			encodedRange = -encodedRange;

		Com_sprintf( strTemp, 128, "%i%02i", encodedRange, encodedOffset );
		client->pers.cameraSettings = atoi(strTemp);
		client->ps.persistant[PERS_CAMERA_SETTINGS] = client->pers.cameraSettings; //This gets reset on clientbegin ? damn
	}

	if (client->pers.timenudge > 200 && client->pers.timenudge != Q3_INFINITE)
		client->pers.timenudge = 200;
	else if (client->pers.timenudge < -1200)
		client->pers.timenudge = -1200;
	if (client->pers.maxPackets > 1000)
		client->pers.maxPackets = 1000;
	else if (client->pers.maxPackets < 0)
		client->pers.maxPackets = 0;
	if (client->pers.maxFPS > 1000)
		client->pers.maxFPS = 1000;
	else if (client->pers.maxFPS < 0)
		client->pers.maxFPS = 0;

//JAPRO - Serverside - Get Clients Mod version, if any - End

	// set name
	Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers.netname, sizeof( client->pers.netname ) );
	Q_strncpyz( client->pers.netname_nocolor, client->pers.netname, sizeof( client->pers.netname_nocolor ) );
	Q_StripColor( client->pers.netname_nocolor );

	if (!g_allowSamePlayerNames.integer)//JAPRO - Serverside - Same player name fix
		CheckDuplicateName(clientNum);

	if ( client->sess.sessionTeam == TEAM_SPECTATOR && client->sess.spectatorState == SPECTATOR_SCOREBOARD )
	{
		Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
		Q_strncpyz( client->pers.netname_nocolor, "scoreboard", sizeof( client->pers.netname_nocolor ) );
	}

	if ( client->pers.connected == CON_CONNECTED )
	{
		if ( strcmp( oldname, client->pers.netname ) ) 
		{
			if ( client->pers.netnameTime > level.time  )
			{
				trap->SendServerCommand( clientNum, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NONAMECHANGE")) );

				Info_SetValueForKey( userinfo, "name", oldname );
				trap->SetUserinfo( clientNum, userinfo );			
				Q_strncpyz( client->pers.netname, oldname, sizeof( client->pers.netname ) );
				Q_strncpyz( client->pers.netname_nocolor, oldname, sizeof( client->pers.netname_nocolor ) );
				Q_StripColor( client->pers.netname_nocolor );
			}
			else
			{				
				trap->SendServerCommand( -1, va( "print \"%s"S_COLOR_WHITE" %s %s\n\"", oldname, G_GetStringEdString( "MP_SVGAME", "PLRENAME" ), client->pers.netname ) );
				G_LogPrintf( "ClientRename: %i [%s] (%s) \"%s^7\" -> \"%s^7\"\n", clientNum, ent->client->sess.IP, ent->client->pers.guid, oldname, ent->client->pers.netname );
				client->pers.netnameTime = level.time + 5000;
				if (g_playerLog.integer && ent && ent->client && !(ent->r.svFlags & SVF_BOT))
					G_AddPlayerLog(client->pers.netname, client->sess.IP, client->pers.guid);
			}
		}
	}

	// set model
	Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );

	if ( d_perPlayerGhoul2.integer )
	{
		if ( Q_stricmp( model, client->modelname ) )
		{
			Q_strncpyz( client->modelname, model, sizeof( client->modelname ) );
			modelChanged = qtrue;
		}
	}

	//WT_TRIBES
	if (g_tribesClass.integer && (!client->sess.raceMode || (level.gametype >= GT_TEAM && client->sess.sessionTeam > TEAM_FREE))) {
		DetectTribesClass(ent, model);
	}
	else if (client->pers.tribesClass) {
		client->pers.tribesClass = 0;
	}

	client->ps.customRGBA[0] = (value=Info_ValueForKey( userinfo, "char_color_red" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[1] = (value=Info_ValueForKey( userinfo, "char_color_green" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[2] = (value=Info_ValueForKey( userinfo, "char_color_blue" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;

	//Prevent skins being too dark
	if ( g_charRestrictRGB.integer && ((client->ps.customRGBA[0]+client->ps.customRGBA[1]+client->ps.customRGBA[2]) < 100) )
		client->ps.customRGBA[0] = client->ps.customRGBA[1] = client->ps.customRGBA[2] = 255;

	client->ps.customRGBA[3]=255;

	Q_strncpyz( forcePowers, Info_ValueForKey( userinfo, "forcepowers" ), sizeof( forcePowers ) );

	if ( level.gametype >= GT_TEAM && level.gametype != GT_SIEGE && !g_jediVmerc.integer )
	{
		char skin[MAX_QPATH] = {0};
		vec3_t colorOverride = {0.0f};

		VectorClear( colorOverride );

		BG_ValidateSkinForTeam( model, skin, client->sess.sessionTeam, colorOverride );
		if ( colorOverride[0] != 0.0f || colorOverride[1] != 0.0f || colorOverride[2] != 0.0f )
			VectorScaleM( colorOverride, 255.0f, client->ps.customRGBA );
	}

	Q_strncpyz(cp_cosmetics, Info_ValueForKey(userinfo, "cp_cosmetics"), sizeof(cp_cosmetics));
	if (g_validateCosmetics.integer)
		G_ValidateCosmetics(client, cp_cosmetics, sizeof(cp_cosmetics)); //Model cosmetics

	// bots set their team a few frames later
	if ( level.gametype >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT )
	{
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) )
			team = TEAM_RED;
		else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) )
			team = TEAM_BLUE;
		else
			team = PickTeam( clientNum ); // pick the team with the least number of players
	}
	else
		team = client->sess.sessionTeam;

	//Testing to see if this fixes the problem with a bot's team getting set incorrectly.
	team = client->sess.sessionTeam;

	//Set the siege class
	if ( level.gametype == GT_SIEGE )
	{
		Q_strncpyz( className, client->sess.siegeClass, sizeof( className ) );

		//Now that the team is legal for sure, we'll go ahead and get an index for it.
		client->siegeClass = BG_SiegeFindClassIndexByName( className );
		if ( client->siegeClass == -1 )
		{ //ok, get the first valid class for the team you're on then, I guess.
			BG_SiegeCheckClassLegality( team, className );
			Q_strncpyz( client->sess.siegeClass, className, sizeof( client->sess.siegeClass ) );
			client->siegeClass = BG_SiegeFindClassIndexByName( className );
		}
		else
		{ //otherwise, make sure the class we are using is legal.
			G_ValidateSiegeClassForTeam( ent, team );
			Q_strncpyz( className, client->sess.siegeClass, sizeof( className ) );
		}

		if ( client->siegeClass != -1 )
		{// Set the sabers if the class dictates
			siegeClass_t *scl = &bgSiegeClasses[client->siegeClass];

			G_SetSaber( ent, 0, scl->saber1[0] ? scl->saber1 : DEFAULT_SABER, qtrue );
			G_SetSaber( ent, 1, scl->saber2[0] ? scl->saber2 : "none", qtrue );

			//make sure the saber models are updated
			G_SaberModelSetup( ent );

			if ( scl->forcedModel[0] )
			{ //be sure to override the model we actually use
				Q_strncpyz( model, scl->forcedModel, sizeof( model ) );
				if ( d_perPlayerGhoul2.integer )
				{
					if ( Q_stricmp( model, client->modelname ) )
					{
						Q_strncpyz( client->modelname, model, sizeof( client->modelname ) );
						modelChanged = qtrue;
					}
				}
			}

			if ( G_PlayerHasCustomSkeleton( ent ) )
			{//force them to use their class model on the server, if the class dictates
				if ( Q_stricmp( model, client->modelname ) || ent->localAnimIndex == 0 )
				{
					Q_strncpyz( client->modelname, model, sizeof( client->modelname ) );
					modelChanged = qtrue;
				}
			}
		}
	}
	else
		Q_strncpyz( className, "none", sizeof( className ) );

	// only set the saber name on the first connect.
	//	it will be read from userinfo on ClientSpawn and stored in client->pers.saber1/2
	if ( !VALIDSTRING( client->pers.saber1 ) || !VALIDSTRING( client->pers.saber2 ) )
	{
		G_SetSaber( ent, 0, Info_ValueForKey( userinfo, "saber1" ), qfalse );
		G_SetSaber( ent, 1, Info_ValueForKey( userinfo, "saber2" ), qfalse );
	}

	// set max health
	if ( level.gametype == GT_SIEGE && client->siegeClass != -1 )
	{
		siegeClass_t *scl = &bgSiegeClasses[client->siegeClass];
		maxHealth = 100;

		if ( scl->maxhealth )
			maxHealth = scl->maxhealth;

		health = maxHealth;
	}
	else
	{
		maxHealth = 100;
		health = Com_Clampi( 1, 100, atoi( Info_ValueForKey( userinfo, "handicap" ) ) );
	}
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > maxHealth )
		client->pers.maxHealth = 100;
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	if ( level.gametype >= GT_TEAM )
		client->pers.teamInfo = qtrue;
	else
	{
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( !*s || atoi( s ) != 0 )
			client->pers.teamInfo = qtrue;
		else
			client->pers.teamInfo = qfalse;
	}

	// team task (0 = none, 1 = offence, 2 = defence)
//	teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	teamLeader = client->sess.teamLeader;

	// colors
	Q_strncpyz( color1, Info_ValueForKey( userinfo, "color1" ), sizeof( color1 ) );
	Q_strncpyz( color2, Info_ValueForKey( userinfo, "color2" ), sizeof( color2 ) );
	Q_strncpyz( cp_sbRGB1, Info_ValueForKey( userinfo, "cp_sbRGB1" ), sizeof(cp_sbRGB1) );//rgbsabers
	Q_strncpyz( cp_sbRGB2, Info_ValueForKey( userinfo, "cp_sbRGB2" ), sizeof(cp_sbRGB2) );//rgbsabers

	// gender hints
	s = Info_ValueForKey( userinfo, "sex" );
	if ( !Q_stricmp( s, "female" ) )
		female = qtrue;

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	buf[0] = '\0';
	Q_strcat( buf, sizeof( buf ), va( "n\\%s\\", client->pers.netname ) );
	Q_strcat( buf, sizeof( buf ), va( "t\\%i\\", client->sess.sessionTeam ) );
	Q_strcat( buf, sizeof( buf ), va( "model\\%s\\", model ) );
	Q_strcat( buf, sizeof( buf ), va( "ds\\%c\\", female ? 'f' : 'm' ) );
	Q_strcat( buf, sizeof( buf ), va( "st\\%s\\", client->pers.saber1 ) );
	Q_strcat( buf, sizeof( buf ), va( "st2\\%s\\", client->pers.saber2 ) );
	Q_strcat( buf, sizeof( buf ), va( "c1\\%s\\", color1 ) );
	Q_strcat( buf, sizeof( buf ), va( "c2\\%s\\", color2 ) );
	Q_strcat( buf, sizeof(buf), va( "c3\\%s\\", cp_sbRGB1 ) );//rgbsabers
	Q_strcat( buf, sizeof(buf), va( "c4\\%s\\", cp_sbRGB2 ) );//rgbsabers
	Q_strcat(buf, sizeof(buf), va("c5\\%s\\", cp_cosmetics));//cosmetics
	Q_strcat( buf, sizeof( buf ), va( "hc\\%i\\", client->pers.maxHealth ) );
	if ( ent->r.svFlags & SVF_BOT )
		Q_strcat( buf, sizeof( buf ), va( "skill\\%s\\", Info_ValueForKey( userinfo, "skill" ) ) );
	if ( level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL ) {
		Q_strcat( buf, sizeof( buf ), va( "w\\%i\\", client->sess.wins ) );
		Q_strcat( buf, sizeof( buf ), va( "l\\%i\\", client->sess.losses ) );
	}
	if ( level.gametype == GT_POWERDUEL )
		Q_strcat( buf, sizeof( buf ), va( "dt\\%i\\", client->sess.duelTeam ) );
	if ( level.gametype >= GT_TEAM ) {
	//	Q_strcat( buf, sizeof( buf ), va( "tt\\%d\\", teamTask ) );
		Q_strcat( buf, sizeof( buf ), va( "tl\\%d\\", teamLeader ) );
	}
	if ( level.gametype == GT_SIEGE ) {
		Q_strcat( buf, sizeof( buf ), va( "siegeclass\\%s\\", className ) );
		Q_strcat( buf, sizeof( buf ), va( "sdt\\%i\\", className ) );
	}

	trap->GetConfigstring( CS_PLAYERS+clientNum, oldClientinfo, sizeof( oldClientinfo ) );
	trap->SetConfigstring( CS_PLAYERS+clientNum, buf );

	if ( modelChanged ) //only going to be true for allowable server-side custom skeleton cases
	{ //update the server g2 instance if appropriate
		char *modelname = Info_ValueForKey( userinfo, "model" );
		SetupGameGhoul2Model( ent, modelname, NULL );

		if ( ent->ghoul2 && ent->client )
			ent->client->renderInfo.lastG2 = NULL; //update the renderinfo bolts next update.

		client->torsoAnimExecute = client->legsAnimExecute = -1;
		client->torsoLastFlip = client->legsLastFlip = qfalse;
	}

	if ( g_logClientInfo.integer )
	{
		if ( strcmp( oldClientinfo, buf ) )
			G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, buf );
		else
			G_LogPrintf( "ClientUserinfoChanged: %i <no change>\n", clientNum );
	}

	return qtrue;
}

//[JAPRO - Serverside - All - Ignore subfunction - Start]
void QINLINE ClientRemoveIgnore(const int targetID) {
	int i;
	for (i = 0; i < level.maxclients; ++i) {
		if (level.clients[i].pers.connected == CON_CONNECTED) {
			level.clients[i].sess.ignore &= ~(1 << targetID);
		}
	}
}
//[JAPRO - Serverside - All - Ignore subfunction - End]

/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournament restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournament
restarts.
============
*/

static qboolean CompareIPs( const char *ip1, const char *ip2 )
{
	char *p = NULL;
	p = strchr(ip1, ':');
	if (p)
		*p = 0;
	p = strchr(ip2, ':');
	if (p)
		*p = 0;

	if (!Q_stricmp(ip1, ip2)) {
		return qtrue;
	}
	return qfalse;

	/*
	while ( 1 ) {
		if ( *ip1 != *ip2 )
			return qfalse;
		if ( !*ip1 || *ip1 == ':' )
			break;
		ip1++;
		ip2++;
	}

	return qtrue;
	*/
}

char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value = NULL;
	gentity_t	*ent = NULL, *te = NULL;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING] = {0},
				tmpIP[NET_ADDRSTRMAXLEN] = {0},
				//name[MAX_NETNAME] = {0}, //not needed?
				guid[33] = {0};

	ent = &g_entities[ clientNum ];

	ent->s.number = clientNum;
	ent->classname = "connecting";

	trap->GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	value = Info_ValueForKey( userinfo, "ja_guid" );
	if( value[0] )
		Q_strncpyz( guid, value, sizeof( guid ) );
	else if( isBot )
		Q_strncpyz( guid, "BOT", sizeof( guid ) );
	else
		Q_strncpyz( guid, "NOGUID", sizeof( guid ) );

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	Q_strncpyz( tmpIP, isBot ? "Bot" : value, sizeof( tmpIP ) );
	if ( G_FilterPacket( value ) ) {
		return "Banned.";
	}

	//Com_Printf("CLIENTCONNECT: IP: %s, OLD SLOT IP: %s\n", tmpIP, level.clients[clientNum].sess.IP);
	if (!isBot && !level.clients[clientNum].sess.IP[0] || !CompareIPs(tmpIP, level.clients[clientNum].sess.IP)) { //New Client, remove ignore if it was there
		ClientRemoveIgnore(clientNum);//JAPRO IGNORE, move this to clientConnect, and only do it if IP does not match previous slot
	}

	//CompareIPs always returns qfalse on linux lol?

	if ( !isBot && g_needpass.integer ) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			static char sTemp[1024];
			Q_strncpyz(sTemp, G_GetStringEdString("MP_SVGAME","INVALID_ESCAPE_TO_MAIN"), sizeof (sTemp) );
			return sTemp;// return "Invalid password";
		}
	}

	if ( !isBot && firstTime )
	{
		if ( g_antiFakePlayer.integer )
		{// patched, check for > g_maxConnPerIP connections from same IP
			int count=0, i=0;
			for ( i=0; i<sv_maxclients.integer; i++ )
			{
				//trap->Print("Theirs: %s, ours: %s\n", strIP, level.clients[i].sess.IP);
				#if 0
					if ( level.clients[i].pers.connected != CON_DISCONNECTED && i != clientNum )
					{
						if ( CompareIPs( clientNum, i ) )
						{
							if ( !level.security.clientConnectionActive[i] )
							{//This IP has a dead connection pending, wait for it to time out
							//	client->pers.connected = CON_DISCONNECTED;
								return "Please wait, another connection from this IP is still pending...";
							}
						}
					}
				#else
					if ( CompareIPs( tmpIP, level.clients[i].sess.IP ) )
						count++;
				#endif
			}
			if ( count > g_maxConnPerIP.integer ) //>= ?
			{
			//	client->pers.connected = CON_DISCONNECTED;
				//trap->Print("Too may connections\n");
				return "Too many connections from the same IP";
			}
		}
	}

	if ( ent->inuse )
	{// if a player reconnects quickly after a disconnect, the client disconnect may never be called, thus flag can get lost in the ether
		G_LogPrintf( "Forcing disconnect on active client: %i\n", clientNum );
		// so lets just fix up anything that should happen on a disconnect
		ClientDisconnect( clientNum );
	}

	ent->r.svFlags &= ~SVF_SINGLECLIENT; //ehh?

	// they can connect
	client = &level.clients[ clientNum ];
	ent->client = client;

	//assign the pointer for bg entity access
	ent->playerState = &ent->client->ps;

	memset( client, 0, sizeof(*client) );

	Q_strncpyz( client->pers.guid, guid, sizeof( client->pers.guid ) );

	client->pers.connected = CON_CONNECTING;
	client->pers.connectTime = level.time;

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitSessionData( client, userinfo, isBot );
	}
	G_ReadSessionData( client );

	if (level.gametype == GT_SIEGE &&
		(firstTime || level.newSession))
	{ //if this is the first time then auto-assign a desired siege team and show briefing for that team
		client->sess.siegeDesiredTeam = 0;//PickTeam(ent->s.number);
		/*
		trap->SendServerCommand(ent->s.number, va("sb %i", client->sess.siegeDesiredTeam));
		*/
		//don't just show it - they'll see it if they switch to a team on purpose.
	}


	if (level.gametype == GT_SIEGE && client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		if (firstTime || level.newSession)
		{ //start as spec
			client->sess.siegeDesiredTeam = client->sess.sessionTeam;
			client->sess.sessionTeam = TEAM_SPECTATOR;
		}
	}
	else if (level.gametype == GT_POWERDUEL && client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		client->sess.sessionTeam = TEAM_SPECTATOR;
	}

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	// get and distribute relevent paramters
	if ( !ClientUserinfoChanged( clientNum ) )
		return "Failed userinfo validation";

	if ( !isBot && firstTime )
	{
		if ( !tmpIP[0] )
		{//No IP sent when connecting, probably an unban hack attempt
			client->pers.connected = CON_DISCONNECTED;
			G_SecurityLogPrintf( "Client %i (%s) sent no IP when connecting.\n", clientNum, client->pers.netname );
			return "Invalid userinfo detected";
		}
	}

	Q_strncpyz(client->sess.IP, tmpIP, sizeof( client->sess.IP )); //always do this i guess? might solve issue of blank IP strings..
	if ( firstTime ) {//loda fixme
		//Q_strncpyz( client->sess.IP, tmpIP, sizeof( client->sess.IP ) );
		if (g_playerLog.integer && ent && ent->client && !isBot)
			G_AddPlayerLog(client->pers.netname, client->sess.IP, client->pers.guid);
	}

	G_LogPrintf( "ClientConnect: %i [%s] (%s) \"%s^7\"\n", clientNum, tmpIP, guid, client->pers.netname );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"", client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCONNECT")) );
	}

	if ( level.gametype >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	te = G_TempEntity( vec3_origin, EV_CLIENTJOIN );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = clientNum;

	if (firstTime)
		ent->client->sess.movementStyle = MV_JKA;//default to JKA style 

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap->AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

	return NULL;
}

void G_WriteClientSessionData( gclient_t *client );

void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
extern qboolean	gSiegeRoundBegun;
extern qboolean	gSiegeRoundEnded;
extern qboolean g_dontPenalizeTeam; //g_cmds.c
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);
void ClientBegin( int clientNum, qboolean allowTeamReset ) {
	gentity_t	*ent;
	gclient_t	*client;
	int			flags, i;
	char		userinfo[MAX_INFO_VALUE], *modelname;
	int			spawnCount;

	ent = g_entities + clientNum;

	if ((ent->r.svFlags & SVF_BOT) && level.gametype >= GT_TEAM)
	{
		if (allowTeamReset)
		{
			const char *team = "Red";
			int preSess;

			//SetTeam(ent, "");
			if (bot_team.integer == 1) {
				ent->client->sess.sessionTeam = TEAM_BLUE;
				team = "Blue";
			}
			else if (bot_team.integer > 1)
			{
				ent->client->sess.sessionTeam = TEAM_RED;
				team = "Red";
			}
			else {
				ent->client->sess.sessionTeam = PickTeam(-1);

				if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
				{
					ent->client->sess.sessionTeam = TEAM_RED;
				}

				if (ent->client->sess.sessionTeam == TEAM_RED)
				{
					team = "Red";
				}
				else
				{
					team = "Blue";
				}
			}
			trap->GetUserinfo(clientNum, userinfo, MAX_INFO_STRING);
			Info_SetValueForKey( userinfo, "team", team );

			trap->SetUserinfo( clientNum, userinfo );

			ent->client->ps.persistant[ PERS_TEAM ] = ent->client->sess.sessionTeam;

			preSess = ent->client->sess.sessionTeam;
			G_ReadSessionData( ent->client );
			ent->client->sess.sessionTeam = preSess;
			G_WriteClientSessionData(ent->client);
			if ( !ClientUserinfoChanged( clientNum ) )
				return;
			ClientBegin(clientNum, qfalse);
			return;
		}
	}

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap->UnlinkEntity( (sharedEntity_t *)ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	//assign the pointer for bg entity access
	ent->playerState = &ent->client->ps;

	client->pers.connected = CON_CONNECTED;
	if (client->pers.teamState.state == TEAM_BEGIN) //For some reason this is being called when you are spectating a player and they leave/spec.  So dont reset your time in that case..
		client->pers.enterTime = level.time; 
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	spawnCount = client->ps.persistant[PERS_SPAWN_COUNT];

	i = 0;

	while (i < NUM_FORCE_POWERS)
	{
		if (ent->client->ps.fd.forcePowersActive & (1 << i))
		{
			WP_ForcePowerStop(ent, i);
		}
		i++;
	}

	i = TRACK_CHANNEL_1;

	while (i < NUM_TRACK_CHANNELS)
	{
		if (ent->client->ps.fd.killSoundEntIndex[i-50] && ent->client->ps.fd.killSoundEntIndex[i-50] < MAX_GENTITIES && ent->client->ps.fd.killSoundEntIndex[i-50] > 0)
		{
			G_MuteSound(ent->client->ps.fd.killSoundEntIndex[i-50], CHAN_VOICE);
		}
		i++;
	}
	i = 0;

	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;
	client->ps.persistant[PERS_SPAWN_COUNT] = spawnCount;

	client->ps.hasDetPackPlanted = qfalse;

	//first-time force power initialization
	WP_InitForcePowers( ent );

	//init saber ent
	WP_SaberInitBladeData( ent );

	// First time model setup for that player.
	trap->GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
	modelname = Info_ValueForKey (userinfo, "model");
	SetupGameGhoul2Model(ent, modelname, NULL);

	if (g_playerLog.integer && ent && ent->client && !(ent->r.svFlags & SVF_BOT))
		G_AddPlayerLog(client->pers.netname, client->sess.IP, client->pers.guid);

	if (g_raceMode.integer == 1 && level.gametype == GT_FFA)//Japro racemode, uhh, cant think of any case where racemode should be turned off since its off by default and this is their first time in server?
		client->sess.raceMode = qtrue;
	else if (g_raceMode.integer && (level.gametype == GT_TEAM || level.gametype == GT_CTF) && client->sess.sessionTeam == TEAM_FREE)
		client->sess.raceMode = qtrue;
	if (client->sess.sessionTeam != TEAM_FREE && client->sess.sessionTeam != TEAM_SPECTATOR)
		client->sess.raceMode = qfalse;
	else if (!g_raceMode.integer) 
		client->sess.raceMode = qfalse;

	if (client->sess.raceMode) 
		client->ps.stats[STAT_RACEMODE] = 1;
	else
		client->ps.stats[STAT_RACEMODE] = 0;

	client->ps.persistant[PERS_CAMERA_SETTINGS] = client->pers.cameraSettings;

	client->pers.noFollow = qfalse;
	ent->r.svFlags &= ~SVF_SINGLECLIENT;

	if ( ent->ghoul2 && ent->client )
		ent->client->renderInfo.lastG2 = NULL; //update the renderinfo bolts next update.

	if ( level.gametype == GT_POWERDUEL && client->sess.sessionTeam != TEAM_SPECTATOR && client->sess.duelTeam == DUELTEAM_FREE )
		SetTeam( ent, "s", qfalse);
	else
	{
		if ( level.gametype == GT_SIEGE && (!gSiegeRoundBegun || gSiegeRoundEnded) )
			SetTeamQuick( ent, TEAM_SPECTATOR, qfalse );
        
		// locate ent at a spawn point
		ClientSpawn( ent );
	}

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		if ( level.gametype != GT_DUEL || level.gametype == GT_POWERDUEL ) {
			trap->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"", client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLENTER")) );
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

	if (client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->sess.sawMOTD == qfalse)
	{
		//char arg1[MAX_STRING_TOKENS];

		if (client->pers.isJAPRO == qfalse)
			trap->SendServerCommand( ent-g_entities, va("print \"^1You do not have the client plugin. Download at www.playja.pro\n\"" ) );

		//if (!strchr( arg1, ';' ) && !strchr( arg1, '\r' ) && !strchr( arg1, '\n' )) //loda idk
		if (Q_stricmp(g_consoleMOTD.string, ""))
			trap->SendServerCommand(ent-g_entities, va("print \"%s\n\"", g_consoleMOTD.string));

		if (Q_stricmp(g_centerMOTD.string, "")) {
			strcpy(ent->client->csMessage, G_NewString(va("^7%s\n", g_centerMOTD.string )));
			ent->client->csTimeLeft = g_centerMOTDTime.integer;
		}

		if (g_playerLog.integer && ent && ent->client && !(ent->r.svFlags & SVF_BOT))
			G_AddPlayerLog(client->pers.netname, client->sess.IP, client->pers.guid);

		ent->client->sess.sawMOTD = qtrue;
	}

	G_ClearClientLog(clientNum);

	if (client->pers.isJAPRO) { //Should this be in userinfochanged or..
		char	msg[1024-128] = {0};
		if (g_validateCosmetics.integer) {
			int		i;

			for (i=0; i<MAX_COSMETIC_UNLOCKS; i++) {
				char *tmpMsg = NULL;
				if (!cosmeticUnlocks[i].active)
					continue;

				tmpMsg = va("%i:%s:%i:%i\n", cosmeticUnlocks[i].bitvalue, cosmeticUnlocks[i].mapname, cosmeticUnlocks[i].style, cosmeticUnlocks[i].duration); //probably have to replace the \n with something so it doesnt flood console of old japro clients
				if (strlen(msg) + strlen(tmpMsg) >= sizeof( msg)) {
					trap->SendServerCommand( ent-g_entities, va("cosmetics \"%s\"", msg));
					msg[0] = '\0';
				}
				Q_strcat(msg, sizeof(msg), tmpMsg);
			}
		}
		trap->SendServerCommand(ent-g_entities, va("cosmetics \"%s\"", msg));
	}
}

static qboolean AllForceDisabled(int force)
{
	int i;

	if (force)
	{
		for (i=0;i<NUM_FORCE_POWERS;i++)
		{
			if (!(force & (1<<i)))
			{
				return qfalse;
			}
		}

		return qtrue;
	}

	return qfalse;
}

//Convenient interface to set all my limb breakage stuff up -rww
void G_BreakArm(gentity_t *ent, int arm)
{
	int anim = -1;

	assert(ent && ent->client);

	if (ent->s.NPC_class == CLASS_VEHICLE || ent->localAnimIndex > 1)
	{ //no broken limbs for vehicles and non-humanoids
		return;
	}

	if (!arm)
	{ //repair him
		ent->client->ps.brokenLimbs = 0;
		return;
	}

	if (ent->client->ps.fd.saberAnimLevel == SS_STAFF)
	{ //I'm too lazy to deal with this as well for now.
		return;
	}

	if (arm == BROKENLIMB_LARM)
	{
		if (ent->client->saber[1].model[0] &&
			ent->client->ps.weapon == WP_SABER &&
			!ent->client->ps.saberHolstered &&
			ent->client->saber[1].soundOff)
		{ //the left arm shuts off its saber upon being broken
			G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
		}
	}

	ent->client->ps.brokenLimbs = 0; //make sure it's cleared out
	ent->client->ps.brokenLimbs |= (1 << arm); //this arm is now marked as broken

	//Do a pain anim based on the side. Since getting your arm broken does tend to hurt.
	if (arm == BROKENLIMB_LARM)
	{
		anim = BOTH_PAIN2;
	}
	else if (arm == BROKENLIMB_RARM)
	{
		anim = BOTH_PAIN3;
	}

	if (anim == -1)
	{
		return;
	}

	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);

	//This could be combined into a single event. But I guess limbs don't break often enough to
	//worry about it.
	G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );
	//FIXME: A nice bone snapping sound instead if possible
	G_Sound(ent, CHAN_AUTO, G_SoundIndex( va("sound/player/bodyfall_human%i.wav", Q_irand(1, 3)) ));
}

//Update the ghoul2 instance anims based on the playerstate values
qboolean BG_SaberStanceAnim( int anim );
qboolean PM_RunningAnim( int anim );
void G_UpdateClientAnims(gentity_t *self, float animSpeedScale)
{
	static int f;
	static int torsoAnim;
	static int legsAnim;
	static int firstFrame, lastFrame;
	static int aFlags;
	static float animSpeed, lAnimSpeedScale;
	qboolean setTorso = qfalse;

	torsoAnim = (self->client->ps.torsoAnim);
	legsAnim = (self->client->ps.legsAnim);

	if (self->client->ps.saberLockFrame)
	{
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "model_root", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "lower_lumbar", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "Motion", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		return;
	}

	if (self->localAnimIndex > 1 &&
		bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame == 0 &&
		bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames == 0)
	{ //We'll allow this for non-humanoids.
		goto tryTorso;
	}

	if (self->client->legsAnimExecute != legsAnim || self->client->legsLastFlip != self->client->ps.legsFlip)
	{
		animSpeed = 50.0f / bgAllAnims[self->localAnimIndex].anims[legsAnim].frameLerp;
		lAnimSpeedScale = (animSpeed *= animSpeedScale);

		if (bgAllAnims[self->localAnimIndex].anims[legsAnim].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		if (animSpeed < 0)
		{
			lastFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame;
			firstFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame + bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames;
		}
		else
		{
			firstFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame;
			lastFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame + bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on server position, but it's here just for the sake of matching them.

		trap->G2API_SetBoneAnim(self->ghoul2, 0, "model_root", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
		self->client->legsAnimExecute = legsAnim;
		self->client->legsLastFlip = self->client->ps.legsFlip;
	}

tryTorso:
	if (self->localAnimIndex > 1 &&
		bgAllAnims[self->localAnimIndex].anims[torsoAnim].firstFrame == 0 &&
		bgAllAnims[self->localAnimIndex].anims[torsoAnim].numFrames == 0)

	{ //If this fails as well just return.
		return;
	}
	else if (self->s.number >= MAX_CLIENTS &&
		self->s.NPC_class == CLASS_VEHICLE)
	{ //we only want to set the root bone for vehicles
		return;
	}

	if ((self->client->torsoAnimExecute != torsoAnim || self->client->torsoLastFlip != self->client->ps.torsoFlip) &&
		!self->noLumbar)
	{
		aFlags = 0;
		animSpeed = 0;

		f = torsoAnim;

		BG_SaberStartTransAnim(self->s.number, self->client->ps.fd.saberAnimLevel, self->client->ps.weapon, f, &animSpeedScale, self->client->ps.brokenLimbs);

		animSpeed = 50.0f / bgAllAnims[self->localAnimIndex].anims[f].frameLerp;
		lAnimSpeedScale = (animSpeed *= animSpeedScale);

		if (bgAllAnims[self->localAnimIndex].anims[f].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on client position, but it's here just for the sake of matching them.

		if (animSpeed < 0)
		{
			lastFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame;
			firstFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame + bgAllAnims[self->localAnimIndex].anims[f].numFrames;
		}
		else
		{
			firstFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame;
			lastFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame + bgAllAnims[self->localAnimIndex].anims[f].numFrames;
		}

		trap->G2API_SetBoneAnim(self->ghoul2, 0, "lower_lumbar", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, /*firstFrame why was it this before?*/-1, 150);

		self->client->torsoAnimExecute = torsoAnim;
		self->client->torsoLastFlip = self->client->ps.torsoFlip;
		
		setTorso = qtrue;
	}

	if (setTorso &&
		self->localAnimIndex <= 1)
	{ //only set the motion bone for humanoids.
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "Motion", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
	}

#if 0 //disabled for now
	if (self->client->ps.brokenLimbs != self->client->brokenLimbs ||
		setTorso)
	{
		if (self->localAnimIndex <= 1 && self->client->ps.brokenLimbs &&
			(self->client->ps.brokenLimbs & (1 << BROKENLIMB_LARM)))
		{ //broken left arm
			char *brokenBone = "lhumerus";
			animation_t *armAnim;
			int armFirstFrame;
			int armLastFrame;
			int armFlags = 0;
			float armAnimSpeed;

			armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_DEAD21 ];
			self->client->brokenLimbs = self->client->ps.brokenLimbs;

			armFirstFrame = armAnim->firstFrame;
			armLastFrame = armAnim->firstFrame+armAnim->numFrames;
			armAnimSpeed = 50.0f / armAnim->frameLerp;
			armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

			trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
		}
		else if (self->localAnimIndex <= 1 && self->client->ps.brokenLimbs &&
			(self->client->ps.brokenLimbs & (1 << BROKENLIMB_RARM)))
		{ //broken right arm
			char *brokenBone = "rhumerus";
			char *supportBone = "lhumerus";

			self->client->brokenLimbs = self->client->ps.brokenLimbs;

			//Only put the arm in a broken pose if the anim is such that we
			//want to allow it.
			if ((//self->client->ps.weapon == WP_MELEE ||
				self->client->ps.weapon != WP_SABER ||
				BG_SaberStanceAnim(self->client->ps.torsoAnim) ||
				PM_RunningAnim(self->client->ps.torsoAnim)) &&
				(!self->client->saber[1].model[0] || self->client->ps.weapon != WP_SABER))
			{
				int armFirstFrame;
				int armLastFrame;
				int armFlags = 0;
				float armAnimSpeed;
				animation_t *armAnim;

				if (self->client->ps.weapon == WP_MELEE ||
					self->client->ps.weapon == WP_SABER ||
					self->client->ps.weapon == WP_BRYAR_PISTOL)
				{ //don't affect this arm if holding a gun, just make the other arm support it
					armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_ATTACK2 ];

					//armFirstFrame = armAnim->firstFrame;
					armFirstFrame = armAnim->firstFrame+armAnim->numFrames;
					armLastFrame = armAnim->firstFrame+armAnim->numFrames;
					armAnimSpeed = 50.0f / armAnim->frameLerp;
					armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

					trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
				}
				else
				{ //we want to keep the broken bone updated for some cases
					trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				}

				if (self->client->ps.torsoAnim != BOTH_MELEE1 &&
					self->client->ps.torsoAnim != BOTH_MELEE2 &&
					(self->client->ps.torsoAnim == TORSO_WEAPONREADY2 || self->client->ps.torsoAnim == BOTH_ATTACK2 || self->client->ps.weapon < WP_BRYAR_PISTOL))
				{
					//Now set the left arm to "support" the right one
					armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_STAND2 ];
					armFirstFrame = armAnim->firstFrame;
					armLastFrame = armAnim->firstFrame+armAnim->numFrames;
					armAnimSpeed = 50.0f / armAnim->frameLerp;
					armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

					trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
				}
				else
				{ //we want to keep the support bone updated for some cases
					trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				}
			}
			else
			{ //otherwise, keep it set to the same as the torso
				trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
			}
		}
		else if (self->client->brokenLimbs)
		{ //remove the bone now so it can be set again
			char *brokenBone = NULL;
			int broken = 0;

			//Warning: Don't remove bones that you've added as bolts unless you want to invalidate your bolt index
			//(well, in theory, I haven't actually run into the problem)
			if (self->client->brokenLimbs & (1<<BROKENLIMB_LARM))
			{
				brokenBone = "lhumerus";
				broken |= (1<<BROKENLIMB_LARM);
			}
			else if (self->client->brokenLimbs & (1<<BROKENLIMB_RARM))
			{ //can only have one arm broken at once.
				brokenBone = "rhumerus";
				broken |= (1<<BROKENLIMB_RARM);

				//want to remove the support bone too then
				trap->G2API_SetBoneAnim(self->ghoul2, 0, "lhumerus", 0, 1, 0, 0, level.time, -1, 0);
				trap->G2API_RemoveBone(self->ghoul2, "lhumerus", 0);
			}

			assert(brokenBone);

			//Set the flags and stuff to 0, so that the remove will succeed
			trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, 0, 1, 0, 0, level.time, -1, 0);

			//Now remove it
			trap->G2API_RemoveBone(self->ghoul2, brokenBone, 0);
			self->client->brokenLimbs &= ~broken;
		}
	}
#endif
}

void GiveClientItems(gclient_t *client) {
	client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
	client->ps.stats[STAT_HOLDABLE_ITEM] = 0;

	//dont trust uses g_startingItems input, so do this. could loop it.
	if (!client->sess.raceMode) {
		//Give tribes loadout.
		if (client->pers.tribesClass == 1) {
			//Light
			client->ps.stats[STAT_HOLDABLE_ITEMS] |= ((1 << HI_SENTRY_GUN) + (1 << HI_JETPACK));
		}
		if (client->pers.tribesClass == 2) {
			//Medium
			client->ps.stats[STAT_HOLDABLE_ITEMS] |= ((1 << HI_JETPACK) + (1 << HI_EWEB));
		}
		else if (client->pers.tribesClass == 3) {
			//Heavy
			client->ps.stats[STAT_HOLDABLE_ITEMS] |= ((1 << HI_SHIELD) + (1 << HI_JETPACK));
		}
		else {
			if (g_startingItems.integer & (1 << HI_SEEKER))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_SEEKER);
			if (g_startingItems.integer & (1 << HI_SHIELD))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_SHIELD);
			if (g_startingItems.integer & (1 << HI_MEDPAC))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_MEDPAC);
			if (g_startingItems.integer & (1 << HI_MEDPAC_BIG))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_MEDPAC_BIG);
			if (g_startingItems.integer & (1 << HI_BINOCULARS))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS);
			if (g_startingItems.integer & (1 << HI_SENTRY_GUN))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_SENTRY_GUN);
			if (g_startingItems.integer & (1 << HI_JETPACK))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_JETPACK);
			if (g_startingItems.integer & (1 << HI_AMMODISP))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_AMMODISP);
			if (g_startingItems.integer & (1 << HI_EWEB))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_EWEB);
			if (g_startingItems.integer & (1 << HI_CLOAK))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_CLOAK);
			if (g_startingItems.integer & (1 << HI_HEALTHDISP))
				client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_HEALTHDISP);
		}
	}
	else {
		client->ps.stats[STAT_HOLDABLE_ITEMS] = ( 1 << HI_BINOCULARS);
	}
}

void Svcmd_ResetScores_f(void);
void PrintStats(int client);
void G_GiveGunGameWeapon(gclient_t* client) {
	int score;
	qboolean finishedGG = qfalse;
	if (level.gametype == GT_TEAM || level.gametype == GT_CTF) {
		score = level.teamScores[client->ps.persistant[PERS_TEAM]] - client->ps.fd.suicides;
	}
	else {
		score = client->pers.stats.kills - client->ps.fd.suicides;
	}
	//set other ammo to 0, force change to wep?
	client->ps.stats[STAT_WEAPONS] = 0;
	client->forcedFireMode = 0;
	client->ps.fd.forcePowersKnown &= ~(1 << FP_SABERTHROW);
	//client->ps.fd.forcePowerLevel[FP_SABERTHROW] = FORCE_LEVEL_0;

	//how is it less than 0 on end? should be kills = 0, suicides = 0.  is one not getting reset?
	if (score <= 0) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_CONCUSSION);
		client->ps.ammo[AMMO_METAL_BOLTS] = 999;
		client->ps.weapon = WP_CONCUSSION;
		client->forcedFireMode = 1;
	}
	else if (score == 1) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_ROCKET_LAUNCHER);
		client->ps.ammo[AMMO_ROCKETS] = 999;
		client->ps.weapon = WP_ROCKET_LAUNCHER;
		client->forcedFireMode = 1;
	}
	else if (score == 2) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_ROCKET_LAUNCHER);
		client->ps.ammo[AMMO_ROCKETS] = 999;
		client->ps.weapon = WP_ROCKET_LAUNCHER;
		client->forcedFireMode = 2;
	}
	else if (score == 3) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_BLASTER);
		client->ps.ammo[AMMO_BLASTER] = 999;
		client->ps.weapon = WP_BLASTER;
	}
	else if (score == 4) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_REPEATER);
		client->ps.ammo[AMMO_METAL_BOLTS] = 999;
		client->ps.weapon = WP_REPEATER;
		client->forcedFireMode = 2;
	}
	else if (score == 5) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_REPEATER);
		client->ps.ammo[AMMO_METAL_BOLTS] = 999;
		client->ps.weapon = WP_REPEATER;
		client->forcedFireMode = 1;
	}
	else if (score == 6) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_FLECHETTE);
		client->ps.ammo[AMMO_METAL_BOLTS] = 999;
		client->ps.weapon = WP_FLECHETTE;
	}
	else if (score == 7) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_CONCUSSION);
		client->ps.ammo[AMMO_METAL_BOLTS] = 999;
		client->ps.weapon = WP_CONCUSSION;
		client->forcedFireMode = 2;
	}
	else if (score == 8) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_STUN_BATON);
		client->ps.weapon = WP_STUN_BATON;
	}
	else if (score == 9) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_DISRUPTOR);
		client->ps.ammo[AMMO_POWERCELL] = 999;
		client->ps.weapon = WP_DISRUPTOR;
	}
	else if (score == 10) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_BOWCASTER);
		client->ps.ammo[AMMO_POWERCELL] = 999;
		client->ps.weapon = WP_BOWCASTER;
	}
	else if (score == 11) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_DEMP2);
		client->ps.ammo[AMMO_POWERCELL] = 999;
		client->ps.weapon = WP_DEMP2;
		client->forcedFireMode = 1;
	}
	else if (score == 12) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_DEMP2);
		client->ps.ammo[AMMO_POWERCELL] = 999;
		client->ps.weapon = WP_DEMP2;
		client->forcedFireMode = 2;
	}
	else if (score == 13) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_THERMAL);
		client->ps.ammo[AMMO_THERMAL] = 999;
		client->ps.weapon = WP_THERMAL;
		client->forcedFireMode = 2;
	}
	else if (score == 14) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_BRYAR_OLD);
		client->ps.ammo[AMMO_BLASTER] = 999;
		client->ps.weapon = WP_BRYAR_OLD;
	}
	else if (score == 15) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_BRYAR_PISTOL);
		client->ps.ammo[AMMO_BLASTER] = 999;
		client->ps.weapon = WP_BRYAR_PISTOL;
	}
	else if (score == 16) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_THERMAL);
		client->ps.ammo[AMMO_THERMAL] = 999;
		client->ps.weapon = WP_THERMAL;
		client->forcedFireMode = 1;
	}
	else if (score == 17) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
		client->ps.weapon = WP_SABER;
		client->ps.fd.forcePowersKnown |= (1 << FP_SABERTHROW);
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = FORCE_LEVEL_3;
		client->forcedFireMode = 2;
	}
	else if (score == 18) {
		client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
		client->ps.weapon = WP_SABER;
		client->forcedFireMode = 1;
	}
	else if (score >= 19) { //if (meansOfDeath == MOD_SABER || (meansOfDeath == WP_MELEE && attacker->client->pers.stats.kills >= 12)) {
		finishedGG = qtrue;
	}

	if (finishedGG) {
		int i;
		gentity_t* other;

		if (level.gametype == GT_TEAM || level.gametype == GT_CTF) {
			trap->SendServerCommand(-1, va("print \"%s team (%s)^3 won the gungame\n\"", TeamName(client->ps.persistant[PERS_TEAM]), client->pers.netname));
			trap->SendServerCommand(-1, va("cp \"%s team (%s)^3 won the gungame\n\n\n\n\n\n\n\n\n\n\n\n\"", TeamName(client->ps.persistant[PERS_TEAM]), client->pers.netname));
		}
		else {
			trap->SendServerCommand(-1, va("print \"%s^3 won the gungame\n\"", client->pers.netname));
			trap->SendServerCommand(-1, va("cp \"%s^3 won the gungame\n\n\n\n\n\n\n\n\n\n\n\n\"", client->pers.netname));
		}
		PrintStats(-1);//JAPRO STATS
		for (i = 0; i < level.numConnectedClients; i++) { //Kill every1? or every1 but me? or just reset weps? 
			other = &g_entities[level.sortedClients[i]];
			if (other->inuse && other->client && !other->client->sess.raceMode) {
				other->client->ps.stats[STAT_WEAPONS] = (1 << WP_CONCUSSION);
				other->client->ps.ammo[AMMO_METAL_BOLTS] = 999;
				other->client->ps.weapon = WP_CONCUSSION;
				other->client->forcedFireMode = 1;
				other->client->ps.zoomMode = 0;
			}
		}
		Svcmd_ResetScores_f();
	}
	else if (level.gametype == GT_TEAM || level.gametype == GT_CTF) { //Update our teams guns
		int i, j;
		gentity_t* other;
		for (i = 0; i < level.numConnectedClients; i++) {
			other = &g_entities[level.sortedClients[i]];
			if (other->inuse && other->client && !other->client->sess.raceMode && (other->client->ps.persistant[PERS_TEAM] == client->ps.persistant[PERS_TEAM])) {
				other->client->ps.stats[STAT_WEAPONS] = client->ps.stats[STAT_WEAPONS];
				for (j = AMMO_BLASTER; j < AMMO_MAX; j++)//w/e
					other->client->ps.ammo[j] = client->ps.ammo[j];
				other->client->ps.weapon = client->ps.weapon;
				other->client->forcedFireMode = client->forcedFireMode;
				other->client->ps.zoomMode = 0;

				if (client->ps.fd.forcePowersKnown & (1<<FP_SABERTHROW))
					other->client->ps.fd.forcePowersKnown |= (1 << FP_SABERTHROW);
				else
					other->client->ps.fd.forcePowersKnown &= ~(1 << FP_SABERTHROW);

				other->client->ps.fd.forcePowerLevel[FP_SABERTHROW] = client->ps.fd.forcePowerLevel[FP_SABERTHROW];
			}
		}
	}
	client->ps.zoomMode = 0;
}

void GiveClientWeapons(gclient_t *client) {


	client->ps.stats[STAT_WEAPONS] = 0;

	client->ps.ammo[AMMO_BLASTER] = 0;
	client->ps.ammo[AMMO_POWERCELL] = 0;
	client->ps.ammo[AMMO_METAL_BOLTS] = 0;
	client->ps.ammo[AMMO_ROCKETS] = 0;
	client->ps.ammo[AMMO_DETPACK] = 0;
	client->ps.ammo[AMMO_TRIPMINE] = 0;
	client->ps.ammo[AMMO_THERMAL] = 0;

	//I guess this could be cleaned up a ton, but that trusts the user to not put a retarded value for g_startingWeapons ?
	if (g_gunGame.integer) {
		G_GiveGunGameWeapon(client);
	}
		//Give tribes loadout.
	else if (client->pers.tribesClass == 1) {
			//Light
			client->ps.stats[STAT_WEAPONS] |= ((1 << WP_DISRUPTOR) + (1 << WP_BOWCASTER) + (1 << WP_THERMAL) + (1 << WP_SABER));
			client->ps.ammo[AMMO_THERMAL] = 2;
			client->ps.ammo[AMMO_POWERCELL] = 900;
	}
	else if (client->pers.tribesClass == 2) {
		//Medium
		client->ps.stats[STAT_WEAPONS] |= ((1 << WP_BLASTER) + (1 << WP_FLECHETTE) + (1 << WP_CONCUSSION) + (1 << WP_STUN_BATON) + (1 << WP_SABER));
		client->ps.ammo[AMMO_POWERCELL] = 900;
		client->ps.ammo[AMMO_BLASTER] = 500;
		client->ps.ammo[AMMO_METAL_BOLTS] = 800;
	}
	else if (client->pers.tribesClass == 3) {
		//Heavy
		client->ps.stats[STAT_WEAPONS] |= ((1 << WP_REPEATER) + (1 << WP_DEMP2) + (1 << WP_ROCKET_LAUNCHER) + (1 << WP_TRIP_MINE) + (1 << WP_SABER));
		client->ps.ammo[AMMO_POWERCELL] = 600;
		client->ps.ammo[AMMO_METAL_BOLTS] = 800;
		client->ps.ammo[AMMO_ROCKETS] = 30;
		client->ps.ammo[AMMO_TRIPMINE] = 2;
	}
	else {
		int i;

		for (i = WP_STUN_BATON; i <= WP_BRYAR_OLD; i++) {
			if (g_startingWeapons.integer & (1 << i))
				client->ps.stats[STAT_WEAPONS] |= (1 << i);
		}

		if (g_tweakWeapons.integer & WT_TRIBES) { //oh right, startingWeapons bit 1 gives more ammo
			if (g_startingWeapons.integer & (1 << WP_BLASTER) || g_startingWeapons.integer & (1 << WP_BRYAR_OLD))
				client->ps.ammo[AMMO_BLASTER] = 600;
			if (g_startingWeapons.integer & (1 << WP_DISRUPTOR) || g_startingWeapons.integer & (1 << WP_BOWCASTER) || g_startingWeapons.integer & (1 << WP_DEMP2))
				client->ps.ammo[AMMO_POWERCELL] = 500;
			if (g_startingWeapons.integer & (1 << WP_REPEATER) || g_startingWeapons.integer & (1 << WP_FLECHETTE) || g_startingWeapons.integer & (1 << WP_CONCUSSION))
				client->ps.ammo[AMMO_METAL_BOLTS] = 900;
			if (g_startingWeapons.integer & (1 << WP_ROCKET_LAUNCHER))
				client->ps.ammo[AMMO_ROCKETS] = 20;
			if (g_startingWeapons.integer & (1 << WP_DET_PACK))
				client->ps.ammo[AMMO_DETPACK] = 1;
			if (g_startingWeapons.integer & (1 << WP_TRIP_MINE))
				client->ps.ammo[AMMO_TRIPMINE] = 2;
			if (g_startingWeapons.integer & (1 << WP_THERMAL))
				client->ps.ammo[AMMO_THERMAL] = 2;
		}
		else if (!(g_startingWeapons.integer & (1 << 0))) { //oh right, startingWeapons bit 1 gives more ammo
			if (g_startingWeapons.integer & (1 << WP_BLASTER) || g_startingWeapons.integer & (1 << WP_BRYAR_OLD))
				client->ps.ammo[AMMO_BLASTER] = 300;
			if (g_startingWeapons.integer & (1 << WP_DISRUPTOR) || g_startingWeapons.integer & (1 << WP_BOWCASTER) || g_startingWeapons.integer & (1 << WP_DEMP2))
				client->ps.ammo[AMMO_POWERCELL] = 200;
			if (g_startingWeapons.integer & (1 << WP_REPEATER) || g_startingWeapons.integer & (1 << WP_FLECHETTE) || g_startingWeapons.integer & (1 << WP_CONCUSSION))
				client->ps.ammo[AMMO_METAL_BOLTS] = 200;
			if (g_startingWeapons.integer & (1 << WP_ROCKET_LAUNCHER))
				client->ps.ammo[AMMO_ROCKETS] = 2;
			if (g_startingWeapons.integer & (1 << WP_DET_PACK))
				client->ps.ammo[AMMO_DETPACK] = 1;
			if (g_startingWeapons.integer & (1 << WP_TRIP_MINE))
				client->ps.ammo[AMMO_TRIPMINE] = 1;
			if (g_startingWeapons.integer & (1 << WP_THERMAL))
				client->ps.ammo[AMMO_THERMAL] = 1;
		}
		else {
			if (g_startingWeapons.integer & (1 << WP_BLASTER) || g_startingWeapons.integer & (1 << WP_BRYAR_OLD))
				client->ps.ammo[AMMO_BLASTER] = 600;
			if (g_startingWeapons.integer & (1 << WP_DISRUPTOR) || g_startingWeapons.integer & (1 << WP_BOWCASTER) || g_startingWeapons.integer & (1 << WP_DEMP2))
				client->ps.ammo[AMMO_POWERCELL] = 600;
			if (g_startingWeapons.integer & (1 << WP_REPEATER) || g_startingWeapons.integer & (1 << WP_FLECHETTE) || g_startingWeapons.integer & (1 << WP_CONCUSSION))
				client->ps.ammo[AMMO_METAL_BOLTS] = 600;
			if (g_startingWeapons.integer & (1 << WP_ROCKET_LAUNCHER))
				client->ps.ammo[AMMO_ROCKETS] = 25;
			if (g_startingWeapons.integer & (1 << WP_DET_PACK))
				client->ps.ammo[AMMO_DETPACK] = 10;
			if (g_startingWeapons.integer & (1 << WP_TRIP_MINE))
				client->ps.ammo[AMMO_TRIPMINE] = 10;
			if (g_startingWeapons.integer & (1 << WP_THERMAL))
				client->ps.ammo[AMMO_THERMAL] = 10;
		}
	}
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
extern qboolean WP_HasForcePowers( const playerState_t *ps );
void ClientSpawn(gentity_t *ent) {
	int					i = 0, index = 0, saveSaberNum = ENTITYNUM_NONE, wDisable = 0, savedSiegeIndex = 0, maxHealth = 100;
	vec3_t				spawn_origin, spawn_angles;
	gentity_t			*spawnPoint = NULL, *tent = NULL;
	gclient_t			*client = NULL;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	forcedata_t			savedForce;
	saberInfo_t			saberSaved[MAX_SABERS];
	int					persistant[MAX_PERSISTANT] = {0};
	int					flags, gameFlags, savedPing, accuracy_hits, accuracy_shots, eventSequence;
	void				*g2WeaponPtrs[MAX_SABERS];
	char				userinfo[MAX_INFO_STRING] = {0}, *key = NULL, *value = NULL, *saber = NULL;
	qboolean			changedSaber = qfalse, inSiegeWithClass = qfalse;

	index = ent - g_entities;
	client = ent->client;

	//first we want the userinfo so we can see if we should update this client's saber -rww
	trap->GetUserinfo( index, userinfo, sizeof( userinfo ) );

	for ( i=0; i<MAX_SABERS; i++ )
	{
		saber = (i&1) ? ent->client->pers.saber2 : ent->client->pers.saber1;
		value = Info_ValueForKey( userinfo, va( "saber%i", i+1 ) );
		if ( saber && value &&
			(Q_stricmp( value, saber ) || !saber[0] || !ent->client->saber[0].model[0]) )
		{ //doesn't match up (or our saber is BS), we want to try setting it
			if ( G_SetSaber( ent, i, value, qfalse ) )
				changedSaber = qtrue;

			//Well, we still want to say they changed then (it means this is siege and we have some overrides)
			else if ( !saber[0] || !ent->client->saber[0].model[0] )
				changedSaber = qtrue;
		}
	}

	if ( changedSaber )
	{ //make sure our new info is sent out to all the other clients, and give us a valid stance
		if ( !ClientUserinfoChanged( ent->s.number ) )
			return;

		//make sure the saber models are updated
		G_SaberModelSetup( ent );

		for ( i=0; i<MAX_SABERS; i++ )
		{
			saber = (i&1) ? ent->client->pers.saber2 : ent->client->pers.saber1;
			key = va( "saber%d", i+1 );
			value = Info_ValueForKey( userinfo, key );
			if ( Q_stricmp( value, saber ) )
			{// they don't match up, force the user info
				Info_SetValueForKey( userinfo, key, saber );
				trap->SetUserinfo( ent->s.number, userinfo );
			}
		}

		if ( ent->client->saber[0].model[0] && ent->client->saber[1].model[0] )
		{ //dual
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = SS_DUAL;
		}
		else if ( (ent->client->saber[0].saberFlags&SFL_TWO_HANDED) )
		{ //staff
			ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = SS_STAFF;
		}
		else
		{
			ent->client->sess.saberLevel = Com_Clampi( SS_FAST, SS_STRONG, ent->client->sess.saberLevel );
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel;

			// limit our saber style to our force points allocated to saber offense
			if ( level.gametype != GT_SIEGE && ent->client->ps.fd.saberAnimLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel = ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE];
		}
		if ( level.gametype != GT_SIEGE )
		{// let's just make sure the styles we chose are cool
			if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
			{
				WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
				ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
			}
		}
	}

	if (client->ps.fd.forceDoInit)
	{ //force a reread of force powers
		WP_InitForcePowers( ent );
		client->ps.fd.forceDoInit = 0;
	}

	if (ent->client->ps.fd.saberAnimLevel != SS_STAFF && ent->client->ps.fd.saberAnimLevel != SS_DUAL &&
		ent->client->ps.fd.saberAnimLevel == ent->client->ps.fd.saberDrawAnimLevel &&
		ent->client->ps.fd.saberAnimLevel == ent->client->sess.saberLevel)
	{
		ent->client->sess.saberLevel = Com_Clampi( SS_FAST, SS_STRONG, ent->client->sess.saberLevel );
		ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel;

		// limit our saber style to our force points allocated to saber offense
		if ( level.gametype != GT_SIEGE && ent->client->ps.fd.saberAnimLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
			ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel = ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE];
	}

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
						spawn_origin, spawn_angles);
	} else if (level.gametype == GT_CTF || level.gametype == GT_CTY) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint ( 
						client->sess.sessionTeam, 
						client->pers.teamState.state, 
						spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
	}
	else if (level.gametype == GT_SIEGE)
	{
		spawnPoint = SelectSiegeSpawnPoint (
						client->siegeClass,
						client->sess.sessionTeam, 
						client->pers.teamState.state, 
						spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
	}
	else {
		if (level.gametype == GT_POWERDUEL)
		{
			spawnPoint = SelectDuelSpawnPoint(client->sess.duelTeam, client->ps.origin, spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
		}
		else if (level.gametype == GT_DUEL)
		{	// duel 
			spawnPoint = SelectDuelSpawnPoint(DUELTEAM_SINGLE, client->ps.origin, spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
		}
		else
		{
			// the first spawn should be at a good looking spot
			if ( !client->pers.initialSpawn && client->pers.localClient ) {
				client->pers.initialSpawn = qtrue;
				spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles, client->sess.sessionTeam, !!(ent->r.svFlags & SVF_BOT) );
			} else {
				// don't spawn near existing origin if possible
				spawnPoint = SelectSpawnPoint ( 
					client->ps.origin, 
					spawn_origin, spawn_angles, client->sess.sessionTeam, !!(ent->r.svFlags & SVF_BOT) );

				if (g_duelRespawn.integer && level.gametype == GT_FFA && VectorLength(client->pers.respawnLocation) && !SpotWouldTelefrag3(client->pers.respawnLocation)) {
					VectorCopy(client->pers.respawnLocation, spawn_origin);
					spawn_angles[YAW] = client->pers.respawnAngle;
				}//sad hack

			}
		}
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT);
	flags ^= EF_TELEPORT_BIT;
	gameFlags = ent->client->mGameFlags & ( PSG_VOTED | PSG_TEAMVOTED);

	// clear everything but the persistant data

	G_ResetTrail( ent );//unlagged

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i=0; i<MAX_PERSISTANT; i++ )
		persistant[i] = client->ps.persistant[i];

	eventSequence = client->ps.eventSequence;

	savedForce = client->ps.fd;

	saveSaberNum = client->ps.saberEntityNum;

	savedSiegeIndex = client->siegeClass;

	for ( i=0; i<MAX_SABERS; i++ )
	{
		saberSaved[i] = client->saber[i];
		g2WeaponPtrs[i] = client->weaponGhoul2[i];
	}

	for ( i=0; i<HL_MAX; i++ )
		ent->locationDamage[i] = 0;

	memset( client, 0, sizeof( *client ) ); // bk FIXME: Com_Memset?
	client->bodyGrabIndex = ENTITYNUM_NONE;

	//Get the skin RGB based on his userinfo
	client->ps.customRGBA[0] = (value=Info_ValueForKey( userinfo, "char_color_red" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[1] = (value=Info_ValueForKey( userinfo, "char_color_green" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[2] = (value=Info_ValueForKey( userinfo, "char_color_blue" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;

	//Prevent skins being too dark
	if ( g_charRestrictRGB.integer && ((client->ps.customRGBA[0]+client->ps.customRGBA[1]+client->ps.customRGBA[2]) < 100) )
		client->ps.customRGBA[0] = client->ps.customRGBA[1] = client->ps.customRGBA[2] = 255;

	client->ps.customRGBA[3]=255;

	if ( level.gametype >= GT_TEAM && level.gametype != GT_SIEGE && !g_jediVmerc.integer )
	{
		char skin[MAX_QPATH] = {0}, model[MAX_QPATH] = {0};
		vec3_t colorOverride = {0.0f};

		VectorClear( colorOverride );
		Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );

		BG_ValidateSkinForTeam( model, skin, savedSess.sessionTeam, colorOverride );
		if ( colorOverride[0] != 0.0f || colorOverride[1] != 0.0f || colorOverride[2] != 0.0f )
			VectorScaleM( colorOverride, 255.0f, client->ps.customRGBA );
	}

	client->siegeClass = savedSiegeIndex;

	for ( i=0; i<MAX_SABERS; i++ )
	{
		client->saber[i] = saberSaved[i];
		client->weaponGhoul2[i] = g2WeaponPtrs[i];
	}

	//or the saber ent num
	client->ps.saberEntityNum = saveSaberNum;
	client->saberStoredIndex = saveSaberNum;

	client->ps.fd = savedForce;

	client->ps.duelIndex = ENTITYNUM_NONE;

	//spawn with 100
	client->ps.jetpackFuel = 100;
	client->ps.cloakFuel = 100;

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i=0; i<MAX_PERSISTANT; i++ )
		client->ps.persistant[i] = persistant[i];

	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	// set max health
	if (level.gametype == GT_SIEGE && client->siegeClass != -1)
	{
		siegeClass_t *scl = &bgSiegeClasses[client->siegeClass];
		maxHealth = 100;

		if (scl->maxhealth)
		{
			maxHealth = scl->maxhealth;
		}
	}
	else
	{
		//maxHealth = Com_Clampi( 1, 100, atoi( Info_ValueForKey( userinfo, "handicap" ) ) );
		maxHealth = 100;//i dont think we want handicap to work..?
		if (g_showHealth.integer && !client->sess.raceMode)
			ent->maxHealth = 100;//JAPRO , kay...
		else 
			ent->maxHealth = 0;
	}
	client->pers.maxHealth = maxHealth;//atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > maxHealth ) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;
	client->mGameFlags = gameFlags;

	client->ps.groundEntityNum = ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->playerState = &ent->client->ps;
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	if (g_raceMode.integer == 1 && level.gametype == GT_FFA)
		client->sess.raceMode = qtrue;
	else if (g_raceMode.integer && (level.gametype == GT_TEAM || level.gametype == GT_CTF) && client->sess.sessionTeam == TEAM_FREE)
		client->sess.raceMode = qtrue;
	if (client->sess.sessionTeam != TEAM_FREE && client->sess.sessionTeam != TEAM_SPECTATOR)
		client->sess.raceMode = qfalse;
	else if (!g_raceMode.integer)
		client->sess.raceMode = qfalse;

	if (client->sess.raceMode)
		client->ps.stats[STAT_RACEMODE] = 1;
	else
		client->ps.stats[STAT_RACEMODE] = 0;
	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);
	if (!client->sess.raceMode && client->pers.tribesClass == 3) {
		client->ps.standheight = DEFAULT_MAXS_2 * 1.25f; //this model has no neck //revert this?
		client->ps.crouchheight = CROUCH_MAXS_2 * 1.25f;
	}
	else if (!client->sess.raceMode && client->pers.tribesClass == 1) {
		client->ps.standheight = DEFAULT_MAXS_2 * 0.94f;
		client->ps.crouchheight = CROUCH_MAXS_2 * 0.94f;
	}
	else {
		client->ps.standheight = DEFAULT_MAXS_2;
		client->ps.crouchheight = CROUCH_MAXS_2;
	}

	client->ps.clientNum = index;
	//give default weapons
	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );

	if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
	{
		wDisable = g_duelWeaponDisable.integer;
	}
	else
	{
		wDisable = g_weaponDisable.integer;
	}

	client->savedJumpLevel = 0;//rabbit

	if ( level.gametype != GT_HOLOCRON 
		&& level.gametype != GT_JEDIMASTER 
		&& !HasSetSaberOnly()
		&& !AllForceDisabled( g_forcePowerDisable.integer )
		&& g_jediVmerc.integer )
	{
		if ( level.gametype >= GT_TEAM && (client->sess.sessionTeam == TEAM_BLUE || client->sess.sessionTeam == TEAM_RED) )
		{//In Team games, force one side to be merc and other to be jedi
			if ( level.numPlayingClients > 0 )
			{//already someone in the game
				int forceTeam = TEAM_SPECTATOR;
				for ( i = 0 ; i < level.maxclients ; i++ ) 
				{
					if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
						continue;
					}
					if ( level.clients[i].sess.sessionTeam == TEAM_BLUE || level.clients[i].sess.sessionTeam == TEAM_RED ) 
					{//in-game
						if ( WP_HasForcePowers( &level.clients[i].ps ) )
						{//this side is using force
							forceTeam = level.clients[i].sess.sessionTeam;
						}
						else
						{//other team is using force
							if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							{
								forceTeam = TEAM_RED;
							}
							else
							{
								forceTeam = TEAM_BLUE;
							}
						}
						break;
					}
				}
				if ( WP_HasForcePowers( &client->ps ) && client->sess.sessionTeam != forceTeam )
				{//using force but not on right team, switch him over
					const char *teamName = TeamName( forceTeam );
					//client->sess.sessionTeam = forceTeam;
					SetTeam( ent, (char *)teamName, qfalse );
					return;
				}
			}
		}

		if ( WP_HasForcePowers( &client->ps ) )
		{
			client->ps.trueNonJedi = qfalse;
			client->ps.trueJedi = qtrue;
			//make sure they only use the saber
			client->ps.weapon = WP_SABER;
			client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
		}
		else
		{//no force powers set
			client->ps.trueNonJedi = qtrue;
			client->ps.trueJedi = qfalse;
			if (!wDisable || !(wDisable & (1 << WP_BRYAR_PISTOL)))
			{
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
			}
			if (!wDisable || !(wDisable & (1 << WP_BLASTER)))
			{
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BLASTER );
			}
			if (!wDisable || !(wDisable & (1 << WP_BOWCASTER)))
			{
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BOWCASTER );
			}
			client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
			client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
			client->ps.ammo[AMMO_POWERCELL] = ammoData[AMMO_POWERCELL].max;
			client->ps.weapon = WP_BRYAR_PISTOL;
		}
	}
	else
	{//jediVmerc is incompatible with this gametype, turn it off!
		if (g_jediVmerc.integer)
			trap->Cvar_Set( "g_jediVmerc", "0" );
		if (level.gametype == GT_HOLOCRON)
		{
			//always get free saber level 1 in holocron
			client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );	//these are precached in g_items, ClearRegisteredItems()
		}
		else
		{
			if (client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
			{
				if ((g_startingWeapons.integer & (1 << WP_SABER)) || (level.gametype == GT_SIEGE))
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );	//these are precached in g_items, ClearRegisteredItems()
			}
			else
			{ //if you don't have saber attack rank then you don't get a saber
				if ((g_startingWeapons.integer & (1 << WP_SABER)) || (level.gametype == GT_SIEGE))
					client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
			}
		}

		if (level.gametype != GT_SIEGE) {
			if (!client->sess.raceMode)
			{ //loda fixme.. this can just be set?
				GiveClientWeapons(client);
			}
		}

		if (level.gametype == GT_JEDIMASTER)
		{
			client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
			client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
		}

		if (client->ps.stats[STAT_WEAPONS] & (1 << WP_SABER) && (!g_tweakWeapons.integer & WT_TRIBES))
		{
			client->ps.weapon = WP_SABER;
		}
		else if (client->sess.sessionTeam != TEAM_SPECTATOR)//huh, stop this weird scope in spec, Loda fixme, this should be a fix to the scope itself
		{
			if (client->pers.tribesClass == 1) {
				client->ps.weapon = WP_BOWCASTER;
			}
			else if (client->pers.tribesClass == 2) {
				client->ps.weapon = WP_CONCUSSION;
			}
			else if (client->pers.tribesClass == 3) {
				client->ps.weapon = WP_ROCKET_LAUNCHER;
			}
			else {
				int weap;

				for (weap = 16; weap >= 1; weap--) {//From 16 downto 1
					if (client->ps.stats[STAT_WEAPONS] & (1 << weap)) {//We have it
						client->ps.weapon = weap;//Set their weapon to the highest one they have, if saber not enabled.
						break;
					}
				}
			}
		}
		if (!client->ps.weapon)//Fall back to melee if they dont have anything I guess, so we dont fallback to pistol later
				client->ps.weapon = WP_MELEE;
	}

	/*
	client->ps.stats[STAT_HOLDABLE_ITEMS] |= ( 1 << HI_BINOCULARS );
	client->ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(HI_BINOCULARS, IT_HOLDABLE);
	*/

	if (level.gametype == GT_SIEGE && client->siegeClass != -1 &&
		client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //well then, we will use a custom weaponset for our class
		int m = 0;

		client->ps.stats[STAT_WEAPONS] = bgSiegeClasses[client->siegeClass].weapons;

		if (client->ps.stats[STAT_WEAPONS] & (1 << WP_SABER))
		{
			client->ps.weapon = WP_SABER;
		}
		else if (client->ps.stats[STAT_WEAPONS] & (1 << WP_BRYAR_PISTOL))
		{
			client->ps.weapon = WP_BRYAR_PISTOL;
		}
		else
		{
			client->ps.weapon = WP_MELEE;
		}
		inSiegeWithClass = qtrue;

		while (m < WP_NUM_WEAPONS)
		{
			if (client->ps.stats[STAT_WEAPONS] & (1 << m))
			{
				if (client->ps.weapon != WP_SABER)
				{ //try to find the highest ranking weapon we have
					if (m > client->ps.weapon)
					{
						client->ps.weapon = m;
					}
				}

				if (m >= WP_BRYAR_PISTOL)
				{ //Max his ammo out for all the weapons he has.
					if ( level.gametype == GT_SIEGE 
						&& m == WP_ROCKET_LAUNCHER )
					{//don't give full ammo!
						//FIXME: extern this and check it when getting ammo from supplier, pickups or ammo stations!
						if ( client->siegeClass != -1 &&
							(bgSiegeClasses[client->siegeClass].classflags & (1<<CFL_SINGLE_ROCKET)) )
						{
							client->ps.ammo[weaponData[m].ammoIndex] = 1;
						}
						else
						{
							client->ps.ammo[weaponData[m].ammoIndex] = 10;
						}
					}
					else
					{
						if ( level.gametype == GT_SIEGE 
							&& client->siegeClass != -1
							&& (bgSiegeClasses[client->siegeClass].classflags & (1<<CFL_EXTRA_AMMO)) )
						{//double ammo
							client->ps.ammo[weaponData[m].ammoIndex] = ammoData[weaponData[m].ammoIndex].max*2;
							client->ps.eFlags |= EF_DOUBLE_AMMO;
						}
						else
						{
							client->ps.ammo[weaponData[m].ammoIndex] = ammoData[weaponData[m].ammoIndex].max;
						}
					}
				}
			}
			m++;
		}
	}

	if (level.gametype == GT_SIEGE &&
		client->siegeClass != -1 &&
		client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //use class-specified inventory
		client->ps.stats[STAT_HOLDABLE_ITEMS] = bgSiegeClasses[client->siegeClass].invenItems;
		client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
	}
	else
	{
		GiveClientItems(client);
	}

	if (level.gametype == GT_SIEGE &&
		client->siegeClass != -1 &&
		bgSiegeClasses[client->siegeClass].powerups &&
		client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //this class has some start powerups
		i = 0;
		while (i < PW_NUM_POWERUPS)
		{
			if (bgSiegeClasses[client->siegeClass].powerups & (1 << i))
			{
				client->ps.powerups[i] = Q3_INFINITE;
			}
			i++;
		}
	}

	if ( client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		client->ps.stats[STAT_WEAPONS] = 0;
		client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
		client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
	}

// nmckenzie: DESERT_SIEGE... or well, siege generally.  This was over-writing the max value, which was NOT good for siege.
	if (!g_startingWeapons.integer && inSiegeWithClass == qfalse)
	{
		client->ps.ammo[AMMO_BLASTER] = 100; //ammoData[AMMO_BLASTER].max; //100 seems fair.
	}
//	client->ps.ammo[AMMO_POWERCELL] = ammoData[AMMO_POWERCELL].max;
//	client->ps.ammo[AMMO_FORCE] = ammoData[AMMO_FORCE].max;
//	client->ps.ammo[AMMO_METAL_BOLTS] = ammoData[AMMO_METAL_BOLTS].max;
//	client->ps.ammo[AMMO_ROCKETS] = ammoData[AMMO_ROCKETS].max;
/*
	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_BRYAR_PISTOL);
	if ( level.gametype == GT_TEAM ) {
		client->ps.ammo[WP_BRYAR_PISTOL] = 50;
	} else {
		client->ps.ammo[WP_BRYAR_PISTOL] = 100;
	}
*/
	client->ps.rocketLockIndex = ENTITYNUM_NONE;
	client->ps.rocketLockTime = 0;

	//rww - Set here to initialize the circling seeker drone to off.
	//A quick note about this so I don't forget how it works again:
	//ps.genericEnemyIndex is kept in sync between the server and client.
	//When it gets set then an entitystate value of the same name gets
	//set along with an entitystate flag in the shared bg code. Which
	//is why a value needs to be both on the player state and entity state.
	//(it doesn't seem to just carry over the entitystate value automatically
	//because entity state value is derived from player state data or some
	//such)
	client->ps.genericEnemyIndex = -1;

	client->ps.isJediMaster = qfalse;

	if (client->ps.fallingToDeath)
	{
		client->ps.fallingToDeath = 0;
		client->noCorpse = qtrue;
	}

	//Do per-spawn force power initialization
	WP_SpawnInitForcePowers( ent );


	if (client->sess.raceMode) {
		ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] = 100;
		if (client->sess.movementStyle == MV_COOP_JKA)
			client->ps.fd.forcePowerLevel[FP_LEVITATION] = 1;
	}
	// health will count down towards max_health
	else if (level.gametype == GT_SIEGE &&
		client->siegeClass != -1 &&
		bgSiegeClasses[client->siegeClass].starthealth)
	{ //class specifies a start health, so use it
		ent->health = client->ps.stats[STAT_HEALTH] = bgSiegeClasses[client->siegeClass].starthealth;
	}
	else if ( level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL )
	{//only start with 100 health in Duel
		if ( level.gametype == GT_POWERDUEL && client->sess.duelTeam == DUELTEAM_LONE )
		{
			if ( duel_fraglimit.integer )
			{
				
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] =
					g_powerDuelStartHealth.integer - ((g_powerDuelStartHealth.integer - g_powerDuelEndHealth.integer) * (float)client->sess.wins / (float)duel_fraglimit.integer);
			}
			else
			{
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] = 150;
			}
		}
		else
		{
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] = 100;
		}
	}
	else if (client->ps.stats[STAT_MAX_HEALTH] <= 100)
	{
		ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 1.25;
	}
	else if (client->ps.stats[STAT_MAX_HEALTH] < 125)
	{
		ent->health = client->ps.stats[STAT_HEALTH] = 125;
	}
	else
	{
		ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
	}

	// Start with a small amount of armor as well.
	if (level.gametype == GT_SIEGE &&
		client->siegeClass != -1 /*&&
		bgSiegeClasses[client->siegeClass].startarmor*/)
	{ //class specifies a start armor amount, so use it
		client->ps.stats[STAT_ARMOR] = bgSiegeClasses[client->siegeClass].startarmor;
	}
	else if ( level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL )
	{//no armor in duel
		int startArmor = 0;
		if (g_startingItems.integer & (1 << (HI_NUM_HOLDABLE+1))) {
			if (g_duelStartArmor.integer > 0)
				startArmor = g_duelStartArmor.integer;
		}
		client->ps.stats[STAT_ARMOR] = startArmor;
	}
	else
	{//Clan Arena starting armor/hp here
		if (client->pers.tribesClass == 1) {//light
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
			client->ps.stats[STAT_ARMOR] = 0;
		}
		else if (client->pers.tribesClass == 2) {//med
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
			client->ps.stats[STAT_ARMOR] = 25;
		}
		else if (client->pers.tribesClass == 3) {//heavy
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
			client->ps.stats[STAT_ARMOR] = 100;
		}
		else if (g_startingItems.integer & (1 << (HI_NUM_HOLDABLE + 2)))//sad
			client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_HEALTH];
		else
			client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_HEALTH] * 0.25;
	}

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap->GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );
	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	if (!level.intermissiontime) {
		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
			G_KillBox(ent);
			// force the base weapon up
			//client->ps.weapon = WP_BRYAR_PISTOL;
			//client->ps.weaponstate = FIRST_WEAPON;
			if (client->ps.weapon <= WP_NONE)
			{
				client->ps.weapon = WP_BRYAR_PISTOL;
			}

			client->ps.torsoTimer = client->ps.legsTimer = 0;

			if (client->ps.weapon == WP_SABER)
			{
				G_SetAnim(ent, NULL, SETANIM_BOTH, BOTH_STAND1TO2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS, 0);
			}
			else
			{
				G_SetAnim(ent, NULL, SETANIM_TORSO, TORSO_RAISEWEAP1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS, 0);
				client->ps.legsAnim = WeaponReadyAnim[client->ps.weapon];
			}
			client->ps.weaponstate = WEAPON_RAISING;
			client->ps.weaponTime = client->ps.torsoTimer;

			if (g_spawnInvulnerability.integer && !ent->client->sess.raceMode)
			{
				ent->client->ps.eFlags |= EF_INVULNERABLE;
				ent->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
			}

			// fire the targets of the spawn point
			G_UseTargets(spawnPoint, ent);

			// positively link the client, even if the command times are weird
			VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

			tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
			tent->s.clientNum = ent->s.clientNum;

			trap->LinkEntity ((sharedEntity_t *)ent);
		}
	} else {
		// move players to intermission
		MoveClientToIntermission(ent);
	}

	//set teams for NPCs to recognize
	if (level.gametype == GT_SIEGE)
	{ //Imperial (team1) team is allied with "enemy" NPCs in this mode
		if (client->sess.sessionTeam == SIEGETEAM_TEAM1)
		{
			client->playerTeam = ent->s.teamowner = NPCTEAM_ENEMY;
			client->enemyTeam = NPCTEAM_PLAYER;
		}
		else
		{
			client->playerTeam = ent->s.teamowner = NPCTEAM_PLAYER;
			client->enemyTeam = NPCTEAM_ENEMY;
		}
	}
	else
	{
		client->playerTeam = ent->s.teamowner = NPCTEAM_PLAYER;
		client->enemyTeam = NPCTEAM_ENEMY;
	}

	/*
	//scaling for the power duel opponent
	if (level.gametype == GT_POWERDUEL &&
		client->sess.duelTeam == DUELTEAM_LONE)
	{
		client->ps.iModelScale = 125;
		VectorSet(ent->modelScale, 1.25f, 1.25f, 1.25f);
	}
	*/
	//Disabled. At least for now. Not sure if I'll want to do it or not eventually.

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things

	client->pers.haste = qfalse; //reset this upon respawn i guess

	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities, NULL );

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW )
		ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

	//rww - make sure client has a valid icarus instance
	trap->ICARUS_FreeEnt( (sharedEntity_t *)ent );
	trap->ICARUS_InitEnt( (sharedEntity_t *)ent );
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap->DropClient(), which will call this and do
server system housekeeping.
============
*/
extern void G_LeaveVehicle( gentity_t* ent, qboolean ConCheck );

void G_ClearVote( gentity_t *ent ) {
	if ( level.voteTime ) {
		if ( ent->client->mGameFlags & PSG_VOTED ) {
			if ( ent->client->pers.vote == 1 ) {
				level.voteYes--;
				trap->SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
			}
			else if ( ent->client->pers.vote == 2 ) {
				level.voteNo--;
				trap->SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
			}
		}
		ent->client->mGameFlags &= ~(PSG_VOTED);
		ent->client->pers.vote = 0;
	}
}
void G_ClearTeamVote( gentity_t *ent, int team ) {
	int voteteam;

		 if ( team == TEAM_RED )	voteteam = 0;
	else if ( team == TEAM_BLUE )	voteteam = 1;
	else							return;

	if ( level.teamVoteTime[voteteam] ) {
		if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
			if ( ent->client->pers.teamvote == 1 ) {
				level.teamVoteYes[voteteam]--;
				trap->SetConfigstring( CS_TEAMVOTE_YES, va( "%i", level.teamVoteYes[voteteam] ) );
			}
			else if ( ent->client->pers.teamvote == 2 ) {
				level.teamVoteNo[voteteam]--;
				trap->SetConfigstring( CS_TEAMVOTE_NO, va( "%i", level.teamVoteNo[voteteam] ) );
			}
		}
		ent->client->mGameFlags &= ~(PSG_TEAMVOTED);
		ent->client->pers.teamvote = 0;
	}
}

void G_AddSimpleStat(gentity_t *self, gentity_t *other, int type);
void G_AddDuel(char *winner, char *loser, int start_time, int type, int winner_hp, int winner_shield);

void G_UpdatePlaytime(int null, char *username, int seconds );
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	gentity_t	*attacker;//JAPRO - Serverside - Fixkillcredit
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if ( !ent->client || ent->client->pers.connected == CON_DISCONNECTED ) {
		return;
	}

//JAPRO - Serverside - Stop those pesky reconnect whores - Start
	if (g_fixKillCredit.integer > 1 && (ent->health > 0) && !(ent->r.svFlags & SVF_BOT) && 0 <= ent->client->ps.otherKiller && ent->client->ps.otherKiller < MAX_CLIENTS && ent->client->ps.otherKillerTime > level.time && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		attacker = &g_entities[ent->client->ps.otherKiller];
		if (attacker->client) {
			trap->SendServerCommand( attacker-g_entities, va("cp \"You pwned\n%s^7!\n\"", ent->client->pers.netname) );
			trap->SendServerCommand( -1, va("print \"%s ^7was pwned by %s\n\"", ent->client->pers.netname, attacker->client->pers.netname));
			AddScore( attacker, ent->r.currentOrigin, 1 );
			G_AddSimpleStat(attacker, ent, 1);
			attacker->client->pers.stats.kills++;//JAPRO STATS
		}	
	}

	if (ent->client->ps.duelInProgress) {
		gentity_t *duelAgainst = &g_entities[ent->client->ps.duelIndex];

		if (ent->client->pers.lastUserName && ent->client->pers.lastUserName[0] && duelAgainst->client && duelAgainst->client->pers.lastUserName && duelAgainst->client->pers.lastUserName[0]) {
			//Trying to dodge the duel, no no no
			if (!(ent->client->sess.accountFlags & JAPRO_ACCOUNTFLAG_NODUEL) && !(duelAgainst->client->sess.accountFlags & JAPRO_ACCOUNTFLAG_NODUEL))
				G_AddDuel(duelAgainst->client->pers.lastUserName, ent->client->pers.lastUserName, duelAgainst->client->pers.duelStartTime, dueltypes[ent->client->ps.clientNum], duelAgainst->client->ps.stats[STAT_HEALTH], duelAgainst->client->ps.stats[STAT_ARMOR]);
		}
	}

	if (ent->client->pers.userName && ent->client->pers.userName[0]) {
		if (ent->client->sess.raceMode && !ent->client->pers.practice && ent->client->pers.stats.startTime) {
			ent->client->pers.stats.racetime += (trap->Milliseconds() - ent->client->pers.stats.startTime)*0.001f - ent->client->afkDuration*0.001f;
			ent->client->afkDuration = 0;
		}
		if (ent->client->pers.stats.racetime > 120.0f) {
			G_UpdatePlaytime(0, ent->client->pers.userName, (int)(ent->client->pers.stats.racetime+0.5f));
			ent->client->pers.stats.racetime = 0.0f;
		}
	}

//JAPRO - Serverside - Stop those pesky reconnect whores - End

	i = 0;

	while (i < NUM_FORCE_POWERS)
	{
		if (ent->client->ps.fd.forcePowersActive & (1 << i))
		{
			WP_ForcePowerStop(ent, i);
		}
		i++;
	}

	i = TRACK_CHANNEL_1;

	while (i < NUM_TRACK_CHANNELS)
	{
		if (ent->client->ps.fd.killSoundEntIndex[i-50] && ent->client->ps.fd.killSoundEntIndex[i-50] < MAX_GENTITIES && ent->client->ps.fd.killSoundEntIndex[i-50] > 0)
		{
			G_MuteSound(ent->client->ps.fd.killSoundEntIndex[i-50], CHAN_VOICE);
		}
		i++;
	}
	i = 0;

	G_LeaveVehicle( ent, qtrue );

	if (ent->client->ourSwoopNum) {
		gentity_t *ourSwoop = &g_entities[ent->client->ourSwoopNum];

		/*
		if (ent->client->ps.m_iVehicleNum) { //If we are in a vehicle, properly eject from it?
			if (ourSwoop && ourSwoop->m_pVehicle && ourSwoop->client && ourSwoop->s.NPC_class == CLASS_VEHICLE && ourSwoop->m_pVehicle->m_pVehicleInfo) {//if ourVeh is a vehicle then perform appropriate checks
				ourSwoop->m_pVehicle->m_pVehicleInfo->Eject( ourSwoop->m_pVehicle, (bgEntity_t *)ent, qtrue );
			}			
		}
		*/

		G_FreeEntity( ourSwoop );
		ent->client->ourSwoopNum = 0;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
	}

	G_LogPrintf( "ClientDisconnect: %i [%s] (%s) \"%s^7\"\n", clientNum, ent->client->sess.IP, ent->client->pers.guid, ent->client->pers.netname );

	// if we are playing in tourney mode, give a win to the other player and clear his frags for this round
	if ( level.gametype == GT_DUEL && !level.intermissiontime && !level.warmupTime ) {
		if ( level.sortedClients[1] == clientNum ) {
			level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] = 0;
			level.clients[ level.sortedClients[0] ].sess.wins++;
			ClientUserinfoChanged( level.sortedClients[0] );
		}
		else if ( level.sortedClients[0] == clientNum ) {
			level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] = 0;
			level.clients[ level.sortedClients[1] ].sess.wins++;
			ClientUserinfoChanged( level.sortedClients[1] );
		}
	}

	if ( level.gametype == GT_DUEL && ent->client->sess.sessionTeam == TEAM_FREE && level.intermissiontime ) {
		trap->SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		level.changemap = NULL;
		level.intermissiontime = 0;
	}

	if (ent->ghoul2 && trap->G2API_HaveWeGhoul2Models(ent->ghoul2))
	{
		trap->G2API_CleanGhoul2Models(&ent->ghoul2);
	}
	i = 0;
	while (i < MAX_SABERS)
	{
		if (ent->client->weaponGhoul2[i] && trap->G2API_HaveWeGhoul2Models(ent->client->weaponGhoul2[i]))
		{
			trap->G2API_CleanGhoul2Models(&ent->client->weaponGhoul2[i]);
		}
		i++;
	}

	G_ClearVote( ent );
	G_ClearTeamVote( ent, ent->client->sess.sessionTeam );

	trap->UnlinkEntity ((sharedEntity_t *)ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;
	ent->r.contents = 0;

	if (ent->client->holdingObjectiveItem > 0)
	{ //carrying a siege objective item - make sure it updates and removes itself from us now in case this is an instant death-respawn situation
		gentity_t *objectiveItem = &g_entities[ent->client->holdingObjectiveItem];

		if (objectiveItem->inuse && objectiveItem->think)
		{
            objectiveItem->think(objectiveItem);
		}
	}

	trap->SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}

	G_ClearClientLog(clientNum);
}


