#include "GameObjectAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "Unit.h"
#include "Player.h"
#include "Creature.h"
#include "InstanceScript.h"
#include "Map.h"
#include "VehicleDefines.h"
#include "SpellInfo.h"

#include "throne_of_thunder.h"

enum spells
{
    SPELL_BURNING_CINDERS_VISUAL_DROP = 136330,
    SPELL_BURNING_CINDERS_VISUAL_DROP_SPIKE = 134758,
    SPELL_BURNING_CINDERS_VISUAL_TRACK = 114464,
    SPELL_SPIKE_THROW_VISUAL = 137668,
    SPELL_SPIKE_DROP_DAMAGE = 134926,
};

class boss_iron_qon : public CreatureScript
{
public:
    boss_iron_qon() : CreatureScript("boss_iron_qon") { }

    struct boss_iron_qonAI : public BossAI
    {
        boss_iron_qonAI(Creature* creature) : BossAI(creature, DATA_IRON_QON)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            _Reset();
            if (instance)
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            events.Reset();
        }

        void EnterCombat(Unit* attacker)
        {

            if (instance)
            {
                instance->SetBossState(DATA_IRON_QON, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
        }

        void JustDied(Unit* killer)
        {
            if (instance)
            {
                instance->SetBossState(DATA_IRON_QON, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove.
            }

        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                }
            }
        }
    };
};

void AddSC_boss_iron_qon()
{

}