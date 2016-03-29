#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "schoolmance.h"

enum eSpells
{
    SPELL_ICE_WALL_TRIGGER = 122443, // visual wall
    SPELL_ICE_WALL_KILL = 111231, // instant die
    SPELL_FRIGID_GRASP = 111239, // on trigger at player
    SPELL_ICE_WRATH = 111610, // on player
    SPELL_WRACK_SOUL = 111631, // only heoic mode
    SPELL_GRAVE_TOUCH = 111224, // on tank

    SPELL_SHADOW_BOLT = 113809,
    SPELL_ARCANE_BOMB = 113859,
    SPELL_BURN = 120027,

    SPELL_FIRE_TOME = 111574,
};

enum eEvents
{
    EVENT_FRIGID_GRASP = 1,
    EVENT_ICE_WRATH = 2,
    EVENT_WRACK_SOUL = 3,
    EVENT_GRAVE_TOUCH = 4
};

enum eTalk
{

};

enum ePhase
{
    PHASE_FIRST_LESSON = 1,
    PHASE_SECOND_LESSON = 2
};

const Position trigger_point = { 199.28f, -5.73f, 119.22f, 1.58f };
class boss_instructor_chillheart : public CreatureScript
{
public:
    boss_instructor_chillheart() : CreatureScript("boss_instructor_chillheart") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_instructor_chillheartAI(creature);
    }

    struct boss_instructor_chillheartAI : public BossAI
    {
        boss_instructor_chillheartAI(Creature* creature) : BossAI(creature, BOSS_INSTRUCTOR_CHILLHEART) {}
        EventMap events;
        uint32 _phase;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
                instance->SetData(BOSS_INSTRUCTOR_CHILLHEART, NOT_STARTED);
            _phase = PHASE_FIRST_LESSON;
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_INSTRUCTOR_CHILLHEART, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
            _phase = PHASE_SECOND_LESSON;

            if (Unit* _Phylactery = Unit::GetCreature(*me, instance->GetData64(BOSS_PHYLACTERY)))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_INSTRUCTOR_CHILLHEART, FAIL);
            }
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_INSTRUCTOR_CHILLHEART, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_FRIGID_GRASP, 10500);
            events.ScheduleEvent(EVENT_GRAVE_TOUCH, 5000);
            events.ScheduleEvent(EVENT_ICE_WRATH, urand(8500, 9000));
            events.ScheduleEvent(EVENT_WRACK_SOUL, 2500);
            me->SummonCreature(NPC_ICE_WALL_MAIN_TRIGGER,trigger_point, TEMPSUMMON_MANUAL_DESPAWN);
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FRIGID_GRASP:
                    if (Unit* _player = SelectTarget(SELECT_TARGET_RANDOM))
                        me->SummonCreature(NPC_FRIGID_GRASP_TRIGGER, _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);
                    events.ScheduleEvent(EVENT_FRIGID_GRASP, 10500);
                    break;
                case EVENT_ICE_WRATH:
                    if (Unit* _player = SelectTarget(SELECT_TARGET_FARTHEST))
                        me->CastSpell(_player,SPELL_ICE_WRATH, false);
                    events.ScheduleEvent(EVENT_ICE_WRATH, urand(8500, 9000));
                    break;
                case EVENT_WRACK_SOUL:
                    if (Player* _player = me->FindNearestPlayer(20.0f))
                        me->CastSpell(_player, SPELL_WRACK_SOUL, false);
                    break;
                case EVENT_GRAVE_TOUCH:
                    me->CastSpell(SelectTarget(SELECT_TARGET_TOPAGGRO), SPELL_GRAVE_TOUCH, false);
                    events.ScheduleEvent(EVENT_GRAVE_TOUCH, 5000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class icewall_main_trigger : public CreatureScript
{
public:
    icewall_main_trigger() : CreatureScript("icewall_main_trigger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new icewall_main_triggerAI(creature);
    }

    struct icewall_main_triggerAI : public ScriptedAI
    {
        icewall_main_triggerAI(Creature* creature) :ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            _move = 7000;
            _trigger = -30.0f;
            for (uint32 i = 0; i < 5; i++)
            {
                if (_trigger != 0.0f)
                    me->SummonCreature(NPC_ICE_WALL_TRIGGER, me->GetPositionX() + _trigger, me->GetPositionY(), me->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);
                _trigger += 10.0f;
            }
            me->CastSpell(me, SPELL_ICE_WALL_TRIGGER, false);
        }

        uint32 _move;
        float _trigger;


        void UpdateAI(const uint32 diff)
        {
            if (pInstance->GetBossState(BOSS_INSTRUCTOR_CHILLHEART) == DONE || pInstance->GetBossState(BOSS_INSTRUCTOR_CHILLHEART) == FAIL)
                me->DespawnOrUnsummon();

            if (Unit* player = me->FindNearestPlayer(2.0f))
                player->CastSpell(player, SPELL_ICE_WALL_KILL, false);

            events.Update(diff);

            if (_move <= diff) { me->SummonCreature(NPC_ICE_WALL_MAIN_TRIGGER, me->GetPositionX(), me->GetPositionY() + frand(2.0f, 3.0f), me->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN); me->DespawnOrUnsummon(); }
            else _move -= diff;
        }
        
        
    };
};

