/*
* Copyright (C) 2012-2013 JadeCore <http://www.pandashan.com/>
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

// TODO : Finish Heroic mode

#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "heart_of_fear.h"
#include "MapManager.h"

enum Spells
{
    SPELL_SONIC_RING_DAMAGE = 122336,
    SPELL_SONIC_RING_MODEL = 122334

};

enum Events
{
    EVENT_SONIC_RING = 13,
    EVENT_SONIC_RING_STOP = 14
};

enum Talk
{
    TALK_AGGRO = 1,
    TALK_DEATH = 2,
    TALK_EVENT_IDLE_1A = 3,    // 1st phase of trash mobs
    TALK_EVENT_IDLE_1B = 4,
    TALK_EVENT_IDLE_1C = 5,
    TALK_EVENT_IDLE_2 = 6,    // 2nd phase of trash mobs
    TALK_EVENT_IDLE_3 = 7,    // 3rd phase of trash mobs
    TALK_EVENT_IDLE_4 = 8,    // 4th phase of trash mobs
    TALK_EVENT_PHASE_1 = 9,
    TALK_EVENT_PHASE_2 = 10,
    TALK_EVENT_PHASE_3 = 11,
    TALK_EVENT_TRASH_A_COMBAT = 12,
    TALK_EVENT_TRASH_A_DIES = 13,
    TALK_EVENT_TRASH_B_COMBAT = 14,
    TALK_EVENT_TRASH_B_DIES = 15,
    TALK_EVENT_TRASH_C_COMBAT = 16,
    TALK_EVENT_TRASH_C_DIES = 17,
    TALK_SLAY_01 = 18,   // Killing a player
    TALK_SLAY_02 = 19,
    TALK_EXHALE = 20,
    TALK_INHALE = 21,
    TALK_CONVERT = 22,
    TALK_PITCH = 23,   // Echoes of power
};

enum Phase
{
    PHASE_PLATFORM_FIRST = 0,
    PHASE_PLATFORM_SECOND = 1,
    PHASE_PLATFORM_THREE = 2,
    PHASE_LAND = 3
};

const Position Platforms[4]
{
    {-2318.48f, 301.65f, 409.89f, 0.0f},
    { -2237.94f, 219.57f, 409.89f, 0.0f },
    { -2315.46f, 218.32f, 409.89f, 0.0f },
    { -2289.89f, 243.93f, 406.38f, 0.78f }
};

Position zorlokPlatforms[3] =
{
    { -2317.21f, 300.67f, 409.90f, 0.0f },  // SW Platform
    { -2234.36f, 216.88f, 409.90f, 0.0f },  // NE Platform
    { -2315.77f, 218.20f, 409.90f, 0.0f },  // SE Platform
};

Position zorlokReachPoints[3] =
{
    { -2317.21f, 300.67f, 420.0f, 0.0f },  // NE Platform
    { -2234.36f, 216.88f, 420.0f, 0.0f },  // SW Platform
    { -2315.77f, 218.20f, 420.0f, 0.0f },  // SE Platform
};

Position oratiumCenter[2] =
{
    { -2274.80f, 259.19f, 420.0f, 0.318021f },
    { -2274.80f, 259.19f, 406.5f, 0.318021f }
};

// 212943 - Final Phase Door
Position finalPhaseWalls1[3] =
{
    { -2299.195f, 282.5938f, 408.5445f, 2.383867f },
    { -2250.401f, 234.0122f, 408.5445f, 2.333440f },
    { -2299.63f, 233.3889f, 408.5445f, 0.7598741f }
};

// 212916 - Arena Walls
Position finalPhaseWalls2[3] =
{
    { -2255.168f, 308.7326f, 406.0f, 0.7853968f },
    { -2240.0f, 294.0f, 406.0f, 0.7853968f },
    { -2225.753f, 280.1424f, 406.381f, 0.7853968f },
};

float tabCenter[3] = { -2274.8f, 259.187f, 406.5f };

float rangeAttenuation1[2][2] =
{
    -2256.0f, -2208.0f,
    190.0f, 240.0f
};

float rangeAttenuation2[2][2] =
{
    // Coords to redone
    -2297.0f, -2250.0f,
    237.0f, 280.0f
};

// Zorlok - 62980
// Echo of Attenuation - 65173
// Echo of Force and Verve - 65174

class boss_zorlok : public CreatureScript
{
public:
    boss_zorlok() : CreatureScript("boss_zorlok") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_zorlokAI(creature);
    }

    struct boss_zorlokAI : public BossAI
    {
        boss_zorlokAI(Creature* creature) : BossAI(creature, DATA_VIZIER) { me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE); }
        EventMap events;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, EQUIP_ZORLOK);
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_PASSIVE);
            if (instance)
                instance->SetData(DATA_VIZIER, NOT_STARTED);
            _sonic_stop = false;
            _sonic_count = 0;
            _PullRing = 0.0f;
            _check = false;
            exhaleTarget = 0;
            _phase = PHASE_PLATFORM_FIRST;
            // Make sure we can target zorlok
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            // Set fying
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 2.5f);
            RemoveWalls();
            _CanStart = false;

            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_CLOUD);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONVERT);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NOISE_CANCELLING);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FORCE_AND_VERVE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            summons.DespawnAll();
            me->RemoveAllAreasTrigger();
        }

        bool _sonic_stop;
        uint32 _sonic_count;
        std::list <float> _RingOrientation;
        float _PullRing;
        bool _check;
        uint32 exhaleTarget;
        uint32 _phase;
        bool _CanStart;

        void MoveInLineOfSight(Unit* attacker)
        {
            // Do nothing if not in attack phase (ie. flying), or if the unit beside isn't a player
            if (!me->HasReactState(REACT_PASSIVE) || attacker->GetTypeId() != TYPEID_PLAYER)
                return;

            // If is using Song of Empress, stop it
            if (me->HasAura(SPELL_SONG_OF_THE_EMPRESS))
                me->RemoveAura(SPELL_SONG_OF_THE_EMPRESS);

            // Start attacking player
            me->AddThreat(attacker, 0.0f);
        }


        void JustDied(Unit* /*who*/)
        {
            events.Reset();
            summons.DespawnAll();
            me->RemoveAllAreasTrigger();

            me->SetCanFly(false);
            me->SetDisableGravity(true);
            RemoveWalls();

            Talk(TALK_DEATH);

            if (instance)
            {
                instance->SetBossState(DATA_VIZIER, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_CLOUD);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONVERT);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FORCE_AND_VERVE);
            }

            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id)
        {

            switch (id)
            {
            case 1:
            {
                me->FindNearestPlayer(255.0f)->SendChatMessage("ok");
                if (_phase == PHASE_PLATFORM_FIRST)
                    DoAction(ACTION_PHASE_ONE);
                break;
            }
            case 2:
            {
                if (_phase == PHASE_PLATFORM_SECOND)
                    DoAction(ACTION_PHASE_TWO);
                break;
            }
            case 3:
            {
                if (_phase == PHASE_PLATFORM_THREE)
                    DoAction(ACTION_PHASE_WHREE);
                break;
            }
            case 4:
            {
                if (_phase == PHASE_LAND)
                    DoAction(ACTION_LAST_PHASE);
                break;
            }
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_VIZIER, FAIL);
            }
            summons.DespawnAll();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_CLOUD);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONVERT);
        }

        void _switch( const Position _point, uint32 p)
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            //me->SendMovementFlagUpdate();
            //me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
            //me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            //me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->AttackStop();
            me->MonsterTextEmote("Imperial Vizier Zor'lok is flying to one of hist platforms!", 0, true);
            me->GetMotionMaster()->MovePoint(p, _point);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            // Removing song of the empress
            if (me->GetDistance(attacker) < 5.0f)
            {
                uint32 spell = me->GetCurrentSpell(CURRENT_CHANNELED_SPELL) ? me->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id : 0;
                if (me->HasAura(SPELL_SONG_OF_THE_EMPRESS) || spell == SPELL_SONG_OF_THE_EMPRESS)
                {
                    me->RemoveAurasDueToSpell(SPELL_SONG_OF_THE_EMPRESS);
                    me->InterruptNonMeleeSpells(true, SPELL_SONG_OF_THE_EMPRESS);
                    me->CastStop(SPELL_SONG_OF_THE_EMPRESS);
                    AttackStart(attacker);
                    me->SetInCombatWith(attacker);
                }
            }

            if (me->GetHealthPct() <= 80 && _phase == PHASE_PLATFORM_FIRST) { _phase = PHASE_PLATFORM_SECOND; _switch(Platforms[1], 2); }
            if (me->GetHealthPct() <= 60 && _phase == PHASE_PLATFORM_SECOND) { _phase = PHASE_PLATFORM_THREE; _switch(Platforms[2], 3); }
            if (me->GetHealthPct() <= 40 && _phase == PHASE_PLATFORM_THREE) { _phase = PHASE_LAND; _switch(Platforms[3], 4); }

        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_SONIC_CARD:
            {
                switch (_sonic_count)
                {
                case 0:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.14f / 2.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 1:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.0f * 3.14f / 4.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 2:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.14f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 3:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 5.0f * 3.14f / 4.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 4:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.0f * 3.14f / 2.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 5:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 7.0f * 3.14f / 4.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 6:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count++;
                    break;
                case 7:
                    me->SummonCreature(62700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.14f / 4, TEMPSUMMON_CORPSE_DESPAWN);
                    _sonic_count = 0;
                    break;
                }
                break;
            }
            case ACTION_PHASE_ONE:
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                me->CastSpell(me, SPELL_SONG_OF_THE_EMPRESS);
                events.ScheduleEvent(EVENT_INHALE, 6000);
                events.ScheduleEvent(EVENT_FORCE_AND_VERVE, urand(10000, 20000));
                break;
            }
            case ACTION_PHASE_TWO:
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                me->CastSpell(me, SPELL_SONG_OF_THE_EMPRESS);
                events.ScheduleEvent(EVENT_SONIC_RING, urand(10000, 20000));
                events.ScheduleEvent(EVENT_INHALE, 6000);
                break;
            }
            case ACTION_PHASE_WHREE:
            {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->CastSpell(me, SPELL_SONG_OF_THE_EMPRESS);
                    events.ScheduleEvent(EVENT_INHALE, 6000);
                    events.ScheduleEvent(EVENT_CONVERT, urand(10000, 20000));
                    break;
            }
            case ACTION_LAST_PHASE:
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                events.ScheduleEvent(EVENT_INHALE, 6000);
                events.ScheduleEvent(ChooseAction(), urand(25000, 35000));
                me->CastSpell(me, SPELL_INHALE_PHEROMONES, false);
                events.ScheduleEvent(EVENT_PULL_RAID, 2000);
                break;
            }
            case ACTION_WIPE:
            {
                _Reset();
                events.Reset();
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, EQUIP_ZORLOK);
                me->setRegeneratingHealth(true);
                me->SetReactState(REACT_PASSIVE);
                if (instance)
                    instance->SetData(DATA_VIZIER, NOT_STARTED);
                _sonic_stop = false;
                _sonic_count = 0;
                _PullRing = 0.0f;
                _check = false;
                exhaleTarget = 0;
                _phase = PHASE_PLATFORM_FIRST;
                // Make sure we can target zorlok
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                // Set fying
                me->SetCanFly(true);
                me->SetSpeed(MOVE_FLIGHT, 2.5f);
                RemoveWalls();
                summons.DespawnAll();
                me->RemoveAllAreasTrigger();

                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_CLOUD);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONVERT);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NOISE_CANCELLING);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FORCE_AND_VERVE);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                break;
            }
            }
        }


        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            if (instance)
            {
                instance->SetData(DATA_VIZIER, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_BERSERK, 600000);
            Talk(TALK_AGGRO);
            me->CastSpell(tabCenter[0], tabCenter[1], tabCenter[2], SPELL_PHEROMONES_CLOUD, false);
            _switch(Platforms[0], 1);
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_SLAY_01 + urand(0, 1));
        }

        uint32 ChooseAction()
        {
            uint32 choice = urand(1, 3);
            switch (choice)
            {
            case 1:
                return EVENT_SONIC_RING;
            case 2:
                return EVENT_CONVERT;
            case 3:
                return EVENT_FORCE_AND_VERVE;
            default:
                break;
            }
            return 0;
        }

        void RemoveWalls()
        {
            std::list<GameObject*> arenaList;
            std::list<GameObject*> wallsList;

            GetGameObjectListWithEntryInGrid(arenaList, me, GOB_ARENA_WALLS, 200.0f);
            GetGameObjectListWithEntryInGrid(wallsList, me, GOB_FINAL_PHASE_WALLS, 200.0f);

            for (auto wall : arenaList)
                me->RemoveGameObject(wall, true);

            for (auto wall : wallsList)
                me->RemoveGameObject(wall, true);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!_CanStart)
            {
                if (instance->GetBossState(DATA_VIZIER) == NOT_STARTED || instance->GetBossState(DATA_VIZIER) == TO_BE_DECIDED)
                {
                    Creature* ShieldMaster = GetClosestCreatureWithEntry(me, NPC_SRATHIK_SHIELD_MASTER, 200.0f, true);
                    Creature* Zealok = GetClosestCreatureWithEntry(me, NPC_ZARTHIK_ZEALOT, 200.0f, true);
                    Creature* Fanatic = GetClosestCreatureWithEntry(me, NPC_SETTHIK_FANATIC, 200.0f, true);
                    Creature* BoneSmasher = GetClosestCreatureWithEntry(me, NPC_ENSLAVED_BONESMASHER, 200.0f, true);
                    Creature* Supplicant = GetClosestCreatureWithEntry(me, NPC_ZARTHIK_SUPPLICANT, 200.0f, true);
                    Creature* Supplicant2 = GetClosestCreatureWithEntry(me, NPC_ZARTHIK_SUPPLICANT_2, 200.0f, true);
                    Creature* Supplicant3 = GetClosestCreatureWithEntry(me, NPC_ZARTHIK_SUPPLICANT_3, 200.0f, true);

                    if (!ShieldMaster&& !Zealok && !Fanatic && !BoneSmasher && !Supplicant && !Supplicant2 && !Supplicant3)
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        _CanStart = true;
                        return;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (instance)
            {
                if (instance->IsWipe())
                {
                    DoAction(ACTION_WIPE);
                    // We stop here to avoid Zor'lok to cast Song of the empress
                    return;
                }
            }

            // Remove/Set auras on players
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    if (Player* playr = i->GetSource())
                    {
                        // Remove convert aura for players who have less than 50% remaining health
                        if (playr->HasAura(SPELL_CONVERT) && playr->HealthBelowPct(51))
                            playr->RemoveAurasDueToSpell(SPELL_CONVERT);
                        // Pheromones of zeal - on phase 1 only
                        if (_phase != PHASE_LAND)
                        {
                            // Remove pheromones of zeal aura from players who aren't on the bottom floor
                            if (playr->HasAura(SPELL_PHEROMONES_OF_ZEAL) && playr->GetPositionZ() >= 408.5f)
                                playr->RemoveAura(SPELL_PHEROMONES_OF_ZEAL);
                            // Set pheromones of zeal aura on players who are on the bottom floor
                            else if (!playr->HasAura(SPELL_PHEROMONES_OF_ZEAL) && playr->GetPositionZ() < 408.5f)
                                playr->AddAura(SPELL_PHEROMONES_OF_ZEAL, playr);
                        }
                    }
                }
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SONIC_RING:
                {
                    if (!_check)
                    {
                        _check = true;
                        events.ScheduleEvent(EVENT_SONIC_RING_STOP, 14000);
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->CastSpell(me, SPELL_ATTENUATION, true);
                        events.ScheduleEvent(EVENT_SONIC_RING, 2000);
                    }
                    else
                    {
                        DoAction(ACTION_SONIC_CARD);
                        events.ScheduleEvent(EVENT_SONIC_RING, 300);
                    }

                    break;
                }
                case EVENT_SONIC_RING_STOP:
                {
                    _sonic_stop = true;
                    _check = false;
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Player* player = me->FindNearestPlayer(VISIBLE_RANGE))
                        me->CombatStart(player);
                    events.CancelEvent(EVENT_SONIC_RING);
                    uint32 action = (_phase == PHASE_LAND ? ChooseAction() : EVENT_SONIC_RING);
                    events.ScheduleEvent(action, 40000);
                    break;
                }
                // All-time events
                case EVENT_INHALE:
                {
                    // Can't inhale if already casting
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        events.RescheduleEvent(EVENT_INHALE, 5000);
                    else
                    {
                        // Inhale (Exhale is triggered when Zor'lok has 3-4 stacks of inhale)
                        Aura* inhale = me->GetAura(SPELL_INHALE);
                        if (!inhale || inhale->GetStackAmount() < 3 || !urand((inhale->GetStackAmount() < 4 ? 0 : 1), 1))
                        {
                            Talk(TALK_INHALE);
                            me->MonsterTextEmote("Imperial Vizier Zor'lok |cFFFF0000|Hspell:122852|h[Inhale]|h|r a big air breath!", 0, true);
                            me->CastSpell(me, SPELL_INHALE, false);
                        }
                        // Exhale
                        else
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                            {
                                exhaleTarget = target->GetGUIDLow();
                                Talk(TALK_EXHALE);
                                DoCast(target, SPELL_EXHALE, true);
                            }
                        }
                        events.ScheduleEvent(EVENT_INHALE, urand(25000, 50000));
                    }
                    break;
                }
                case EVENT_CONVERT:
                {
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        events.RescheduleEvent(EVENT_CONVERT, 10000);
                        return;
                    }

                    me->MonsterTextEmote("Imperial Vizier Zor'lok is using is voice to |cFFFF0000|Hspell:122740|h[Convert]|h|r members of the raid and to call them by his side !", 0, true);

                    // Creating target list
                    Talk(TALK_CONVERT);
                    DoCast(SPELL_CONVERT);

                    uint32 action = (_phase == PHASE_LAND ? ChooseAction() : EVENT_CONVERT);
                    events.ScheduleEvent(action, 40000);

                    break;
                }
                // Force and Verve platform
                case EVENT_FORCE_AND_VERVE:
                {
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        events.RescheduleEvent(EVENT_FORCE_AND_VERVE, 10000);
                        return;
                    }
                    // Creating Noise Cancelling zones
                    for (int i = 0; i < 3; ++i)
                    {
                        float x = me->GetPositionX() + frand(-10.0f, 10.0f);
                        float y = me->GetPositionY() + frand(-10.0f, 10.0f);

                        me->CastSpell(x, y, me->GetPositionZ(), SPELL_MISSILE_NOISE_CANC, false);
                    }
                    me->AddUnitState(UNIT_STATE_CASTING);
                    events.ScheduleEvent(EVENT_CAST_FANDV, 2000);
                    break;
                }
                case EVENT_CAST_FANDV:
                {
                    me->MonsterTextEmote("Imperial Vizier Zor'lok shouts with |cFFFF0000|Hspell:122713|h[Force et brio]|h|r!", 0, true);
                    me->CastSpell(me, SPELL_FORCE_AND_VERVE, true);
                    uint32 action = (_phase == PHASE_LAND ? ChooseAction() : EVENT_FORCE_AND_VERVE);
                    events.ScheduleEvent(action, 40000);
                    break;
                }
                case EVENT_BERSERK:
                {
                    me->CastSpell(me, SPELL_BERSERK, false);
                    break;
                }
                case EVENT_PULL_RAID:
                {
                    // Pulling far players
                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 300.0f);
                    for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                        // The point is that if they're on a platform, they'll be blocked by walls, so they have to be pulled
                        if ((*itr)->GetPositionZ() > 408.5f)
                            (*itr)->CastSpell(me, SPELL_SPRING_RABBIT_JUMP, false);

                    // Creating Walls
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        // Arena walls
                        me->SummonGameObject(GOB_ARENA_WALLS, finalPhaseWalls2[i].GetPositionX(), finalPhaseWalls2[i].GetPositionY(), finalPhaseWalls2[i].GetPositionZ(), finalPhaseWalls2[i].GetOrientation(), 0, 0, 0, 0, 7200);

                        // Final phase Doors
                        me->SummonGameObject(GOB_FINAL_PHASE_WALLS, finalPhaseWalls1[i].GetPositionX(), finalPhaseWalls1[i].GetPositionY(), finalPhaseWalls1[i].GetPositionZ(), finalPhaseWalls1[i].GetOrientation(), 0, 0, 0, 0, 7200);
                    }

                    summons.DespawnAll();
                    me->RemoveAllAreasTrigger();

                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_CLOUD);

                    break;
                }
                }

            }

            DoMeleeAttackIfReady();
        }
    };
};

