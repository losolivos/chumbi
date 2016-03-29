#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "schoolmance.h"

enum eSpells
{
    SPELL_FLASH_BANG = 113866,
    SPELL_WONDROUSE_RAPIDITY = 114062, // cast
    SPELL_DAMAGE_WONDROUSE_RAPIDITY = 114061, // damage
    SPELL_GRAVITY_FLUX = 114035,
    SPELL_WHIRL_ILLUSION = 114048,
    SPELL_WHIRL_ILLUSION_VISUAL = 113808,
    SPELL_INVIS_AURA = 16380
};

enum eEvents
{
    EVENT_WONDROUSE_RAPIDITY = 1,
    EVENT_GRAVITY_FLUX = 2,
    EVENT_WHIRL_ILLUSION = 3
};

#define _count 15

Position _FakeSpawn[_count]  = 
{
    {305.52f,45.75f,113.40f,3.11f},
    {295.78f, 45.81f, 113.40f, 3.11f },
    { 286.53f, 46.03f, 113.40f, 3.11f },
    { 277.48f, 46.05f, 113.40f, 3.11f },
    { 268.34f, 46.06f, 113.40f, 3.11f },
    { 305.04f, 36.15f, 113.40f, 3.12f },
    { 295.89f, 36.26f, 113.40f, 3.12f },
    { 286.55f, 36.37f, 113.40f, 3.12f },
    { 277.07f, 36.48f, 113.40f, 3.12f },
    { 268.14f, 36.58f, 113.40f, 3.12f },
    { 305.33f, 26.62f, 113.40f, 3.14f },
    { 296.02f, 26.62f, 113.40f, 3.14f },
    { 286.60f, 26.66f, 113.40f, 3.14f },
    { 277.25f, 26.70f, 113.40f, 3.14f },
    { 268.12f, 26.73f, 113.40f, 3.14f }

};

class boss_jandice_barov : public CreatureScript
{
public:
    boss_jandice_barov() : CreatureScript("boss_jandice_barov") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jandice_barovAI(creature);
    }

    struct boss_jandice_barovAI : public BossAI
    {
        boss_jandice_barovAI(Creature* creature) : BossAI(creature, BOSS_JANDICE_BAROV) {}
        EventMap events; uint32 _position;
        bool _at33percent, _at66percent;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
                instance->SetData(BOSS_JANDICE_BAROV, NOT_STARTED);
            _at33percent = false; _at66percent = false;
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_JANDICE_BAROV, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void _illusion()
        {
            _position = urand(0, 20);

            for (uint32 i = 0; i < _count; i++)
                if (i != _position)
                    me->SummonCreature(NPC_JANDICE_ILLUSION, _FakeSpawn[i], TEMPSUMMON_MANUAL_DESPAWN);
            me->SummonCreature(NPC_JANDICE_TRUE_ILLUSION, _FakeSpawn[_position], TEMPSUMMON_MANUAL_DESPAWN);

        }

        void DamageTaken(Unit* /*killer*/, uint32 &damage) override
        {
            if (me->HealthBelowPctDamaged(33, damage) && !_at33percent)
            {
                events.Reset();
                    events.ScheduleEvent(EVENT_WHIRL_ILLUSION, 1000);
                _at33percent = true;
            }

            if (me->HealthBelowPctDamaged(66, damage) && !_at66percent)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_WHIRL_ILLUSION, 1000);
                _at66percent = true;
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_JANDICE_BAROV, FAIL);
            }
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who) override
        {

            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_JANDICE_BAROV, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_WONDROUSE_RAPIDITY, urand(7000,7500));
            events.ScheduleEvent(EVENT_GRAVITY_FLUX, 4500);
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
                case EVENT_WONDROUSE_RAPIDITY:
                    me->CastSpell(me, SPELL_WONDROUSE_RAPIDITY, false);
                    me->SummonCreature(NPC_RAPIDITY_TRIGGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                    events.ScheduleEvent(EVENT_WONDROUSE_RAPIDITY, 14000);
                    break;
                case EVENT_GRAVITY_FLUX:
                    if (Unit* _player = SelectTarget(SELECT_TARGET_FARTHEST))
                        me->CastSpell(_player, SPELL_GRAVITY_FLUX, false);
                    events.ScheduleEvent(EVENT_GRAVITY_FLUX, 14500);
                    break;
                case EVENT_WHIRL_ILLUSION:
                    _illusion();
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class jandice_illusion : public CreatureScript
{
public:
    jandice_illusion() : CreatureScript("jandice_illusion") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new jandice_illusionAI(creature);
    }

    struct jandice_illusionAI : public ScriptedAI
    {
        jandice_illusionAI(Creature* creature) :ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell(me, SPELL_WHIRL_ILLUSION, false);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
        }

        void DamageTaken(Unit* /*killer*/, uint32 &damage) override
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                me->CastSpell(me, SPELL_FLASH_BANG, false);
                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (pInstance->GetBossState(BOSS_TRUE_ILLUSION) == SPECIAL)
                me->DespawnOrUnsummon();
        }
    };
};