class icewall_trigger : public CreatureScript
{
public:
    icewall_trigger() : CreatureScript("icewall_trigger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new icewall_triggerAI(creature);
    }

    struct icewall_triggerAI : public ScriptedAI
    {
        icewall_triggerAI(Creature* creature) :ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            _desp = 7000;
        }

        uint32 _desp;

        void UpdateAI(const uint32 diff)
        {
            if (pInstance->GetBossState(BOSS_INSTRUCTOR_CHILLHEART) == DONE || pInstance->GetBossState(BOSS_INSTRUCTOR_CHILLHEART) == FAIL)
                me->DespawnOrUnsummon();

            if (Unit* player = me->FindNearestPlayer(2.0f))
                player->CastSpell(player, SPELL_ICE_WALL_KILL, false);

            events.Update(diff);

            if (_desp <= diff)
                me->DespawnOrUnsummon();
            else _desp -= diff;
        }
    };
};

class frigid_grasp_trigger : public CreatureScript
{
public:
    frigid_grasp_trigger() : CreatureScript("frigid_grasp_trigger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new frigid_grasp_triggerAI(creature);
    }

    struct frigid_grasp_triggerAI : public ScriptedAI
    {
        frigid_grasp_triggerAI(Creature* creature) :ScriptedAI(creature) {}
        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell(me, SPELL_FRIGID_GRASP, true);
            _desp = 7000;
        }
        uint32 _desp;

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (_desp <= diff)
                me->DespawnOrUnsummon();
            else _desp -= diff;

        }

    };
};

class boss_phylactery : public CreatureScript
{
public:
    boss_phylactery() : CreatureScript("boss_phylactery") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_phylacteryAI(creature);
    }

    struct boss_phylacteryAI : public BossAI
    {
        boss_phylacteryAI(Creature* creature) : BossAI(creature, BOSS_PHYLACTERY) {}
        EventMap events;
        uint32 _phase;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            if (instance)
                instance->SetData(BOSS_PHYLACTERY, NOT_STARTED);
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_PHYLACTERY, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_PHYLACTERY, FAIL);
            }
            summons.DespawnAll();
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    };
};

class Instructor_Chillhearts_Phylactery : public CreatureScript
{
public:
    Instructor_Chillhearts_Phylactery() : CreatureScript("Instructor_Chillhearts_Phylactery") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new Instructor_Chillhearts_PhylacteryAI(creature);
    }

    struct Instructor_Chillhearts_PhylacteryAI : public ScriptedAI
    {
        Instructor_Chillhearts_PhylacteryAI(Creature* creature) :ScriptedAI(creature) {}
        EventMap events;


        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

        }
    };
};

void AddSC_boss_instructor_chillheart()
{
    new boss_instructor_chillheart();
    new icewall_main_trigger();
    new icewall_trigger();
    new frigid_grasp_trigger();
    new Instructor_Chillhearts_Phylactery();
}