// Sonic Ring - 62689, 62694, 62696, 62716, 62717, 62718, 62719, 62726, 62727, 62743, 62744, 62746
class mob_sonic_ring : public CreatureScript
{
public:
    mob_sonic_ring() : CreatureScript("mob_sonic_ring") { }

    struct mob_sonic_ringAI : public ScriptedAI
    {
        mob_sonic_ringAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;
        Position pos;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);;
            me->CastSpell(me, SPELL_SONIC_RING_VISUAL);
            me->SetWalk(true);
            me->SetSpeed(MOVE_WALK, 1.5f);

            float x, y;
            GetPositionWithDistInOrientation(me, 50.0f, me->GetOrientation(), x, y);

            if (me->IsWithinLOS(x, y, me->GetPositionZ()))
            {
                pos.Relocate(x, y, me->GetPositionZ());
                me->GetMotionMaster()->MovePoint(1, pos);
            }
            else  me->GetMotionMaster()->MovePoint(1, x, y, me->GetPositionZ());

        }

        uint32 checkNearPlayerTimer;

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
                me->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* /*doneBy*/, uint32 &damage)
        {
            damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (Unit* nearPlayer = me->SelectNearestPlayer(1.0f))
                me->CastSpell(nearPlayer, SPELL_SONIC_RING_DAMAGE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_sonic_ringAI(creature);
    }
};

