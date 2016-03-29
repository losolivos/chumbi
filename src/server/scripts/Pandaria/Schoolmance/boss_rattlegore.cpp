#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "schoolmance.h"

class boss_rattlegore : public CreatureScript
{
public:
    boss_rattlegore() : CreatureScript("boss_rattlegore") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_rattlegoreAI(creature);
    }

    struct boss_rattlegoreAI : public BossAI
    {
        boss_rattlegoreAI(Creature* creature) : BossAI(creature, BOSS_RATTLEGORE) {}

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
                instance->SetData(BOSS_RATTLEGORE, NOT_STARTED);
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_RATTLEGORE, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_RATTLEGORE, FAIL);
            }
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who) override
        {

            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_RATTLEGORE, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
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
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_rattlegore()
{
    new boss_rattlegore();
}