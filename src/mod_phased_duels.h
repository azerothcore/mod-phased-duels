#ifndef MODULE_PHASEDDUELS_H
#define MODULE_PHASEDDUELS_H

#include "Configuration/Config.h"
#include "Object.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "GridNotifiers.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "SpellAuraEffects.h"

class PhasedDueling : public PlayerScript
{
public:
    PhasedDueling() : PlayerScript("PhasedDueling") {}

    // Attempt in storing the player phase with spell phases included.
    uint32 getNormalPhase(Player* player) const;

    void OnLogin(Player* player) override;
    void OnDuelStart(Player* firstplayer, Player* secondplayer) override;
    void OnDuelEnd(Player* firstplayer, Player* secondplayer, DuelCompleteType /*type*/) override;
};

void AddPhasedDuelsScripts()
{
    new PhasedDueling();
}

#endif //MODULE_PHASEDDUELS_H
