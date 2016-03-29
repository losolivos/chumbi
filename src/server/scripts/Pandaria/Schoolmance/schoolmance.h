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

#ifndef SCHOOLMANCE_H_
#define SCHOOLMANCE_H_
#define DataHeader "SCHM"
#define scarlet_monastery_scriptname "instance_schoolmance"
uint32 const encounternumber = 5;

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

enum eData
{
    BOSS_INSTRUCTOR_CHILLHEART = 0,
    BOSS_JANDICE_BAROV = 1,
    BOSS_RATTLEGORE = 2,
    BOSS_LILIAN_VOSS = 3,
    BOSS_GRANDMASTER_GANDLING = 4,
    BOSS_PHYLACTERY = 5,
    BOSS_TRUE_ILLUSION = 6
};

enum eObjects
{
    GO_KORLOFF_EXIT = 210564,
    GO_WHITEMANE_ENTRANCE = 210563
};

enum eCreature
{
    NPC_INSTRUCTOR_CHILLHEART = 58633,
    NPC_JANDICE_BAROV = 59184,
    NPC_RATTLEGORE = 59153,
    NPC_LILIAN_VOSS = 59200,
    NPC_GRANDMASTER_GANDLING = 59080,
    NPC_PHYLACTERY = 58664,
    NPC_ICE_WALL_MAIN_TRIGGER = 3570615,
    NPC_ICE_WALL_TRIGGER = 3570616,
    NPC_FRIGID_GRASP_TRIGGER = 3570617,
    NPC_JANDICE_ILLUSION = 59185,
    NPC_JANDICE_TRUE_ILLUSION = 59192,
    NPC_RAPIDITY_TRIGGER = 59194
};

#define MAX_TYPES 14
#endif // SCHOOLMANCE_H_