class true_jandice_illusion : public CreatureScript
{
public:
    true_jandice_illusion() : CreatureScript("true_jandice_illusion") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new true_jandice_illusionAI(creature);
    }

    struct true_jandice_illusionAI : public BossAI
    {
        true_jandice_illusionAI(Creature* creature) :BossAI(creature,BOSS_TRUE_ILLUSION) {}
        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell(me, SPELL_WHIRL_ILLUSION, false);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
        }

        void DamageTaken(Unit* /*killer*/, uint32 &damage) override
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                instance->SetBossState(BOSS_TRUE_ILLUSION, SPECIAL);
                if (Unit* jandice = Unit::GetCreature(*me, instance->GetData64(BOSS_JANDICE_BAROV)))
                {
                    jandice->RemoveAllAuras();
                    jandice->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    jandice->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    jandice->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                }
                me->DespawnOrUnsummon();
            }
        }
    };
};

class gravity_flux_trigger : public CreatureScript
{
public:
    gravity_flux_trigger() : CreatureScript("gravity_flux_trigger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new gravity_flux_triggerAI(creature);
    }

    struct gravity_flux_triggerAI : public ScriptedAI
    {
        gravity_flux_triggerAI(Creature* creature) :ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;
        uint32 _desp;
        uint32 count;

        void IsSummonedBy(Unit* summoner)
        {
            _desp = 1500;
            count = 3;
        }


        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (_desp <= diff)
            {
                me->CastSpell(me, SPELL_DAMAGE_WONDROUSE_RAPIDITY, true);
                count--;
                if (count==0)
                    me->DespawnOrUnsummon();
            }
            else _desp -= diff;
        }
    };
};

// WONDROUSE_RAPIDITY = 114062
class spell_wondrouse_rapidity : public SpellScriptLoader
{
public:
    spell_wondrouse_rapidity() : SpellScriptLoader("spell_wondrouse_rapidity") { }

    class spell_wondrouse_rapidity_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_wondrouse_rapidity_SpellScript);

        void HandleAfterCast()
        {
            if (!GetCaster())
                return;
            GetCaster()->CastSpell(GetCaster(), SPELL_DAMAGE_WONDROUSE_RAPIDITY, false);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_wondrouse_rapidity_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_wondrouse_rapidity_SpellScript();
    }
};

// SPELL_WHIRL_ILLUSION_VISUAL

class spell_whirl_ullusion_visual : public SpellScriptLoader
{
public:
    spell_whirl_ullusion_visual() : SpellScriptLoader("spell_whirl_ullusion_visual") { }

    class spell_whirl_ullusion_visual_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_whirl_ullusion_visual_SpellScript);

        void HandleAfterCast()
        {
            if (!GetCaster())
                return;
            GetCaster()->CastSpell(GetCaster(), SPELL_INVIS_AURA, false);
            GetCaster()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            GetCaster()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            GetCaster()->ToCreature()->SetReactState(REACT_PASSIVE);
            GetCaster()->AttackStop();
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_whirl_ullusion_visual_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_whirl_ullusion_visual_SpellScript();
    }
};

void AddSC_boss_jandice_barov()
{
    new boss_jandice_barov();
    new jandice_illusion();
    new true_jandice_illusion();
    new gravity_flux_trigger();
    new spell_wondrouse_rapidity();
    new spell_whirl_ullusion_visual();
}