// Inhale - 122852
class spell_inhale : public SpellScriptLoader
{
public:
    spell_inhale() : SpellScriptLoader("spell_inhale") { }

    class spell_inhale_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_inhale_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, false);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_inhale_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_inhale_SpellScript();
    }
};

// Attenuation - 122440
class spell_attenuation : public SpellScriptLoader
{
public:
    spell_attenuation() : SpellScriptLoader("spell_attenuation") { }

    class spell_attenuation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_attenuation_SpellScript);

        void Apply()
        {
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_attenuation_SpellScript::Apply);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_attenuation_SpellScript();
    }
};

// Force and Verve (Aura) - 122718
class spell_force_verve : public SpellScriptLoader
{
public:
    spell_force_verve() : SpellScriptLoader("spell_force_verve") { }

    class spell_force_verve_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_force_verve_SpellScript);

        void ApplyEffect()
        {
            if (Player* target = GetHitPlayer())
                if (target->HasAura(SPELL_NOISE_CANCELLING))
                    SetHitDamage(GetHitDamage() * 0.25);
        }

        void SetReact()
        {
            if (Creature* caster = GetCaster()->ToCreature())
            {
                caster->SetReactState(REACT_PASSIVE);
                caster->AttackStop();
            }
        }

        void ReturnReact()
        {
            if (Creature* caster = GetCaster()->ToCreature())
            {
                caster->SetReactState(REACT_AGGRESSIVE);
                if (Player* player = caster->FindNearestPlayer(VISIBLE_RANGE))
                   caster->CombatStart(player);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_force_verve_SpellScript::ApplyEffect);
            BeforeCast += SpellCastFn(spell_force_verve_SpellScript::SetReact);
            AfterCast += SpellCastFn(spell_force_verve_SpellScript::ReturnReact);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_force_verve_SpellScript();
    }
};

