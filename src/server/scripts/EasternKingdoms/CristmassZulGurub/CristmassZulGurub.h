/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHRISTMASS_ZULGURUB_H_
#define CHRISTMASS_ZULGURUB_H_
#define DataHeader "CZG"
#define cristmass_zulgurub_scriptname "instance_cristmass_zulgurub"
uint32 const encounternumber = 5;

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

enum eData
{
    BOSS_DOOM_LORD = 1,
    BOSS_PIT_LORD = 2,
    BOSS_JUBEKA = 3,
    BOSS_KANRETHAD = 4
};

enum eCreature
{
    NPC_DOOM_LORD = 70073,
    NPC_PIT_LORD = 70075,
    NPC_JUBEKA_SHADOWBREAKER = 70166,
    NPC_KANRETHAD_EBONLOCKE = 69964
};

enum eObjects
{

};

#define MAX_TYPES 14
#endif // CHRISTMASS_ZULGURUB_H_