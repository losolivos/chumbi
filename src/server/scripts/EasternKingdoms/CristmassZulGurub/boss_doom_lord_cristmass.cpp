#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CristmassZulGurub.h"

enum spells
{
    SPELL_FELBLADE = 103966,
    SPELL_FEL_BREATH = 138813,
    SPELL_CATACLYSM = 138564, // each over 3 [99/66/33]
    SPELL_OMNIPOTENCE_AURA = 138563, // at 50 per hp
    SPELL_CHARGE = 138796
};

enum events
{
    EVENT_FELBLADE = 1,
    EVENT_FEL_BREATH = 2,
    EVENT_CATACLYSM = 3,
    EVENT_FEL_CHARGE = 4
};
class boss_doom_lord_cristmass : public CreatureScript
{
public:
    boss_doom_lord_cristmass() : CreatureScript("boss_doom_lord_cristmass") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_doom_lord_cristmassAI(creature);
    }

    struct boss_doom_lord_cristmassAI : public BossAI
    {
        boss_doom_lord_cristmassAI(Creature* creature) : BossAI(creature, BOSS_DOOM_LORD) {}
        EventMap events;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
                instance->SetData(BOSS_DOOM_LORD, NOT_STARTED);
            _berserk = false;
            _phase = 96;
            _cata = false;
        }

        bool _berserk;
        bool _cata;
        uint32 _phase;

        void JustDied(Unit* killer) override
        {
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_DOOM_LORD, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_DOOM_LORD, FAIL);
            }
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_DOOM_LORD, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            _MainPhase();
        }

        void _MainPhase()
        {
            events.ScheduleEvent(EVENT_FELBLADE, 4500);
            events.ScheduleEvent(EVENT_FEL_CHARGE, 8000);
            events.ScheduleEvent(EVENT_FEL_BREATH, urand(12000,14000));
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (me->GetHealthPct() <= 50 && !_berserk) { _berserk = true; me->CastSpell(me, SPELL_OMNIPOTENCE_AURA); }

            if (me->GetHealthPct() <= _phase&&!_cata)
            {
                _phase -= 33;
                events.Reset();
                me->CastSpell(me, SPELL_CATACLYSM, false);
                _MainPhase();

                if (_phase == 33)
                    _cata = true;

            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FELBLADE:
                    me->CastSpell(me, SPELL_FELBLADE, false);
                    events.ScheduleEvent(EVENT_FELBLADE, urand(10000,11000));
                    break;
                case EVENT_FEL_BREATH:
                    if (Unit* player = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(player, SPELL_FEL_BREATH, false);
                    events.ScheduleEvent(EVENT_FEL_BREATH, urand(12000, 14000));
                    break;
                case EVENT_FEL_CHARGE:
                    if (Unit* player = SelectTarget(SELECT_TARGET_FARTHEST))
                        me->CastSpell(player, SPELL_CHARGE, false);
                    events.ScheduleEvent(EVENT_CHARGE, 8000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_doom_lord_cristmass()
{
    new boss_doom_lord_cristmass();
}