// Sonic Ring (Aura) - 122336
class spell_sonic_ring : public SpellScriptLoader
{
public:
    spell_sonic_ring() : SpellScriptLoader("spell_sonic_ring") { }

    class spell_sonic_ring_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sonic_ring_AuraScript);

        void ApplyAura(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->AddAura(SPELL_SONIC_RING_AURA, caster);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_sonic_ring_AuraScript::ApplyAura, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_sonic_ring_AuraScript::ApplyAura, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    class spell_sonic_ring_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sonic_ring_SpellScript);

        void Effect()
        {
            if (Player* target = GetHitPlayer())
                if (target->HasAura(SPELL_NOISE_CANCELLING))
                    SetHitDamage(GetHitDamage() * 0.4);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_sonic_ring_SpellScript::Effect);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sonic_ring_AuraScript();
    }

    SpellScript* GetSpellScript() const
    {
        return new spell_sonic_ring_SpellScript();
    }
};

// Pheromones of Zeal - 124018
class spell_pheromones_of_zeal : public SpellScriptLoader
{
public:
    spell_pheromones_of_zeal() : SpellScriptLoader("spell_pheromones_of_zeal") { }

    class spell_pheromones_of_zeal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pheromones_of_zeal_AuraScript);
        InstanceScript* pInstance;

        void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->AddAura(SPELL_INHALE_PHEROMONES, caster);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_pheromones_of_zeal_AuraScript::Apply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    class spell_pheromones_of_zeal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pheromones_of_zeal_SpellScript);

        void Effect()
        {
            if (Creature* caster = GetCaster()->ToCreature())
                caster->AI()->DoAction(ACTION_INHALE_PHEROMONES);
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_pheromones_of_zeal_SpellScript::Effect);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pheromones_of_zeal_AuraScript();
    }

    SpellScript* GetSpellScript() const
    {
        return new spell_pheromones_of_zeal_SpellScript();
    }

};

class ExhaleTargetFilter : public std::unary_function<Unit*, bool>
{
public:
    explicit ExhaleTargetFilter(Unit* caster) : _caster(caster) { }

    bool operator()(WorldObject* object) const
    {
        uint32 exhaleLowId = CAST_AI(boss_zorlok::boss_zorlokAI, _caster->GetAI())->GetData(TYPE_EXHALE_TARGET);
        Player* exhaleTarget = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(exhaleLowId, 0, HIGHGUID_PLAYER));

        if (!exhaleTarget)
            return false;

        // Remove all the other players (stun only Exhale target).
        return (object == exhaleTarget) ? false : true;
    }

private:
    Unit* _caster;
};

// Exhale: 122761
class spell_zorlok_exhale : public SpellScriptLoader
{
public:
    spell_zorlok_exhale() : SpellScriptLoader("spell_zorlok_exhale") { }

    class spell_zorlok_exhale_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_zorlok_exhale_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();

            Unit* caster = GetCaster();

            if (!caster)
                return;

            Player* target = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(TYPEID_PLAYER, 0, caster->GetAI()->GetData(TYPE_EXHALE_TARGET)));

            // No target? Then we pick a random one
            if (!target || !target->IsAlive())
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, caster, 60.0f);

                if (playerList.empty())
                    return;

                bool searching = true;
                std::list<Player*>::iterator itr = playerList.begin();

                while (searching)
                {
                    if (urand(0, 1))
                    {
                        target = *itr;
                        searching = false;
                    }
                    ++itr;

                    if (itr == playerList.end())
                        itr = playerList.begin();
                }
            }

            if (target)
                targets.push_back(target);
        }

        void RemoveStack()
        {
            if (Unit* caster = GetCaster())
                caster->RemoveAurasDueToSpell(SPELL_INHALE);
        }

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit* caster = GetCaster())
                if (Player* target = GetHitPlayer())
                    caster->CastSpell(target, SPELL_EXHALE_DMG, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_zorlok_exhale_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_zorlok_exhale_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            AfterCast += SpellCastFn(spell_zorlok_exhale_SpellScript::RemoveStack);
            OnEffectHitTarget += SpellEffectFn(spell_zorlok_exhale_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_zorlok_exhale_SpellScript();
    }
};

// Exhale (damage): 122760
class spell_zorlok_exhale_damage : public SpellScriptLoader
{
public:
    spell_zorlok_exhale_damage() : SpellScriptLoader("spell_zorlok_exhale_damage") { }

    class spell_zorlok_exhale_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_zorlok_exhale_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Unit* caster = GetCaster();

            if (targets.empty() || !caster)
                return;

            Unit* currentTarget = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(TYPEID_PLAYER, 0, caster->GetAI()->GetData(TYPE_EXHALE_TARGET)));
            if (!currentTarget)
                return;

            // Remove players not between Zorlok and his target.
            std::list<WorldObject*>::iterator itr, next;
            for (itr = targets.begin(); itr != targets.end(); itr = next)
            {
                next = itr;
                ++next;

                // Keeping current target
                if ((*itr) == currentTarget)
                    continue;

                if ((*itr)->GetTypeId() != TYPEID_PLAYER || !(*itr)->IsInBetween(caster, currentTarget))
                    targets.remove(*itr);
            }

            // Two or more targets, means there's someone between Zor'lok and his target.
            if (targets.size() > 1)
            {
                // Select first target between Zor'lok and the Exhale target.
                WorldObject* nearestTarget = 0;
                float distance = 1000.0f;

                for (itr = targets.begin(); itr != targets.end(); ++itr)
                {
                    if (caster->GetDistance2d(*itr) < distance)
                    {
                        nearestTarget = *itr;
                        distance = caster->GetDistance2d(*itr);
                    }
                }

                if (nearestTarget != currentTarget)
                {
                    // Set Zor'lok's current Exhale target to that nearest player.
                    uint32 targetLowGuid = nearestTarget->GetGUIDLow();
                    caster->GetAI()->SetData(TYPE_EXHALE_TARGET, targetLowGuid);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_zorlok_exhale_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_129);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_zorlok_exhale_damage_SpellScript();
    }
};

// 122740 - Convert
class spell_convert : public SpellScriptLoader
{
public:
    spell_convert() : SpellScriptLoader("spell_convert") { }

    class spell_convertSpellScript : public SpellScript
    {
        PrepareSpellScript(spell_convertSpellScript);

        void SelectTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();

            if (Unit* caster = GetCaster())
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, caster, 60.0f);

                // Removing dead or already converted players
                std::list<Player*>::iterator itr, next;
                for (itr = playerList.begin(); itr != playerList.end(); itr = next)
                {
                    next = itr;
                    ++next;

                    if (!(*itr)->IsAlive() || (*itr)->HasAura(SPELL_CONVERT))
                        playerList.remove(*itr);
                }

                uint8 maxVictims = caster->GetInstanceScript()->instance->Is25ManRaid() ? 5 : 2;

                for (itr = playerList.begin(); itr != playerList.end(); itr = next)
                {
                    next = itr;
                    ++next;

                    if (targets.size() == maxVictims)
                        return;

                    targets.push_back(*itr);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_convertSpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_convertSpellScript();
    }
};

void AddSC_boss_zorlok()
{
    new boss_zorlok();                  // 62980 - Imperial Vizier Zor'lok
    new mob_sonic_ring();               // 62687, 62746, 62744, 62743, 62698, 62699, 62700, 62702, 62703, 62704, 62714, 62715, 62727, 62726, 62719, 62718, 62720, 62721, 62722, 62723, 62724, 62725, 62717, 62716, 62728, 62729, 62696, 62694, 62689, 63340, 63341, 67163, 67164 - Sonic Ring
    new spell_inhale();                 // 122852 - Inhale
    new spell_attenuation();            // 122440 - Attenuation
    new spell_force_verve();            // 122718 - Force and verve
    //new spell_sonic_ring();             // 122336 - Sonic Ring
    new spell_pheromones_of_zeal();     // 124018 - Pheromones of Zeal
    new spell_zorlok_exhale();          // 122761 - Exhale
    new spell_zorlok_exhale_damage();   // 122760 - Exhale (damage aura)
    new spell_convert();                // 122740 - Convert
}